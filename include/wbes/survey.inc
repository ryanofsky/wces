<?

require_once("wbes/wbes.inc");

class Survey
{
  var $components = array();
  var $orig_id = false;

  function insert($component,$position = 0)
  {
    $position = ((int)$position < 0) ? count($this->components) + (int)$position + 1 : (int)$position;
    array_splice($this->components,(int)$position,0,array($component));
    return $position;
  }

  function delete($componentno)
  {
    array_splice($this->components,$componentno,1);
  }

  function movedown($componentno)
  {
    if ($componentno >= count($this->components) - 1) return;
    $q = $this->components[$componentno];
    $this->delete($componentno);
    $this->insert($q,$componentno + 1);
  }

  function moveup($componentno)
  {
    if ($componentno < 1) return;
    $q = $this->components[$componentno];
    $this->delete($componentno);
    $this->insert($q,$componentno - 1);
  }

  function load($topic_id, $base_branch_id, &$cfactories, $latest = false)
  {
    global $wbes;
    wbes_connect();

    $factories = array();
    for(reset($cfactories); list($k) = each($cfactories); )
      $factories[$cfactories[$k]->code()] = &$cfactories[$k];

    $latest = (int)$latest;
    if (!$latest) $latest = "NULL";

    $topic_id = (int)$topic_id;
    $result = pg_query("SELECT topic_contents($topic_id)", $wbes, __FILE__, __LINE__);
    $existing_topic = (int)pg_result($result,0,0);

    $result = pg_query("SELECT branch_latest($base_branch_id, $existing_topic, $latest)", $wbes, __FILE__, __LINE__);

    $this->orig_id = (int)pg_result($result,0,0);

    // store the results of branch updates in a temporary table. the temporary
    // table is needed because the first and second queries can't be combined.
    // in postgres a select statement will never return rows that are inserted
    // in the course of the query's execution.

    $result = pg_query("

      CREATE TEMPORARY TABLE tmp$this->orig_id AS
      SELECT branch_latest(bt.branch_id, $latest) AS revision_id, ordinal
      FROM list_items AS li
      INNER JOIN branch_topics_cache AS bt ON bt.base_branch_id = li.item_id AND bt.topic_id = $existing_topic
      WHERE li.revision_id = (SELECT revision_contents($this->orig_id));

      SELECT t.ordinal, t.revision_id, r.type
      FROM tmp$this->orig_id AS t
      INNER JOIN revisions as r USING (revision_id)
      ORDER BY type, ordinal;

    ", $wbes, __FILE__, __LINE__);

    pg_query("DROP TABLE tmp$this->orig_id", $wbes, __FILE__, __LINE__);

    $n = pg_numrows($result);
    $this->components = array_pad(array(), $n, false);

    $lasttype = false;
    for($i = 0; ; ++$i)
    {
      if($i < $n)
        extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      else
        $type = -1;

      if ($type !== $lasttype && $lasttype !== false)
      {
        if (!isset($factories[$lasttype]))
        {
          print("<h4>Fatal Error: Can't load component type $lasttype</h4>");
          print("<p>Either the type doesn't exist and the database is corrupted,"
            . " or you forgot to include this component's factory in the factories array.</p>");
          print("<p>Here is a list of factories that were passed:</p>");
          print("<table>\n");
          foreach($factories as $k => $v)
          {
            print("<tr><td>" . $v->code() . "</td><td>" . get_class($v) . "</td><td>" . $v->name() . "</td></tr>");
          }
          print("</table>\n");
          die(); 
        }
        $factories[$lasttype]->load($this->components, $ids, $ordinals, $existing_topic, $latest);
      }

      if ($i >= $n) break;

      if ($type === $lasttype)
      {
        $ids[] = (int)$revision_id;
        $ordinals[] = $ordinal - 1;
      }
      else
      {
        $ids = array((int)$revision_id);
        $ordinals = array($ordinal - 1);
      }
      $lasttype = $type;
    }
  }

  function save($topic_id, $user_id, $save_id = false)
  {
    global $wbes;
    wbes_connect();

    $this->orig_id = (int)$this->orig_id;

    $result = pg_query("SELECT branch_id FROM revisions WHERE revision_id = $this->orig_id", $wbes, __FILE__, __LINE__);
    $branch_id = pg_numrows($result) == 1 ? (int)pg_result($result,0,0) : "NULL";

    $components = pg_query("SELECT item_id FROM list_items WHERE revision_id = $this->orig_id ORDER BY ordinal", $wbes, __FILE__, __LINE__);
    $components_num = pg_numrows($components);

    if(!$save_id)
    {
      $result = pg_query("
        BEGIN WORK;
        INSERT INTO saves (user_id) VALUES ($user_id);
        SELECT currval('save_ids')
      ", $wbes, __FILE__, __LINE__);
      $save_id = (int)pg_result($result,0,0);
    }

    $i = 0;
    $ncomponents = array();
    $list_changed = count($this->components) != $components_num;
    foreach(array_keys($this->components) as $k)
    {
      $cbranch_id = $this->components[$k]->save($topic_id, $save_id);
      $ncomponents[] = $cbranch_id;
      if (!$list_changed && $components)
        if($i >= $components_num)
        {
          $components = false;
          $list_changed = true;
        }
        else
          $list_changed = pg_result($components, $i, 0) != $cbranch_id;
      ++$i;
    }

    if ($list_changed || $this->orig_id == 0)
    {
      $result = pg_query("SELECT branch_save($topic_id, $branch_id)", $wbes, __FILE__, __LINE__);
      $branch_id = (int)pg_result($result,0,0);

      $o = $this->orig_id ? $this->orig_id : "NULL";

      $result = pg_query("SELECT revision_save_start($branch_id, $o, 1, $save_id)", $wbes, __FILE__, __LINE__);
      $saved_id = (int)pg_result($result,0,0);
      $sql = "";
      foreach($ncomponents as $i => $id)
      {
        $ordinal = $i + 1;
        $sql .= "INSERT INTO list_items(revision_id, ordinal, item_id) VALUES ($saved_id, $ordinal, $id);\n";
      }
      $sql .= "SELECT revision_save_end($branch_id, $saved_id)";
      pg_query($sql, $wbes, __FILE__, __LINE__);
    }
    else
      pg_query("SELECT branch_topics_add($topic_id, $branch_id)", $wbes, __FILE__, __LINE__);

    pg_query("COMMIT", $wbes, __FILE__, __LINE__);

    $this->orig_id = 0;
    $this->components = array();
    return $branch_id;
  }
};

?>

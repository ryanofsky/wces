<?

$wbes = false;
function wbes_connect()
{
  global $wbes;
  if (!$wbes) $wbes = server_pginit("wbes");
}

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

  function load($branch_id, &$cfactories)
  {
    global $wbes;
    wbes_connect();

    $factories = array();
    for(reset($cfactories); list($k) = each($cfactories); )
      $factories[$cfactories[$k]->code()] = &$cfactories[$k];

    $result = pg_query("SELECT branch_update($branch_id)", $wbes, __FILE__, __LINE__);
    $this->orig_id = (int)pg_result($result, 0, 0);

    $result = pg_query("
      CREATE TEMPORARY TABLE tmp$this->orig_id AS
      SELECT branch_update(li.item_id) AS revision_id, ordinal
      FROM list_items AS li
      WHERE li.revision_id = (SELECT revision_contents($this->orig_id));
      
      SELECT t.ordinal, t.revision_id, r.type
      FROM tmp$this->orig_id AS t
      INNER JOIN revisions as r USING (revision_id)
      ORDER BY type, ordinal;
       
     ", $wbes, __FILE__, __LINE__);
     
    pg_query("DROP TABLE tmp$this->orig_id", $wbes, __FILE__, __LINE__);

    $n = pg_numrows($result);
    $type = $lasttype = false;
    $ordinals = $branches = array();
    $this->components = array_pad(array(), $n, false);
    for($i = 0; ; ++$i)
    {
      if($i < $n)
        extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      else
        $type = -1;

      if ($type !== $lasttype && $lasttype !== false)
      {
        $factory = &$factories[$lasttype];
        $factory->load($this->components, $components, $ordinals);
      }

      if ($i >= $n) break;

      if ($type === $lasttype)
      {
        $ordinals[] = (int)$ordinal;
        $components[] = (int)$revision_id;
      }
      else
      {
        $components = array((int)$revision_id);
        $ordinals = array((int)$ordinal);
      }
      $lasttype = $type;
    }
  }

  function save($branch_id, $user_id)
  {
    global $wbes;
    wbes_connect();

    $this->orig_id = (int)$this->orig_id;

    $result = pg_query("SELECT branch_id FROM revisions WHERE revision_id = $this->orig_id", $wbes, __FILE__, __LINE__);
    if (pg_numrows($result) <> 1 || (int) pg_result($result,0,0) <> $branch_id) // obscurity
      return false;

    $components = pg_query("SELECT item_id FROM list_items WHERE revision_id = $this->orig_id ORDER BY ordinal", $wbes, __FILE__, __LINE__);
    $components_num = pg_numrows($components);

    $result = pg_query("BEGIN WORK", $wbes, __FILE__, __LINE__);
      if (!$result) return false;

    $result = pg_query("
      INSERT INTO saves (user_id, revision_id) VALUES ($user_id, $this->orig_id);
      SELECT currval('save_ids')
    ", $wbes, __FILE__, __LINE__);
    $save_id = (int)pg_result($result,0,0);

    $list_changed = $i = 0; $ncomponents = array();
    foreach(array_keys($this->components) as $k)
    {
      $cbranch_id = $this->components[$k]->save($save_id);
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

    if ($list_changed)
    {
      $result = pg_query("SELECT revision_save_start($branch_id, $this->orig_id, 1, $save_id)", $wbes, __FILE__, __LINE__);
      $saved_id = (int)pg_result($result,0,0);
      $sql = "";
      foreach($ncomponents as $i => $id)
        $sql .= "INSERT INTO list_items(revision_id, ordinal, item_id) VALUES ($saved_id, $i, $id);\n";
      $sql .= "SELECT revision_save_end($branch_id, $saved_id)";
      pg_query($sql, $wbes, __FILE__, __LINE__);
    }
    
    pg_query("COMMIT", $wbes, __FILE__, __LINE__);

    $this->orig_id = 0;
    $this->components = array();
    return true;
  }
}

?>

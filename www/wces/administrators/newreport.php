<?

require_once("wces/wces.inc");
require_once("wbes/postgres.inc");

// basic process:

function method_thunk(&$instance, $method, $args)
{
  $id = uniqid("thunk");
  $GLOBALS[$id] = $instance;
  $call_args = str_replace("&", "", $args);
  return create_function($args, "\$instance =& $GLOBALS['$id']; "
                         . "return \$instance->$method($call_args);");
}

class TreeNode
{
  // associative array
  var $children = array();

  // list of keys
  var $childOrds = array();

  function TreeNode()
  {
    $this->cmp_func = method_thunk($this, 'cmp', '$key1, $key2');
  }

  function sort()
  {
    sort($this->childOrds, $this->cmp_func);
  }

  function cmp($arg1, $arg2) { assert(0); }
  function getChild($keys) { assert(0); }
  function setChild(&$node, $keys) { assert(0); }
}

class SurveyTreeNode
{
  var $subsurvey_id;
  var $item_id;
  var $revision_id;
  
  function SurveyTreeNode($subsurvey_id, $parent, $item_id, $revision_id, $topic_id)
  {
    $this->subsurvey_id = $subsurvey_id;
    $this->parent = $parent;
    $this->item_id = $item_id;
    $this->revision_id = $revision_id;
    $this->topic_id = $topic_id;
  }
}

class MultiMap
{
  var $a = array();
  
  // return by reference sometimes causes segfaults, so
  // instead assign reference to member and return
  // by value  
  function get()
  {
    $key = implode(',', func_get_args());
    $this->get_ret =& $this->a[$key];
    return $this->get_ret;
  }

  function set(&$val)
  {
    $key = implode(',', array_slice(func_get_args(), 1));
    $this->a[$key] = $val;
  }
}

$subsurveys = array();
$topNode = null;

foreach ()
{
  if (!isset($subsurveys[$subsurvey_id]))
  {
    $node =& new SurveyTreeNode($subsurvey_id, $parent, $item_id,
                                $revision_id, $topic_id);

    if (!$parent)
    {
      $topNode =& $node;
    }
    else
    {
      $parent_node =& $subsurveys[$parent];
    }
  
  }
  
}

$jklk = pg_go("
  SELECT t.topic_id, r.response_id, r.parent, 0 AS depth,
    r.item_id AS survey_item_id, r.revision_id AS survey_revision_id,
    r.response_id AS top_response_id, random() AS top_ordinal,
    get_subsurvey_id(NULL, NULL, r.item_id) AS subsurvey_id", 
", $wces, __FILE__, __LINE__);



function go()
{
  global $wces;

  $topics = 'rwtopics';
  $tables = array
  ( array
    ( 'name' => 'responses_instructor_subsurvey',
      'topic' => $GROUP[PROFESSORS] ? 'pt.user_id' : 'NULL',
      'join' => $GROUP[PROFESSORS]
        ? 'INNER JOIN wces_prof_topics AS pt ON pt.topic_id = t.topic_id' : ''
    )
  );

  $pageNumbers = false;
  $outhtml = true;

  $result = pg_go("
    SELECT ordinal FROM rwtopics GROUP BY ordinal ORDER BY ordinal
  ", $wces, __FILE__, __LINE__);

  $pacer =& new pg_wrapper($result);

  $pages = pg_numrows($result);

  for($page = 1; $page <= $pages; ++$page)
  {
    $ordinal = (int)pg_result($result, $page - 1, 0);

    if ($pageNumbers)
    {
      if ($outtext) $text .= "\n{$line}[ $page / $pages ]\n\n";

      if ($outhtml || $stdout)
      {
        $pagehtml = '';
        if (!isset($options['skipLines']) || !$options['skipLines'])
        {
          $pagehtml .= '<hr style="page-break-before:always">' . "\n";
          $pagehtml .= "<font size=1>[ $page / $pages ]</font>\n";
        }
      }
      else
        $pagehtml = false;
    }
    else
      $pagehtml = ($outhtml || $stdout) ? '' : false;

    $get_subsurvey_id = pg_unique_id();

    pg_go("
      CREATE TEMPORARY TABLE subsurveys
      (
        subsurvey_id INTEGER NOT NULL DEFAULT NEXTVAL...
        item_id INTEGER NOT NULL DEFAULT NEXTVAL,
        topic_id INTEGER,
        parent INTEGER
      );

      CREATE OR REPLACE FUNCTION $get_subsurvey_id(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
        DECLARE
          parent_subsurvey_id ALIAS FOR $1;
          topic_id_ ALIAS FOR $2;
          item_id_ ALIAS FOR $3;
          i INTEGER;
        BEGIN
          SELECT INTO i subsurvey_id FROM subsurveys
          WHERE item_id = item_id_
            AND (parent = parent_subsurvey_id
              OR (parent IS NULL AND parent_subsurvey_id IS NULL))
            AND (topic_id = topic_id_
              OR (topic_id IS NULL AND topic_id_ IS NULL));
          IF FOUND THEN
            RETURN i;
          ELSE
            INSERT INTO subsurveys (item_id, topic_id, parent)
            VALUES (item_id_, topic_id_, parent_subsurvey_id);
            RETURN currval(''subsurvey_ids'');
          END IF;
        END;
      ' LANGUAGE 'plpgsql';
    ", $wces, __FILE__, __LINE__);

    pg_go("
      CREATE TEMPORARY TABLE resps AS
      SELECT t.topic_id, r.response_id, r.parent, 0 AS depth,
        r.item_id AS survey_item_id, r.revision_id AS survey_revision_id,
        r.response_id AS top_response_id, random() AS top_ordinal,
        get_subsurvey_id(NULL, NULL, r.item_id) AS subsurvey_id
      FROM $topics AS t
      INNER JOIN responses_survey AS r USING (topic_id)
      WHERE t.ordinal = $ordinal AND r.response_id IS NOT NULL
    ", $wces, __FILE__, __LINE__);

    for ($depth = 1; ; ++$depth)
    {
      $pd = $depth - 1;
      $fillup = '';

      foreach ($tables as $table)
      {
        $fillup .= "
          INSERT INTO resps (topic_id, response_id, parent, depth, survey_item_id, survey_revision_id, top_response_id, top_ordinal, subsurvey_id)
          SELECT t.topic_id, t.response_id, t.parent, $depth, t.item_id, t.revision_id, r.top_response_id, r.top_ordinal,
            get_subsurvey_id(parent_subsurvey_id, $table[topic], t.item_id)
          FROM resps AS r
          INNER JOIN $table[name] AS t ON t.parent = r.response_id
          $table[join]
          WHERE r.depth = $pd;
        ";
      }

      $r = pg_go("
        $fillup
        SELECT EXISTS (SELECT * FROM resps WHERE depth = $depth);
      ", $wces, __FILE__, __LINE__);

      if (pg_result($r, 0, 0) != 't')
        break;
    }

    $ords = pg_go("
      SELECT s.survey_revision_id, i.child_ordinal, i.child_item_id
      FROM (SELECT DISTINCT survey_revision_id FROM resps) AS s
      INNER JOIN revisions AS r USING (revision_id)
      INNER JOIN component_items AS i USING (component_id)
    ", $wces, __FILE__, __LINE__);

    $textq = pg_go("
      SELECT tt.revision_id, tt.item_id, c.ctext
      FROM
      ( SELECT DISTINCT re.subsurvey_id, rt.revision_id, rt.item_id
        FROM resps AS re
        INNER JOIN responses_text_question AS rt ON rt.parent = re.response_id
      ) AS tt
      INNER JOIN revisions AS r USING (revision_id)
      INNER JOIN components_text_question AS c USING (component_id)
    ", $wces, __FILE__, __LINE__);

    $choicer = pg_go("
      SELECT whatever
      FROM resps AS re
      INNER JOIN responses_choice AS rc AS rc.parent = re.response_id
      INNER JOIN responses_choice_question AS rq AS rq.parent = rc.response_id
    ", $wces, __FILE__, __LINE__);

    $choiceq = pg_go("
      SELECT tt.revision_id, tt.item_id, c.ctext
      FROM
      ( SELECT DISTINCT rc.revision_id
        FROM resps AS re
        INNER JOIN responses_choice AS rc ON rc.parent = re.response_id
      ) AS tt
      INNER JOIN revisions AS r USING (revision_id)
      INNER JOIN components_text_question AS c USING (component_id)
    ", $wces, __FILE__, __LINE__);





    $textr = pg_go("
      SELECT rt.revision_id, rt.item_id, rtext
      FROM resps AS re
      INNER JOIN responses_text_question AS rt ON rt.parent = re.response_id
    ", $wces, __FILE__, __LINE__);


    $choiceo = pg_go("
      SELECT s.revision_id, i.ordinal, i.item_id
      FROM (SELECT DISTINCT revision_id FROM resps) AS s
      INNER JOIN revisions AS r USING (revision_id)
      INNER JOIN component_items AS i USING (component_id)
    ", $wces, __FILE__, __LINE__);



    pg_go("DROP TABLE resps", $wces, __FILE__, __LINE__);

    report_makepage($pacer, $text, $pagehtml, $stats, $classes, $professors, $departments, $questionperiods, $textresponses, $choiceComponents, $choiceQuestions, $choiceResponses, $ords, $taresponses, $options);

    if ($stdout) print($pagehtml);
    if ($outhtml) $html .= $pagehtml;
  }


}

$db_debug = 1;
wces_connect();

pg_go("
  CREATE TEMPORARY TABLE rwtopics AS
  SELECT 278 AS topic_id, 0 AS ordinal
", $wces, __FILE__, __LINE__);

go();

?>
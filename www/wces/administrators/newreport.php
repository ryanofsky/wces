<?

require_once("wces/wces.inc");
require_once("wbes/postgres.inc");

//////////////////////////////////////////////////////////////////////////
// Generally

// Returns a new function that forwards to a method of an object. Useful
// when you want to maintain state between callbacks with functions like
// usort
// a method to be used where a callback is expected
// instance - which object to use
// method - name of method to call on object
// args - list of arguments as a string. arguments can be references 
          and names aren't important
           
function method_thunk(&$instance, $method, $args)
{
  $id = uniqid("thunk");
  $GLOBALS[$id] = $instance;
  $call_args = str_replace("&", "", $args);
  return create_function($args, "\$instance =& $GLOBALS['$id']; "
                         . "return \$instance->$method($call_args);");
}

function list_insert(&$list, &$item)
{
  if (isset($list))
    $list[] =& $item;
  else
    $list = array(&$item);
}

function cmp($a, $b)
{
  return $a < $b ? -1 : ($b < $a ? 1 : 0);
}

function tuple_key()
{
  return implode(',', func_get_args());
}

function topic_desc($topic_id)
{
  return "Topic #$topic_id";
}

//////////////////////////////////////////////////////////////////////////
// Nodey

class Node
{
  var $item_id;
  var $col;
  
  function Node($item_id)
  {
    $this->item_id = $item_id;
  }
  
  // assign column number
  function number(&$col) { assert(0); }
  
  // sort children, if any
  function sort() { }
}

class ParentNode extends Node
{
  // associative array
  var $children = array();

  // list of keys
  var $childOrds = array();

  function ParentNode($item_id)
  {
    $this->cmp_func = method_thunk($this, 'cmp', '$key1, $key2');
  }

  function sort()
  {
    foreach($this->childOrds as $k)
      $this->children[$k]->sort());  
    sort($this->childOrds, $this->cmp_func);    
  }
  
  function addChild(&$node)
  {
    $key = len($children);
    $this->children[] = &$node;
    $this->childOrds[] = $key;
  }

  function cmp($key1, $key2)
  {
    $i1 =& $this->children[$key1];
    $i2 =& $this->children[$key2];
    return cmp($this->itemOrds[$i1->item_id], $this->itemOrds[$i2->item_id]);
  }
  
  function header(&$row)
  {
    foreach($this->childOrds as $k)
      $this->children[$k]->header());  
  }
    
  function getChild($keys) { assert(0); }
  function setChild(&$node, $keys) { assert(0); }
}

class SurveyTreeNode extends TreeNode
{
  var $subsurvey_id;
  var $topic_id;

  // ordinals indexed by $item_id indicating relative order of items
  var $itemOrds = array();

  // Constructor
  function SurveyTreeNode($subsurvey_id)
  {
    $this->subsurvey_id = $subsurvey_id;
  }
  
  function number(&$col)
  {
    $this->col = $col;
    if (!empty($this->topic_id)) ++$col;
    foreach($this->childOrds as $k)
      $this->children[$k]->number($col);
  }
  
  function header(&$row)
  {
    if (!empty($this->topic_id))
      
    
    $row[$this->colno] = make_topic($this->topic_id);
  }
  
}

class TextNode
{
  var $textnode;
  
  function TextNode($item_id, $revision_id, $text)
  {
    $this->item_id = $item_id;
    $this->revision_id;
    $this->text = $text;
  }
  
  function cmp($key1, $key2)
  {
    $i1 =& $this->children[$key1];
    $i2 =& $this->children[$key2];
    $cmp = cmp($this->itemOrds[$i1->item_id], $this->itemOrds[$i2->item_id]);
    if ($cmp) return $cmp;
    return cmp($this->itemOrds[$i1->revision_id], 
      $this->itemOrds[$i2->revision_id]);
  }
  
  function number(&$col)
  {
    $this->col = $col++;
  }
};

function make_page($rwtopic_ordinal)
{
  // each subsurvey needs to have its own id so grouping can
  // occur on multiple responses
  $get_subsurvey_id = pg_unique_id();

  $topics = 'rwtopics';
  $tables = array
  ( array
    ( 'name' => 'responses_instructor_subsurvey',
      'topic' => $GROUP[PROFESSORS] ? 'pt.user_id' : 'NULL',
      'join' => $GROUP[PROFESSORS]
        ? 'INNER JOIN wces_prof_topics AS pt ON pt.topic_id = t.topic_id' : ''
    )
  );

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
            OR (topic_id IS NULL AND topic_id_ IS NULL)
            OR (parent IS NULL));
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
    SELECT t.topic_id, r.response_id, r.parent AS parent_topic, 0 AS depth,
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
        INSERT INTO resps (topic_id, response_id, parent_topic, depth, survey_item_id, survey_revision_id, top_response_id, top_ordinal, subsurvey_id)
        SELECT t.topic_id, t.response_id, t.parent, $depth, t.item_id, t.revision_id, r.top_response_id, r.top_ordinal
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

  $resps = pg_go("
    SELECT DISTINCT r1.subsurvey_id, r2.subsurvey_id AS r2.parent_subsurvey
    FROM resps AS r1
    INNER JOIN resps AS r2 ON r2.topic_id = r1.parent_topic
  ", $wces, __FILE__, __LINE__);
  
  // all SurveyNodes indexed by subsurvey_id
  $survey_subs = array(); 
  
  // survey items (text & multiple choic questions) indexed by 
  // tuple of subsurvey_id, item_id, and revision_id
  $survey_items = array(); 
  
  function & make_survey_node(&$survey_subs, $subsurvey_id)
  {
    if (!isset($survey_subs[$subsurvey_id]))
      $survey_subs[$subsurvey_id] =& new SurveyTreeNode($subsurvey_id);
    return $survey_subs[$subsurvey_id];
  }
   
  $n = pg_numrows($resps);
  for ($i = 0; $i < $n; ++$i)
  {
    $row = pg_fetch_row($resps, $i, PGSQL_ASSOC);
    $sub = (int) $row['subsurvey_id'];
    $par = (int) $row['parent_subsurvey'];
    $par_survey =& make_survey_node($survey_subs, $par);  
    $survey =& make_survey_node($survey_subs, $sub);
    $par_survey->addChild($survey);
  }

  $ords = pg_go("
    SELECT s.subsurvey_id, s.survey_revision_id, 
      i.child_ordinal, i.child_item_id
    FROM (SELECT DISTINCT subsurvey_id, survey_revision_id FROM resps) AS s
    INNER JOIN revisions AS r USING (revision_id)
    INNER JOIN component_items AS i USING (component_id)
    ORDER BY survey_revision_id DESC, child_ordinal
  ", $wces, __FILE__, __LINE__);
  
  $n = pg_numrows($ords);
  for ($i = 0; $i < $n; ++$i)
  {
    $row = pg_fetch_row($ords, $i, PGSQL_ASSOC);
    $subsurvey_id = $row['subsurvey_revision_id'];
    $item = $row['child_item_id'];
    $ord =& $survey_subs[$subsurvey_id]->itemOrds[$item];
    if (!isset($ord)) $ord = $i;
  }

  $textq = pg_go("
    SELECT tt.subsurvey_id, tt.revision_id, tt.item_id, c.ctext
    FROM
    ( SELECT DISTINCT re.subsurvey_id, rt.revision_id, rt.item_id
      FROM resps AS re
      INNER JOIN responses_text_question AS rt ON rt.parent = re.response_id
    ) AS tt
    INNER JOIN revisions AS r USING (revision_id)
    INNER JOIN components_text_question AS c USING (component_id)
  ", $wces, __FILE__, __LINE__);
  
  $n = pg_numrows($textq);
  for ($i = 0; $i < $n; ++$i)
  {
    $row = pg_fetch_row($textq, $i, PGSQL_ASSOC);
    $sub = $row['subsurvey_id'];
    $iid = $row['item_id'];
    $rev = $row['revision_id'];
    
    $survey =& $survey_subs[$sub];
    $child =& new TextNode($iid, $rev, $row['ctext']);
    
    $survey->addChild($child);
    $survey_items[tuple_key($sub, $iid, $rev)] =& $child;
  }

  $top_survey =& $survey_subs[0];
  $top_survey->sort();
  
  $cols = 0;
  $top_survey->number($col);
  
  $row = array_fill(array(), $cols, "") // xxx: arguments?
  $top_survey->header();
  
  $top_survey
  
  for ($i = 0; $i < $cols; ++$i)
  {
    
  }

  
  

/*

  XXX: for now, skip choice questions
  
  $choicer = pg_go("
    SELECT tt.subsurvey_id, tt.citem_id, tt.crevision_id, tt.qitem_id, 
      tt.qrevision_id tt.citem,
      SELECT DISTINCT re.subsurvey_id, 
        rc.item_id AS citem_id, rc.revision_id AS crevision_id,
        rq.item_id AS qitem_id, rq.revision_id AS qrevision_id
      FROM resps AS re
      INNER JOIN responses_choice AS rc AS rc.parent = re.response_id
      INNER JOIN responses_choice_question AS rq AS rq.parent = rc.response_id
    ) AS tt
    INNER JOIN revisions AS vc ON vc.revision_id = tt.crevision_id
    INNER JOIN components_choice AS cc ON cc.component_id = vc.component_id
    INNER JOIN revisions AS vq ON vq.revision_id = tt.qrevision_id
    INNER JOIN components AS cq ON cq.component_id = vq.component_id
  ", $wces, __FILE__, __LINE__);
  
  $n = pg_numrows($choicer);

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

  $choiceo = pg_go("
    SELECT s.revision_id, i.ordinal, i.item_id
    FROM (SELECT DISTINCT revision_id FROM resps) AS s
    INNER JOIN revisions AS r USING (revision_id)
    INNER JOIN component_items AS i USING (component_id)
  ", $wces, __FILE__, __LINE__);


*/
  $textr = pg_go("
    SELECT rt.revision_id, rt.item_id, rtext
    FROM resps AS re
    INNER JOIN responses_text_question AS rt ON rt.parent = re.response_id
  ", $wces, __FILE__, __LINE__);

}

function go()
{
  global $wces;

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

    make_page($ordinal);


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
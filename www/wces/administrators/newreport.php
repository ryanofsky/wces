<?

/*

  CREATE SEQUENCE subsurvey_ids INCREMENT 1 START 1;
  
  CREATE OR REPLACE FUNCTION revision_latest_component_i(INTEGER, INTEGER) RETURNS INTEGER AS '
    SELECT CASE WHEN rs.save_id > rv.save_id 
    THEN rs.component_id ELSE rv.component_id END
    FROM revisions AS rv
    LEFT JOIN revisions AS rs ON rs.revision_id = $1
    WHERE rv.revision_id = $2
  ' LANGUAGE 'sql'; 
  
  select revision_latest_component_i(215, 1312);
  
  SELECT * FROM 
  revisions AS r1
  FULL JOIN revisions AS r2 ON r2.revision_id = -1
  WHERE r1.revision_id = 12
  
  CREATE AGGREGATE revision_latest_component
  (
    BASETYPE = INTEGER,
    SFUNC = revision_latest_component_i,
    STYPE = INTEGER   
  );
*/

require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("wces/report_page.inc");

class SubSurvey
{
  var $subsurvey_id;
  
  function SubSurvey($subsurvey_id)
  {
    $this->subsurvey_id = $subsurvey_id;
  }
  
  function arrIndex()
  {
    return "ss$subsurvey_id";
  }
  
  // static methods
  function MakeResps($topics)
  {
    $tables = array
    ( array
      ( 'name' => 'responses_instructor_subsurvey', 
        'topic' => empty($groups[PROFESSORS]) ? 'NULL' : 'pt.user_id',
        'join'  => empty($groups[PROFESSORS]) 
          ? '' : 'INNER JOIN wces_prof_topics AS pt ON pt.topic_id = t.topic_id'
      )
    );

    $get_subsurvey_id = pg_uniqid();

    pg_go("
      CREATE TEMPORARY TABLE subsurveys
      (
        subsurvey_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('subsurvey_ids'),
        item_id INTEGER,
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
        $get_subsurvey_id(NULL, NULL, r.item_id) AS subsurvey_id,
        NULL::INTEGER AS parent_subsurvey_id
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
          INSERT INTO resps (topic_id, response_id, parent, depth, survey_item_id, survey_revision_id, top_response_id, top_ordinal, subsurvey_id, parent_subsurvey_id)
          SELECT t.topic_id, t.response_id, t.parent, $depth, t.item_id, t.revision_id, r.top_response_id, r.top_ordinal,
            $get_subsurvey_id(r.subsurvey_id, $table[topic], t.item_id), r.subsurvey_id
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
    
    pg_go("
      DROP TABLE subsurveys;
      DROP FUNCTION $get_subsurvey_id(INTEGER, INTEGER, INTEGER);
    ", $wces, __FILE__, __LINE__);
  }
  
  function AddItems(&$rel)
  {
    $r = pg_go("
      SELECT DISTINCT depth, parent_subsurvey_id, subsurvey_id, survey_item_id
      FROM resps
    ", $wces, __FILE__, __LINE__);
    
    $n = pg_numrows($r);
    for ($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($r, $i, PGSQL_ASSOC));
      if (!isset($parent_subsurvey_id)) $parent_subsurvey_id = 0;
      $rel[$parent_subsurvey_id][$survey_item_id][] 
        = new SubSurvey($subsurvey_id);
    }
  }
  
  function OrderItems(&$rel)
  {
    $r = pg_go("
      SELECT s.subsurvey_id, i.ordinal AS child_ordinal, i.item_id AS child_item_id
      FROM
      ( SELECT r.subsurvey_id, revision_latest_component(s.survey_revision_id) AS component_id
        (SELECT DISTINCT subsurvey_id, survey_revision_id FROM resps) AS r
        GROUP BY r.subsurvey_id
      ) AS s
      INNER JOIN component_items USING component_id
      ORDER BY s.subsurvey_id, i.ordinal
    ", $wces, __FILE__, __LINE__);   

    $n = pg_numrows($r);

    for ($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($r, $I, PGSQL_ASSOC));
      $ords[$subsurvey_id][] = $child_item_id;
    }
    
    $ordering = array();
    SubSurvey::_OrderItemImpl($rel, $ords, $ordering);
    
    return $ordering();
  }
    

  // array indexed by subsurvey_id holding lists of item_ids in order 
  var $ord = array();

  function _OrderItemsImpl($rel, $ords, &$out, $subsurvey_id = 0)
  {
    $ord = $ords[$subsurvey_id];
    
    $rel = $rel[$subsurvey_id];
    unset($this->rel[$subsurvey_id]);
    
    foreach ($ord as $item_id)
    {
      foreach($rel[$item_id] as $child_subsurvey_id)
      {
        SubSurvey::_OrderItemImpl($rel, $ords, $child_subsurvey_id);
        $this->columns[$child_subsurvey_id] = $this->nextCol++;
      }
      unset($rel[$item_id]);
    }
    
    foreach($rel as $item_id => $subs)
    foreach($subs as $subsurvey_id)
      $this->columns[$subsurvey_id] = $this->nextCol++;
  }
}    

  }
}

class TextQuestion
{
  var $revision_id;
  var $ctext;
  
  function TextQuestion($revision_id, $ctext)
  {
    $this->revision_id = $revision_id;
    $this->ctext = $ctext;
  }
  
  function arrIndex()
  {
    return "tq$this->revision_id";
  }
  
  // static methods
  function AddItems(&$rel)
  {
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
      extract(pg_fetch_row($textq, $i, PGSQL_ASSOC));
      if (!isset($parent_subsurvey_id)) $parent_subsurvey_id = 0;
      $rel[$parent_subsurvey_id][$survey_item_id][] 
        = new TextQuestion($revision_id, $ctext);
    }
  }
}

function go($topics, $groups)
{
  global $wces;
  
  $pageNumbers = false;
  $outhtml = true;

  $result = pg_go("
    SELECT ordinal FROM $topics GROUP BY ordinal ORDER BY ordinal
  ", $wces, __FILE__, __LINE__);
  
  $pacer =& new pg_wrapper($result);
  
  $pages = pg_numrows($result);

  for($page = 1; $page <= $pages; ++$page)
  {
    $ordinal = (int)pg_result($result, $page - 1, 0);
    
    SubSurvey::MakeResps($topics);
    
    // 2d associative array indexed by parent subsurvey id and
    // item id holding lists of item arrays()

    $rel = array();
    SubSurvey::AddItems($rel);
    TextQuestion::AddItems($rel);
    SubSurvey::OrderItems($rel);























    



    
    // come up with heading list, assuming there are only text questions
    
    
    // array indexed by subsurvey_id of number of items in that subsurvey
    $subsurvey_counts = array();
    
    $headings = array();
    
    $n = pg_numrows($textq);
    for ($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($textq, $i, PGSQL_ASSOC));
      $headings[$subsurvey_id][$item_id] = $ctext;
    }
    
    $nextHeading = 0;
    foreach($sorter->columns);
    
    
    
    
    
    
    
    // come up with response list, assuming only text questions
    
    
    
    
    
    
    
    
    
    
    
    
    
    $choiceq = pg_go("
      SELECT tt.revision
    
    ", $wces, __FILE__, __LINE__);
    
           
    
    $choicer = pg_go("
      SELECT *
      FROM resps AS re
      INNER JOIN responses_choice AS rc ON rc.parent = re.response_id
      INNER JOIN responses_choice_question AS rq ON rq.parent = rc.response_id
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
      SELECT rt.revision_id, rt.item_id, re.rtext 
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

go('rwtopics', array());

?>
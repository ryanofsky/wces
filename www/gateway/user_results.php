<?

require_once("wces/page.inc");
require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/survey.inc");
require_once("widgets/basic.inc");

require_once("wces/report_page.inc");
require_once("wces/report_generate.inc");

param($question_period_id);
param($class_id);
param($survey);
$class_id = (int)$class_id;
wces_connect();

$report_debug = true;

page_top("Results");

param($response_id);
$response_id = (int)$response_id;

param($save_id);
$save_id = (int)$save_id;

if ($response_id)
{
  wces_connect();

  $r = pg_go("SELECT topic_id FROM survey_responses WHERE response_id = $response_id", $wces, __FILE__, __LINE__);
  if (pg_numrows($r) != 1)
    $topic_id = 0;
  else
    $topic_id = pg_result($r, 0, 0);      
}

if ($response_id)
{
  print("<p><a href=\"$PHP_SELF\">Back</a></p>");
  $branch_id = get_base($topic_id);
  
  $makeall_survey_responses = 't1'; //'survey_responses';
  
  $class_list = $class_id ? $class_id : implode(',', $classes);
  
  $r = pg_go("
    CREATE TEMPORARY TABLE t1 AS
    SELECT s.* FROM survey_responses AS s
    WHERE response_id = $response_id
    ;
    
    CREATE TEMPORARY TABLE rwtopics AS
    SELECT $topic_id AS topic_id, cl.class_id, $branch_id AS branch_id, 
      $question_period_id AS question_period_id, 0 as ordinal, 
      NULL::INTEGER AS department_id, cl.course_id, e.user_id
    FROM classes AS cl
    INNER JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = 3
    WHERE cl.class_id IN ($class_list)
  ", $wces, __FILE__, __LINE__);
  
  $html = $text = false;
  $groups = array(CLASSES => true);
  
  makeall(true, $html, $text, $groups);
}
else
{
/*
DROP FUNCTION find_last_save(INTEGER);
CREATE FUNCTION find_last_save(INTEGER) RETURNS INTEGER AS '
  DECLARE
    response_id_ ALIAS FOR $1;
    parent_save_id INTEGER;
    child_save_id INTEGER;
  BEGIN
    SELECT INTO parent_save_id s.save_id
    FROM responses AS r
    INNER JOIN revisions AS s USING (revision_id)
    WHERE r.response_id = response_id_;
    
    SELECT INTO child_save_id MAX(find_last_save(response_id))
    FROM responses
    WHERE parent = response_id_;
    
    IF child_save_id > parent_save_id THEN
      RETURN child_save_id;
    ELSE
      RETURN parent_save_id;
    END IF;
  END;
' LANGUAGE 'plpgsql' ISCACHEABLE;

CREATE TABLE dp_response_saves AS
SELECT s.response_id, find_last_save(s.response_id) AS save_id
FROM survey_responses AS s
WHERE s.question_period_id IN (19, 99902);
*/

  $r = pg_go("
    SELECT t.parent, s.date, s.response_id, u.uni, t.parent, t.department_name, v.save_id, v.date AS vdate
    FROM survey_responses AS s
    INNER JOIN users AS u USING (user_id)
    INNER JOIN dp_topics AS t USING (topic_id)
    LEFT JOIN dp_response_saves USING (response_id)
    LEFT JOIN saves AS v USING (save_id)
    WHERE s.question_period_id IN (19, 99902)
    ORDER BY t.parent, v.date, s.date
  ", $wces, __FILE__, __LINE__);
  
  $result = new pg_segmented_wrapper($r, 'parent');
  
  $colors = array('FFFFFF', 'DDDDDD');
  
  $last_save_id = $colori = -1;
  while($result->row)
  {
    $row =& $result->row;
    if ($result->split)
    {
      print("<h4>" . $topics[$row['parent']]. "</h4>\n");
      print("<table border=1>\n");
    }
    
    if ($last_save_id != $row['save_id'])
    {
      $last_save_id = $row['save_id'];
      $colori = ($colori + 1) % count($colors);
    }

    $bg = " bgcolor=\"#$colors[$colori]\"";
    
    print("<tr><td$bg>$row[uni]</td><td$bg>$row[department_name]</td>");
    print("<td$bg><a href=\"$PHP_SELF?response_id=$row[response_id]$ASID\">$row[date]</a></td>");
    print("<td$bg><a href=\"$PHP_SELF?save_id=$row[save_id]$ASID\">$row[vdate]</a></td>");
    print("</tr>\n");
    
    $result->advance();
    
    if ($result->split)
    {
      print("</table>\n");
    }
  }
  
  

  $sql = '';
  $template = 'SELECT %d AS class_id, get_class(%d) AS info';  
  foreach($classes as $cid)
  {
    $sql .= sprintf($template, $cid, $cid);
    $template = ' UNION SELECT %d, get_class(%d)';
  }
  
  $c = pg_go($sql, $wces, __FILE__, __LINE__);
  $template = "<ul>\n";
  $n = pg_numrows($c);
  for ($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_row($c, $i, PGSQL_ASSOC));
    $name = format_class($info, 'Section %s');
    $template .= "  <li><a href=\"$PHP_SELF?topic_id=%TOPIC_ID%&class_id=$class_id$ASID\">$name</a></li>\n";
  }
  $template .= "  <li><a href=\"$PHP_SELF?topic_id=%TOPIC_ID%$ASID\">All Sections Combined</a></li>\n";
  $template .= "</ul>\n";
  
  $r = pg_go("SELECT ordinal, parent, topic_id, department_name FROM dp_topics ORDER BY ordinal, parent", $wces, __FILE__, __LINE__);
  $n = pg_numrows($r);
  $result = new pg_segmented_wrapper($r, 'ordinal');
  
  while ($result->row)
  {
    if ($result->split)
    {
      print("<h4>{$result->row['department_name']}</h4>\n");
    }
    $sname = $topics[$result->row['parent']];
    print("<h5>$sname</h5>\n");
    print(str_replace('%TOPIC_ID%', $result->row['topic_id'], $template));
    $result->advance();
  }
}

page_bottom();

?>

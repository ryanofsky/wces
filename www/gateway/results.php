<?

require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/database.inc");
require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/survey.inc");
require_once("wces/database.inc");
require_once("widgets/basic.inc");

require_once("wces/report_page.inc");
require_once("wces/report_generate.inc");

LoginProtect(LOGIN_ADMIN);

//$db_debug = 1;
//$report_debug = 1;

param('question_period_id');
param('class_id');
param('topic_id');

$class_id = (int)$class_id;
wces_connect();

$topic_id = (int)$topic_id;
if ($topic_id)
{
  wces_connect();

  $r = pg_go("
    SELECT d.department_name, d.department_people, t.year AS yr, t.semester AS sem, s.parent
    FROM gateway_topics AS t
    NATURAL JOIN gateway_departments AS d
    NATURAL JOIN specializations AS s
    WHERE t.topic_id = $topic_id
  ", $wces, __FILE__, __LINE__);
  if (pg_numrows($r) != 1)
    $topic_id = 0;
  else
  {
    $topic_info = pg_fetch_row($r, 0, PGSQL_ASSOC);      
    $topic = $topic_info['department_name'] . ' ' . $topics[$topic_info['parent']];
  }
}

page_top("Results", (bool)$topic_id);

if ($topic_id)
{
  print("<p><a href=\"$_SERVER[PHP_SELF]\">Back</a></p>\n<hr>\n");
  $question_branches = "813,814,815,964,965,966";
  
  if ($class_id)
  {
    $user_cond = "AND user_id IN (SELECT user_id FROM enrollments WHERE class_id = $class_id)";
    $class_cond = "AND cl.class_id = $class_id";
  }
  else
  {
    $user_cond = '';
    $class_cond = '';
  }
  
  $r = pg_go("
    CREATE TEMPORARY TABLE t1 AS
    SELECT s.* FROM survey_responses AS s
    WHERE topic_id = $topic_id $user_cond;
    
    CREATE TEMPORARY TABLE t2 AS
    SELECT t.* FROM textresponse_responses AS t
    INNER JOIN t1 AS s ON t.parent = s.response_id
    INNER JOIN revisions AS r ON r.revision_id = t.revision_id
    WHERE r.branch_id NOT IN ($question_branches);
    
    CREATE TEMPORARY TABLE rwtopics AS
    SELECT $topic_id AS topic_id, cl.class_id, 0 as ordinal, 
      NULL::INTEGER AS department_id, cl.course_id, e.user_id
    FROM classes AS cl
    LEFT JOIN enrollments AS e ON e.status = 3 AND e.class_id = cl.class_id
    WHERE cl.course_id = $course_id AND year = $topic_info[yr] AND semester = $topic_info[sem]
      $class_cond
  ", $wces, __FILE__, __LINE__);
  
  $html = $text = false;
  $groups = array(CLASSES => true);
  
  $options = array
  ( 'questionPeriods' => false,
    'choiceCache' => false,
    'surveyResponses' => 't1',
    'textResponses' => 't2',
    'alwaysGraph' => true,
    'skipLines' => true,
    'headings' => array(array('Topic:', $topic), 
      array('Semester:', $wces_semesters[$topic_info['sem']] . ' ' . $topic_info['yr']))
  );
 
  ob_start();
  makeall(true, $html, $text, $groups, $options);
  $contents = ob_get_contents();
  ob_end_clean();
  $replacements = array
  (  '%DEPARTMENT%' => $topic_info['department_name'],
     '%DEPARTMENT_PEOPLE%' => $topic_info['department_people']
  );
  print(str_replace(array_keys($replacements), array_values($replacements), $contents));

  print("<hr>\n");

  $r = pg_go("
    SELECT u.user_id, u.lastname, u.firstname, u.uni, c.ctext, t.rtext
    FROM textresponse_responses AS t
    INNER JOIN t1 AS s ON t.parent = s.response_id
    INNER JOIN users AS u USING (user_id)
    INNER JOIN revisions AS r ON r.revision_id = t.revision_id
    INNER JOIN textresponse_components AS c ON c.component_id = r.component_id
    WHERE r.branch_id IN ($question_branches)
    ORDER BY lastname, firstname, uni, user_id, r.branch_id
", $wces, __FILE__, __LINE__);

  if (pg_numrows($r) > 0)
  {
    print("<h4>Suggested Questions</h4>\n");
    
    print("<table border=1>\n");
    $questions =& new pg_segmented_wrapper($r, "user_id");
    while($questions->row)
    {
      extract($questions->row);
  
      if ($questions->split)
        print("<tr>\n  <td valign=top>$firstname $lastname<br><i>$uni</i></td>\n  <td>");
      
      print("<h5>$ctext</h5><p>" . htmlspecialchars($rtext) . "</p>\n");
      
      $questions->advance();
      
      if ($questions->split) print("</td>\n</tr>\n");    
    }
    print("</table>\n");
        
    print("<hr>\n");
  }
  print("<p>&nbsp;</p>");

  if ($class_id)
    $class_list = $class_id;
  else
    $class_list = "SELECT class_id FROM classes WHERE course_id = $course_id AND year = $topic_info[yr] AND semester = $topic_info[sem]";
   
  $r = pg_go("
    SELECT u.uni AS \"UNI\", u.firstname AS \"First Name\", u.lastname AS \"Last Name\", CASE WHEN r.response_id IS NOT NULL THEN 'Yes' ELSE 'No' END AS \"Surveyed\"
    FROM enrollments AS e
    INNER JOIN users AS u USING (user_id)
    LEFT JOIN t1 AS r ON r.user_id = e.user_id
    WHERE e.class_id IN ($class_list) AND e.status = 1
    ORDER BY \"Surveyed\" DESC, \"Last Name\", \"First Name\"
  ", $wces, __FILE__, __LINE__);
  
  pg_show($r, "<center><b>Response Statistics</b></center>");
}
else
{
  /*
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
*/

  
  $r = pg_go("
    SELECT t.year, t.semester, d.ordinal, s.parent, t.topic_id, d.department_name, cl.class_id, get_class(class_id) AS info
    FROM gateway_topics AS t
    NATURAL JOIN gateway_departments AS d
    INNER JOIN specializations AS s ON s.specialization_id = t.specialization_id
    INNER JOIN classes AS cl ON cl.course_id = t.course_id AND cl.year = t.year AND cl.semester = t.semester
    ORDER BY t.year DESC, t.semester DESC, d.ordinal, s.parent, cl.section
  ", $wces, __FILE__, __LINE__);
  $n = pg_numrows($r);
 
  $result = new pg_segmented_wrapper($r, array('year', 'semester', 'ordinal', 'topic_id'));
  
  while ($result->row)
  {
    if ($result->split[2])
    {
      print("<h4>{$wces_semesters[$result->row['semester']]} {$result->row['year']} {$result->row['department_name']}</h4>\n");
    }
    
    if ($result->split[3])
    {
      $sname = $topics[$result->row['parent']];
      print("<h5>$sname</h5>\n");
      print("<ul>\n");
    }
 
    $name = format_class($result->row['info'], 'Section %s');
    $topic_id = $result->row['topic_id'];
    print("  <li><a href=\"$_SERVER[PHP_SELF]?topic_id=$topic_id&class_id={$result->row['class_id']}$ASID\">$name</a></li>\n");
   
    
    $result->advance();
    
    if ($result->split[3])
    {
      print("  <li><a href=\"$_SERVER[PHP_SELF]?topic_id=$topic_id$ASID\">All Sections Combined</a></li>\n");
      print("</ul>\n");
    }
  }
}

page_bottom();

?>

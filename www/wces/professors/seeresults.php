<?

  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wbes/component.inc");
  require_once("wbes/component_choice.inc");
  require_once("wbes/component_textresponse.inc");
  require_once("wbes/component_heading.inc");
  require_once("wbes/component_text.inc");
  require_once("wbes/survey.inc");
  require_once("wces/database.inc");
  require_once("widgets/basic.inc");

  require_once("wces/report_page.inc");
  require_once("wces/report_generate.inc");

  login_protect(login_professor);
  $user_id = login_getuserid();
  wces_connect();
  $profname = login_getname();


function listclasses()
{
  global $profid,$profname,$db, $wces;

  $uid = login_getuserid();
  wces_connect();

  print("<h3>$profname - Survey Responses</h3>\n");
  
  $result = pg_query("
    SELECT t.topic_id, t.class_id, get_class(t.class_id) AS cl, q.question_period_id, 
      COUNT(r.user_id) AS count, q.displayname, q.year, q.semester
    FROM enrollments AS e
    INNER JOIN wces_topics AS t USING (class_id)
    INNER JOIN classes AS cl ON cl.class_id = e.class_id
    INNER JOIN dartmouth_question_periods AS q ON q.year = cl.year AND q.semester = cl.semester AND q.profdate < NOW()
    LEFT JOIN survey_responses AS r ON r.topic_id = t.topic_id AND r.question_period_id = q.question_period_id
    WHERE e.user_id = $uid AND e.status = 3
    GROUP BY t.class_id, q.question_period_id, q.displayname, t.topic_id, q.year, q.semester
    ORDER BY q.question_period_id DESC, cl
  ", $wces, __FILE__, __LINE__);

  $sems = array("Spring", "Summer", "Winter", "Fall");
  $q = new pg_segmented_wrapper($result, array("question_period_id"));
  while($q->row)
  {
    extract($q->row);
    if ($q->split[0]) print("<h4>$sems[$semester] $year $displayname</h4>\n<ul>\n");
    if ($count == 0)
      print("  <li>" . format_class($cl) . " (No Responses Found)</li>");
    else
      print("  <li><a href=\"seeresults.php?question_period_id=$question_period_id&class_id=$class_id\">" . format_class($cl) . "</a></li>");

    $q->advance();
    if ($q->split[0]) print("</ul>\n");
  }
}

function tshowresults($question_period_id,$class_id)
{
  global $profid, $wces;

  $user_id = login_getuserid();

  print('<h3><a href="seeresults.php">Back</a></h3><hr>');

  $sqloptions = array ("standard" => true, "custom" => true);

  $criteria = array
  (
    PROFESSORS => array($user_id),
    CLASSES => array($class_id),
    COURSES => false,
    DEPARTMENTS => false,
    QUESTION_PERIODS => array($question_period_id),
    CATEGORIES => false
  );

  $sort = array(QUESTION_PERIODS, COURSES, DEPARTMENTS, PROFESSORS, CATEGORIES);
  $groups = array(QUESTION_PERIODS => 1, CLASSES => 1, COURSES => 1, DEPARTMENTS => 1, PROFESSORS => 1, CATEGORIES => 0);

  wces_connect();
  report_findtopics("rwtopics", $criteria);
  //$result = pg_query("SELECT * FROM rwtopics", $wces, __FILE__, __LINE__);
  //pg_show($result);
  report_findgroups("rwtopics", $groups, $sort);

  $html = $text = false;
  makeall(true, $html, $text, $groups);
}

///////////////////////////////////////////////////////////////////////////////

param($question_period_id);
param($topic_id);
param($class_id);

$question_period_id = (int) $question_period_id;
$class_id = (int) $class_id;

$showcsv = $server_url->xpath ? true : false;

if (!$showcsv)
  page_top("Survey Results");


if ($question_period_id && $class_id)
  tshowresults($question_period_id,$class_id);
else
  listclasses();

if (!$showcsv)
  page_bottom();

?>

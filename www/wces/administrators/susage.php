<?
ini_set("include_path", ini_get("include_path") . ";:.:/afs/thayer/web/eval/include");

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);

param($survey_category_id);
wces_connect();

$result = pg_query("
  SELECT question_period_id, displayname, year, semester
  FROM semester_question_periods
  WHERE question_period_id = (SELECT get_question_period())
", $wces, __FILE__, __LINE__);
extract(pg_fetch_array($result,0,PGSQL_ASSOC));

page_top("Student Usage Data for $displayname]");

///////////////////////////////////////////////////////////////////////////////

print("<p>Filtering: ");
$survey_categories = pg_query("SELECT survey_category_id, name FROM survey_categories", $wces, __FILE__, __LINE__);
$first = true;
$foundfilter = false;
$n = pg_numrows($survey_categories);
for($i=0; $i < $n; ++$i)
{
  $survey_category = pg_fetch_array($survey_categories,$i,PGSQL_ASSOC);
  if ($first) $first = false; else print (" | ");
  if ($survey_category["survey_category_id"] == $survey_category_id)
  {
    $foundfilter = true;
    print($survey_category["name"]);
  }
  else
    print('<a href="susage.php?survey_category_id=' . $survey_category["survey_category_id"] . '">' . $survey_category["name"] . "</a>");
}
if (!$first) print (" | ");
if (!$foundfilter)
{
  $survey_category_id = 0;
  print("None</p>");
}
else
  print('<a href="susage.php">None</a></p>');

///////////////////////////////////////////////////////////////////////////////

$times = array();
$times["begin"] = microtime();

$cat = $survey_category_id ? "AND t.category_id = $survey_category_id" : "";

//todo: use user_id instead of response_id
pg_query("
  CREATE TEMPORARY TABLE surveycounts AS
  SELECT t.class_id, COUNT(DISTINCT response_id) AS responses
  FROM wces_topics AS t
  INNER JOIN classes AS cl USING (class_id)
  LEFT JOIN survey_responses AS s ON (s.topic_id = t.topic_id AND s.question_period_id = $question_period_id)
  WHERE cl.year = $year AND cl.semester = $semester $cat
  GROUP BY t.class_id
",$wces,__FILE__, __LINE__);

$times["findquestionsets"] = microtime();

pg_query("
  CREATE TEMPORARY TABLE surveyclasses AS
  SELECT c.course_id, cc.class_id, cl.section, s.code AS scode, c.code, c.name, u.firstname, u.lastname, u.user_id, COALESCE(cl.students,0) AS students, COALESCE(cc.responses,0) AS responses
  FROM surveycounts AS cc
  INNER JOIN classes AS cl USING (class_id)
  INNER JOIN courses AS c USING (course_id)
  INNER JOIN subjects AS s USING (subject_id)
  LEFT JOIN enrollments AS e ON (e.class_id = cl.class_id AND e.status = 3)
  LEFT JOIN users AS u USING (user_id)
",$wces,__FILE__, __LINE__);

$times["getbigclasslist"] = microtime();

$y = pg_query("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$wces,__FILE__, __LINE__);

$times["sumclasslist"] = microtime();

extract(pg_fetch_array($y,0,PGSQL_ASSOC));

///////////////////////////////////////////////////////////////////////////////

?>

<h3>Aggregate Student Usage</h3>

Total number of surveys: <b><?=$students?></b><br>
Number of surveys completed: <b><?=$responses?></b><br>

<img src="<?=$wces_path?>media/graphs/susagegraph.php?blank=<?=$students-$responses?>&filled=<?=$responses?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/susagelegend.gif" width=147 height=31><br>

<h3>Individual Class Usage</h3>
<p><font size="-1">Sorted by number of surveys that haven't been filled out</font></p>

<?

$classes = pg_query("
  SELECT CASE WHEN students < responses THEN responses ELSE students END AS students,
    responses, class_id, user_id, scode, code, section, name, firstname, lastname
  FROM surveyclasses
  ORDER BY students DESC
",$wces, __FILE__, __LINE__);

$n = pg_numrows($classes);

print("<ul>\n");
for($i=0; $i<$n; ++$i)
{
  extract(pg_fetch_array($classes,$i,PGSQL_ASSOC));
  $numbers = $students == 0 ? "$responses surveys completed" : (($students - $responses) . " / $students surveys left");
  print ("  <li>$numbers, <a href=\"info.php?class_id=$class_id&surveys=1\">$scode$code$section <i>$name</i></a> - Professor <a href=\"info.php?user_id=$user_id&surveys=1\">$firstname $lastname</a></li>\n");
}
print("</ul>");

flush();

///////////////////////////////////////////////////////////////////////////////

// TODO: this section has still not been converted for new database
//
// pg_query("CREATE TEMPORARY TABLE studsurvs( cunix TINYTEXT, surveys INTEGER, surveyed INTEGER )", $wces, __FILE__, __LINE__);
// pg_query("
//
//   REPLACE INTO studsurvs(cunix, surveys, surveyed)
//   SELECT u.cunix, COUNT(DISTINCT q.questionsetid), COUNT(DISTINCT cs.answersetid)
//   FROM qsets AS qs
//   INNER JOIN enrollments AS e ON e.classid = qs.classid
//   INNER JOIN users AS u ON u.userid = e.userid
//   INNER JOIN questionsets AS q ON q.questionsetid = qs.questionsetid
//   LEFT JOIN answersets AS a ON a.questionsetid = q.questionsetid AND a.classid = e.classid AND a.questionperiodid = '$questionperiodid'
//   LEFT JOIN completesurveys AS cs ON cs.userid = e.userid AND cs.answersetid = a.answersetid
//   GROUP BY u.userid
//
// ", $wces, __FILE__, __LINE__);
//
// $times["getstudentusage"] = microtime();
//
// $students = pg_query("SELECT cunix, IF(surveys-surveyed<=0,1,0) AS didall, IF(surveyed>0,1,0) AS didone FROM studsurvs ORDER BY didall DESC, didone DESC, RAND()", $wces, __FILE__, __LINE__);
//
// print("<h3>Individual Student Usage</h3>\n");
//
// $levels = array
// (
//   2 => "<h4>Students who completed all of their surveys</h4>\n<blockquote>",
//   1 => "\n</blockquote>\n<h4>Students who completed at least one of their surveys</h4>\n<blockquote>",
//   0 => "\n</blockquote>\n<h4>Students who completed at none of their surveys</h4>\n<blockquote>"
// );
//
// $oldlevel = "";
//
// while ($student = mysql_fetch_array($students))
// {
//   $didall = $didone = "";
//   extract($student);
//   $level = $didall + $didone;
//   if (!($level === $oldlevel))
//   {
//     print($levels[$level]);
//     $first = true;
//   }
//   if ($first) $first = false; else print(", ");
//   print("\n  <a href=\"${wces_path}administrators/enrollment.php?unilist=$cunix\">$cunix</a>");
//   $oldlevel = $level;
// }
// print("</blockquote>");

printtimes($times);

page_bottom();
?>
<?
require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);

param($survey_category_id);
param($sort);

wces_connect();

$result = pg_query("
  SELECT question_period_id, displayname, year, semester
  FROM semester_question_periods
  WHERE question_period_id = (SELECT get_question_period())
", $wces, __FILE__, __LINE__);
extract(pg_fetch_array($result,0,PGSQL_ASSOC));

page_top("Student Usage Data for $displayname");

///////////////////////////////////////////////////////////////////////////////
$ssid = $survey_category_id = (int) $survey_category_id;
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
    print('<a href="susage.php?survey_category_id=' 
      . $survey_category["survey_category_id"] . '&sort='. $sort . '">'
      . $survey_category["name"] . "</a>");
}
if (!$first) print (" | ");
if (!$foundfilter)
{
  $survey_category_id = 0;
  print("None</p>");
}
else
  print('<a href="susage.php">None</a><br>');
///////////////////////////////////////////////////////////////////////////////

print("Sorting: ");

$ordersql = array("cl", "students - responses DESC", "responses DESC");
$order = array("Course Code", "Number of surveys left", "Number of surveys completed"); 

$sort = (int)$sort;
if (!isset($order[$sort])) $sort = 0;

$first = true;
foreach($order as $k => $v)
{
  if ($first) $first = false; else print (" | ");
  if ($sort == $k)
    print($v);
  else
    print('<a href="susage.php?survey_category_id=' 
      . $ssid . "&sort=" . $k  . '">'
      . $v . "</a>");
}
print("</p>");

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
  WHERE t.category_id IS NOT NULL AND cl.year = $year AND cl.semester = $semester $cat
  GROUP BY t.class_id
",$wces,__FILE__, __LINE__);

$times["count_responses"] = microtime();

pg_query("
  CREATE TEMPORARY TABLE surveyclasses AS
  SELECT cc.class_id, COALESCE(cl.students,0) AS students, COALESCE(cc.responses,0) AS responses
  FROM surveycounts AS cc
  INNER JOIN classes AS cl USING (class_id)
",$wces,__FILE__, __LINE__);

$times["count_enrollments"] = microtime();

$y = pg_query("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$wces,__FILE__, __LINE__);

$times["sum_counts"] = microtime();

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
  SELECT CASE WHEN students < responses THEN responses ELSE students END AS students, responses, get_class(class_id) AS cl, get_profs(class_id) AS p
  FROM surveyclasses
  ORDER BY $ordersql[$sort]
",$wces, __FILE__, __LINE__);

$times["get_names"] = microtime();

$n = pg_numrows($classes);

?>
<table border=1>
<tr>
  <td align=center valign=bottom><img src="<?=$wces_path?>media/report/surveys_completed.gif" width=16 height=151 border=0 alt="Surveys Completed"></td>
  <td align=center valign=bottom><img src="<?=$wces_path?>media/report/surveys_left.gif" width=16 height=94 border=0 alt="Surveys Left"></td>
  <td align=center valign=bottom><img src="<?=$wces_path?>media/report/total_surveys.gif" width=16 height=105 border=0 alt="Total Surveys"></td>  
  <td align=center valign=bottom>Class Information</td>
</tr>
<?

for($i=0; $i<$n; ++$i)
{
  extract(pg_fetch_array($classes,$i,PGSQL_ASSOC));
  print("<tr>\n  <td>$responses</td>\n  <td>" . ($students - $responses)  . "</td>\n  <td>$students</td>\n");
  $classinfo = format_class($cl, "%c %n Section %s", true) . format_profs($p, true, "<br>Professor ");
  print("  <td>$classinfo</td>\n");  
  print("</tr>\n");
}

?>

</table>
<?



flush();

///////////////////////////////////////////////////////////////////////////////

$cat = $survey_category_id ? "AND t.category_id = $survey_category_id" : "";

$students = pg_query("
  SELECT i.user_id, u.uni, CASE WHEN i.responses = 0 THEN 0 WHEN i.classes <= i.responses THEN 2 ELSE 1 END AS level
  FROM
    (SELECT e.user_id, COUNT (DISTINCT t.topic_id) AS classes, COUNT(DISTINCT s.topic_id) AS responses
    FROM wces_topics AS t
    INNER JOIN enrollments AS e ON e.class_id = t.class_id AND e.status = 1
    LEFT JOIN survey_responses AS s ON s.user_id = e.user_id AND s.topic_id = t.topic_id AND s.question_period_id = $question_period_id
    WHERE t.class_id IS NOT NULL $cat
    GROUP BY e.user_id) AS i
  INNER JOIN users AS u USING (user_id)
  ORDER BY level DESC, random();
", $wces, __FILE__, __LINE__);

$times["get_individual_students"] = microtime();

print("<h3>Individual Student Usage</h3>\n");

$levels = array
(
  2 => "<h4>Students who completed all of their surveys</h4>\n<blockquote>",
  1 => "\n</blockquote>\n<h4>Students who completed at least one of their surveys</h4>\n<blockquote>",
  0 => "\n</blockquote>\n<h4>Students who completed at none of their surveys</h4>\n<blockquote>"
);

$oldlevel = "";

$n = pg_numrows($students);
for($i = 0; $i < $n; ++$i)
{
  extract(pg_fetch_row($students, $i, PGSQL_ASSOC));
  if (!($level === $oldlevel))
  {
    print($levels[$level]);
    $first = true;
  }
  if ($first) $first = false; else print(", ");
  print("\n  <a href=\"${wces_path}administrators/info.php?user_id=$user_id\">$uni</a>");
  $oldlevel = $level;
}
print("</blockquote>");

$times["print_individual_students"] = microtime();

printtimes($times);

page_bottom();
?>

<?
require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");

login_protect(login_administrator);

param('category_id');
param('sort');
param('question_period_id');

wces_connect();

// generate lists of options
$questionPeriodChoices = usage_sql_array("
  SELECT q.question_period_id, q.displayname
  FROM semester_question_periods AS q
  WHERE EXISTS (SELECT * FROM wces_topics AS t WHERE t.question_period_id = q.question_period_id)
  ORDER BY begindate
", $wces, __FILE__, __LINE__);

$categoryChoices = usage_sql_array("
  SELECT 0 AS id, 'None'
  UNION
  SELECT c.survey_category_id, c.name
  FROM survey_categories AS c
  ORDER BY id
", $wces, __FILE__, __LINE__);

$sortChoices = array("Course Code", "Number of surveys left", "Number of surveys completed"); 
$sortSql = array("cl", "students - responses DESC", "responses DESC");

// figure out current values
if (!isset($sort) || !isset($sortChoices[$sort])) $sort = 0;
if (!isset($category_id) || !isset($categoryChoices[$category_id])) $category_id = 0;
if (isset($question_period_id) && isset($questionPeriodChoices[$question_period_id]))
  set_question_period($question_period_id);
else
  $question_period_id = get_question_period();

$currentValues = array('question_period_id' => $question_period_id,
  'category_id' => $category_id, 'sort' => $sort);

// get information about selected question period
$result = pg_go("
  SELECT question_period_id, displayname, year, semester
  FROM semester_question_periods
  WHERE question_period_id = $question_period_id
", $wces, __FILE__, __LINE__);

extract(pg_fetch_array($result,0,PGSQL_ASSOC));

page_top("Student Usage Data for $displayname");


print("<p>Question Period: "); 
usage_menu($questionPeriodChoices, 'question_period_id', $currentValues);
print("<br>\n");

print("Filtering: "); 
usage_menu($categoryChoices, 'category_id', $currentValues);
print("<br>\n");

print("Sorting: "); 
usage_menu($sortChoices, 'sort', $currentValues);
print("</p>\n");




///////////////////////////////////////////////////////////////////////////////

$times = array();
$times["begin"] = microtime();

$cat = $category_id ? "AND t.category_id = $category_id" : "";

//todo: use user_id instead of response_id
pg_go("
  CREATE TEMPORARY TABLE surveycounts AS
  SELECT t.class_id, COUNT(DISTINCT response_id)::INTEGER AS responses
  FROM wces_topics AS t
  INNER JOIN classes AS cl USING (class_id)
  LEFT JOIN survey_responses AS s USING (topic_id)
  WHERE t.question_period_id = $question_period_id $cat
  GROUP BY t.class_id
",$wces,__FILE__, __LINE__);

$times["count_responses"] = microtime();

pg_go("
  CREATE TEMPORARY TABLE surveyclasses AS
  SELECT cc.class_id, COALESCE(cl.students,0) AS students, COALESCE(cc.responses,0) AS responses
  FROM surveycounts AS cc
  INNER JOIN classes AS cl USING (class_id)
",$wces,__FILE__, __LINE__);

$times["count_enrollments"] = microtime();

$y = pg_go("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$wces,__FILE__, __LINE__);

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

$classes = pg_go("
  SELECT CASE WHEN students < responses THEN responses ELSE students END AS students, responses, get_class(class_id) AS cl, get_profs(class_id) AS p
  FROM surveyclasses
  ORDER BY $sortSql[$sort]
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

/*

$cat = $survey_category_id ? "AND t.category_id = $survey_category_id" : "";

$students = pg_go("
  SELECT i.user_id, u.uni, CASE WHEN i.responses = 0 THEN 0 WHEN i.classes <= i.responses THEN 2 ELSE 1 END AS level
  FROM
    (SELECT e.user_id, COUNT (DISTINCT t.topic_id) AS classes, COUNT(DISTINCT s.topic_id) AS responses
    FROM question_periods_topics AS qt
    INNER JOIN wces_topics AS t USING (topic_id)
    INNER JOIN enrollments AS e ON e.class_id = t.class_id AND e.status = 1
    LEFT JOIN survey_responses AS s ON s.user_id = e.user_id AND s.topic_id = t.topic_id AND s.question_period_id = qt.question_period_id
    WHERE qt.question_period_id = $question_period_id $cat
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
  print("\n  <a href=\"${wces_path}administrators/info.php?user_id=$user_id$ASID\">$uni</a>");
  $oldlevel = $level;
}
print("</blockquote>");

$times["print_individual_students"] = microtime();

*/

printtimes($times);

page_bottom();
?>

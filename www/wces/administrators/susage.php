<?
require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);

param($topicid);

$db = wces_connect();
wces_GetCurrentQuestionPeriod($db, $questionperiodid, $questionperiod, $year, $semester);
$semester = ucfirst($semester);
page_top("Student Usage Data for $semester $year $questionperiod");

flush();


///////////////////////////////////////////////////////////////////////////////

print("<p>Filtering: ");
$topics = db_exec("SELECT topicid, name FROM topics", $db, __FILE__, __LINE__);
$first = true;
$foundfilter = false;
while($topic = mysql_fetch_assoc($topics))
{
  if ($first) $first = false; else print (" | ");
  if ($topic["topicid"] == $topicid)
  {
    $foundfilter = true;
    print($topic["name"]);
  }
  else
    print('<a href="susage.php?topicid=' . $topic["topicid"] . $ASID . '">' . $topic["name"] . "</a>");
}
if (!$first) print (" | ");
if (!$foundfilter)
{
  $topicid = 0;
  print("None</p>");
}
else
  print('<a href="susage.php' . $QSID . '">None</a></p>');

///////////////////////////////////////////////////////////////////////////////

$times = array();
$times["begin"] = microtime();

db_exec("
  CREATE TEMPORARY TABLE surveyclasses(
  courseid INTEGER NOT NULL, classid INTEGER NOT NULL, section CHAR(3) NOT NULL,
  scode CHAR(4), code CHAR(5), name TINYTEXT, pname TINYTEXT, 
  professorid INTEGER, students INTEGER, responses INTEGER,
  PRIMARY KEY(classid))
",$db,__FILE__, __LINE__);

$times["sql1_create_table_aggregate"] = microtime();

db_exec("
  REPLACE INTO surveyclasses (courseid, classid, section, scode, code, name, pname, professorid, students, responses)
  SELECT c.courseid, cl.classid, cl.section, s.code , c.code, c.name, p.name, p.professorid, IFNULL(cl.students,0), COUNT(distinct a.userid)
  FROM groupings AS g
  INNER JOIN classes AS cl ON (cl.classid = g.linkid)
  INNER JOIN courses AS c ON (c.courseid = cl.courseid)
  INNER JOIN subjects AS s ON (s.subjectid = c.subjectid)
  LEFT JOIN professors AS p ON (cl.professorid = p.professorid)
  LEFT JOIN cheesyresponses AS a ON (g.linkid = a.classid AND a.questionperiodid = '$questionperiodid')
  WHERE g.linktype = 'classes' " . ($topicid ? " AND g.topicid = $topicid " : "") . "
  GROUP BY g.linkid
",$db,__FILE__, __LINE__);

$times["sql2_get_big_class_list_aggregate"] = microtime();

$y = db_exec("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$db,__FILE__, __LINE__);

$times["sql3_sum_class_list_aggregate"] = microtime();

extract(mysql_fetch_array($y));

///////////////////////////////////////////////////////////////////////////////

?>

<h3>Aggregate Student Usage</h3>

Total number of surveys: <b><?=$students?></b><br>
Number of surveys completed: <b><?=$responses?></b><br>

<img src="<?=$wces_path?>media/graphs/susagegraph.php?blank=<?=$students-$responses?>&filled=<?=$responses?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/susagelegend.gif" width=147 height=31><br>

<h3>Individual Class Usage</h3>
<p><font size="-1">Sorted by number of surveys that haven't been filled out</font></p>

<?

$classes = mysql_query("SELECT IF(students < responses, responses, students) AS students, responses, classid, professorid, scode, code, section, name, pname FROM surveyclasses ORDER BY IF(students - responses < 0, 0, students - responses) DESC, students DESC",$db);
print("<ul>\n");
while ($class = mysql_fetch_array($classes))
{
  $students = $responses = $classid = $professorid = 0; $scode = $code = $section = $name = $pname = "Unknown";
  extract($class);
  $numbers = $students == 0 ? "$responses surveys completed" : (($students - $responses) . " / $students surveys left");
  print ("  <li>$numbers, <a href=\"info.php?classid=$classid&surveys=1$ASID\">$scode$code$section <i>$name</i></a> - Professor <a href=\"info.php?professorid=$professorid&surveys=1$ASID\">$pname</a></li>\n");
}
print("</ul>");

flush();

///////////////////////////////////////////////////////////////////////////////

$times["print_aggregate"] = microtime();

db_exec("
  CREATE TEMPORARY TABLE studsurvs AS
  SELECT u.cunix AS cunix, COUNT(DISTINCT g.linkid) AS surveys, COUNT(DISTINCT a.classid) AS surveyed
  FROM groupings AS g
  INNER JOIN enrollments AS e ON e.classid = g.linkid
  INNER JOIN users AS u ON u.userid = e.userid
  LEFT JOIN cheesyresponses AS a ON a.userid = e.userid AND a.classid = g.linkid AND a.questionperiodid = '$questionperiodid'" . ($topicid ? "
  WHERE g.topicid = $topicid " : "") . "  
  GROUP BY e.userid
", $db, __FILE__, __LINE__);
$times["sql1_individual"] = microtime();

$students = db_exec("SELECT cunix, IF(surveys-surveyed<=0,1,0) AS didall, IF(surveyed>0,1,0) AS didone FROM studsurvs ORDER BY didall DESC, didone DESC, RAND()", $db, __FILE__, __LINE__);
$times["sql2_individual"] = microtime();

print("<h3>Individual Student Usage</h3>\n");

$levels = array
(
  2 => "<h4>Students who completed all of their surveys</h4>\n<blockquote>",
  1 => "\n</blockquote>\n<h4>Students who completed at least one of their surveys</h4>\n<blockquote>",
  0 => "\n</blockquote>\n<h4>Students who completed at none of their surveys</h4>\n<blockquote>"
);  

$oldlevel = "";

while ($student = mysql_fetch_array($students))
{
  $didall = $didone = "";
  extract($student);
  $level = $didall + $didone;
  if (!($level === $oldlevel))
  {
    print($levels[$level]);
    $first = true;
  }  
  if ($first) $first = false; else print(", ");
  print("\n  <a href=\"${wces_path}administrators/info.php?cunix=$cunix&surveys=1&$ASID\">$cunix</a>");
  $oldlevel = $level;
}
print("</blockquote>");
$times["print_individual"] = microtime();


printtimes($times);

page_bottom();
?>
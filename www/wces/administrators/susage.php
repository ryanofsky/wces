<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);

$db = wces_connect();

wces_GetCurrentQuestionPeriod($db, &$questionperiodid, &$questionperiod, &$year, &$semester);
$semester = ucfirst($semester);

page_top("Student Usage Data for $semester $year $questionperiod");

wces_Findclasses($db,"currentclasses", &$questionperiodid, &$year, &$semester);
db_exec("CREATE TEMPORARY TABLE surveyclasses(
courseid INTEGER NOT NULL,
classid INTEGER NOT NULL,
section CHAR(3) NOT NULL,
scode CHAR(4),
code CHAR(5),
name TINYTEXT,
pname TINYTEXT,
professorid INTEGER,
students INTEGER,
responses INTEGER,
PRIMARY KEY(classid))",$db,__FILE__, __LINE__);
db_exec("REPLACE INTO surveyclasses (courseid, classid, section, scode, code, name, pname, professorid, students, responses)
SELECT c.courseid, cc.classid, cl.section, s.code, c.code, c.name, p.name, p.professorid, IFNULL(cl.students,0), IFNULL(MAX(a.responses),0)
FROM currentclasses AS cc
LEFT JOIN classes AS cl USING (classid)
LEFT JOIN courses AS c USING (courseid)
LEFT JOIN subjects AS s USING (subjectid)
LEFT JOIN professors AS p ON (cl.professorid = p.professorid)
LEFT JOIN answersets AS a ON (cl.classid = a.classid AND a.questionperiodid = $questionperiodid)
GROUP BY cc.classid",$db,__FILE__, __LINE__);

$y = db_exec("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$db,__FILE__, __LINE__);
extract(mysql_fetch_array($y));
%>

<h3>Aggregate Student Usage</h3>

Total number of surveys: <b><%=$students%></b><br>
Number of surveys completed: <b><%=$responses%></b><br>

<img src="<%=$server_wcespath%>media/graphs/susagegraph.php?blank=<%=$students-$responses%>&filled=<%=$responses%>" width=200 height=200><img src="<%=$server_wcespath%>media/graphs/susagelegend.gif" width=147 height=31><br>

<h3>Individual Class Usage</h3>
<p><font size="-1">Sorted by number of surveys that haven't been filled out</font></p>

<%

$classes = mysql_query("SELECT * FROM surveyclasses ORDER BY (students - responses) DESC, students DESC",$db);
print("<ul>\n");
while ($class = mysql_fetch_array($classes))
{
  $students = $responses = $classid = $professorid = 0; $scode = $code = $section = $name = $pname = "Unknown";
  extract($class);
  $numbers = $students == 0 ? "$responses surveys completed" : (($students - $responses) . " / $students surveys left");
  print ("  <li>$numbers, <a href=\"${server_wcespath}info/classinfo.php?classid=$classid\">$scode$code$section <i>$name</i></a> - Professor <a href=\"${server_wcespath}info/profinfo.php?professorid=$professorid\">$pname</a></li>\n");
}
print("</ul>");

/*

$sql = "SELECT u.cunix, MIN(e.surveyed) AS didall, MAX(e.surveyed) AS didone FROM currentclasses AS cc INNER JOIN enrollments AS e USING (classid) LEFT JOIN users AS u USING (userid) GROUP BY e.userid HAVING didone = 'yes' ORDER BY didall DESC, RAND()";
$students = mysql_query($sql,$db);
print("<h3>Individual Student Usage</h3>\n");
$first = true; $didalllast = 0;
while ($student = mysql_fetch_array($students))
{
  extract($student);
  if (!($didall === $didalllast))
  {
    $first = true;
    if ($didall == 'yes')
      print("<h4>Students who completed all of their surveys</h4>\n<blockquote>");
    else
      print("</blockquote>\n<h4>Students who completed at least one of their surveys</h4>\n<blockquote>");
  };
  if ($first) $first = false; else print (",\n");
  print ("<a href=\"http://www.columbia.edu/cgi-bin/lookup.pl?$cunix\">$cunix</a>");
  $didalllast = $didall;
}
print("</blockquote>");

*/

mysql_query("DROP TABLE currentclasses",$db);
mysql_query("DROP TABLE surveyclasses",$db);

page_bottom();
%>








<%
require_once("server.inc");
require_once("login.inc");
require_once("page.inc");
login_protect(login_administrator);
page_top("Usage Data");

$db = wces_connect();

$y = mysql_query("CREATE TEMPORARY TABLE currentclasses (classid INTEGER NOT NULL, PRIMARY KEY(classid))",$db);
$y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.classid
  WHERE g.linktype = 'classes' && cl.year = 2000 && cl.semester = 'fall'",$db);
$y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.courseid
  WHERE g.linktype = 'courses' && cl.year = 2000 && cl.semester = 'fall' && NOT (ASCII(cl.section) IN (82,114,86,118))",$db);
$y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes AS cl ON g.linkid = cl.professorid
  WHERE g.linktype = 'professors'  && cl.year = 2000 && cl.semester = 'fall'",$db);
$y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.subjectid INNER JOIN Classes as cl ON c.courseid = cl.courseid
  WHERE g.linktype = 'subjects' && cl.year = 2000 && cl.semester = 'fall'",$db);
$y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.departmentid INNER JOIN Classes as cl ON c.courseid = cl.courseid 
  WHERE g.linktype = 'departments' && cl.year = 2000 && cl.semester = 'fall'",$db);

mysql_query("CREATE TEMPORARY TABLE surveyclasses (courseid INTEGER NOT NULL, classid INTEGER NOT NULL, section CHAR(3) NOT NULL, scode CHAR(4), code CHAR(5), name TINYTEXT, pname TINYTEXT, professorid INTEGER, students INTEGER, responses INTEGER, PRIMARY KEY(classid))",$db);
mysql_query("REPLACE INTO surveyclasses (courseid, classid, section, scode, code, name, pname, professorid, students, responses)
SELECT c.courseid, cc.classid, cl.section, s.code, c.code, c.name, p.name, p.professorid, IFNULL(cl.students,0), IFNULL(MAX(a.responses),0)
FROM currentclasses AS cc
LEFT JOIN Classes AS cl USING (classid)
LEFT JOIN Courses AS c USING (courseid)
LEFT JOIN Subjects AS s USING (subjectid)
LEFT JOIN Professors AS p ON (cl.professorid = p.professorid)
LEFT JOIN AnswerSets AS a ON (cl.classid = a.classid AND a.questionperiodid = 5)
GROUP BY cc.classid",$db);

$y = mysql_query("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$db);
extract(mysql_fetch_array($y));
%>

<h3>Aggregate Student Usage</h3>

Total number of surveys: <b><%=$students%></b><br>
Number of surveys completed: <b><%=$responses%></b><br>

<img src="susagegraph.php?blank=<%=$students-$responses%>&filled=<%=$responses%>" width=200 height=200><img src=susagelegend.gif width=147 height=31><br>

<h3>Individual Class Usage</h3>
<p><font size=-1>Sorted by number of surveys that haven't been filled out</font></p>

<%


$classes = mysql_query("SELECT * FROM surveyclasses ORDER BY (students - responses) DESC, students DESC",$db);
print("<ul>\n");
while ($class = mysql_fetch_array($classes))
{
  extract($class);
  $numbers = $students == 0 ? "$responses surveys completed" : (($students - $responses) . " / $students surveys left");
  print ("  <li>$numbers, <a href=\"${server_wcespath}students/classinfo.php?classid=$classid\">$scode$code$section <i>$name</i></a> - Professor <a href=\"${server_wcespath}students/profinfo.php?professorid=$professorid\">$pname</a></li>\n");
}
print("</ul>");

$sql = "SELECT u.cunix, MIN(e.surveyed) AS didall, MAX(e.surveyed) AS didone FROM currentclasses AS cc INNER JOIN Enrollments AS e USING (classid) LEFT JOIN Users AS u USING (userid) GROUP BY e.userid HAVING didone = 'yes' ORDER BY didall DESC, RAND()";
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
%>


<%

mysql_query("DROP TABLE currentclasses",$db);
mysql_query("DROP TABLE surveyclasses",$db);

page_bottom();
%>
<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  page_top("Class Information","1000");
  
$db = wces_connect();

$infoq = mysql_query(

"SELECT cl.name AS clname, cl.section, cl.year, cl.semester, cl.students, p.name as pname, p.professorid, c.code, c.name, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname, c.courseid, d.departmentid, s.subjectid, sc.schoolid, dv.code AS dvcode, dv.divisionid
FROM classes as cl
LEFT JOIN courses as c USING (courseid)
LEFT JOIN departments as d USING (departmentid)
LEFT JOIN subjects as s ON (c.subjectid = s.subjectid)
LEFT JOIN divisions as dv ON (cl.divisionid = dv.divisionid)
LEFT JOIN schools as sc ON (c.schoolid = sc.schoolid)
LEFT JOIN professors as p ON (cl.professorid = p.professorid)
WHERE cl.classid = '" . addslashes($classid) . "'", $db); 

$info = mysql_fetch_array($infoq);
if ($info)
{
  $name = $clname = $section = $year = $semester = $students = $pname = $professorid = $code = $name = $information = $dcode = $dname = $scode = $sname = $dvname = $scname = $courseid = $departmentid = $subjectid = $schoolid = $dvcode = $divisionid = "";
  extract($info);
  
  print("<h3>$name" . ($clname ? " - $clname" : "") . "</h3>\n");
  print("<p>" . ucwords($semester) . " $year Section $section</p>\n");
  
  if ($information) print("<h4>Information</h4>\n<p>$information</p>\n");
  if ($professorid) print ("<h4>Professor</h4>\n<a href=\"profinfo.php?professorid=$professorid\">" . ($pname ? $pname : "Unknown") . "</a></p>\n");
  print("<h4>Other Information</h4>");
  if ($students) print ("<i>Enrollment:</i> $students Students</p>");
  if ($dcode) print ("<p><i>Department:</i> $dcode - $dname ($departmentid)</p>");
  if ($scode) print ("<p><i>Subject:</i> $scode - $sname ($subjectid)</p>");
  if ($scname) print ("<p><i>School:</i> $scname ($schoolid)</p>");
  if ($dvname) print ("<p><i>Division:</i> $dvcode - $dvname ($divisionid)</p>");
  if ($code) print ("<p><i>Course Code:</i> <a href=\"courseinfo.php?courseid=$courseid\">$code</a></p>");
}

wces_Findquestionsets($db,"qs",$classid);
$result = db_exec("SELECT COUNT(*) FROM qs", $db, __FILE__, __LINE__);
if (mysql_result($result,0) > 0)
  print("<p align=center><strong> &gt; <a href=\"previewsurvey.php?classid=$classid\">Preview Survey Questions</a> &lt; </strong></p>");

  page_bottom();
%>

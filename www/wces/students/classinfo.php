<%
  require_once("page.inc");
  require_once("login.inc");
  require_once("wces.inc");
  page_top("Professor Profile","1000");
  
$db = wces_connect();

$infoq = mysql_query(

"SELECT cl.section, cl.year, cl.semester, cl.students, p.name as pname, p.professorid, c.code, c.name, c.description, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname
FROM Classes as cl
LEFT JOIN Courses as c USING (courseid)
LEFT JOIN Departments as d USING (departmentid)
LEFT JOIN Subjects as s ON (c.subjectid = s.subjectid)
LEFT JOIN Divisions as dv ON (c.divisionid = dv.divisionid)
LEFT JOIN Schools as sc ON (c.schoolid = sc.schoolid)
LEFT JOIN Professors as p ON (cl.professorid = p.professorid)
WHERE cl.classid = '" . addslashes($classid) . "'", $db); 

$info = mysql_fetch_array($infoq);
if ($info)
{
  extract($info);
  
  if ($professorid) print ("<h4>Professor</h4>\n<a href=\"profinfo.php?professorid=$professorid\">" . ($pname ? $pname : "Unknown") . "</a></p>\n");
}
%>
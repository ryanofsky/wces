<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  page_top("Professor Profile","1000");
  
$db = wces_connect();

$infoq = db_exec(

"SELECT c.code, c.name, c.information, d.departmentid, d.code as dcode, d.name as dname, s.subjectid, s.code as scode, s.name as sname, sc.schoolid, sc.name as scname
FROM courses as c
LEFT JOIN departments as d USING (departmentid)
LEFT JOIN subjects as s ON (c.subjectid = s.subjectid)
LEFT JOIN schools as sc ON (c.schoolid = sc.schoolid)
WHERE courseid = '" . addslashes($courseid) . "'", $db, __FILE__, __LINE__); 

$info = mysql_fetch_assoc($infoq);
if ($info)
{
  extract($info);
  print("<h3>$name</h3>\n");
  if ($information) print("<h4>Information</h4>\n<p>$information</p>\n");
  print("<h4>Identifiers</h4>");
  if ($code) print ("<p><i>Course Code:</i> $code</p>");
  if ($dcode) print ("<p><i>Department:</i> $dcode - $dname ($departmentid)</p>");
  if ($scode) print ("<p><i>Subject:</i> $scode - $sname($subjectid)</p>");
  if ($scname) print ("<p><i>School:</i> $scname ($schoolid)</p>");
}

wces_Findquestionsets($db,"qs");

$infoq = db_exec(

"SELECT cl.classid, cl.name, cl.section, cl.year, cl.semester, p.professorid, p.name AS pname, COUNT(qs.classid) AS cnt
FROM classes AS cl
LEFT JOIN professors AS p USING (professorid)
LEFT JOIN qs ON (qs.classid = cl.classid)
WHERE cl.courseid = '" . addslashes($courseid) . "'
GROUP BY cl.classid
ORDER BY cl.year DESC, cl.semester DESC, cl.section", $db, __FILE__, __LINE__);

print("<h4>Sections</h4>\n<ul>");
while($info = mysql_fetch_assoc($infoq))
{
  $cnt = $classid = $name = $section = $year = $professorid = $pname = "";
  extract($info);
  print("  <li><a href=\"classinfo.php?classid=$classid\">" . ucfirst($semester) . " $year - Section $section" . ($name ? " - $name" : "") . "</a>" . ($professorid ? " - <a href=\"profinfo.php?professorid=$professorid\">$pname</a>" : ""));
  if($cnt > 0) print(" - <a href=\"previewsurvey.php?classid=$classid\">Preview Survey</a>");
  print("</li>\n");
}
print("</ul>\n");
  page_bottom();
%>









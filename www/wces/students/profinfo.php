<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  page_top("Professor Profile","1000");
  
$db = wces_connect();
if ($professorid && $info = db_getrow($db,"Professors",Array("professorid" => $professorid),0)){
  extract($info);  $cunix = db_getvalue($db,"Users",Array("userid" => $userid),"cunix");  
  
  print("<h3>$name</h3>\n");    if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>");  if ($statement) print("<h4>Statement</h4>\n<p>$statement</p>\n");  if ($profile) print("<h4>Profile</h4>\n<p>$profile</p>\n");  if ($education) print("<h4>Education</h4>\n<p>$education</p>\n");  if ($email || $url ||$cunix) print("<h4>Contact Information</h4>\n");  if ($email) print ("<p><a href=\"mailto:$email\">$email</a></p>\n");  if ($url) print ("<p><a href=\"$url\">$url</a></p>\n");
  if ($cunix) print ("<p>CUNIX ID: <a href=\"http://www.columbia.edu/cgi-bin/lookup.pl?$cunix\">$cunix</a></p>\n");
  
  print ("<h4>Classes</h4>\n<UL>\n");  $classes = mysql_query("SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode FROM Classes as cl LEFT JOIN Courses AS c USING (courseid) LEFT JOIN Subjects AS s USING (subjectid) WHERE cl.professorid = '$professorid' ORDER BY cl.year DESC, cl.semester DESC LIMIT 50",$db);
  while ($class = mysql_fetch_assoc($classes))
  {
    $classid = $section = $year = $scode = $code = $name = $section = "";
    extract($class);
    print ("  <LI><A HREF=\"classinfo.php?classid=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</A></LI>\n");
  }
  print("</UL>\n");
}  page_bottom();
%>
<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  page_top("Professor Profile","1000");
  
$db = wces_connect();
if ($professorid && $info = db_getrow($db,"professors",Array("professorid" => $professorid),0))
{
  extract($info);
  $cunix = db_getvalue($db,"users",Array("userid" => $userid),"cunix");
  
  print("<h3>$name</h3>\n");
  
  if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>");
  if ($statement) print("<h4>Statement</h4>\n<p>$statement</p>\n");
  if ($profile) print("<h4>Profile</h4>\n<p>$profile</p>\n");
  if ($education) print("<h4>Education</h4>\n<p>$education</p>\n");
  if ($email || $url ||$cunix) print("<h4>Contact Information</h4>\n");
  if ($email) print ("<p><a href=\"mailto:$email\">$email</a></p>\n");
  if ($url) print ("<p><a href=\"$url\">$url</a></p>\n");
  if ($cunix) print ("<p>CUNIX ID: <a href=\"ldap.php?uni=$cunix\">$cunix</a></p>\n");
  
  print ("<h4>Classes</h4>\n<UL>\n");

  wces_Findquestionsets($db,"qs");

  $classes = db_exec("

  SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode, COUNT(qs.classid) AS cnt
  FROM classes as cl
  LEFT JOIN courses AS c USING (courseid)
  LEFT JOIN subjects AS s USING (subjectid)
  LEFT JOIN qs ON (qs.classid = cl.classid)
  WHERE cl.professorid = '$professorid'
  GROUP BY cl.classid
  ORDER BY cl.year DESC, cl.semester DESC LIMIT 50",$db,__FILE__,__LINE__);
  
  while ($class = mysql_fetch_assoc($classes))
  {
    $cnt = $classid = $section = $year = $scode = $code = $name = $section = "";
    extract($class);
    print ("  <LI><A HREF=\"classinfo.php?classid=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</A>");
    if($cnt > 0) print(" - <a href=\"previewsurvey.php?classid=$classid\">Preview Survey</a>");
    print("</LI>\n");
  }
  print("</UL>\n");
}

  page_bottom();
%>







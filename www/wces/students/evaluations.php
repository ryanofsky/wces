<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");  require_once("wces/wces.inc");  login_protect(login_student);  page_top("Evaluation Listing","0100");
  
      $db = wces_connect();
  
  wces_FindClasses($db,"currentclasses");
  
  $classes = mysql_query(
  "SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode 
  FROM currentclasses AS cc
  LEFT JOIN Classes AS cl ON cc.classid = cl.classid  LEFT JOIN Courses AS c ON cl.courseid = c.courseid  LEFT JOIN Subjects AS s ON c.subjectid = s.subjectid  ORDER BY s.code, c.code, cl.section",$db);
  print ("<p>List of classes that have evalutions available during the current evaluation period:</p>");  print ("<UL>\n");  $found = false;  while ($class = mysql_fetch_array($classes))
  {
    $found = true;    extract($class);
      print ("  <li>$scode$code <i>$name</i> (section $section)</LI>\n");  }
  print("</ul>");
  
  mysql_query("DROP TABLE currentclasses",$db);
%>
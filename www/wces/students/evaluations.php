<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  
    
  
  wces_FindClasses($db,"currentclasses");
  
  $classes = mysql_query(
  "SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode 
  FROM currentclasses AS cc
  LEFT JOIN Classes AS cl ON cc.classid = cl.classid

  {
    $found = true;
      print ("  <li>$scode$code <i>$name</i> (section $section)</LI>\n");
  print("</ul>");
  
  mysql_query("DROP TABLE currentclasses",$db);
%>
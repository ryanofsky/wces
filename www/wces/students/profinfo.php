<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  page_top("Professor Profile","1000");
  
$db = wces_connect();
if ($professorid && $info = db_getrow($db,"Professors",Array("professorid" => $professorid),0))
  extract($info);
  
  print("<h3>$name</h3>\n");
  if ($cunix) print ("<p>CUNIX ID: <a href=\"http://www.columbia.edu/cgi-bin/lookup.pl?$cunix\">$cunix</a></p>\n");
  
  print ("<h4>Classes</h4>\n<UL>\n");
  while ($class = mysql_fetch_array($classes))
  {
    extract($class);
    print ("  <LI>" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>\n");
  }
  print("</UL>\n");
}
%>
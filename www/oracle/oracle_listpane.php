<head>
<style>
<!--
a { text-decoration: none; }
a:hover { text-decoration: underline; ) 
-->
</style>
</head>
<body bgcolor="#6699CC" link="#000000" vlink="#9C6531">
<base target=info>
<script language="JavaScript">
<!--

var happy = document.images && window.Image;

// fix for problematic Netscape 3 string object
if (happy && !String.prototype.slice) String.prototype.slice = new Function("start","len","if(len<0 || len>this.length) len = this.length; var ret = new String(); for(var i = start; i < len; i++) ret += this.charAt(i); return ret");

function swap(img)
{
  if(happy)
  {
    var i = document.images[img];
    var temp = i.src;
    i.src = i.flipimage.src;
    i.flipimage.src = temp;
  };
}

function AttachImage(img, filename)
{
  if(happy)
  {
    var i = document.images[img];
    var p = i.src.lastIndexOf("/");
    var q = i.src.lastIndexOf("\\");
    i.flipimage = new Image();  
    i.flipimage.src = i.src.slice(0,(p>q && p>0) ? p+1 : (q>0)?q+1 : 0) + filename;;
  };
}

// -->
</script>

<style>
<!--
body                    { font-family: Arial, Helvetica, sans-serif; }
p                       { font-family: Arial, Helvetica, sans-serif; }
li                      { font-family: Arial, Helvetica, sans-serif; }
ul                      { font-family: Arial, Helvetica, sans-serif; list-style-type: square; list-style-position: outside; }
-->
</style>

<%

require_once("wces/wces.inc");
require_once("wces/reporting.inc");

$db = wces_connect();

if ($mode == "professors")
{

  %>
  <p align=center><a href="oracle_listpane.php?mode=courses" target="_self" onmouseover="swap('courses')" onmouseout="swap('courses')"><img src="<%=$oracleroot%>media/courses.gif" name=courses border=0></a>
  <img src="<%=$oracleroot%>media/professors_down.gif"></p>
  <script>AttachImage('courses','courses_on.gif')</script>
  <% 

  $y = mysql_query("

  SELECT p.professorid, p.name
  FROM answersets AS a
  INNER JOIN classes as cl USING (classid)
  INNER JOIN professors AS p USING (professorid)
  WHERE a.questionperiodid < 6
  GROUP BY p.professorid

  ",$db);

  $plist = array();
  while($row = mysql_fetch_array($y))
    if ($row["name"]) 
    {
      $pos = strrpos($row["name"]," ");
      array_push($plist,array("first" => substr($row["name"],0,$pos), "last" => substr($row["name"],$pos), "professorid" => $row["professorid"]));
    }

  usort($plist,"pcmp");

  print("<font size=-1>\n");
  print('<ul>');

  foreach($plist as $p)
  {
    $professorid=0; $last=""; $first="";
    extract($p);
    print("  <li><a href=\"oracle_infopane.php?professorid=$professorid\">$last, $first</a></li>\n");
  }
print('</ul>');  
print('</font>');
  
}
else // $mode == "courses"
{

  %>
  <p align=center><img src="<%=$oracleroot%>media/courses_down.gif">
  <a href="oracle_listpane.php?mode=professors" target="_self" onmouseover="swap('profs')" onmouseout="swap('profs')"><img src="<%=$oracleroot%>media/professors.gif" name=profs border=0></a></p>
  <script>AttachImage('profs','professors_on.gif')</script>
  <%  

  $y = mysql_query("
  
  SELECT c.courseid, c.name, c.code, s.code AS scode, d.name AS dname, d.departmentid FROM answersets AS a
  INNER JOIN classes as cl USING (classid)
  INNER JOIN courses AS c USING (courseid)
  INNER JOIN departments AS d USING (departmentid)
  LEFT JOIN subjects AS s ON (c.subjectid = s.subjectid)
  WHERE (a.responses > 0) AND (a.questionsetid = 1) AND (a.questionperiodid < 6)
  GROUP BY c.courseid ORDER BY d.name, s.code, c.code",$db);
  
  
  $departmentidprev = -1;
  
  $first = true;
  while($row = mysql_fetch_array($y))
  {
    $departmentid = 0; $courseid = 0; $scode = ""; $code = ""; $name = ""; $dname = "";
    extract($row);
    if ($departmentidprev != $departmentid)
    {
      if ($first) $first = false; else print("</ul></font>");
      print("<h5>$dname</h5>\n<font size=-1><ul>\n");
    };  
    $departmentidprev = $departmentid;  
    print("<li><a href=\"oracle_infopane.php?courseid=$courseid\">($scode$code) $name </a></li>\n");
  }
  print("</ul></font>");
}

%>

</body>








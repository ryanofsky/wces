<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);
page_top("Usage Data");

$db = wces_connect();
wces_Findclasses($db,"currentclasses");

$y = mysql_query("CREATE TEMPORARY TABLE customprofessors (professorid INTEGER NOT NULL, dummy INTEGER, PRIMARY KEY (professorid))",$db);
$y = mysql_query("REPLACE INTO customprofessors (professorid, dummy) SELECT p.professorid, (1) FROM groupings as g INNER JOIN professors as p ON g.linkid = p.professorid 
  INNER JOIN questionsets AS q ON g.questionsetid = q.questionsetid WHERE g.linktype = 'professors' AND q.type = 'private'",$db);
$y = mysql_query("REPLACE INTO customprofessors (professorid, dummy) SELECT cl.professorid, (1) FROM groupings as g INNER JOIN classes as cl ON g.linkid = cl.classid
  INNER JOIN questionsets as q ON g.questionsetid = q.questionsetid WHERE g.linktype = 'classes' AND q.type = 'private'",$db);
$y = mysql_query("CREATE TEMPORARY TABLE currentprofessors (professorid INTEGER NOT NULL, hasloggedin INTEGER, hascustom INTEGER, students INTEGER, PRIMARY KEY (professorid))",$db);
$y = mysql_query("REPLACE INTO currentprofessors (professorid, hasloggedin, hascustom, students) SELECT p.professorid, IF(p.userid IS NULL,NULL,1), cmp.dummy, SUM(cl.students) as students FROM currentclasses
  INNER JOIN classes AS cl ON currentclasses.classid = cl.classid LEFT JOIN professors AS p ON cl.professorid = p.professorid
  LEFT JOIN customprofessors AS cmp ON p.professorid = cmp.professorid
  GROUP BY p.professorid",$db);
$y = mysql_query("DROP TABLE customprofessors",$db);

$notloggedin = mysql_query("SELECT p.name, cp.professorid, cp.students FROM currentprofessors as cp INNER JOIN professors as p USING (professorid) WHERE cp.hasloggedin IS NULL ORDER BY cp.students DESC",$db);
$nocustoms   = mysql_query("SELECT p.name, cp.professorid, cp.students FROM currentprofessors as cp INNER JOIN professors as p USING (professorid) WHERE cp.hasloggedin IS NOT NULL AND cp.hascustom IS NULL ORDER BY cp.students DESC",$db);
$hascustoms  = mysql_query("SELECT p.name, cp.professorid, cp.students FROM currentprofessors as cp INNER JOIN professors as p USING (professorid) WHERE cp.hasloggedin IS NOT NULL AND cp.hascustom IS NOT NULL ORDER BY cp.students DESC",$db);

$notloggedincount = mysql_num_rows($notloggedin);
$hascustomscount = mysql_num_rows($hascustoms);
$nocustomscount = mysql_num_rows($nocustoms);

$profcount = mysql_result(mysql_query("SELECT COUNT(*) FROM currentprofessors",$db),0);
$profcustomized = mysql_num_rows($hascustoms);
$profloggedin = $profcustomized + mysql_num_rows($nocustoms);

function printproflist($title,$result,$count)
{
  global $server_wcespath;
  print("<p>$title<br><font size=-1>$count professors sorted by number of students</font></p>");
  print("<ul>");
  while($row = mysql_fetch_array($result))
  {
    extract($row);
    if (!$name) $name = "** Unknown **";
    print("<li><a href=\"${server_wcespath}students/profinfo.php?professorid=$professorid\">$name</a> ($students students)</li>");
  }
print("</ul>");
}
%>

<h3>Aggregate Professor Usage</h3>

Number of professors: <b><%=$notloggedincount+$hascustomscount+$nocustomscount%></b><br>
Number of professors who have logged In: <b><%=$hascustomscount+$nocustomscount%></b><br>
Number of professors with custom surveys: <b><%=$hascustomscount%></b><br>
<img src="<%=$server_wcespath%>media/graphs/pusagegraph.php?neverloggedin=<%=$notloggedincount%>&custom=<%=$hascustomscount%>&nocustom=<%=$nocustomscount%>" width=200 height=200><img src="<%=$server_wcespath%>media/graphs/pusagelegend.gif" width=133 height=49><br>
<p>&nbsp;</p>
<h3>Individual Professor Usage</h3>

<%

printproflist("professors who have not logged in",$notloggedin,$notloggedincount);
printproflist("professors who have logged in without creating custom questions",$nocustoms,$nocustomscount);
printproflist("professors who have logged in and created custom questions",$hascustoms,$hascustomscount);

mysql_query("DROP TABLE currentprofessors");
mysql_query("DROP TABLE currentclasses");

page_bottom();
%>






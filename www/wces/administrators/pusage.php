<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);
page_top("Usage Data");

$db_debug = true;

$db = wces_connect();
wces_Findclasses($db,"currentclasses");

$y = db_exec("
CREATE TEMPORARY TABLE customprofessors
(
  professorid INTEGER NOT NULL,
  PRIMARY KEY (professorid))
",$db,__FILE__,__LINE__);

$y = db_exec("

REPLACE INTO customprofessors (professorid)
SELECT p.professorid
FROM groupings as g
INNER JOIN professors as p ON g.linkid = p.professorid 
INNER JOIN questionsets AS q ON g.questionsetid = q.questionsetid
WHERE g.linktype = 'professors' AND q.type = 'private'
",$db,__FILE__,__LINE__);

$y = db_exec("
REPLACE INTO customprofessors (professorid)
SELECT cl.professorid
FROM groupings as g INNER JOIN classes as cl ON g.linkid = cl.classid
INNER JOIN questionsets as q ON g.questionsetid = q.questionsetid
WHERE g.linktype = 'classes' AND q.type = 'private'
",$db,__FILE__,__LINE__);

$y = db_exec("
CREATE TEMPORARY TABLE currentprofessors
(
  professorid INTEGER NOT NULL,
  hasloggedin INTEGER,
  hascustom INTEGER,
  students INTEGER,
  PRIMARY KEY (professorid))
",$db,__FILE__,__LINE__);

$y = db_exec("
REPLACE INTO currentprofessors (professorid, hasloggedin, hascustom, students)
SELECT p.professorid, IF(u.lastlogin > '2001-02-01',1,0), IF(cmp.professorid IS NULL,0,1), SUM(cl.students) as students
FROM currentclasses
INNER JOIN classes AS cl ON currentclasses.classid = cl.classid
INNER JOIN professors AS p ON cl.professorid = p.professorid
LEFT JOIN users AS u ON p.userid = u.userid
LEFT JOIN customprofessors AS cmp ON p.professorid = cmp.professorid
GROUP BY p.professorid",$db,__FILE__,__LINE__);

$y = db_exec("DROP TABLE customprofessors",$db,__FILE__,__LINE__);

$notloggedin = db_exec("SELECT p.name, cp.professorid, cp.students FROM currentprofessors as cp INNER JOIN professors as p USING (professorid) WHERE NOT cp.hasloggedin ORDER BY cp.students DESC",$db,__FILE__,__LINE__);
$nocustoms   = db_exec("SELECT p.name, cp.professorid, cp.students FROM currentprofessors as cp INNER JOIN professors as p USING (professorid) WHERE cp.hasloggedin AND NOT cp.hascustom ORDER BY cp.students DESC",$db,__FILE__,__LINE__);
$hascustoms  = db_exec("SELECT p.name, cp.professorid, cp.students FROM currentprofessors as cp INNER JOIN professors as p USING (professorid) WHERE cp.hasloggedin AND cp.hascustom ORDER BY cp.students DESC",$db,__FILE__,__LINE__);

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
Number of professors who have logged in: <b><%=$hascustomscount+$nocustomscount%></b><br>
Number of professors with custom surveys: <b><%=$hascustomscount%></b><br>
<img src="<%=$server_wcespath%>media/graphs/pusagegraph.php?neverloggedin=<%=$notloggedincount%>&custom=<%=$hascustomscount%>&nocustom=<%=$nocustomscount%>" width=200 height=200><img src="<%=$server_wcespath%>media/graphs/pusagelegend.gif" width=133 height=49><br>
<p>&nbsp;</p>
<h3>Individual Professor Usage</h3>

<%

printproflist("Professors who have not logged in during the past semester",$notloggedin,$notloggedincount);
printproflist("Professors who have logged in without creating custom questions",$nocustoms,$nocustomscount);
printproflist("Professors who have logged in and created custom questions",$hascustoms,$hascustomscount);

mysql_query("DROP TABLE currentprofessors");
mysql_query("DROP TABLE currentclasses");

page_bottom();
%>






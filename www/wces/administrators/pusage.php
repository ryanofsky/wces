<?
require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);
$db = wces_connect();
wces_GetCurrentQuestionPeriod($db, $questionperiodid, $questionperiod, $year, $semester);
$semester = ucfirst($semester);

page_top("Professor Usage Data for $semester $year $questionperiod");

$y = db_exec("
CREATE TEMPORARY TABLE pu AS
SELECT p.professorid, cl.classid, CONCAT(s.code, c.code) AS code, cl.section, cl.students, c.name AS cname, p.name AS pname, u.cunix, u.lastlogin, IF(u.lastlogin >= '2001-11-16', 1, 0) AS loggedin, IF(cc.classid IS NOT NULL, 1, 0) AS customized
FROM groupings AS g
INNER JOIN classes AS cl ON cl.classid = g.linkid AND g.linktype = 'classes'
INNER JOIN courses AS c USING (courseid)
INNER JOIN subjects AS s USING (subjectid)
LEFT JOIN cheesyclasses AS cc ON cc.classid = cl.classid AND cc.questionperiodid = $questionperiodid
LEFT JOIN professors AS p ON p.professorid = cl.professorid
LEFT JOIN users AS u USING (userid)
",$db,__FILE__,__LINE__);

$profcount = mysql_result(db_exec("SELECT COUNT(*) FROM pu",$db,__FILE__,__LINE__),0);
$profcustomized = mysql_result(db_exec("SELECT COUNT(*) FROM pu WHERE customized <> 0",$db,__FILE__,__LINE__),0);
$profloggedin = mysql_result(db_exec("SELECT COUNT(*) FROM pu WHERE loggedin <> 0",$db,__FILE__,__LINE__),0);

?>

<h3>Aggregate Usage</h3>

Number of classes: <b><?=(int)$profcount?></b><br>
Number of classes with professors who logged in during the customization period: <b><?=(int)$profloggedin?></b><br>
Number of classes with professors who created custom surveys: <b><?=(int)$profcustomized?></b><br>
<img src="<?=$wces_path?>media/graphs/pusagegraph.php?neverloggedin=<?=(int)$profcount-(int)$profloggedin?>&custom=<?=(int)$profcustomized?>&nocustom=<?=(int)$profloggedin-(int)$profcustomized?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/pusagelegend.gif" width=133 height=49><br>
<p>&nbsp;</p>

<h3>Individual Class Usage</h3>

<?
$result = db_exec("SELECT professorid, classid, code, section, students, cname, pname, cunix, loggedin, DATE_FORMAT(lastlogin,'%Y-%m-%d') AS lastlogin, customized FROM pu ORDER BY customized DESC, loggedin DESC, code, section", $db, __FILE__, __LINE__);
print("<table cellspacing=1 cellpadding=1 border=1>\n");
print("<tr><td><b>Code</b></td><td><b>Name</b></td><td><b>Size</b></td><td><b>Professor</b></td><td><b>Last Login</b></td></tr>\n");
print("<tr><td colspan=5><b><font color=\"#0030E7\">Classes with custom surveys:</font></b></td></tr>\n");
$lastcnt = 0;
$lastcustom = 1;
$stage = 0;
while($row = mysql_fetch_assoc($result))
{
  $professorid = $classid = $code = $section = $students = $cname = $pname = $cunix = $lastlogin = $customized = "";
  extract($row);
  
  if (!$lastlogin) $lastlogin = "<i>never</i>";
  
  if ($stage == 0 && $lastcustom && !$customized)
  {
    if ($lastcnt == 0)
      print("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n");

    print("<tr><td colspan=5><b><font color=\"#0030E7\">Classes with professors who logged in during or after the customization period:</font></b></td></tr>\n");
    $lastloggedin = 1;
    $stage = 1;
    $lastcnt = 0;
  }
  
  if ($stage == 1 && $lastloggedin && !$loggedin)
  {
    if ($lastcnt == 0)
      print("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n");

    print("<tr><td colspan=5><b><font color=\"#0030E7\">Classes with professors who have not recently logged in:</font></b></td></tr>\n");
    $lastcnt = 0;
    $stage = 2;
  }

  if ($cunix) $pname .= " ($cunix)";

  print("<tr><td nowrap>$code $section</td><td>$cname</td><td>$students</td><td>$pname</td><td nowrap>$lastlogin</td></tr>");
  ++$lastcnt;
  $lastcustom = $customized;
  $lastloggedin = $loggedin;
}
print("</table>\n");


mysql_query("DROP TABLE pu");

page_bottom();
?>

<?
require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator);

wces_connect();

$result = pg_go("
  SELECT question_period_id, displayname, year, semester
  FROM semester_question_periods
  WHERE question_period_id = 23 -- (SELECT get_question_period())
", $wces, __FILE__, __LINE__);
extract(pg_fetch_array($result,0,PGSQL_ASSOC));

page_top("Professor Usage Data for $displayname]");

pg_go("
  CREATE TEMPORARY TABLE profcounts AS
  SELECT t.class_id, t.topic_id, CASE WHEN COUNT(DISTINCT b.branch_id) >0 THEN 1 ELSE 0 END AS customized
  FROM wces_topics AS t
  INNER JOIN classes AS cl USING (class_id)
  LEFT JOIN branches AS b ON (b.topic_id = t.topic_id)
  WHERE t.category_id IS NOT NULL AND cl.year = $year AND cl.semester = $semester
  GROUP BY t.class_id, t.topic_id
",$wces,__FILE__, __LINE__);

$y = pg_go("
  CREATE TEMPORARY TABLE pu AS
  SELECT pc.class_id, pc.topic_id, s.code || c.divisioncode || c.code AS code, c.name AS cname, cl.section,
    cl.students, u.user_id, u.uni, u.firstname, u.lastname, u.lastlogin,
    CASE WHEN u.lastlogin >= '2002-11-15' THEN 1 ELSE 0 END AS loggedin,
    pc.customized
  FROM profcounts AS pc
  INNER JOIN classes AS cl USING (class_id)
  INNER JOIN courses AS c USING (course_id)
  INNER JOIN subjects AS s USING (subject_id)
  INNER JOIN enrollments AS e ON (e.class_id = cl.class_id AND e.status = 3)
  INNER JOIN users AS u USING (user_id)
",$wces,__FILE__,__LINE__);

$profcount = pg_result(pg_go("SELECT COUNT(*) FROM pu",$wces,__FILE__,__LINE__),0,0);
$profcustomized = pg_result(pg_go("SELECT COUNT(*) FROM pu WHERE customized <> 0",$wces,__FILE__,__LINE__),0,0);
$profloggedin = pg_result(pg_go("SELECT COUNT(*) FROM pu WHERE loggedin <> 0",$wces,__FILE__,__LINE__),0,0);

?>

<h3>Aggregate Usage</h3>

Number of classes: <b><?=(int)$profcount?></b><br>
Number of classes with professors who logged in during the customization period: <b><?=(int)$profloggedin?></b><br>
Number of classes with professors who created custom surveys: <b><?=(int)$profcustomized?></b><br>
<img src="<?=$wces_path?>media/graphs/pusagegraph.php?neverloggedin=<?=(int)$profcount-(int)$profloggedin?>&custom=<?=(int)$profcustomized?>&nocustom=<?=(int)$profloggedin-(int)$profcustomized?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/pusagelegend.gif" width=133 height=49><br>
<p>&nbsp;</p>

<h3>Individual Class Usage</h3>

<?

$result = pg_go("
  SELECT user_id, class_id, topic_id, code, section, students, firstname, lastname, uni, loggedin,
   cname, to_char(lastlogin,'YYYY-MM-DD') AS lastlogin, customized
  FROM pu
  ORDER BY customized DESC, loggedin DESC, code, section
", $wces, __FILE__, __LINE__);

$n = pg_numrows($result);

pg_go("DROP TABLE profcounts; DROP TABLE pu;", $wces, __FILE__, __LINE__);

print("<table cellspacing=1 cellpadding=1 border=1>\n");
print("<tr><td><b>Code</b></td><td><b>Name</b></td><td><b>Size</b></td><td><b>Professor</b></td><td><b>Last Login</b></td></tr>\n");
print("<tr><td colspan=5><b><font color=\"#0030E7\">Classes with custom surveys:</font></b></td></tr>\n");
$lastcnt = 0;
$lastcustom = 1;
$stage = 0;
for($i=0; $i<$n; ++$i)
{
  extract(pg_fetch_array($result,$i,PGSQL_ASSOC));

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

  $pname = "$firstname $lastname";
  if ($uni) $pname .= " ($uni)";

  print("<tr><td nowrap><a href=\"{$wces_path}administrators/info.php?class_id=$class_id\">$code $section</a></td><td><a href=\"{$wces_path}administrators/surveys.php?topic_id=$topic_id\">$cname</a></td><td>$students</td><td><a href=\"{$wces_path}administrators/info.php?user_id=$user_id\">$pname</a></td><td nowrap>$lastlogin</td></tr>");
  ++$lastcnt;
  $lastcustom = $customized;
  $lastloggedin = $loggedin;
}
print("</table>\n");

page_bottom();
?>

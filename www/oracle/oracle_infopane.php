<?

require_once("wces/oracle.inc");
require_once("wces/wces.inc");
require_once("wces/report_help.inc");
require_once("wbes/general.inc");

?>

<style type="text/css">
<!--
body                    { font-family: Arial, Helvetica, sans-serif; }
p                       { font-family: Arial, Helvetica, sans-serif; }
h3                      { font-family: Arial, Helvetica, sans-serif; }
h4                      { font-family: Arial, Helvetica, sans-serif; }
-->
</style>

<body bgcolor="#6699CC"><table vspace=20 hspace=20 width=100% height=100% bordercolor=black border=1 cellpadding=5 cellspacing=0><tr>
<?

param($professorid);
param($courseid);
param($classid);

$db = wces_oldconnect();

if ($professorid && $info = db_getrow($db,"professors",Array("professorid" => $professorid),0))
{
  print("<td bgcolor=\"#B5CFE8\" valign=top>");
  $cunix = $userid = $name = $picname = "";
  extract($info);
  $cunix = db_getvalue($db,"users",Array("userid" => $userid),"cunix");

  print("<h3>$name</h3>");
  if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>");
?>
<h4>Courses Taught</h4>
<form method=get name=classes>
<p><select name=classid onchange="this.form.submit()" size=1>
<?
  $classes = mysql_query("

  SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode
  FROM classes as cl
  INNER JOIN answersets AS a USING (classid)
  INNER JOIN courses AS c ON (cl.courseid = c.courseid)
  INNER JOIN subjects AS s USING (subjectid)
  WHERE cl.professorid = '$professorid' AND a.questionperiodid IN (1,2,4,5,7) AND a.topicid IN (1,2,4)
  GROUP BY cl.classid 
  ORDER BY cl.year DESC, cl.semester DESC LIMIT 50",$db);
  while ($class = mysql_fetch_array($classes))
  {
    $classid = $section = $year = $semester = $code = $name = $scode = "";
    extract($class);
    print ("  <option value=$classid>" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</option>\n");
  }
?>  
</select></p>
<p><input type=image name=go src="<?=$oracle_root?>media/go.gif"></p>
</form>
<?
  if ($statement) print("<h4>Statement</h4>\n<p>$statement</p>\n");
  if ($profile) print("<h4>Profile</h4>\n<p>$profile</p>\n");
  if ($education) print("<h4>Education</h4>\n<p>$education</p>\n");
  if ($email || $url ||$cunix) print("<h4>Contact Information</h4>\n");
  if ($email) print ("<p><a href=\"mailto:$email\">$email</a></p>\n");
  if ($url) print ("<p><a href=\"$url\">$url</a></p>\n");
  if ($cunix) print ("<p>CUNIX ID: <a href=\"http://www.columbia.edu/cgi-bin/lookup.pl?$cunix\">$cunix</a></p>\n");
  print("</td>");
}
else if ($classid || $courseid)
{
  print('<td bgcolor="#B5CFE8" valign=top>');
  
  if (!$courseid)
    $courseid = db_getvalue($db,"classes",Array("classid" => $classid),"courseid");
  else if (!$classid)
    $classid = mysql_result(mysql_query("SELECT classid FROM classes WHERE courseid = '" . addslashes($courseid) . "' ORDER BY year DESC, semester DESC, section LIMIT 1",$db),0);

  $infoq = mysql_query(
  "SELECT cl.section, cl.year, cl.semester, cl.students, cl.name as cname, p.name as pname, p.professorid, c.code, c.name, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname
  FROM classes as cl
  INNER JOIN courses as c USING (courseid)
  INNER JOIN subjects as s ON (c.subjectid = s.subjectid)
  LEFT JOIN departments as d ON (d.departmentid = cl.departmentid)
  LEFT JOIN divisions as dv ON (cl.divisionid = dv.divisionid)
  LEFT JOIN schools as sc ON (c.schoolid = sc.schoolid)
  LEFT JOIN professors as p ON (cl.professorid = p.professorid)
  WHERE cl.classid = '$classid'", $db);

  $info = mysql_fetch_array($infoq);
  $section = $year = $semester = $students = $cname = $pname = $professorid = $code = $name = $information = $dcode = $dname = $scode = $sname = $dvname = $scname = "";
  extract($info);
  if (isset($cname) && $cname) $name .= " - $cname";
  
  print("<h3>$name</h3>\n");

?>
<table width=100% border=0 cellpadding=0 cellspacing=0>
<tr>
  <td width=50% valign=top>
    <form method=get name=classes>
    <h4>Section</h4>
    <p><select name=classid onchange="this.form.submit()" size=1>
<?
  $classes = mysql_query("

  SELECT cl.classid, cl.section, cl.year, cl.semester
  FROM classes as cl
  INNER JOIN answersets AS a USING (classid)
  WHERE cl.courseid = '$courseid' AND a.questionperiodid IN (1,2,4,5,7) AND a.topicid IN (1,2,4)
  GROUP BY cl.classid
  ORDER BY cl.year DESC, cl.semester, cl.section DESC LIMIT 50

  ",$db);

  while ($class = mysql_fetch_array($classes))
    print ("      <option value=" . $class['classid'] . ($classid == $class['classid'] ? " selected" : "") . ">" . ucfirst($class['semester']) . " " . $class['year'] . " - Section " . $class['section'] . "</option>\n");
?>    </select></p>
    <p><input type=image name=go src="<?=$oracle_root?>media/go.gif"></p>
    </form>
  </td>
  <td width=50% valign=top>
    <h4>Professor</h4>
    <p><a href="oracle_infopane.php?professorid=<?=$professorid?>"><?=$pname?></a></p>
  </td>
</tr>
</table>    

<?    
  
  $sql_columns = "c.students, a.responses";
    
  $choices = array("a","b","c","d","e");
  for($i = 1; $i <= 10; ++$i)
  {
    $sql_columns .= ", q.MC$i";
    foreach($choices as $choice)
      $sql_columns .= ", a.MC$i$choice";
  };
  
  $n = db_exec("SELECT $sql_columns FROM answersets AS a INNER JOIN classes as c USING (classid) LEFT JOIN questionsets as q ON (a.questionsetid = q.questionsetid) WHERE a.questionsetid = '1' AND a.classid = '$classid' AND a.questionperiodid IN (1,2,4,5,7) AND a.topicid IN (1,2,4) ORDER BY a.responses DESC LIMIT 1", $db, __FILE__, __LINE__);
  
  if (mysql_num_rows($n) > 0)
  {
    $result = mysql_fetch_array($n);
    print("<h4>Survey Results</h4>");
    print('<table border=0 cellpadding=0 cellspacing=5');
    print("<tr><td colspan=3><p><i>" . $result['responses'] . " of " . $result['students'] . " students responded</i></p></td></tr>\n");
    for($i = 1; $i <= 10; ++$i)
    if ($result["MC$i"])
    {
      $scores = array(5 => $result["MC${i}a"], 4 => $result["MC${i}b"], 3=> $result["MC${i}c"], 2=> $result["MC${i}d"], 1=> $result["MC${i}e"]);
      $avg = report_avg($scores);
      print("<tr><td>" . $result["MC$i"] . "</td><td>" . round($avg,2) . "</td><td nowrap>");
      print(report_meter(round($avg * 20)));
      print("</td></tr>\n");
    }
    print("</table>");
  };
  
  print ("<h4>Course Information</h4>\n");
  if ($information) print("<p>$information</p>\n");
  if ($dcode) print ("<p><i>Department:</i> $dname($dcode)</p>");
  if ($scode) print ("<p><i>Subject:</i> $sname($scode)</p>");
  if ($scname) print ("<p><i>School:</i> $scname</p>");
  if ($dvname) print ("<p><i>Division:</i> $dvname</p>");
  if ($code) print ("<p><i>Course ID:</i> $code</p>");
}
else
{
  ?>
  <td bgcolor="B5CFE8" valign=center>
  <table align=center width=200 height=200 border=1 cellpadding=0 cellspacing=0 bordercolor=black><tr><td bgcolor="#FFFFFF" valign=middle>
  <p align=center><img src="<?=$oracle_root?>media/seas_anim.gif" width=100 height=100></p>
  <p align=center>Choose a course or professor from the list in the left pane.</p>
  </td></tr>
  </table>
  </td>
  <?
}
?>
</tr></table></body>












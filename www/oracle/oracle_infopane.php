<style type="text/css">
<!--

body                    { font-family: Arial, Helvetica, sans-serif; }
p                       { font-family: Arial, Helvetica, sans-serif; }
h3                      { font-family: Arial, Helvetica, sans-serif; }
h4                      { font-family: Arial, Helvetica, sans-serif; }
-->
</style>

<body bgcolor="#6699CC"><table vspace=20 hspace=20 width=100% height=100% bordercolor=black border=1 cellpadding=5 cellspacing=0><tr>
<%
require_once("wces/wces.inc");
require_once("wces/reporting.inc");

$professorid = $professorid + 0;
$courseid = $courseid + 0;
$classid = $classid + 0;

$db = wces_connect();

if ($professorid && $info = db_getrow($db,"Professors",Array("professorid" => $professorid),0))
  print("<td bgcolor=\"#B5CFE8\" valign=top>");
  extract($info);
  $cunix = db_getvalue($db,"Users",Array("userid" => $userid),"cunix");
  if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>");
%>
<form method=get name=classes>
<p><select name=classid onchange="this.form.submit()" size=1>
<%
  $classes = mysql_query("SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode FROM Classes as cl INNER JOIN AnswerSets USING (classid) LEFT JOIN Courses AS c ON (cl.courseid = c.courseid) LEFT JOIN Subjects AS s USING (subjectid) WHERE cl.professorid = '$professorid' ORDER BY cl.year DESC, cl.semester DESC LIMIT 50",$db);
  while ($class = mysql_fetch_array($classes))
  {
    extract($class);
    print ("  <option value=$classid>" . ucfirst($semester) . "$year  - $scode$code <i>$name</i> (section $section)</option>\n");
  }
%>  
</select></p>
<p><input type=image name=go src="<%=$oracleroot%>media/go.gif"></p>
</form>
<%
  if ($cunix) print ("<p>CUNIX ID: <a href=\"http://www.columbia.edu/cgi-bin/lookup.pl?$cunix\">$cunix</a></p>\n");
  print("</td>");
}
else if ($classid || $courseid)
{
  print('<td bgcolor="#B5CFE8" valign=top>');
  
  if (!$courseid)
    $courseid = db_getvalue($db,"Classes",Array("classid" => $classid),"courseid");
  else if (!$classid)
    $classid = mysql_result(mysql_query("SELECT classid FROM Classes WHERE courseid = '" . addslashes($courseid) . "'",$db),0);

  $infoq = mysql_query(
  "SELECT cl.section, cl.year, cl.semester, cl.students, cl.name as cname, p.name as pname, p.professorid, c.code, c.name, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname
  FROM Classes as cl
  LEFT JOIN Courses as c USING (courseid)
  LEFT JOIN Departments as d USING (departmentid)
  LEFT JOIN Subjects as s ON (c.subjectid = s.subjectid)
  LEFT JOIN Divisions as dv ON (cl.divisionid = dv.divisionid)
  LEFT JOIN Schools as sc ON (c.schoolid = sc.schoolid)
  LEFT JOIN Professors as p ON (cl.professorid = p.professorid)
  WHERE cl.classid = '$classid'", $db);

  if ($cname) $name .= " - $cname";

  $info = mysql_fetch_array($infoq);
  extract($info);

%>
    <h4>Section</h4>
<%
  $classes = mysql_query("SELECT cl.classid, cl.section, cl.year, cl.semester FROM Classes as cl INNER JOIN AnswerSets USING (classid) WHERE cl.courseid = '$courseid' GROUP BY cl.classid ORDER BY cl.year DESC, cl.semester, cl.section DESC LIMIT 50",$db);
  while ($class = mysql_fetch_array($classes))
    print ("      <option value=" . $class['classid'] . ($classid == $class['classid'] ? " selected" : "") . ">" . ucfirst($class['semester']) . " " . $class['year'] . " - Section " . $class['section'] . "</option>\n");
%>    </select></p>
    <p><input type=image name=go src="<%=$oracleroot%>media/go.gif"></p>
    </form>
  </td>
  <td width=50% valign=top>
    <h4>Professor</h4>
  </td>
</tr>
</table>    

  
  $sql_columns = "c.students, a.responses";
    
  for($i = 1; $i <= 10; ++$i)
  {
    $sql_columns .= ", q.MC$i";
    foreach($choices as $choice)
      $sql_columns .= ", a.MC$i$choice";
  };
  $n = mysql_query("SELECT $sql_columns FROM AnswerSets AS a INNER JOIN Classes as c USING (classid) LEFT JOIN QuestionSets as q ON (a.questionsetid = q.questionsetid) WHERE a.questionsetid = '1' AND a.classid = '$classid' ORDER BY a.responses DESC LIMIT 1", $db);
  
  if (mysql_num_rows($n) > 0)
  {
    print("<h4>Survey Results</h4>");
    print("<tr><td colspan=3><p><i>" . $result['responses'] . " of " . $result['students'] . " students responded</i></p></td></tr>\n");
    if ($result["MC$i"])
      $avg = report_avg($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"]);
    }
    print("</table>");
  };
  print ("<h4>Course Information</h4>\n");
  if ($dcode) print ("<p><i>Department:</i> $dname($dcode)</p>");
}
else
{
  %>
  <td bgcolor="B5CFE8" valign=center>
  <table align=center width=200 height=200 border=1 cellpadding=0 cellspacing=0 bordercolor=black><tr><td bgcolor="#FFFFFF" valign=middle>
  <p align=center><img src="<%=$oracleroot%>media/seas_anim.gif" width=100 height=100></p>
  <p align=center>Choose a course or professor from the list in the left pane.</p>
  </td></tr>
  </table>
  </td>
  <%
}
%>
</tr></table></body>
<%

require_once("wces/general.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wces/login.inc");
require_once("wces/oldquestions.inc");

param($classid);
param($courseid);
param($userid);
param($professorid);
param($cunix);
param($surveyid);
param($back);

$surveys = isset($surveys) ? "&surveys=1" : "";

function PrintLoginInfo($db, &$cunix, &$userid, &$professorid)
{
  if ($cunix)
    $query = "LEFT JOIN professors AS p USING (userid) WHERE cunix = '" . addslashes($cunix) . "'";
  else if ($userid)
    $query = "LEFT JOIN professors AS p USING (userid) WHERE userid = '" . addslashes($userid) . "'";
  else if ($professorid)
    $query = "INNER JOIN professors AS p USING (userid) WHERE professorid = '" . addslashes($professorid) . "'";
  else
    return false;  
  
  $result = db_exec("
    SELECT u.cunix, u.userid, u.email, u.isprofessor, u.isadmin, DATE_FORMAT(u.lastlogin, '%W, %M %e, %Y at %l:%i:%s%p') AS lastlogin, p.professorid
    FROM users AS u $query", $db, __FILE__, __LINE__);
  
  if(mysql_num_rows($result) != 1)
    return false;

  $row = mysql_fetch_assoc($result);
  $userid = $row["userid"];
  $cunix = $row["cunix"];
  $professorid = $row["professorid"];
  $email = $row["email"];
  $isprofessor = $row["isprofessor"];
  $isadmin = $row["isadmin"];
  $lastlogin = $row["lastlogin"];
  
  print("<h3>Login Information</h3>\n");
  print("<table>\n");
  print("<tr><td><strong>User Name:</strong></td><td>$cunix ($userid)</td></tr>\n");
  print("<tr><td><strong>Last Login:</strong></td><td>$lastlogin</td></tr>\n");
  if ($isprofessor == "true")
    print("<tr><td colspan=2><strong><font color=red>$cunix is a professor</font></strong></td></tr>");
  if ($isadmin == "true")
    print("<tr><td colspan=2><strong><font color=red>$cunix is an administrator</font></strong></td></tr>");
  print("</table>\n");
  print("<hr>\n");
  return true;
}

function PrintEnrollments($db, $userid)
{
  global $surveys, $questionperiodid;
  
  if (mysql_result(db_exec("SELECT COUNT(*) FROM enrollments WHERE userid = '$userid'", $db, __FILE__, __LINE__),0) == 0)
    return false;
  
  print("<h3>Cached AcIS Enrollments</h3>");
    
  if (!login_isvalid(login_administrator) && !(login_getuserid() == $userid))
  {
    print("<font color=red><strong>You must be logged in as administrator to view this information</strong></font>");
    return false;
  }  

  $y = db_exec(
    "SELECT p.professorid, p.name AS pname, c.courseid, cl.classid, cl.year, cl.semester, concat(s.code, c.code) AS code, c.name, cl.section " .
    ($surveys ? ", IF(qs.classid,IF(cs.userid,'yes','no'),'n/a') AS surveyed " : "") .
    "FROM enrollments as e
    INNER JOIN classes as cl USING (classid)
    INNER JOIN courses as c using (courseid)
    INNER JOIN subjects as s using (subjectid)
    LEFT JOIN professors AS p ON cl.professorid = p.professorid " . 
    ($surveys ? 
    "LEFT JOIN qsets AS qs ON (qs.classid = cl.classid)
    LEFT JOIN answersets AS a ON (a.classid = e.classid AND a.questionperiodid = '$questionperiodid' AND a.questionsetid = qs.questionsetid)
    LEFT JOIN completesurveys AS cs ON (cs.userid = e.userid AND cs.answersetid = a.answersetid) " : "") . 
    "WHERE e.userid = '$userid'
    GROUP BY classid ORDER BY cl.year DESC, cl.semester DESC", $db, __FILE__, __LINE__);
    
  print("<table border=1 cellspacing=0 cellpadding=2>\n");
  print("  <tr><td><b>Year</b></td><td><b>Semester</b></td><td><b>Course Code</b></td><td><b>Section</b></td><td><b>Course Name</b></td><td><b>Professor</b></td>" . ($surveys ? "<td><b>Surveyed</b></td>" : "") . "</tr>\n");
  while($result = mysql_fetch_array($y))
  {
    $name = "&nbsp;"; $professorid = $pname = $courseid = $classid = $year = $semester = $code = $section = $surveyed = "";
    extract($result);
    $pfield = $professorid ? "<a href=\"info.php?professorid=$professorid$surveys\">$pname</a>" : "&nbsp;";
    print("  <tr><td>$year</td><td>$semester</td><td><a href=\"info.php?courseid=$courseid$surveys\">$code</a></td><td><a href=\"info.php?classid=$classid$surveys\">$section</a></td><td>$name</td><td>$pfield</td>" . ($surveys ? "<td>$surveyed</td>" : "") . "</tr>\n");
  };
  print("</table>\n");
  print("<hr>\n");
}

function PrintLDAP($cunix)
{
  if (!$cunix) return false;
  print("<h3>LDAP Information</h3>\n");
  $ds=ldap_connect("ldap.columbia.edu");
  $r=ldap_bind($ds);
  $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$cunix");  
  $info = ldap_get_entries($ds, $sr);
  print("<p><strong>" . $info["count"] . " results found</strong></p>");
  foreach($info as $number => $result)
  if (strcmp($number,"count") != 0)
  {
    print("<h5>" . $result["cn"][0] . "</h5>\n");
    print("<ul>\n");
    foreach($result as $itemname => $itemarray)
    if(is_array($itemarray))
      foreach($itemarray as $key => $value)
      if (strcmp($key,"count") != 0)
        print("  <li><code><b>$itemname</b> = $value</code></li>\n");
    print("</ul>\n");
  }
  ldap_close($ds);
  print("<hr>\n");
  return true;
}

function PrintProfessorInfo($db, $professorid)
{
  global $surveys, $back;
  if (!$professorid || !$info = db_getrow($db,"professors",Array("professorid" => $professorid),0))
    return false;

  extract($info);
  
  print("<h3>Professor Information</h3>\n");
  print("<h4>$name</h4>");
  if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>");
  if ($statement) print("<h4>Statement</h4>\n<p>$statement</p>\n");
  if ($profile) print("<h4>Profile</h4>\n<p>$profile</p>\n");
  if ($education) print("<h4>Education</h4>\n<p>$education</p>\n");
  if ($email || $url) print("<h4>Contact Information</h4>\n");
  if ($email) print ("<p><a href=\"mailto:$email\">$email</a></p>\n");
  if ($url) print ("<p><a href=\"$url\">$url</a></p>\n");
  if ($departmentid)
  {
    $department = db_getvalue($db, "departments", array("departmentid" => $departmentid), "name");
    if ($department) print("<p>$department Department</p>");
  } 

  print ("<h4>Classes Taught</h4>\n<UL>\n");

  $classes = db_exec("

  SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode"
  . ($surveys ? ", COUNT(qs.classid) AS cnt " : ""). "
  FROM classes as cl
  LEFT JOIN courses AS c USING (courseid)
  LEFT JOIN subjects AS s USING (subjectid)
  " . ($surveys ? "LEFT JOIN qsets AS qs ON (qs.classid = cl.classid) " : "") . "
  WHERE cl.professorid = '$professorid'
  GROUP BY cl.classid
  ORDER BY cl.year DESC, cl.semester DESC LIMIT 50",$db,__FILE__,__LINE__);
  
  while ($class = mysql_fetch_assoc($classes))
  {
    $cnt = $classid = $section = $year = $scode = $code = $name = $section = "";
    extract($class);
    print ("  <LI><A HREF=\"info.php?classid=$classid$surveys\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</A>");
    if($cnt > 0) print(" - <a href=\"info.php?surveyid=$classid$back\">Preview Survey</a>");
    print("</LI>\n");
  }
  print("</UL>\n");
  
  print("<hr>\n");
}

function PrintClassInfo($db, $classid)
{
  global $surveys, $back;
  
  $infoq = db_exec(
  "SELECT cl.name AS clname, cl.section, cl.year, cl.semester, cl.students, p.name as pname, p.professorid, c.code, c.name, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname, c.courseid, d.departmentid, s.subjectid, sc.schoolid, dv.code AS dvcode, dv.divisionid, cl.time, cl.location, cl.callnumber
  FROM classes as cl
  LEFT JOIN courses as c USING (courseid)
  LEFT JOIN departments as d USING (departmentid)
  LEFT JOIN subjects as s ON (c.subjectid = s.subjectid)
  LEFT JOIN divisions as dv ON (cl.divisionid = dv.divisionid)
  LEFT JOIN schools as sc ON (c.schoolid = sc.schoolid)
  LEFT JOIN professors as p ON (cl.professorid = p.professorid)
  WHERE cl.classid = '" . addslashes($classid) . "'", $db, __FILE__, __LINE__); 
  
  $info = mysql_fetch_array($infoq);
  if ($info)
  {
    $time = $callnumber = $location = $name = $clname = $section = $year = $semester = $students = $pname = $professorid = $code = $name = $information = $dcode = $dname = $scode = $sname = $dvname = $scname = $courseid = $departmentid = $subjectid = $schoolid = $dvcode = $divisionid = "";
    extract($info);
    
    print("<h3>$name" . ($clname ? " - $clname" : "") . "</h3>\n");
    print("<p>" . ucwords($semester) . " $year Section $section</p>\n");
    
    if ($information) print("<h4>Information</h4>\n<p>$information</p>\n");
    if ($professorid) print ("<h4>Professor</h4>\n<a href=\"info.php?professorid=$professorid$surveys\">" . ($pname ? $pname : "Unknown") . "</a></p>\n");
    print("<h4>Other Information</h4>");
    if ($students) print ("<i>Enrollment:</i> $students Students</p>");
    if ($dcode) print ("<p><i>Department:</i> $dcode - $dname ($departmentid)</p>");
    if ($scode) print ("<p><i>Subject:</i> $scode - $sname ($subjectid)</p>");
    if ($scname) print ("<p><i>School:</i> $scname ($schoolid)</p>");
    if ($dvname) print ("<p><i>Division:</i> $dvcode - $dvname ($divisionid)</p>");
    if ($time) print ("<p><i>Time:</i> $time</p>");
    if ($location) print ("<p><i>Location:</i> $location</p>");
    if ($callnumber) print ("<p><i>Call Number:</i> $callnumber</p>");
    if ($code) print ("<p><i>Course Code:</i> <a href=\"info.php?courseid=$courseid$surveys\">$code</a></p>");
  }
  
  if ($surveys)
  {
    $result = db_exec("SELECT COUNT(*) FROM qsets WHERE classid = '" . addslashes($classid) . "'", $db, __FILE__, __LINE__);
    if (mysql_result($result,0) > 0)
      print("<p align=center><strong> &gt; <a href=\"info.php?surveyid=$classid$back\">Preview Survey Questions</a> &lt; </strong></p>");
  }    

  $result = db_exec("
    SELECT u.cunix, ld.cn
    FROM enrollments AS e 
    INNER JOIN users AS u USING (userid)
    LEFT JOIN ldapcache AS ld USING (userid)
    WHERE classid = '" . addslashes($classid) . "'", $db, __FILE__, __LINE__
  );
  
  if (mysql_num_rows($result) > 0)
  {
    print("<h4>Known Enrollments</h4>\n");
   
    if (!login_isvalid(login_administrator) && !($professorid && login_getprofid(false) == $professorid))
      print("<font color=red><strong>You must be logged in as administrator to view this information</strong></font>");
    else
    {     
      print("<ul>\n");
      while($row = mysql_fetch_assoc($result))
      {
        $cn = $cunix = "";
        extract($row);
        print("  <li>$cn (<a href=\"info.php?cunix=$cunix$surveys\">$cunix</a>)</li>\n");
      }
      print("</ul>\n");
    }  
  }
}

function PrintCourseInfo($db, $courseid)
{
  global $surveys, $back;
  
  $infoq = db_exec(
    "SELECT c.code, c.name, c.information, d.departmentid, d.code as dcode, d.name as dname, s.subjectid, s.code as scode, s.name as sname, sc.schoolid, sc.name as scname
    FROM courses as c
    LEFT JOIN departments as d USING (departmentid)
    LEFT JOIN subjects as s ON (c.subjectid = s.subjectid)
    LEFT JOIN schools as sc ON (c.schoolid = sc.schoolid)
    WHERE courseid = '" . addslashes($courseid) . "'", $db, __FILE__, __LINE__); 
    
  $info = mysql_fetch_assoc($infoq);
  if ($info)
  {
    extract($info);
    print("<h3>$name</h3>\n");
    if ($information) print("<h4>Information</h4>\n<p>$information</p>\n");
    print("<h4>Identifiers</h4>");
    if ($code) print ("<p><i>Course Code:</i> $code</p>");
    if ($dcode) print ("<p><i>Department:</i> $dcode - $dname ($departmentid)</p>");
    if ($scode) print ("<p><i>Subject:</i> $scode - $sname($subjectid)</p>");
    if ($scname) print ("<p><i>School:</i> $scname ($schoolid)</p>");
  }
  
  $infoq = db_exec(
  
  "SELECT cl.classid, cl.name, cl.section, cl.year, cl.semester, p.professorid, p.name AS pname" . ($surveys ? ", COUNT(qs.classid) AS cnt" : "") . "
  FROM classes AS cl
  LEFT JOIN professors AS p USING (professorid)
  " . ($surveys ? "LEFT JOIN qsets AS qs ON (qs.classid = cl.classid)" : "") . "
  WHERE cl.courseid = '" . addslashes($courseid) . "'
  GROUP BY cl.classid
  ORDER BY cl.year DESC, cl.semester DESC, cl.section", $db, __FILE__, __LINE__);
  
  print("<h4>Sections</h4>\n<ul>");
  while($info = mysql_fetch_assoc($infoq))
  {
    $cnt = $classid = $name = $section = $year = $professorid = $pname = "";
    extract($info);
    print("  <li><a href=\"info.php?classid=$classid$surveys\">" . ucfirst($semester) . " $year - Section $section" . ($name ? " - $name" : "") . "</a>" . ($professorid ? " - <a href=\"info.php?professorid=$professorid$surveys\">$pname</a>" : ""));
    if($cnt > 0) print(" - <a href=\"info.php?surveyid=$classid$back\">Preview Survey</a>");
    print("</li>\n");
  }
}

function ShowSurvey($db, $classid, $back)
{
  print('<strong><a href="' . htmlspecialchars($back ? $back : "info.php?classid=$classid") . "\">Back</a></strong><hr>\n");
  print("<form method=post>\n");
  $s = new OldSurvey($db, $classid, false, false, "preview", "f", WIDGET_POST);
  $s->loadvalues();
  $s->display();
  print("</form>\n");
}

page_top("Information Viewer");
$db = wces_connect();

if ($surveys && !$surveyid)
{
  $questionperiodid = wces_Findquestionsetsta($db,"qsets");
  $back = "&back=" . urlencode(server_getrequest());
}  

if ($cunix || $userid || $professorid) 
{
  PrintLoginInfo($db, $cunix, $userid, $professorid);
  PrintProfessorInfo($db, $professorid);
  PrintEnrollments($db, $userid);
  PrintLDAP($cunix);      
}
else if ($classid)
  PrintClassInfo($db, $classid);
else if ($courseid)
  PrintCourseInfo($db, $courseid);
else if ($surveyid)
  ShowSurvey($db, $surveyid, $back);
else  
  print("<p>Nothing to display.</p>");
page_bottom();


%>
<?

require_once("wbes/general.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wces/login.inc");
require_once("wces/oldquestions.inc");

param($class_id);
param($course_id);
param($user_id);
param($uni);
param($surveyid);
param($fake);
param($nofake);

function PrintUser(&$uni, &$user_id)
{
  global $wces, $ASID;
  
  if ($uni)
    $query = "WHERE u.uni = '" . addslashes($uni) . "'";
  else if ($user_id)
    $query = "WHERE u.user_id = '" . addslashes($user_id) . "'";
  else
    return false;  
  
  $result = pg_query("
    SELECT 
      u.uni, u.user_id, u.email, u.flags, d.code, u.firstname, u.lastname,
      to_char(u.lastlogin, 'Day, Month DD, YYYY at HH12:MI:SS AM') AS lastlogin
    FROM users AS u 
    LEFT JOIN departments AS d USING (department_id)
    $query
  ", $wces, __FILE__, __LINE__);
  
  if(pg_numrows($result) != 1)
    return false;

  $row = pg_fetch_array($result,0,PGSQL_ASSOC);
  $user_id = (int)$row["user_id"];
  $uni = $row["uni"];
  $flags = (int)$row["flags"];
  
  if ($row['lastname'])
  {
    $head = "$row[firstname] $row[lastname]";
    if ($name = $row['uni']) $head .= " ($name)"; else $name = $head;
  }  
  else if ($row['uni'])
    $name = $head = $row['uni'];
  else
    $head = "<i>Unknown User</i>";  
    
  
  print("<h3>$head</h3>\n");

  if ((login_getstatus() & login_administrator) && ($user_id != login_getuserid()))
    print("<p><a href=\"info.php?fake=$user_id$ASID\">Log on as this user...</a></p>\n");

  print("<table>\n");
  
  if (!$row['lastlogin']) $row['lastlogin'] = "<i>unknown</i>";
  print("<tr><td><strong>Last Login:</strong></td><td>$row[lastlogin]</td></tr>\n");

  $row['email'] = $row['email'] ? "<a href=\"mailto:$row[email]\">$row[email]</a>" : "<i>unknown</i>";
  print("<tr><td><strong>Email:</strong></td><td>$row[email]</td></tr>\n");

  $access = array();
  if ($flags & 8) $access[] = "student";
  if ($flags & 4) $access[] = "professor";
  if ($flags & 2) $access[] = "$row[code] department administrator";
  if ($flags & 1) $access[] = "administrator";
  if (count($access) == 0) $access[] = "none";

  print("<tr><td><strong>Access:</strong></td><td>" . implode(", ", $access) . "</td></tr>\n");  
  print("</table>\n");
  
  print("<hr>\n");
  
  PrintAffils($user_id);
  PrintProfessorInfo($user_id);
  PrintEnrollments($user_id);
  //PrintLDAP($uni);     
}

function PrintEnrollments($user_id)
{
  global $wces, $wces_path, $server_url, $ASID;
  
  print("<h3>Known Enrollments</h3>");
  
  $userid = (int)login_getuserid();
  $restricted = !(login_getstatus() & login_administrator) && $user_id != $userid;

  pg_query("
    CREATE TEMPORARY TABLE enclasses AS
    SELECT e.status, e.class_id, c.course_id, cl.year, cl.semester, cl.section, (s.code || c.divisioncode || c.code) AS code, cl.name AS clname, c.name AS cname
    FROM enrollments AS e" . ($restricted ? "
    LEFT JOIN enrollments AS my ON my.user_id = $userid AND my.class_id = e.class_id" : "") . "
    INNER JOIN classes AS cl ON cl.class_id = e.class_id
    INNER JOIN courses AS c USING (course_id)
    INNER JOIN subjects AS s USING (subject_id)
    WHERE e.user_id = $user_id" . ($restricted ? "
    AND (e.status > 1 OR my.class_id IS NOT NULL)" : "")
  ,$wces,__FILE__,__LINE__);

  $classes = pg_query("
    SELECT class_id, course_id, year, semester, section, code, clname, cname, status
    FROM enclasses
    ORDER BY year DESC, semester DESC, status DESC, code, section
  ", $wces, __FILE__, __LINE__);
  
  $professors = pg_query("
    SELECT cl.class_id, e.user_id, btrim(u.firstname || ' ' || u.lastname) AS name
    FROM enclasses AS cl
    INNER JOIN enrollments AS e ON cl.class_id = e.class_id AND e.status = 3
    INNER JOIN users AS u USING (user_id)
    ORDER BY cl.year DESC, cl.semester DESC, cl.status DESC, cl.code, cl.section
  ", $wces, __FILE__, __LINE__);
  
  pg_query("DROP TABLE enclasses", $wces, __FILE__, __LINE__);

  $sems = array(0 => "spring", 1 => "summer", 2 => "fall");
  $stats = array(0 => "dropped", 1 => "student", 2 => "ta", 3 => "professor");

  print("<table border=1 cellspacing=0 cellpadding=2>\n");
  print("  <tr><td><b>Year</b></td><td><b>Semester</b></td><td><b>Course Code</b></td><td><b>Section</b></td><td><b>Course Name</b></td><td><b>Professor</b></td><td><b>Status</b></td></tr>\n");
  $classlen = pg_numrows($classes);
  $profs = new pg_wrapper($professors); 
  $prow = &$profs->row;
  for($i = 0; $i < $classlen; ++$i)
  {
    $row = pg_fetch_array($classes,$i,PGSQL_ASSOC);
    $name = $row['clname'] ? "$row[cname]: $row[clname]" : $row['cname'];
    $sem = $sems[$row['semester']];
    print("  <tr><td>$row[year]</td><td>$sem</td><td><a href=\"info.php?course_id=$row[course_id]$ASID\">$row[code]</a></td><td><a href=\"info.php?class_id=$row[class_id]$ASID\">$row[section]</a></td><td>$name</td><td>");
    for($firstp = true; $prow && $row['class_id'] == $prow['class_id']; $profs->advance())
    {
      if ($firstp) $firstp = false; else print("<br>");
      print("<a href=\"info.php?user_id=$prow[user_id]$ASID\">$prow[name]</a>");
    }
    if ($firstp) print("&nbsp;");
    
    $status = $stats[$row['status']];
   
    print("</td><td>$status</td></tr>\n");
  }
  print("</table>\n");
  
  if ($restricted)
  {
    print("<p><a href=\"${wces_path}login/login.php?url=" . urlencode($server_url->toString(true, true, true)) . "$ASID\">Log in as an administrator to see more enrollments...</a></p>\n");
  }  
 
  print("<hr>\n");
}

function PrintAffils($user_id)
{
  global $wces, $wces_path, $server_url;
  
  $userid = (int)login_getuserid();
  $restricted = !(login_getstatus() & login_administrator) && $user_id != $userid;

  $result = pg_query("
    SELECT g.acis_group_id, g.code
    FROM acis_affiliations AS a" . ($restricted ? "
    INNER JOIN acis_affiliations AS my ON my.user_id = $userid AND my.acis_group_id = a.acis_group_id" : "") . "
    INNER JOIN acis_groups AS g ON g.acis_group_id = a.acis_group_id
    WHERE a.user_id = $user_id
    ORDER BY g.code"
  ,$wces,__FILE__,__LINE__);

  $n = pg_numrows($result);

  if ($n || $restricted)
  {
    print("<h3>AcIS Affiliations</h3>");

    print("<ul>\n");  
    for($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_array($result, $i, PGSQL_ASSOC);
      print("  <li><a noref=\"info.php?group_id=$row[acis_group_id]\">$row[code]</a></li>\n");
    }
    print("</ul>\n");
    
    if ($restricted)
      print("<p><a href=\"${wces_path}login/login.php?url=" . urlencode($server_url->toString(true, true, true)) . "\">Log in as an administrator to see more affiliations...</a></p>\n");

    print("<hr>\n");
  }  
}

function PrintLDAP($uni)
{
  if (!$uni) return false;
  print("<h3>LDAP Information</h3>\n");
  $ds=ldap_connect("ldap.columbia.edu");
  $r=ldap_bind($ds);
  $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$uni");  
  $info = ldap_get_entries($ds, $sr);
  if (!is_array($info)) $info = array("count" => 0);
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

function PrintProfessorInfo($user_id)
{
  global $wces;
  $result = pg_query("SELECT url, picname, statement, profile, education FROM professor_data WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
  if (!pg_numrows($result)) return false;
  
  $info = pg_fetch_array($result,0,PGSQL_ASSOC);
  
  extract($info);
  
  print("<h3>Professor Information</h3>\n");
  $found = false;
  if ($picname)
  {
    $found = true;
    print("<p><img src=\"/oracle/prof_images/$picname\"></p>");
  }  
  if ($statement)
  {
    $found = true;
    print("<h4>Statement</h4>\n<p>$statement</p>\n");
  }  
    
  if ($profile)
  {
    $found = true;
    print("<h4>Profile</h4>\n<p>$profile</p>\n");
  }  
    
  if ($education)
  {
    $found = true;
    print("<h4>Education</h4>\n<p>$education</p>\n");
  }
    
  if ($url)
  {
    $found = true;
    print ("<p><a href=\"$url\">$url</a></p>\n");
  }  

  if (!$found)
    print("<p><i>None found</i></p>");

  print("<hr>\n");
}

function PrintClassInfo($class_id)
{
  global $wces, $wces_path, $server_url, $ASID;
  
  $class_id = (int)$class_id;
  
  $result = pg_query("
    SELECT cl.name AS clname, cl.section, cl.year, cl.semester, cl.students, c.code, c.name, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname, c.course_id, d.department_id, s.subject_id, sc.school_id, dv.code AS dvcode, dv.division_id, cl.time, cl.location, cl.callnumber
    FROM classes as cl
    INNER JOIN courses as c USING (course_id)
    INNER JOIN subjects as s USING (subject_id)
    LEFT JOIN departments as d ON d.department_id = cl.department_id
    LEFT JOIN divisions as dv ON (dv.division_id = cl.division_id)
    LEFT JOIN schools as sc ON (sc.school_id = cl.school_id)
    WHERE cl.class_id = $class_id
  ", $wces, __FILE__, __LINE__); 
  
  $sems = array(0 => "Spring", 1 => "Summer", 2 => "Fall");
  if (pg_numrows($result) == 1)
  {
    $time = $callnumber = $location = $name = $clname = $section = $year = $semester = $students = $pname = $professorid = $code = $name = $information = $dcode = $dname = $scode = $sname = $dvname = $scname = $course_id = $department_id = $subject_id = $school_id = $dvcode = $division_id = "";
    extract(pg_fetch_array($result,0,PGSQL_ASSOC));
    
    print("<h3>$name" . ($clname ? " - $clname" : "") . "</h3>\n");
    print("<p>$sems[$semester] $year Section $section</p>\n");
    if ($information) print("<h4>Information</h4>\n<p>$information</p>\n");
    print("<h4>Other Information</h4>");
    if ($students) print ("<i>Enrollment:</i> $students Students</p>");
    if ($dcode) print ("<p><i>Department:</i> $dcode - $dname ($department_id)</p>");
    if ($scode) print ("<p><i>Subject:</i> $scode - $sname ($subject_id)</p>");
    if ($scname) print ("<p><i>School:</i> $scname ($school_id)</p>");
    if ($dvname) print ("<p><i>Division:</i> $dvcode - $dvname ($division_id)</p>");
    if ($time) print ("<p><i>Time:</i> $time</p>");
    if ($location) print ("<p><i>Location:</i> $location</p>");
    if ($callnumber) print ("<p><i>Call Number:</i> $callnumber</p>");
    if ($code) print ("<p><i>Course Code:</i> <a href=\"info.php?course_id=$course_id$ASID\">$code</a></p>");
  }
  
  print("<hr>\n<h3>Known Enrollments</h3>");
  
  if (login_getstatus() & login_administrator)
    $restricted = false;
  else
  {
    $userid = (int)login_getuserid();
    $result = pg_query("SELECT COUNT(*) FROM enrollments WHERE user_id = $userid AND class_id = $class_id", $wces, __FILE__, __LINE__);
    $restricted = pg_result($result,0,0) ? false : true;
  }

  $result = pg_query("
    SELECT e.user_id, e.status, u.uni, (u.lastname || ', ' || u.firstname) AS name
    FROM enrollments AS e
    INNER JOIN users AS u USING (user_id)
    WHERE e.class_id = $class_id" . ($restricted ? "
    AND (e.status > 1)" : "") . "
    ORDER BY e.status DESC, u.lastname, u.firstname, u.uni
  ",$wces,__FILE__,__LINE__);

  $stat = array(0 => "Dropped", 1 => "Student", 2 => "TA", 3 => "Professor");

  print("<table border=1 cellspacing=0 cellpadding=2>\n");
  print("  <tr><td><b>Status</b></td><td><b>DND</b></td><td><b>Name</b></td></tr>\n");
  $n = pg_numrows($result);
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    if (!$uni) $uni = "<i>unknown</i>";
    if (!$name) $name = "&nbsp;";
    print("<td>$stat[$status]</td><td><a href=\"info.php?user_id=$user_id$ASID\">$uni</a></td><td>$name</td></tr>\n");
  }
  print("</table>\n");
  
  if ($restricted)
  {
    print("<p><a href=\"${wces_path}login/login.php?url=" . urlencode($server_url->toString(true, true, true)) . "$ASID\">Log in as an administrator to see more enrollments...</a></p>\n");
  }  
 
  print("<hr>\n");
}

function PrintCourseInfo($course_id)
{
  global $surveys, $wces, $ASID;
  
  $course_id = (int)$course_id;
  
  $result = pg_query(
    "SELECT c.code, c.name, c.information, s.subject_id, s.code as scode, s.name as sname
    FROM courses as c
    LEFT JOIN subjects as s ON (c.subject_id = s.subject_id)
    WHERE course_id = '" . addslashes($course_id) . "'", $wces, __FILE__, __LINE__); 
    
  if (pg_numrows($result) == 1)
  {
    $row = pg_fetch_array($result,0,PGSQL_ASSOC);
    print("<h3>$row[name]</h3>\n");
    if ($row['information']) print("<h4>Information</h4>\n<p>$row[information]</p>\n");
    print("<h4>Identifiers</h4>");
    if ($row['code']) print ("<p><i>Course Code:</i> $row[code]</p>");
        if ($row['scode']) print ("<p><i>Subject:</i> $row[scode] - $row[sname] ($row[subject_id])</p>");
  }
  
  $result = pg_query("
    SELECT cl.class_id, cl.name, cl.section, cl.year, cl.semester, get_profs(cl.class_id) AS pname
    FROM classes AS cl
    WHERE cl.course_id = $course_id
    ORDER BY cl.year DESC, cl.semester DESC, cl.section
  ", $wces, __FILE__, __LINE__);

  $sems = array(0 => 'Spring', 1 => 'Summer', 2 => 'Fall');
  print("<h4>Sections</h4>\n<ul>");
  $n = pg_numrows($result);
  for($i=0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    
    print("  <li><a href=\"info.php?class_id=$class_id$surveys$ASID\">$sems[$semester] $year - Section $section" . ($name ? " - $name" : "") . "</a>");
    $p = explode("\n",$pname);
    $first = true;
    while(count($p) >= 3)
    {
      if ($first) { print (" - Professor "); $first = false; } else print(", ");
      $q = array_splice($p, 0, 3);
      print("<a href=\"info.php?user_id=$q[0]$ASID\">$q[2] $q[1]</a>");
    }
    print("</li>\n");
  }
}

function SearchPage()
{
  global $wces, $user_uni, $user_last, $course_subject, $course_code, $course_name, $user_find, $course_find, $ISID, $ASID;

  if ($user_find)
  {
    $user_uni = trim($user_uni);
    $user_last = trim($user_last);
    
    $where = "";
    if ($user_uni)
    {
      if ($where) $where .= " AND ";
      $where .= "uni ILIKE '" . addslashes($user_uni) . "%'";
    }

    if ($user_last)
    {
      if ($where) $where .= " AND ";
      $where .= "lastname ILIKE '" . addslashes($user_last) . "%'";
    }
    
    if ($where)
    {
      $users = pg_query("SELECT user_id, firstname, lastname, uni FROM users WHERE $where ORDER BY lastname LIMIT 300", $wces, __FILE__, __LINE__);
      $usern = pg_numrows($users);
    }  
    else
      $usern = 0;
  }
 
  if ($course_find)
  {
    $course_subject = (int)$course_subject;
    $course_name = trim($course_name);
    
    $a = array();
    if (preg_match("/[0123456789]+/",$course_code,$a))
      $course_code = (int)$a[0];
    else
      $course_code = "";  
   
    $where = "";
    if ($course_subject)
    {
      if ($where) $where .= " AND ";
      $where .= "c.subject_id = $course_subject";
    }

    if ($course_code)
    {
      if ($where) $where .= " AND ";
      $where .= "c.code = $course_code";
    }

    if ($course_name)
    {
      if ($where) $where .= " AND ";
      $where .= "c.name = '%" . addslashes($course_name) . "%'";
    }
    //$GLOBALS['db_debug'] = true;
    if ($where)
    {
      $courses = pg_query("
        SELECT c.course_id AS course_id, s.code AS scode, c.name AS name, c.divisioncode AS divisioncode, c.code
        FROM courses AS c
        INNER JOIN subjects AS s USING (subject_id)
        WHERE $where
        ORDER BY s.code, c.code, c.divisioncode, c.name
        LIMIT 300
      ", $wces, __FILE__, __LINE__);
      $coursen = pg_numrows($courses);
    }  
    else
      $coursen = 0;
  } 
  
  if (!$course_find) {
?>
<form method=get>
<?=$ISID?>
<input type=hidden name=user_find value=1>
<h4>Search for a user...</h4>
<table>
<tr>
  <td><label for=user_uni>UNI:</label></td>
  <td><input type=text name=user_uni id=user_uni value="<?=htmlspecialchars($user_uni)?>"></td>
</tr>  
<tr>
  <td><label for=user_last>Last Name:</label></td>
  <td><input type=text name=user_last id=user_last value="<?=htmlspecialchars($user_last)?>"></td>
</tr>
<tr>
  <td>&nbsp;</td>
  <td><input type=submit name=user_submit value="Go"></td>
</tr>
</table>
</form>
<?
  } // !$course_find

  if (!$user_find) {
?>
<form method=get>
<?=$ISID?>
<input type=hidden name=course_find value=1>
<h4>Search for a course...</h4>
<table>
<tr>
  <td><label for=course_subject>Subject Code:</label></td>
  <td>
    <select name=course_subject id=course_subject>
    <option value="">
<?
  $result = pg_query("SELECT subject_id, code, name FROM subjects ORDER BY code", $wces, __FILE__, __LINE__);
  $n = pg_numrows($result);
  for($i=0; $i<$n; ++$i)
  {
    $row = pg_fetch_array($result, $i, PGSQL_ASSOC);
    $selected = $row['subject_id'] == $course_subject ? " selected" : "";
    print("    <option value=\"$row[subject_id]\"$selected>$row[code] - $row[name]\n");
  }
?>     
    </select>
</tr>  
<tr>
  <td><label for=course_code>Course Code:</label></td>
  <td><input type=text name=course_code id=course_code value="<?=htmlspecialchars($course_code)?>"></td>
</tr>
<tr>
  <td><label for=course_name>Course Name:</label></td>
  <td><input type=text name=course_name id=course_name value="<?=htmlspecialchars($course_name)?>"></td>
</tr>
<tr>
  <td>&nbsp;</td>
  <td><input type=submit name=user_submit value="Go"></td>
</tr>
</table>
</form>
<?
  } // !$user_find

  if ($user_find)
  {
    print("<hr>\n");
    
    if (!$usern)
      print("<p><i>No matches found.</i></p>");
    else
    {
      print("<ul>\n");
      for($i = 0; $i < $usern; ++$i)
      {
        $row = pg_fetch_array($users,$i,PGSQL_ASSOC);
        
        print("  <li><a href=\"info.php?user_id=$row[user_id]$ASID\">");
        if ($row['lastname'])
        {
          print(trim("$row[firstname] $row[lastname]</a>"));
          if ($row['uni']) print(" ($row[uni])");
        }
        else if ($row['uni'])
          print("$row[uni]</a>");
        else
          print("<i>unknown</i></a>");
          
        print("</li>\n");
      }  
      print("</ul>\n");  
    }
  }
  
  if ($course_find)
  {
    print("<hr>\n");
    
    if (!$coursen)
      print("<p><i>No matches found.</i></p>");
    else
    {
      print("<ul>\n");
      for($i = 0; $i < $coursen; ++$i)
      {
        $row = pg_fetch_array($courses,$i,PGSQL_ASSOC);
        print("  <li><a href=\"info.php?course_id=$row[course_id]$ASID\">$row[scode]$row[divisioncode]$row[code] $row[name]</a></li>\n");
      }  
      print("</ul>\n");  
    }
  }
}

wces_connect();

if ($fake && (login_getstatus() & login_administrator))
{
  global $QSID;
  login_update($fake, login_getuserid(), login_getuni());
  redirect("{$wces_path}index.php$QSID");  
}

if ($nofake && ($fake = login_getfake()))
{
  $user_id = login_getuserid();
  login_update($fake);
}

page_top("Information Viewer");

if ($uni || $user_id || $class_id || $course_id)
  print("<p><a href=info.php$QSID>Search</a></p>");  

if ($uni || $user_id) 
{
  PrintUser($uni, $user_id);
}
else if ($class_id)
  PrintClassInfo($class_id);
else if ($course_id)
  PrintCourseInfo($course_id);
else
  SearchPage();
  
page_bottom();

?>
<title>Transfer</title>
<?

define("SRC_WWW",      1);
define("SRC_PID",      2);
define("SRC_ORACLE",   3);
define("SRC_REGFILE",  4);

require_once("wbes/server.inc");
require_once("wbes/postgres.inc");
require_once("wces/wces.inc");
require_once("wbes/general.inc");

$my = wces_connect();
$pg = server_pginit("wces");

// db_exec("", $my, __FILE__, __LINE__);
// pg_query("", $pg, __FILE__, __LINE__);

$i = 0;

$do_subject      = false;
$do_department   = false;
$do_division     = false;
$do_course       = false;
$do_classid      = false;
$do_users        = false;
$do_enrollments  = false;
$do_penrollments = false;
$do_profhooks    = true;

if($do_subject)
{
  print("<h3>Subjects</h3>\n");
  $s = pg_query("DELETE FROM temp_subj", $pg, __FILE__, __LINE__);
  $result = db_exec("SELECT subjectid, code, name FROM subjects", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    aarray_map("nullquot", $row);
    pg_query("SELECT temp_subji($row[subjectid], subject_update($row[code], $row[name]))", $pg, __FILE__, __LINE__);
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    if ($first) $first = false; else print(",");
    print($row['code']);
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if($do_department)
{  
  print("<h3>Departments</h3>\n");
  pg_query("DELETE FROM temp_dept", $pg, __FILE__, __LINE__);
  $result = db_exec("SELECT departmentid, code, name FROM departments", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    aarray_map("nullquot", $row);
    pg_query("SELECT temp_depti($row[departmentid], department_update($row[code], $row[name]))", $pg, __FILE__, __LINE__);
    if ($first) $first = false; else print(",");
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    print($row['code']);
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if($do_division)
{ 
  print("<h3>Divisions</h3>\n");
  pg_query("DELETE FROM temp_div", $pg, __FILE__, __LINE__);
  $result = db_exec("SELECT divisionid, code, shortcode as scode, name FROM divisions", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    aarray_map("quot", $row);
    pg_query("SELECT temp_divi($row[divisionid], division_update($row[code], $row[scode], $row[name]))", $pg, __FILE__, __LINE__);
    if ($first) $first = false; else print(",");
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    print($row['code']);
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if($do_course)
{ 
  print("<h3>Courses</h3>\n");
  pg_query("DELETE FROM temp_course", $pg, __FILE__, __LINE__);
  $result = db_exec("
    SELECT c.courseid, c.subjectid, c.code, cl.divisioncode, c.name, c.information
    FROM courses AS c
    INNER JOIN classes AS cl USING (courseid)
    GROUP BY c.courseid, cl.divisioncode
  ", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    aarray_map("nullquot", $row);
    pg_query("SELECT temp_coursei($row[courseid], $row[divisioncode], course_update(temp_subjr($row[subjectid]), $row[code], $row[divisioncode], $row[name], $row[information]))", $pg, __FILE__, __LINE__);
    if ($first) $first = false; else print(",");
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    print($row['code']);
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if($do_classid)  
{
  print("<h3>Classes</h3>\n");
  $sem = array("spring" => 0, "summer" => 1, "fall" => 2);
  pg_query("DELETE FROM temp_class", $pg, __FILE__, __LINE__);
  $result = db_exec("
    SELECT 
      cl.classid, cl.courseid, cl.section, cl.year, cl.semester, cl.name, 
      cl.time, cl.location, cl.students, cl.callnumber, cl.departmentid, 
      cl.divisionid, c.schoolid, cl.divisioncode
    FROM classes AS cl
    INNER JOIN courses AS c USING (courseid)
  ", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    $row["semester"] = $sem[$row["semester"]];
    aarray_map("nullquot", $row);
    pg_query("SELECT temp_classi($row[classid], class_update(temp_courser($row[courseid],$row[divisioncode]), $row[section], $row[year], $row[semester], $row[name], $row[time], $row[location], $row[students], $row[callnumber], temp_deptr($row[departmentid]), temp_divr($row[divisionid]), temp_schr($row[schoolid])))", $pg, __FILE__, __LINE__);
    if ($first) $first = false; else print(",");
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    print($row['classid']);
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if($do_users)
{  
  print("<h3>Users/h3>\n");
  pg_query("DELETE FROM temp_user", $pg, __FILE__, __LINE__);
  pg_query("DELETE FROM temp_prof", $pg, __FILE__, __LINE__);
  $result = array(); // mysql doesn't do full outer joins. or unions...
  $result[] = db_exec("
    SELECT
      u.userid, p.professorid, u.isprofessor, u.isadmin, u.cunix, u.lastlogin, p.email,
      SUBSTRING_INDEX(p.name,' ',-1) AS last,
      SUBSTRING(p.name,1,LENGTH(p.name)-LOCATE(' ',REVERSE(p.name))) AS first,
      p.departmentid, a.departmentid AS adid,
      p.url, p.picname, p.statement, p.profile, p.education
    FROM professors AS p
    LEFT JOIN users AS u ON u.userid = p.userid
    LEFT JOIN admins AS a ON u.userid = a.userid
  ", $my, __FILE__, __LINE__);
  
  $result[] = db_exec("
    SELECT
      u.userid, NULL AS professorid, u.isprofessor, u.isadmin, u.cunix, u.lastlogin, NULL AS email,
      NULL AS last, NULL AS first, NULL AS departmentid, a.departmentid AS adid,
      NULL AS url, NULL AS picname, NULL AS statement, NULL AS profile, NULL AS education
    FROM users AS u
    LEFT JOIN professors AS p USING (userid)
    LEFT JOIN admins AS a ON u.userid = a.userid
    WHERE p.professorid IS NULL
  ", $my, __FILE__, __LINE__);
  
  foreach($result as $res)
  {
    $first = true;
    while($row = mysql_fetch_assoc($res))
    {
      $flags = 0;
      if ($row['isadmin'] == 'true')
        $flags |= 1;
    
      if ($row['adid'])
      {
        $row['departmentid'] = $row['adid'];
        $flags |= 2;
      }  
      
      $uid = $row['userid'];
      $pid = $row['professorid'];
      
      if ($row['isprofessor'] == 'true' || $pid)
        $flags |= 4;
      
      aarray_map("nullquot", $row);
      $r = pg_query("SELECT user_update($row[cunix], $row[last], $row[first], $row[email], $flags, $row[lastlogin], temp_deptr($row[departmentid]))", $pg, __FILE__, __LINE__);
      $user_id = pg_result($r,0,0);
    
      if ($uid)
      {
        pg_query("SELECT temp_useri($row[userid], $user_id)", $pg, __FILE__, __LINE__);
      }
      
      if ($pid)
      {
        pg_query("SELECT temp_profi($row[professorid],professor_data_update($user_id, $row[url], $row[picname], $row[statement], $row[profile], $row[education]))", $pg, __FILE__, __LINE__);
      }
      
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print("$row[userid]/$row[professorid]");
    }
    print("<br>\n" . mysql_num_rows($res) . " rows<br><br>\n");
  }
}  

if($do_enrollments)
{
  $result = db_exec("
    SELECT e.userid, e.classid, u.lastlogin FROM enrollments AS e INNER JOIN users AS u USING (userid)
  ", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    aarray_map("nullquot", $row);
    pg_query("SELECT enrollment_update(temp_userr($row[userid]), temp_classr($row[classid]), 1, $row[lastlogin])", $pg, __FILE__, __LINE__);
    if ($first) $first = false; else print(",");
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    print("$row[userid]/$row[classid]");
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if($do_penrollments)
{  
  $result = db_exec("
    SELECT professorid, classid FROM classes WHERE professorid IS NOT NULL
  ", $my, __FILE__, __LINE__);
  $first = true;
  while($row = mysql_fetch_assoc($result))
  {
    aarray_map("nullquot", $row);
    pg_query("SELECT enrollment_update(temp_profr($row[professorid]), temp_classr($row[classid]), 3, NULL)", $pg, __FILE__, __LINE__);
    if ($first) $first = false; else print(",");
    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    print("$row[professorid]/$row[classid]");
  }
  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
}

if ($do_profhooks)
{
  print("todo");
}






?>
<?

define("SRC_WWW",      1);
define("SRC_PID",      2);
define("SRC_ORACLE",   3);
define("SRC_REGFILE",  4);

require_once("wbes/server.inc");
require_once("wbes/postgres.inc");
require_once("wces/wces.inc");

function prof_update($db, $professorid, $email, $url)
{
  $professorid = (int)$professorid;
  $result = pg_go("SELECT email, url FROM professors WHERE user_id = '$professorid'", $db, __FILE__, __LINE__);
  if (($rows = pgsql_num_rows($result)) != 1) return false;
  
  $row = pgsql_fetch_row($result,0);

  if ($email && strpos($row["email"],$email) === false)
  {
    if ($row["email"]) $row["email"] .= "\t$email"; else $row["email"] = $email;
  }
  
  if ($url && strpos($row["url"],$url) === false)
  {
    if ($row["url"]) $row["url"] .= "\t$url"; else $row["url"] = $url;
  }
}

function prof_merge($db, $userids)
{
  $pids = array_map("intval", $userids);
  $result = pg_go("
    SELECT 
      user_id, uni, lastname, firstname, email, department_id, flags,
      affiliations, url, picname, statement, profile, education
    FROM professors
    WHERE user_id IN (" . implode($userids, ", ") . ")
    ORDER BY lastlogin DESC
  ", $db, __FILE__, __LINE__);
    
  $rows = pgsql_num_rows($result);
  if ($rows <= 1) return;
  
  $masta = pgsql_fetch_row($result,0);
  $deletes = array();
   
  $replacements = array("uni", "lastname", "firstname", "department_id", "affiliations");
  $appends = array("email", "url", "picname", "statement", "profile", "education");
  
  for($r = 1; $r < $rows; ++$r)
  {
    $row = pgsql_fetch_row($result,$r);
    if ($userid != (int)$row['user_id']) $deletes[] = (int)$row['user_id'];

    $masta['flags'] |= $row['flags'];

    foreach($replacements as $key)
      if ($row[$key] && !$masta[$key]) $masta[$key] = $row[$key];

    foreach($appends as $key)
    {
      $temp = explode("\t",$row);
      if (!is_array($temp)) $temp = array($temp);
      foreach($temp as $str)
      {
        if ($str && strpos($masta[$key], $str) === false)
        {
          if ($masta[$key]) 
            $masta[$key] .= "\t" . $str;
          else 
            $masta[$key] = $str;
        }  
      }
    }    
  }
  
  $masta = array_map("quot",$masta);
  pg_exec("
    UPDATE professors SET 
      user_id       = $masta[user_id],
      uni           = $masta[uni],
      lastname      = $masta[lastname],
      firstname     = $masta[firstname],
      email         = $masta[email],
      department_id = $masta[department_id],
      flags         = $masta[flags],
      affiliations  = $masta[affiliations],
      url           = $masta[url],
      picname       = $masta[picname],
      statement     = $masta[statement],
      profile       = $masta[profile],
      education     = $masta[education],
    WHERE user_id = $masta[user_id]
  ", $db, __FILE__, __LINE);
  
  pg_go("UPDATE enrollments SET user_id = $user_id WHERE userid IN (" . implode($deletes,",") . ")", $db, __FILE__, __LINE__);
  pg_go("DELETE FROM users WHERE user_id IN (" . implode($deletes,",") . ")", $db, __FILE__, __LINE__);
}

print("<h3>Test Page</h3>\n");

//$times = array();
//$times["begin"] = microtime();
//$times["connect to server"] = microtime();
//printtimes($times);

$db = server_pginit("wces");

// $result = pg_go("SELECT * FROM professor_hooks",$db, __FILE__, __LINE__);
// pg_show($result,"professor_hooks");
// print("<p>&nbsp;</p>"); 
//                         
// $result = pg_go("SELECT * FROM professors",$db, __FILE__, __LINE__);
// pg_show($result,"professors");
// print("<p>&nbsp;</p>"); 

//pg_go("execute class_update(2, '023', 2342, 2, 'maa class', 'time', 'location', 32, 32210, 1, 4, 1)", $db, __FILE__, __LINE__);

$result = pg_go("SELECT * FROM users",$db, __FILE__, __LINE__);
pg_show($result,"users");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM enrollments",$db, __FILE__, __LINE__);
pg_show($result,"enrollments");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM acis_groups",$db, __FILE__, __LINE__);
pg_show($result,"acis_groups");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM acis_affiliations",$db, __FILE__, __LINE__);
pg_show($result,"acis_affiliations");
print("<p>&nbsp;</p>");

$result = pg_go("
  SELECT cl.class_id, s.code || c.divisioncode || c.code AS code, cl.year, cl.semester, cl.section, cl.name
  FROM classes AS cl
  INNER JOIN courses AS c ON c.course_id = cl.course_id
  INNER JOIN subjects AS s ON s.subject_id = c.subject_id
  ",$db, __FILE__, __LINE__);
pg_show($result,"classes");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM courses",$db, __FILE__, __LINE__);
pg_show($result,"courses");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM subjects",$db, __FILE__, __LINE__);
pg_show($result,"subjects");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM departments",$db, __FILE__, __LINE__);
pg_show($result,"departments");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM divisions",$db, __FILE__, __LINE__);
pg_show($result,"divisions");
print("<p>&nbsp;</p>");

$result = pg_go("SELECT * FROM professor_hooks",$db, __FILE__, __LINE__);
pg_show($result,"professor_hooks");
print("<p>&nbsp;</p>");

?>
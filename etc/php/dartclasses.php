<?

require_once("wbes/general.inc");
require_once("wbes/postgres.inc");
require_once("wces/wces.inc");
require_once("wces/import.inc");



/*

 generally we label a course by "type", "number", and "section".

By "type" I mean whether it is Engineering (ENGG), Engineering Science (ENGS), or Engineering Management (ENGM).  By "number" I mean the course number (e.g., ENGS 33).  Most of the courses do not have "sections", but for those few which do, we have taken to numbering them in the obvious way -- 1, 2, etc.  

In our current evaluation system, I label courses by constructing a string consisting of these three parts:

       ttttNNN-S

... where "tttt" is the type (ENGG, ENGS, ENGM), NNN is the course number (padded with zeroes), and S is the section number (omitted if there is only one section).  But there is nothing special about this particular label -- it was just a convenient format at the time.  

Beyond that, the only other information I think is probably relevant about the course is the title (e.g., "Introduction to VLSI") and the instructor.  That is not really part of the course _naming_ scheme, but in our current evaluation system, we use it as a more human-readable output than "ENGM999-3" would otherwise be.
*/

/*
INSERT INTO subjects (code, name) VALUES ('ENGG','Engineering');
INSERT INTO subjects (code, name) VALUES ('ENGS','Engineering Science');
INSERT INTO subjects (code, name) VALUES ('ENGM','Engineering Management');
INSERT INTO semester_question_periods (displayname, begindate, enddate, year, semester) VALUES ('Spring 2002', '2002-02-01', '2002-06-01', 2002, 0);

INSERT INTO survey_categories (survey_category_id,name) VALUES (1, 'Base Questions Only');
INSERT INTO survey_categories (survey_category_id,name) VALUES (2, 'Objective 1');
INSERT INTO survey_categories (survey_category_id,name) VALUES (3, 'Objective 2');

INSERT INTO wces_topics(topic_id, parent, category_id, class_id) VALUES (1, NULL, NULL, NULL);
INSERT INTO wces_topics(topic_id, parent, category_id, class_id) VALUES (2, 1, 1, NULL);
INSERT INTO wces_topics(topic_id, parent, category_id, class_id) VALUES (3, 1, 2, NULL);
INSERT INTO wces_topics(topic_id, parent, category_id, class_id) VALUES (4, 1, 3, NULL);
update wces_topics set category_id = 1, parent = 2 WHERE class_id IS NOT NULL;
update classes set students = (select count(*) from enrollments as e where e.class_id = classes.class_id and e.status = 1);
update users set flags = flags | case when exists (select * from enrollments AS e where e.user_id = users.user_id and status = 1) then 8 else 0 end;
update users set flags = flags | case when exists (select * from enrollments AS e where e.user_id = users.user_id and status = 3) then 4 else 0 end;

*/
function getClass($code, $year, $semester, $coursename = false)
{
  global $wces;

  $l = strlen($code);
  if ($l == 7)
    $section = "''"; 
  else if ($l > 7 && $code[7] == '-')
  {
    $section = substr($code,8);
    if (is_numeric($section))
      $section = sprintf("'%03d'",(int)$section);
    else
      return false;
  }
  else
    return false;
    
  $scode = "'" . addslashes(strtoupper(substr($code,0,4))) . "'";
  $ccode = substr($code,4,3);
  if (is_numeric($ccode)) $ccode = (int)$ccode; else return false;
  
  
  $result = pg_query("SELECT subject_id FROM subjects WHERE code = $scode", $wces,__FILE__, __LINE__);
  if (pg_numrows($result) == 1)
    $subject_id = (int)pg_result($result,0,0);
  else
  {
    $result = pg_query("
      INSERT INTO subjects (code) VALUES ($scode);
      SELECT currval('subject_ids');
    ", $wces,__FILE__, __LINE__);
    $subject_id = (int)pg_result($result,0,0);
  }

  $result = pg_query("SELECT course_id FROM courses WHERE code = $ccode AND subject_id = $subject_id", $wces,__FILE__, __LINE__);
  if (pg_numrows($result) == 1)
  {
    $course_id = (int)pg_result($result,0,0);
    if ($coursename)
    {
      $coursename = "'" . addslashes($coursename) . "'";
      $result = pg_query("
        UPDATE courses SET name = $coursename WHERE course_id = $course_id
       ", $wces,__FILE__, __LINE__);
    }
  }
  else
  {
    $coursename = $coursename ? "'" . addslashes($coursename) . "'" : "NULL";
    $result = pg_query("
      INSERT INTO courses (subject_id, code, name, divisioncode) VALUES ($subject_id, $ccode, $coursename, '');
      SELECT currval('course_ids');
    ", $wces,__FILE__, __LINE__);
    $course_id = (int)pg_result($result,0,0);
  }
  
  $result = pg_query("SELECT class_id FROM classes WHERE section = $section AND course_id = $course_id AND year = $year AND semester = $semester", $wces,__FILE__, __LINE__);
  if (pg_numrows($result) == 1)
    $class_id = (int)pg_result($result,0,0);
  else
  {
    $result = pg_query("
      INSERT INTO classes (course_id, section, year, semester) VALUES ($course_id, $section, $year, $semester);
      SELECT currval('class_ids');
    ", $wces,__FILE__, __LINE__);
    $class_id = (int)pg_result($result,0,0);
    pg_query("INSERT INTO wces_topics (class_id) VALUES ($class_id)", $wces,__FILE__, __LINE__);    
  }    
  
  return $class_id;
}

function opencsv($filename)
{
  $fp = fopen($filename,"r");
  fgetcsv ($fp, 8192, ","); // eat first line
  return $fp;
}

function importclasses($filename,$year,$semester)
{
  global $wces;

  $fp = opencsv($filename);
  wces_connect();

  $i = 0;
  $row = 0;

  $classes = array();

  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != 3)
      print("<b>Warning:</b> Row $row does not contain the correct number of fields. (3 expected, " . count($data) . " found)<br>\n");
    else
    {
      
      $class_id = getClass($data[0], $year, $semester, $data[1]);
      if (!$class_id) die ("Unable to parse class $data[0] on row $row");
      
      $user_id = (int)$data[2];
      if (!$user_id) die ("Unable to add professor $data[2] to class $data[0] on row $row");
      
      if (!isset($classes[$class_id]))
      {
        pg_query("
          DELETE FROM enrollments WHERE class_id = $class_id AND status = 3;
        ", $wces, __FILE__, __LINE__);
        $classes[$class_id] = true;        
      }
      pg_query("
        INSERT INTO enrollments (user_id, class_id, status, lastseen) VALUES ($user_id, $class_id, 3, null);
      ", $wces, __FILE__, __LINE__);
      print("$data[0] added<br>\n");
    };
    if (((++$i) % 10) == 1) flush();
  }
  fclose ($fp);
};

function importenrollments($filename, $year, $semester)
{
  global $wces;

  $fp = opencsv($filename);
  wces_connect();

  $i = 0;
  $row = 0;

  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != 2)
      print("<b>Warning:</b> Row $row does not contain the correct number of fields. (2 expected, " . count($data) . " found)<br>\n");
    else
    {
      $user_id = (int)$data[0];
      $class_id = getClass($data[1], $year, $semester);
      if (!$class_id) die ("Unable to parse class $data[1] on row $row");
      
      pg_query("
        DELETE FROM enrollments WHERE class_id = $class_id AND user_id = $user_id;
        INSERT INTO enrollments (user_id, class_id, status, lastseen) VALUES ($user_id, $class_id, 1, null);
      ", $wces, __FILE__, __LINE__);
      print("$data[0], $data[1] added<br>\n");
    };
    if (((++$i) % 10) == 1) flush();
  }
  fclose ($fp);
};

function importusers($filename, $flags)
{
  global $wces;

  $fp = opencsv($filename);
  wces_connect();

  $i = 0;
  $row = 0;

  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != 3)
      print("<b>Warning:</b> Row $row does not contain the correct number of fields. (3 expected, " . count($data) . " found)<br>\n");
    else
    {
      $user_id = (int)$data[0];
      if (!$user_id) die("Invalid user id '$data[0]' on row $row");
      
      $name = $data[1];
      $email = $data[2];

      $name = trim($name);
      $p = strrpos($name,' ');
      if ($p === false)
      {
        $first = "";
        $last = $name;
      }
      else
      {
        $first = substr($name,0,$p);
        $last = substr($name,$p+1);
      }

      $email = quot($email);
      $first = quot($first);
      $last = quot($last);

      $result = pg_query("SELECT user_id FROM users WHERE user_id = $user_id", $wces,__FILE__, __LINE__);
      if (pg_numrows($result) == 1)
      {
        pg_query("UPDATE users SET firstname = $first, lastname = $last, email = $email, flags = flags | $flags WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
      }
      else
      {
        pg_query("INSERT INTO users (user_id, firstname, lastname, email, flags, lastlogin) VALUES ($user_id, $first, $last, $email, $flags, NULL)", $wces,__FILE__, __LINE__);
      }        
      print("$data[0] added<br>\n");
    };
    if (((++$i) % 10) == 1) flush();
  }
  fclose ($fp);
};

$path = "M:/server/special/rland/thayer-data/";
$path = "/afs/thayer/web/eval/thayer-data/";

wces_connect();
pg_query("BEGIN", $wces,__FILE__,__LINE__);
//importusers($path ."admins-w2002.txt",9);
importusers($path . "students-s2002.txt",0);
importclasses($path . "courses-s2002.txt",2002,0);
importenrollments($path . "enroll-s2002.txt",2002,0);
pg_query("END", $wces,__FILE__,__LINE__);

?>

<?

require_once("wbes/server.inc");
require_once("wbes/wbes.inc");

$wces = NULL;
$wces_verbose = false;

function wces_connect()
{
  global $wces, $wbes;
  wbes_connect();
  $wces = $wbes;
}

function wces_oldconnect()
{
  $db = server_mysqlinit();
  mysql_select_db("wces",$db);
  return $db;
}

// the order of data out of the get_profs and get_class functions

define("FORMAT_CLASS_SUBJECT_CODE",  0);
define("FORMAT_CLASS_DIVISION_CODE", 1);
define("FORMAT_CLASS_NUMBER",        2);
define("FORMAT_CLASS_SECTION",       3);
define("FORMAT_CLASS_YEAR",          4);
define("FORMAT_CLASS_SEMESTER",      5);
define("FORMAT_CLASS_NAME",          6);
define("FORMAT_CLASS_SNAME",         7);
define("FORMAT_CLASS_CLASS_ID",      8);
define("FORMAT_CLASS_COURSE_ID",     9);

define("FORMAT_COURSE_SUBJECT_CODE",  0);
define("FORMAT_COURSE_DIVISION_CODE", 1);
define("FORMAT_COURSE_NUMBER",        2);
define("FORMAT_COURSE_NAME",          3);
define("FORMAT_COURSE_COURSE_ID",     4);

define("FORMAT_PROF_COUNT",          3);
define("FORMAT_PROF_USER_ID",        0);
define("FORMAT_PROF_LASTNAME",       1);
define("FORMAT_PROF_FIRSTNAME",      2);

function explode_class($class)
{
  $c = explode("\n", $class);
  $courseCode = "{$c[FORMAT_CLASS_SUBJECT_CODE]} " . (int) $c[FORMAT_CLASS_NUMBER];
  $courseName = $className = $c[FORMAT_CLASS_NAME];
  if ($c[FORMAT_CLASS_SNAME]) $className .= ": {$c[FORMAT_CLASS_SNAME]}";
  $section = $c[FORMAT_CLASS_SECTION];
  $term = "";
  switch ((int)$c[FORMAT_CLASS_SEMESTER])
  {
    case 0:
      $term .= "Spring";
      break;
    case 1:
      $term .= "Summer";
      break;
    case 2:
      $term .= "Fall";
     break;
    case 3:
      $term .= "Winter";
     break;
    default:
      $term .= "???";
  }
  $term .= " {$c[FORMAT_CLASS_YEAR]}";
  $course_id = (int)$c[FORMAT_CLASS_COURSE_ID];
  $class_id = (int)$c[FORMAT_CLASS_CLASS_ID];
  return compact("courseCode", "courseName", "className", "section", "term", "class_id", "course_id");
}

function explode_course($course)
{
  $c = explode("\n", $course);
  $courseCode = "{$c[FORMAT_COURSE_SUBJECT_CODE]} {$c[FORMAT_COURSE_DIVISION_CODE]}" . (int) $c[FORMAT_COURSE_NUMBER];
  $courseName = $c[FORMAT_COURSE_NAME];
  $course_id = (int)$c[FORMAT_COURSE_COURSE_ID];
  return compact("courseCode", "courseName", "course_id");
}

function format_class($c, $format = "%c %n Section %s", $link = false)
{
  global $wces_path, $ASID;
  if (!is_array($c)) $c = explode_class($c);
  $str = str_replace(
    array("%c", "%n", "%N", "%s", "%t"),
    array($c['courseCode'], $c['courseName'], $c['className'], $c['section'], $c['term']),
    $format
  );

  if ($link)
  {
    $str = "<a href=\"{$wces_path}administrators/info.php?class_id=$c[class_id]$ASID\">$str</a>";
  }
  
  return $str;
}

function format_course($c, $format = "%c %n", $link = false)
{
  global $wces_path, $ASID;
  if (!is_array($c)) $c = explode_course($c);
  $str = str_replace(
    array("%c", "%n"),
    array($c['courseCode'], $c['courseName']),
    $format
  );

  if ($link)
  {
    $str = "<a href=\"{$wces_path}administrators/info.php?course_id=$c[course_id]$ASID\">$str</a>";
  }
  return $str;
}

function format_profs($profs, $link = false, $before = "", $after = "", $between = "")
{
  global $wces_path;
  if (strlen($profs) == 0) return "";
  $c = explode("\n", $profs);
  $str = "";
  for ($i = 0; $i < count($c); $i += FORMAT_PROF_COUNT)
  {
    if ($str) $str .= $between;
    $str .= $before;
    if ($link) $str .= "<a href=\"{$wces_path}administrators/info.php?user_id=" . $c[$i + FORMAT_PROF_USER_ID] . '">';
    $str .= $c[$i + FORMAT_PROF_FIRSTNAME] . " " . $c[$i + FORMAT_PROF_LASTNAME];
    if ($link) $str .= "</a>";
    $str .= $after;
  }
  return $str;
}

?>

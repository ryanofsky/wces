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
define("FORMAT_CLASS_NUMBER",        1);
define("FORMAT_CLASS_SECTION",       2);
define("FORMAT_CLASS_YEAR",          3);
define("FORMAT_CLASS_SEMESTER",      4);
define("FORMAT_CLASS_NAME",          5);
define("FORMAT_CLASS_SNAME",         6);
define("FORMAT_CLASS_CLASS_ID",      7);
define("FORMAT_CLASS_COURSE_ID",     8);
define("FORMAT_CLASS_LEN",           9);
                                      
define("FORMAT_COURSE_SUBJECT_CODE",  0);
define("FORMAT_COURSE_NUMBER",        1);
define("FORMAT_COURSE_NAME",          2);
define("FORMAT_COURSE_COURSE_ID",     3);

define("FORMAT_PROF_COUNT",          3);
define("FORMAT_PROF_USER_ID",        0);
define("FORMAT_PROF_LASTNAME",       1);
define("FORMAT_PROF_FIRSTNAME",      2);

$wces_semesters = array
( 0 => 'Spring',
  1 => 'Summer',
  2 => 'Fall'
);

function explode_class($class)
{
  global $wces_semesters;
  
  $c = explode("\n", $class);
  if (count($c) != FORMAT_CLASS_LEN) return $class;
  
  $courseCode = "{$c[FORMAT_CLASS_SUBJECT_CODE]} {$c[FORMAT_CLASS_NUMBER]}";
  $courseName = $className = $c[FORMAT_CLASS_NAME];
  if ($c[FORMAT_CLASS_SNAME]) $className .= ": {$c[FORMAT_CLASS_SNAME]}";
  $section = $c[FORMAT_CLASS_SECTION];
  $term = "";
  
  $term = $wces_semesters[(int)$c[FORMAT_CLASS_SEMESTER]];
  $term .= " {$c[FORMAT_CLASS_YEAR]}";
  $course_id = (int)$c[FORMAT_CLASS_COURSE_ID];
  $class_id = (int)$c[FORMAT_CLASS_CLASS_ID];
  return compact("courseCode", "courseName", "className", "section", "term", "class_id", "course_id");
}

function explode_course($course)
{
  $c = explode("\n", $course);
  $courseCode = "{$c[FORMAT_COURSE_SUBJECT_CODE]} {$c[FORMAT_COURSE_NUMBER]}";
  $courseName = $c[FORMAT_COURSE_NAME];
  $course_id = (int)$c[FORMAT_COURSE_COURSE_ID];
  return compact("courseCode", "courseName", "course_id");
}

function format_class($c, $format = "%c %n Section %s", $link = false)
{
  global $wces_path, $ASID;
  if (!is_array($c)) $c = explode_class($c);
  if (!is_array($c)) return $c;
  
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

function format_ccourse($c, $format = "%c %n", $link = false)
{
  if (!is_array($c)) $c = explode_course($c);
  registrar_clean($c['courseName']);
  return format_course($c, $format, $link);
};

function format_profs($profs, $link = false, $before = "", $after = "", $between = "")
{
  global $wces_path, $QSID;
  if (strlen($profs) == 0) return "";
  $c = explode("\n", $profs);
  $str = "";
  
  if ($link && !is_string($link))
  {
    $q = $QSID ? "$QSID&" : "?";
    $link = "{$wces_path}administrators/info.php{$q}user_id=";
  };
  
  for ($i = 0; $i < count($c); $i += FORMAT_PROF_COUNT)
  {
    if (!strlen($c[$i + FORMAT_PROF_FIRSTNAME]) 
      && !strlen($c[$i + FORMAT_PROF_LASTNAME]))
      continue;
      
    if ($str) $str .= $between;
    $str .= $before;
    if (is_string($link)) $str .= "<a href=\"$link" . $c[$i + FORMAT_PROF_USER_ID] . '">';
    $str .= $c[$i + FORMAT_PROF_FIRSTNAME] . " " . $c[$i + FORMAT_PROF_LASTNAME];
    if (is_string($link)) $str .= "</a>";
    $str .= $after;
  }
  return $str;
}

function registrar_clean(&$str)
{
  $str = str_replace(array("/", ","), array(" / ",", "), $str);
  $str = str_replace(array(" - ", "Iv", "Iii", "Ii"), array("-", "IV", "III", "II"), ucwords(strtolower(str_replace("-", " - ", $str))));  
};

function picture_src($id)
{
  global $wces, $WCES_CONFIG_UPLOAD_PATH;
  if (!$id) return false;
  wces_connect();
  $r = pg_go("SELECT name FROM pictures WHERE file_id = $id", $wces, __FILE__, __LINE__);
  return $WCES_CONFIG_UPLOAD_PATH . pg_result($r, 0, 0);
};

function set_question_period($question_period_id)
{
  $_SESSION['wces/question_period_id'] = $question_period_id;
}

function get_question_period()
{
  if (isset($_SESSION['wces/question_period_id']))
    return $_SESSION['wces/question_period_id'];
  else
  {
    global $wces;
    wces_connect();      
    // give priority to current question periods according
    // to which one began most recently, then to future
    // question periods according to which one begins first
    // then to past question periods according to which ended
    // most recently

    $result = pg_go("
      SELECT question_period_id, begindate, enddate
      FROM semester_question_periods
      ORDER BY 
        CASE WHEN enddate < (SELECT NOW()) THEN 3   -- past
             WHEN begindate > (SELECT NOW()) THEN 2 -- future
             ELSE 1                                 -- present
        END,
        CASE WHEN enddate < (SELECT NOW()) THEN (SELECT NOW()) - enddate     -- past
             WHEN begindate > (SELECT NOW()) THEN begindate - (select now()) -- future
             ELSE ((select now()) - begindate)                               -- present
        END
      LIMIT 1
    ", $wces, __FILE__, __LINE__);
    
    if (pg_numrows($result) == 0) return NULL;
    return $_SESSION['wces/question_period_id'] = (int)pg_result($result, 0, 0);
  }
}

function get_question_periods()
{
  global $wces;
  wces_connect();
  return pg_select_array("
    SELECT q.question_period_id, q.displayname
    FROM semester_question_periods AS q
    WHERE EXISTS (SELECT * FROM wces_topics AS t WHERE t.question_period_id = q.question_period_id AND NOT cancelled)
    ORDER BY begindate
  ", $wces, __FILE__, __LINE__);
}  

?>
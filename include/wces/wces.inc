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

define("FORMAT_PROF_COUNT",          3);
define("FORMAT_PROF_USER_ID",        0);
define("FORMAT_PROF_LASTNAME",       1);
define("FORMAT_PROF_FIRSTNAME",      2);

function format_class($class, $link = false, $code = true, $name = true, $semester = false, $sectionname = false)
{
  global $wces_path;
  $c = explode("\n", $class);
  $str = "";

  if ($code)
  {
    if ($str) $str .= " ";
    $str .= "{$c[FORMAT_CLASS_SUBJECT_CODE]} {$c[FORMAT_CLASS_DIVISION_CODE]}{$c[FORMAT_CLASS_NUMBER]}";
  }
  if ($name)
  {
    if ($str) $str .= " ";
    $str .= "{$c[FORMAT_CLASS_NAME]}";
    if ($sectionname && $c[FORMAT_CLASS_SNAME])
      $str .= ": {$c[FORMAT_CLASS_SNAME]}";
  }
  if ($str) $str .= " ";
  $str .= "Section {$c[FORMAT_CLASS_SECTION]}";
  if ($semester)
  {
    if ($str) $str .= " ";
    switch ((int)$c[FORMAT_CLASS_SEMESTER])
    {
      case 0:
        $str .= "Spring";
        break;
      case 1:
        $str .= "Summer";
        break;
      case 2:
        $str .= "Fall";
        break;
      default:
        $str .= "???";
    }
    $str .= " {$c[FORMAT_CLASS_YEAR]}";
  }

  if ($link)
  {
    $str = "<a href=\"{$wces_path}administrators/info.php?class_id={$c[FORMAT_CLASS_CLASS_ID]}\">$str</a>";
  }
  return $str;
}

function format_profs($profs, $link = false, $before = "", $after = "", $between = "")
{
  
  global $wces_path;
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

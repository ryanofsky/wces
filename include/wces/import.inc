<?

require_once("wbes/server.inc");
require_once("wces/wces.inc");

// CSV Columns
define("COURSE",0);             // course name
define("COURSECODE",1);         // 4 digit course number
define("DEPARTMENT",2);         // department name
define("DEPARTMENTCODE",3);     // 4 letter department abbreviation. Not on page, taken from department hash table deptlist
define("SUBJECT",4);            // full subject name
define("SUBJECTCODE",5);        // 4 letter subject abbreviation
define("DIVISION",6);           // division name
define("DIVISIONCODE",7);       // 1 or 2 letter division code
define("DIVISIONSCODE",8);      // 1 letter division code
define("CLASSNAME",9);          // class name
define("CLASSSECTION",10);      // 3 digit section ie  '001', '002', 'R01'
define("YEAR",11);              // four digit year
define("SEMESTER",12);          // 'spring', 'fall', 'summer'
define("INSTRUCTOR",13);        // full name
define("STUDENTS",14);          // number of students
define("SCHOOL",15);            // school name
define("TIME",16);              // class time
define("LOCATION",17);          // class location
define("CALLNUMBER",18);        // 5 digit call number

function importregcsv($filename)
{
  global $wces;

  $fp = fopen ($filename,"r");
  wces_connect();

  $i = 0;
  $row = 0;

  $subject_cache = $school_cache = $div_cache = $dept_cache = array();

  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != 19 && count($data) != 16)
    {
      print("<b>Warning:</b> Row $row does not contain the correct number of fields. (19 expected, " . count($data) . " found)<br>\n");
    }
    else
    {
      $semester = $data[SEMESTER] == 'spring' ? 0 : $data[SEMESTER] == 'summer' ? 1 : 2;
      $divstr = "${data[DIVISIONCODE]},{$data[DIVISIONSCODE]},{$data[DIVISION]}";
      $pstr = $data[INSTRUCTOR];
      $sdata = array_map("mnullquot",$data);

      if (isset($subject_cache[$data[SUBJECTCODE]]))
        $subject_id = $subject_cache[$data[SUBJECTCODE]];
      else
        $subject_id = $subject_cache[$data[SUBJECTCODE]] = (int)pg_result(pg_query("
          SELECT subject_update({$sdata[SUBJECTCODE]},{$sdata[SUBJECT]})
        ", $wces, __FILE__, __LINE__),0,0);

      if (!$subject_id) die("subject_id not found for subject {$data[SUBJECTCODE]} on row $row");

      if (isset($school_cache[$data[SCHOOL]]))
        $school_id = $school_cache[$data[SCHOOL]];
      else
        $school_id = $school_cache[$data[SCHOOL]] = (int)pg_result(pg_query("
          SELECT school_update({$sdata[SCHOOL]})
        ", $wces, __FILE__, __LINE__),0,0);

      if (isset($dept_cache[$data[DEPARTMENTCODE]]))
        $dept_id = $dept_cache[$data[DEPARTMENTCODE]];
      else
        $dept_id = $dept_cache[$data[DEPARTMENTCODE]] = (int)pg_result(pg_query("
          SELECT department_update({$sdata[DEPARTMENTCODE]},{$sdata[DEPARTMENT]})
        ", $wces, __FILE__, __LINE__),0,0);

      if (isset($div_cache[$divstr]))
        $div_id = $div_cache[$divstr];
      else
        $div_id = $div_cache[$divstr] = (int)pg_result(pg_query("
          SELECT division_update({$sdata[DIVISIONCODE]},{$sdata[DIVISIONSCODE]},{$sdata[DIVISION]})
        ", $wces, __FILE__, __LINE__),0,0);

      $course_id = pg_result(pg_query("SELECT course_update($subject_id,{$sdata[COURSECODE]},{$sdata[DIVISIONSCODE]},{$sdata[COURSE]},NULL)", $wces, __FILE__, __LINE__),0,0);
      if (!$course_id) die("course_id not found for course {$data[SUBJECTCODE]} {$sdata[COURSECODE]} on row $row");

      $cname = (!$data[CLASSNAME] || $data[CLASSNAME] == $data[COURSE]) ? "NULL" : quot($data[CLASSNAME]);
      $class_id  = pg_result(pg_query("SELECT class_update($course_id,{$sdata[CLASSSECTION]},{$sdata[YEAR]},$semester,$cname,{$sdata[TIME]},{$sdata[LOCATION]},{$sdata[STUDENTS]},{$sdata[CALLNUMBER]},$dept_id,$div_id,$school_id)", $wces, __FILE__, __LINE__),0,0); //cache
      if (!$class_id) die("class_id not found for class {$data[SUBJECTCODE]} {$sdata[COURSECODE]} {$sdata[CLASSSECTION]} on row $row");

      $professors = parse_csv_professor($pstr);
      
      if (count($professors) > 0)
        pg_query("DELETE FROM enrollments WHERE class_id = $class_id AND status = 3", $wces,__FILE__,__LINE__); 
      
      foreach($professors as $p)
      {
        $prof = adarray_map("nullquot", $p);
        $user_id = (int)pg_result(pg_query("SELECT professor_find($prof[full], $prof[first], $prof[middlei], $prof[last], NULL, 1)", $wces, __FILE__, __LINE__),0,0) ;   
        if (!$user_id) die("user_id not found for professor $prof[last] on row $row");
        pg_query("INSERT INTO enrollments (user_id, class_id, status, lastseen) VALUES ($user_id, $class_id, 3, NULL)", $wces, __FILE__, __LINE__); 
        if ($p['url'])
          pg_query("SELECT professor_data_update($user_id,$prof[url],NULL,NULL,NULL,NULL)", $wces, __FILE__, __LINE__);
        if ($p['email'])
          pg_query("UPDATE users SET email = COALESCE(email,$prof[email]) WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
        
      }

      print($data[SUBJECTCODE].$data[COURSECODE]." Section ".$data[CLASSSECTION]." ".$data[SEMESTER]." ".$data[YEAR]." (class_id = $class_id, course_id = $course_id,professor_id = $user_id)<br>\n");
    };
    if (((++$i) % 10) == 1) flush();
  }
  fclose ($fp);
};

function prof_fixname(&$first,&$last)
{
  $first = ucwords(strtolower(trim($first)));
  $last = ucwords(strtolower(trim($last)));
  if ($last == "Faculty")
    { $first = ""; $last = ""; }
  else if ($first == ".")
    $first = "";
}

function prof_parsemiddlei(&$first,&$middlei)
{
  $l = strlen($first);
  if ($l > 1 && $first[$l-2] == " ")
  {
    $middlei = $first[$l-1];
    $first = substr($first,0,$l-2);
  }
  else
    $middlei = "";
}

function prof_parsefull(&$name, &$first, &$middlei, &$last)
{
  $name = trim($name);
  $p = strrpos($name,' ');
  if ($p === false)
  {
    $first = $middlei = "";
    $last = $name;
  }
  else
  {
    $first = substr($name,0,$p);
    $last = substr($name,$p+1);
    prof_parsemiddlei($first,$middlei);
  }
}

function prof_parsepidname($name,&$first,&$last,&$middlei,&$dept)
{
  $first = ""; $last = ""; $middlei = ""; $dept = "";
  $find = strpos($name,",");
  if ($find === false) return false;
  $last = substr($name,0,$find);
  $first = substr($name,$find+1);
  $find = strpos($first," pid=");
  if (!($find === false)) $first = substr($first,0,$find);
  $find = strpos($first," dept=");
  if (!($find === false)) $first = substr($first,0,$find);
  $find = strpos($last," pid=");
  if (!($find === false)) $last = substr($last,$find + 5);
  $find = strpos($name," dept=");
  $dept = ($find === false) ? "" : substr($name,$find+5,4);
  prof_fixname($first,$last);
  prof_parsemiddlei($first,$middlei);
  return true;
}

function parse_csv_professor($pstr)
{
  $professors = array();
  $strs = explode("<br>", $pstr);
  
  foreach($strs as $str)
  {
    if (!$str) continue;
    
    $email = $url = ""; $full = $str; 
    if (ereg("([^-<>]*) -.*",$str,$result))
    {
      $full = $result[1];
      if (ereg(".*<a href=\"mailto:([^<>]+)\">.*",$str,$result))
        $email = $result[1];
      if (ereg(".*<a [^>]*href=\"?(http[^>]*)\"?>",$str,$result))
        $url = $result[1];
    };    
    
    prof_parsefull($full, $first, $middlei, $last);
    
    if (!$last)
      sendwarning("Unable to professor informationparse:\n\n  $str\n\nin:\n\n  $pstr", __FILE__, __LINE__);
    else if (strtolower($last) != "faculty")
      $professors[] = compact("full", "first", "middlei", "last", "email", "url");
  }
  return $professors;
};

function prof_makefull($first,$middlei,$last)
{
  return trim($first . " " . trim($middlei . " " . $last));
}

// ----------------------------------------------------------------------------
// This function imports data from registrar pid files
//
// todo: broken function. make it work on the new database

function importregpid($filename)
{
  return 0;
  global $db_debug, $wces_verbose;

  taskwindow_start("Progress Window");
  taskwindow_cprint("<h2>Importing Professor List</h2>\n");
  taskwindow_flush();

  $fp = fopen($filename,"r");
  $db = wces_connect();

  $i = 0;
  $begin = "<tr><td>";
  $middle = "</td><td>";
  $end = "</td></tr>";

  while (!feof ($fp))
  {
    $buffer = fgets($fp, 4096);
    $b = strpos($buffer, $begin);
    $m = strpos($buffer, $middle, $b);
    $e = strpos($buffer, $end, $m);

    if ($b === false || $m === false || $e === false )
      taskwindow_cprint("Skipping malformed row: '" . htmlspecialchars($buffer) . "'<br>\n");
    else
    {
      $name = substr($buffer,$b + strlen($begin), $m - strlen($begin) - $b);
      $regid = substr($buffer, $m + strlen($middle), $e - strlen($middle) - $m);

      prof_parsepidname($name,$first,$last,$middlei,$dept);
      $full = prof_makefull($first,$middlei,$last);

      $pids = prof_findwithfirstlast($db,$first,$last);
      $pids = array_merge($pids,prof_findwithfull($db,$full));
      $pids = array_merge($pids,prof_findwithpid($db,$regid));
      $pid = prof_merge($db,$pids,Array("name" => $full),Array("first" => $first, "middle" => $middlei, "last" => $last, "fullname" => $full, "pid" => $regid, "source" => "regpid"));

      taskwindow_cprint("Adding Professor '$full' as #$pid<hr>\n");
    }
    if (((++$i) % 10) == 1) taskwindow_flush();
  }
  fclose ($fp);
  taskwindow_end();
};

/**
* Import one of the dean's excel spreadsheets after they've been converted to csv
*
* @param string  Path to uploaded file
* @param string  4 digit year
* @param string  1 digit semester (1 = spring, 2 = summer, 3 = summer)
* @param array   Associative array which has integer column numbers at these values:
*                  "coursecode" - column number of 9 digit course code (eg 'COMSW3203')
*                  "section" - column number of 3 digit section number (eg '001', '002', 'R01')
*                  "professor" - column number of professor name in Last, First format
*                  "students" - column number containing number of students per class
*                  "cunix" - optional column number containing a student's cunix id
* @param integer If this parameter is not 0, the courses in this file will be
*                associated with the specified question set during upcoming
*                question periods.
* @access public
*/

function importdeancsv($userfile, $year, $semester, $fieldorder = array("coursecode" => 0, "section" => 1, "professor" => 2, "students" => 3), $questionsetid = 0)
{
  $classcache = array();

  if (strlen($year) == 4 && strlen($semester) && ($semester == 1 || $semester == 2 || $semester == 3) && is_array($fieldorder) && isset($fieldorder["coursecode"]) && isset($fieldorder["section"]) && isset($fieldorder["professor"]) && isset($fieldorder["students"]))
  {
    $fp = fopen($userfile,"r");
    $db = wces_connect();

    $i = 0;
    $row = 0;

    while ($data = fgetcsv ($fp, 8192, ","))
    {
      ++$row;
      if (count($data) != count($fieldorder))
        taskwindow_cprint("<b>Warning:</b> Row $row does not contain the correct number of fields. (" . count($fieldorder) . "expected, " . count($data) . " found)<br>\n");
      else
      {
        $coursecode = $data[$fieldorder["coursecode"]];
        $section = $data[$fieldorder["section"]];
        if (strlen($coursecode) == 9 && strlen($section) == 3)
        {
          $classcode = "${coursecode}_${section}_${year}_${semester}";
          if (isset($classcache[$classcode]))
          {
            $classid = $classcache[$classcode];
          }
          else
          {
            $classid = class_update($db, $classcode, $courseid);
            $classcache[$classcode] = $classid;
            db_exec("DELETE FROM enrollments WHERE classid = '$classid'", $db, __FILE__, __LINE__);
            prof_parsepidname($data[$fieldorder["professor"]],$first,$last,$middlei,$dept);
            $pids = prof_findwithfirstlast($db, $first, $last);
            $pid = prof_merge($db, $pids,Array("name" => prof_makefull($first,$middlei,$last)),Array("first" => $first, "middle" => $middlei, "last" => $last, "source" => "oldclasses"));
            $i = Array("professorid" => $pid);
            if ($data[$fieldorder["students"]]) $i["students"] = $data[$fieldorder["students"]];
            db_updatevalues($db, "classes", Array("classid" => $classid), $i);
            //if ($questionsetid) db_replace($db,"groupings",Array("linkid" => $courseid, "linktype" => "courses", "questionsetid" => $questionsetid), 0, "questionsetid");
          }
          $cunix = $data[$fieldorder["cunix"]];
          if ($cunix)
          {
            $userid = db_replace($db,"users", Array("cunix" => $cunix), false, "userid");
            if ($userid) db_exec("REPLACE INTO enrollments(classid, userid) VALUES ('$classid','$userid')",$db,__FILE__,__LINE__);
            taskwindow_cprint("<hr>");
          }
          else
            $userid = "";
          taskwindow_cprint("$coursecode Section $section added (courseid = $courseid, classid = $classid, professorid = $pid, userid = $userid)<br>\n");
        }
        else
          taskwindow_cprint("<b>Warning:</b> Invalid course or section code (course = '$coursecode' && section = '$section')<br>\n");
      };
      if (((++$i) % 10) == 1) taskwindow_flush();
    }
    fclose ($fp);
    taskwindow_end();
  }
};


?>

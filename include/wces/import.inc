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

/**
* Import a regripper registrar file. If a year and semester are specified, all 
* class listings for other year or semesters will be skipped.
*
* @param string  Path to uploaded file
* @param string  4 digit year
* @param string  1 digit semester (0 = spring, 1 = summer, 2 = summer)
*/

function importRegCsv($filename, $onlyYear = false, $onlySemester = false)
{
  global $wces;

  $fp = fopen ($filename,"r");
  wces_connect();

  $row = 0;

  $subject_cache = $school_cache = $div_cache = $dept_cache = array();

  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != 19 && count($data) != 16)
    {
      bprint("<b>Warning:</b> Row $row does not contain the correct number of fields. (19 expected, " . count($data) . " found)<br>\n");
      continue;
    }
    else
    {
      $semester = $data[SEMESTER] == 'spring' ? 0 : ($data[SEMESTER] == 'summer' ? 1 : 2);
      
      if ($onlyYear !== false && $onlyYear != $data[YEAR])
        continue;
        
      if ($onlySemester !== false && $onlySemester != $semester)
        continue;

      // XXX: our database stores course numbers as integers while the registrar
      // apparently stores them as strings of 4 characters. our database
      // should be changed to handle these cases, but in the meantime this import
      // script will skip over them:
      if (!is_numeric($data[COURSECODE]))
      {
         bprint("<i>Skipping {$data[SUBJECTCODE]} {$data[DIVISIONSCODE]} {$data[COURSECODE]}</i>\n");
         continue;
      }

      $divstr = "${data[DIVISIONCODE]},{$data[DIVISIONSCODE]},{$data[DIVISION]}";
      $pstr = $data[INSTRUCTOR];
      $sdata = array_map("mnullquot",$data);

      if (isset($subject_cache[$data[SUBJECTCODE]]))
        $subject_id = $subject_cache[$data[SUBJECTCODE]];
      else
        $subject_id = $subject_cache[$data[SUBJECTCODE]] = (int)pg_result(pg_go("
          SELECT subject_update({$sdata[SUBJECTCODE]},{$sdata[SUBJECT]},'f')
        ", $wces, __FILE__, __LINE__),0,0);

      if (!$subject_id) die("subject_id not found for subject {$data[SUBJECTCODE]} on row $row");

      if (isset($school_cache[$data[SCHOOL]]))
        $school_id = $school_cache[$data[SCHOOL]];
      else
        $school_id = $school_cache[$data[SCHOOL]] = (int)pg_result(pg_go("
          SELECT school_update({$sdata[SCHOOL]})
        ", $wces, __FILE__, __LINE__),0,0);

      if (isset($dept_cache[$data[DEPARTMENTCODE]]))
        $dept_id = $dept_cache[$data[DEPARTMENTCODE]];
      else
        $dept_id = $dept_cache[$data[DEPARTMENTCODE]] = (int)pg_result(pg_go("
          SELECT department_update({$sdata[DEPARTMENTCODE]},{$sdata[DEPARTMENT]})
        ", $wces, __FILE__, __LINE__),0,0);

      if (isset($div_cache[$divstr]))
        $div_id = $div_cache[$divstr];
      else
        $div_id = $div_cache[$divstr] = (int)pg_result(pg_go("
          SELECT division_update({$sdata[DIVISIONCODE]},{$sdata[DIVISIONSCODE]},{$sdata[DIVISION]})
        ", $wces, __FILE__, __LINE__),0,0);

      $course_id = pg_result(pg_go("SELECT course_update($subject_id,{$sdata[COURSECODE]},{$sdata[DIVISIONSCODE]},{$sdata[COURSE]},NULL)", $wces, __FILE__, __LINE__),0,0);
      if (!$course_id) die("course_id not found for course {$data[SUBJECTCODE]} {$sdata[COURSECODE]} on row $row");

      $cname = (!$data[CLASSNAME] || $data[CLASSNAME] == $data[COURSE]) ? "NULL" : quot($data[CLASSNAME]);
      $class_id  = pg_result(pg_go("SELECT class_update($course_id,{$sdata[CLASSSECTION]},{$sdata[YEAR]},$semester,$cname,{$sdata[TIME]},{$sdata[LOCATION]},{$sdata[STUDENTS]},{$sdata[CALLNUMBER]},$dept_id,$div_id,$school_id)", $wces, __FILE__, __LINE__),0,0); //cache
      if (!$class_id) die("class_id not found for class {$data[SUBJECTCODE]} {$sdata[COURSECODE]} {$sdata[CLASSSECTION]} on row $row");

      $professors = parse_csv_professor($pstr);
      
      if (count($professors) > 0)
        pg_go("DELETE FROM enrollments WHERE class_id = $class_id AND status = 3", $wces,__FILE__,__LINE__); 
      
      foreach($professors as $p)
      {
        $prof = adarray_map("nullquot", $p);
        $user_id = (int)pg_result(pg_go("SELECT professor_find($prof[full], $prof[first], $prof[middlei], $prof[last], NULL, 1)", $wces, __FILE__, __LINE__),0,0) ;   
        if (!$user_id) die("user_id not found for professor $prof[last] on row $row");
        pg_go("INSERT INTO enrollments (user_id, class_id, status, lastseen) VALUES ($user_id, $class_id, 3, NULL)", $wces, __FILE__, __LINE__); 
        if ($p['url'])
          pg_go("SELECT professor_data_update($user_id,$prof[url],NULL,NULL,NULL,NULL)", $wces, __FILE__, __LINE__);
        if ($p['email'])
          pg_go("UPDATE users SET email = COALESCE(email,$prof[email]) WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
        
      }

      bprint($data[SUBJECTCODE].$data[COURSECODE]." Section ".$data[CLASSSECTION]." ".$data[SEMESTER]." ".$data[YEAR]." (class_id = $class_id, course_id = $course_id,professor_id = $user_id)<br>\n");
    };
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

  print("<h2>Importing Professor List</h2>\n");
  flush();

  $fp = fopen($filename,"r");
  $db = wces_connect();

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
      bprint("Skipping malformed row: '" . htmlspecialchars($buffer) . "'<br>\n");
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

      bprint("Adding Professor '$full' as #$pid<hr>\n");
    }
  }
  fclose ($fp);
};

/**
* Import one of the dean's excel spreadsheets after they've been converted to csv
*
* @param string  Path to uploaded file
* @param string  4 digit year
* @param string  1 digit semester (1 = spring, 2 = summer, 3 = fall)
* @param integer parent topic
* @param integer question period
* @param array   array of strings indicating column order.
*                  "coursecode" - column number of 9 digit course code (eg 'COMSW3203')
*                  "section" - column number of 3 digit section number (eg '001', '002', 'R01')
*                  "lastname" - column number of professor last name
                   "firstname" - column number of professor first name
*                  "students" - column number containing number of students per class
*                  "cunix" - optional column number containing a student's cunix id
*                  "cruft" - column whose contents should be ignored
* @access public
*/

function importDeanCsv($userfile, $year, $semester, $parent_topic, $category_id, $question_period_id, $fieldorder = array("cunix", "coursecode", "section", "lastname", "firstname", "students"), $time = NULL)
{
  global $wces;

  if (strlen($year) != 4 || strlen($semester) != 1 || ($semester != 1 && $semester != 2 && $semester != 3))
    die("Invalid year or semester passed to importdeancsv in " . __FILE__ . ":" . __LINE__);

  $date = pg_date($time);
  
  $fields = array_flip(array_values($fieldorder));
  
  if (!isset($fields['coursecode']) || !isset($fields['section']))
    die("Invalid field order passed to importdeancsv in" . __FILE__ . ":" . __LINE__);
 
  $fp = fopen($userfile,"r");   
  
  $classcache = array();

  $row = 0;
  
  $gparent_topic = NULL;
  if ($parent_topic)
  {
    $r = pg_go("SELECT parent FROM topics WHERE topic_id = $parent_topic", $wces, __FILE__, __LINE__);
    $gparent_topic = pg_result($r, 0, 0);
  }
  
  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != count($fieldorder))
    {
      bprint("<b>Warning:</b> Row $row does not contain the correct number of fields. (" . count($fieldorder) . " expected, " . count($data) . " found)<br>\n");
      continue;
    }
    $coursecode = $data[$fields["coursecode"]];
    $section = $data[$fields["section"]];
    if (strlen($coursecode) != 9 && strlen($section) != 3)
      die("<b>Warning:</b> Invalid course or section code (course = '$coursecode' && section = '$section') on row $row<br>\n");
    
    $classcode = "${coursecode}_${section}_${year}_${semester}";
    if (isset($classcache[$classcode]))
      $class_id = $classcache[$classcode];
    else
    {
      $sclasscode = "'" . addslashes($classcode) . "'";
      $result = pg_go("SELECT class_find($sclasscode)", $wces, __FILE__, __LINE__);
      if (!$result) die("Failed to find class_id for $classcode on row $row");
      $classcache[$classcode] = $class_id = (int)pg_result($result,0,0);
      
      $puser_id = NULL;
      if (isset($fields["lastname"]) && isset($fields["firstname"]))
      {
        $firstname = $data[$fields["firstname"]];
        $lastname = $data[$fields["lastname"]];
        
        if ($firstname || $lastname)
        {
          pg_go("
            DELETE FROM enrollments WHERE class_id = $class_id AND status = 3
              AND (lastseen IS NULL OR lastseen <= $date)
          ", $wces,__FILE__,__LINE__);       

          $fullname = quot(trim("$firstname $lastname"));
          $firstname = quot($firstname);
          $lastname = quot($lastname);
          
          $puser_id = (int)pg_result(pg_go("SELECT professor_find($fullname, $firstname, NULL, $lastname, NULL, 4)", $wces, __FILE__, __LINE__),0,0) ;   
          if (!$puser_id) die("user_id not found for professor $fullname on row $row");
          pg_go("SELECT enrollment_update($puser_id, $class_id, 3, $date)", $wces, __FILE__, __LINE__); 
        }
        
      }

      if (isset($fields["cunix"]))
        pg_go("
          UPDATE enrollments SET status = 0
          WHERE class_id = $class_id AND status = 1 
            AND (lastseen IS NULL OR lastseen <= $date)
        ", $wces, __FILE__, __LINE__);
        
      $students = isset($fields["students"]) ? (int)$data[$fields["students"]] : 0;
      if ($students)
        pg_go("UPDATE classes set students = $students WHERE class_id = $class_id", $wces, __FILE__, __LINE__);
      
      bprint("<p>$classcode added (class_id = $class_id, prof user_id = $puser_id)</p>\n");
      $GLOBALS['db_debug'] = 1;
      if ($parent_topic)
      {
        $result = pg_go("SELECT parent, topic_id FROM wces_topics WHERE class_id = $class_id", $wces, __FILE__, __LINE__);
        if (pg_numrows($result) == 1)
        {
          $parent = (int)pg_result($result, 0, 0);
          $topic_id = (int)pg_result($result, 0, 1);
          
          pg_go("UPDATE wces_topics SET category_id = $category_id WHERE topic_id = $topic_id", $wces, __FILE__, __LINE__);
          
          if ($parent == $gparent_topic)
          {
            $r = pg_go("
              UPDATE wces_topics SET 
                category_id = $category_id, parent = $parent_topic
              WHERE topic_id = $topic_id;
              
              SELECT branch_id, base_branch_id FROM branches
              WHERE topic_id = $topic_id AND parent IS NOT NULL
            ", $wces, __FILE__, __LINE__);

            $n = pg_numrows($r);

            for ($i = 0; $i < $n; ++$i)
            {
              $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
              $s = pg_go("
                SELECT branch_id FROM branches
                WHERE topic_id = $parent_topic
                  AND base_branch_id = $row[base_branch_id]
              ", $wces, __FILE__, __LINE__);
              
              if (pg_numrows($s) == 1)
              {
                $pbranch = pg_result($s, 0, 0); 
                pg_go("SELECT move_branch($row[branch_id], $pbranch)", $wces, __FILE__, __LINE__);  
              }

            }

          }
          else if ($parent != $parent_topic)
          {
            bprint("<h1>Unable to move topic $parent.$topic_id to $gparent_topic.$parent_topic</h1>\n"); 
          }  
        }
        else
        {        
          $result = pg_go("
            INSERT INTO wces_topics(parent, class_id, category_id) values ($parent_topic, $class_id, $category_id);
            SELECT currval('topic_ids');
          ", $wces, __FILE__, __LINE__);
          $topic_id = (int)pg_result($result, 0, 0);
        }
      }

      $GLOBALS['db_debug'] = 0;

      if ($question_period_id)
      {
        $result = pg_go("SELECT EXISTS (SELECT * FROM question_periods_topics WHERE question_period_id = $question_period_id AND topic_id = $topic_id)", $wces, __FILE__, __LINE__);
        if (pg_result($result, 0, 0) == 'f')
          pg_go("INSERT INTO question_periods_topics(topic_id, question_period_id) values ($topic_id, $question_period_id)", $wces, __FILE__, __LINE__);
      }
    }
    
    if (isset($fields["cunix"]))
    {
      $cunix = $data[$fields["cunix"]];
      if ($cunix) 
      {
        $cunix = "'" . addslashes($cunix) . "'";
        $user_id = (int)pg_result(pg_go("SELECT user_update($cunix,NULL,NULL,NULL,8,NULL,NULL)", $wces, __FILE__, __LINE__),0,0);
        if (!$user_id) die("user_id not found for $cunix on row $row");
        pg_go("SELECT enrollment_update($user_id, $class_id, 1, $date)", $wces, __FILE__, __LINE__); 
        bprint("$cunix enrolled in $classcode (class_id = $class_id, user_id = $user_id)<br>\n");
      }
    }
  }
  fclose ($fp);
};

/**
* Import one of the dean's professor listing excel spreadsheets after they've been converted to csv
*
* @param string  Path to uploaded file
* @param array   array of strings indicating column order.
*                  "firstname", "lastname", "cunix", "email", "cruft"
* @access public
*/

function importProfessors($userfile, $fieldorder = array("lastname", "firstname", "email", "cunix"))
{
  global $wces, $wces_path;
  
  $fields = array_flip(array_values($fieldorder));
  
  if (!isset($fields['lastname'])) die(__FILE__ . ':' . __LINE__ . ' missing lastname column');
  if (!isset($fields['firstname'])) die(__FILE__ . ':' . __LINE__ . ' missing firstname column');
  
  $fp = fopen($userfile,"r");   
  wces_connect();
  
  $row = 0;

  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != count($fields))
      print("<b>Warning:</b> Row $row does not contain the correct number of fields. (" . count($fieldorder) . "expected, " . count($data) . " found)<br>\n");

    $firstname = trim($data[$fields["firstname"]]);
    $lastname = trim($data[$fields["lastname"]]);
    
    if (!$firstname &&  !$lastname) continue;
   
    $fullname = quot(trim("$firstname $lastname"));
    $firstname = quot($firstname);
    $lastname = quot($lastname);
    $user_id = (int)pg_result(pg_go("SELECT professor_find($fullname, $firstname, NULL, $lastname, NULL, 4)", $wces, __FILE__, __LINE__),0,0) ;
    if (!$user_id) die("user_id not found for professor $fullname on row $row");

    if (isset($fields["cunix"]) && $cunix = trim($data[$fields["cunix"]]))
    {
      $result = pg_go("SELECT user_id FROM users WHERE uni = '" . addslashes($cunix) . "'", $wces, __FILE__, __LINE__);
      $cunix_user_id = pg_numrows($result) == 1 ? (int)pg_result($result,0,0) : 0;
      if ($cunix_user_id != $user_id) 
      {
        if ($cunix_user_id)
        {
          bprint("Conflict between users <a href=\"{$wces_path}administrators/info.php?user_id=$user_id\" target=w1>$user_id</a>, <a href=\"{$wces_path}administrators/info.php?user_id=$cunix_user_id\" target=w2>$cunix_user_id</a> . they might need to be merged <br>");
        }
        else
          pg_go("UPDATE users SET uni = '" . addslashes($cunix) . "' WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
      }
    }
    else 
      $cunix_user_id = 0;
    
    if (isset($fields["email"]) && $email = trim($data[$fields["email"]]))
      pg_go("UPDATE users SET email = '" . addslashes($email) . "' WHERE user_id = $user_id", $wces, __FILE__, __LINE__);

    bprint("$firstname $lastname (user_id = $user_id, cunix = $cunix, email = $email, cunix_user_id = $cunix_user_id)<br>\n");
  }
  fclose($fp);
};


?>

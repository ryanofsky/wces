<%

require_once("wces/server.inc");
require_once("wces/database.inc");
require_once("wces/wces.inc");
require_once("wces/taskwindow.inc");

// CSV Columns
define ("COURSE",0);             // course name
define ("COURSECODE",1);         // 4 digit course number
define ("DEPARTMENT",2);         // department name
define ("DEPARTMENTCODE",3);     // 4 letter department abbreviation. Not on page, taken from department hash table deptlist
define ("SUBJECT",4);            // full subject name
define ("SUBJECTCODE",5);        // 4 letter subject abbreviation
define ("DIVISION",6);           // division name
define ("DIVISIONCODE",7);       // 1 or 2 letter division code
define ("DIVISIONSCODE",8);      // 1 letter division code
define ("CLASSNAME",9);          // class name
define ("CLASSSECTION",10);      // 3 digit section ie  '001', '002', 'R01'
define ("YEAR",11);              // four digit year
define ("SEMESTER",12);          // 'spring', 'fall', 'summer'
define ("INSTRUCTOR",13);        // full name
define ("STUDENTS",14);          // number of students
define ("SCHOOL",15);            // school name
define ("TIME",16);              // class time
define ("LOCATION",17);          // class location
define ("CALLNUMBER",18);        // 5 digit call number

function array_clean($arr)
{
  foreach($arr as $key => $value)
    if (!$value) unset($arr[$key]);
  return $arr;
}

function importregcsv($filename)
{
  taskwindow_start("Progress Window");
  taskwindow_cprint("<h2>Importing Registrar Data</h2>\n");
  taskwindow_flush();

  $fp = fopen ($filename,"r");
  $db = wces_connect();
  
  $i = 0;
  $row = 0;
  
  while ($data = fgetcsv ($fp, 8192, ","))
  {
    ++$row;
    if (count($data) != 19 && count($data) != 16)
    {
      taskwindow_cprint("<b>Warning:</b> Row $row does not contain the correct number of fields. (19 expected, " . count($data) . " found)<br>\n");
    }
    else
    {
      $cname = (!$data[CLASSNAME] || $data[CLASSNAME] == $data[COURSE]) ? "" : $data[CLASSNAME];
      $schoolid     = db_replace($db, "Schools",     Array("name" => $data[SCHOOL]),0,"schoolid");
      $subjectid    = db_replace($db, "Subjects",    Array("code" => $data[SUBJECTCODE]), Array("name" => $data[SUBJECT]),"subjectid");
      $divisionid   = db_replace($db, "Divisions",   Array("name" => $data[DIVISION]), array_clean(Array("code" => $data[DIVISIONCODE], "shortcode" => $data[DIVISIONSCODE])), "divisionid");
      $departmentid = db_replace($db, "Departments", Array("code" => $data[DEPARTMENTCODE]), array_clean(Array("name" => $data[DEPARTMENT])), "departmentid");
      $courseid     = db_replace($db, "Courses",     Array("subjectid" => $subjectid, "code" => $data[COURSECODE]), Array("departmentid" => $departmentid, "schoolid" => $schoolid, "name" => $data[COURSE]),"courseid");
      $classid      = db_replace($db, "Classes",     Array("courseid" => $courseid, "section" => $data[CLASSSECTION], "divisioncode" => $data[DIVISIONSCODE], "year" => $data[YEAR], "semester" => $data[SEMESTER]), Array("name" => $cname, "students" => $data[STUDENTS], "time" => $data[TIME], "location" => $data[LOCATION], "callnumber" => $data[CALLNUMBER], "departmentid" => $departmentid, "divisionid" => $divisionid), "classid");
      prof_parserrname($data[INSTRUCTOR],$full,$email,$url);
      $pids = prof_findwithfull($db,$full);
      //$pids = array_merge($pids,prof_findwithclassid($db,$classid));
      $pid = prof_merge($db, $pids,Array("name" => $full, "email" => $email, "url" => $url),Array("fullname" => $full, "source" => "regweb"));
      db_updatevalues($db, "Classes", Array("classid" => $classid), Array("professorid" => $pid));
      taskwindow_cprint($data[SUBJECTCODE].$data[COURSECODE]." Section ".$data[CLASSSECTION]." ".$data[SEMESTER]." ".$data[YEAR]." (classid = $classid, courseid = $courseid,professorid = $pid)<br>\n");
      if (!$data[COURSE]) taskwindow_cprint("<b>Warning:</b> Coursename not specified in row $row<br>\n");
      if (!$subjectid || !$classid || !$courseid) taskwindow_cprint("<b>Warning:</b> Insertion error in row $row<br>\n");
      //if (0 + $data[COURSECODE] < 1000) taskwindow_cprint("<b>Warning:</b> Invalid course number in row $row<br>\n");
    };
    if (((++$i) % 10) == 1) taskwindow_flush();  
  }
  fclose ($fp); 
  taskwindow_end("Progress Window");   
};


function arrayagrees($base,$newbie)
{
  foreach($newbie as $key => $value)
    if ($base[$key] && $base[$key] != $value) return false;
  return true;  
}

function prof_merge($db,$professorids,$newprofessordata,$newdupedata,$forcemerge = false)
{
  global $db_debug;
  
  $professorids = array_values(array_unique($professorids)); // eliminate duplicates in $professorids
  
  // unless forcemerge is defined, do not merge different professors that come from the same source
  // this can be rewriten to be more efficient
  if (!$forcemerge)  
  {
    $pids = array();
    $sources = array();
    if (is_array($newdupedata)) $sources[$newdupedata["source"]] = $newdupedata;
    foreach($professorids as $professorid)
    {
      $result = db_exec("SELECT source, first, middle, last, fullname, pid FROM ProfessorDupeData WHERE professorid = '$professorid' AND source IS NOT NULL GROUP BY source",$db,__FILE__,__LINE__);
      while($row = mysql_fetch_assoc($result))
      {
        $s = $row["source"];
        if (!is_array($sources[$s]))
          $sources[$s] = $row;
        else if (!arrayagrees($sources[$s],$row))
          continue(2);
      }
      array_push($pids,$professorid);
    }
    $professorids = $pids;
  };

  if (count($professorids) == 0) // just add the new values if there is nothing to merge
  {
    if (is_array($newprofessordata) && is_array($newdupedata) && $newprofessordata["name"])
    {
      $professorid = db_addrow($db,"Professors",$newprofessordata);
      $newdupedata["professorid"] = $professorid;
      db_addrow($db,"ProfessorDupeData",$newdupedata);
      return $professorid;
    }
    else  
      return false;
  };

  sort($professorids);
  $profcondition = "professorid IN " . db_set($professorids);

  // if there is a professor entry whose data came from the oracle database
  // then that entry's information is chosen so it will not be overwritten
  // otherwise the first professorid is chosen
  
  $result = db_exec("SELECT professorid FROM ProfessorDupeData WHERE source = 'oracle' AND $profcondition GROUP BY professorid",$db,__FILE__,__LINE__);
  $rows = mysql_num_rows($result);
  if ($rows == 1)
  { 
    $prow = mysql_fetch_assoc($result);
    $chosen = $prow["professorid"];
  }  
  else
    $chosen = $professorids[0];
  mysql_free_result($result);  
  
  // get chosen row data 
  $chosenrow = db_getrow($db,"Professors",Array("professorid" => $chosen),0);
  $chosenchanged = false;
    
  // get the other rows, copy their data, clean up and delete
  $result = db_exec("SELECT * FROM Professors as p WHERE $profcondition",$db,__FILE__,__LINE__);
  while ($professor = mysql_fetch_assoc($result))
    if ($professor["professorid"] != $chosen)
    {
      foreach($professor as $key => $value)
        if ($value && !$chosenrow[$key]) { $chosenrow[$key] = $value; $chosenchanged = true;  };
      db_updatevalues($db,"Classes",           Array("professorid" => $professor["professorid"]), Array("professorid" => $chosen));
      db_updatevalues($db,"ProfessorDupeData", Array("professorid" => $professor["professorid"]), Array("professorid" => $chosen));
      db_updatevalues($db,"Groupings",         Array("linkid" => $professor['professorid'], "linktype" => "professors"), Array("linkid" => $chosen));
      db_delete($db,"Professors", Array("professorid" => $professor["professorid"]));
    };    
  mysql_free_result($result);

  // add new data
  if (is_array($newprofessordata))
    foreach($newprofessordata as $key => $value)
      if ($value && !$chosenrow[$key]) { $chosenrow[$key] = $value; $chosenchanged = true; };  

  if ($chosenchanged)
  {
    if (!$chosenrow["userid"]) $chosenrow["userid"] = false;
    db_updatevalues($db,"Professors",Array("professorid" => $chosen),$chosenrow);  
  }  

  // add newdupedata if it is actually new
  if (is_array($newdupedata))
  {
    $newdupedata["professorid"] = $chosen;
    $result = db_select($db,"ProfessorDupeData",$newdupedata,0," LIMIT 1");
    if (mysql_num_rows($result) == 0)
      db_addrow($db,"ProfessorDupeData",$newdupedata);
    mysql_free_result($result);  
  }   
  
  return $chosen;    
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

function prof_fixfull(&$name)
{
  $name = ucwords(strtolower(trim($name)));
  if ($name == "Faculty" || $name == ". Faculty") $name = "";
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

function prof_parserrname($name,&$full,&$email,&$url)
{
  $full = $name;
  $email = "";
  $url = "";
  if (ereg("([^-<>]*) -.*",$name,$result))
  {
    $full = $result[1];
    if (ereg(".*<a href=\"mailto:([^<>]+)\">.*",$name,$result))
      $email = $result[1];
    if (ereg(".*<a [^>]*href=\"?(http[^>]*)\"?>",$data[INSTRUCTOR],$result))
      $url = $result[1];
  };
  prof_fixfull($full);
};

function prof_makefull($first,$middlei,$last)
{
  return trim($first . " " . trim($middlei . " " . $last));
}

function prof_findwithfull($db, $name)
{
  global $wces_verbose,$db_debug;
  
  $professorids = array();
  
  if ($name)
  {
    $result = db_exec("SELECT professorid FROM ProfessorDupeData WHERE fullname = '" . addslashes($name) . "' GROUP BY professorid",$db,__FILE__,__LINE__);
    while($row = mysql_fetch_assoc($result))
      array_push($professorids,$row['professorid']);
  }    
 
  if ($wces_verbose)
  {
    if (count($professorids) >0)
      taskwindow_cprint("Professor '$name' found<br>\n");
    else
      taskwindow_cprint("Professor '$name' not found<br>\n");
  }
  return $professorids;
};

function prof_findwithfirstlast($db, $first, $last)
{
  global $wces_verbose,$db_debug;
  $pids = Array();
  if ($last)
  {
    $result = db_exec("SELECT professorid FROM ProfessorDupeData WHERE first = '" . addslashes($first) . "' AND last = '" . addslashes($last) . "' GROUP by professorid",$db,__FILE__,__LINE__);
    while($row = mysql_fetch_assoc($result))
      array_push($pids,$row['professorid']);
  }    
  
  if ($wces_verbose)
  {
    if (count($pids) >0)
      taskwindow_cprint("Professor '$first $last' found<br>");
    else
      taskwindow_cprint("Professor '$first $last' not found<br>");
  }
  return $pids;
};

// Importing code should no longer use this function to identify professors
// to be merged. It causes problems when the professor for a class changes
function prof_findwithclassid($db, $classid)
{
  $pid = db_getvalue($db, "Classes", Array("classid" => $classid),"professorid");
  
  if ($wces_verbose)
  {
    if ($pid)
      taskwindow_cprint("Professor for class #$classid found<br>");
    else
      taskwindow_cprint("Professor for class #$classid not found<br>");
  }
  return $pid ? array($pid) : array();
};

function prof_findwithpid($db, $pid)
{
  global $wces_verbose,$db_debug;
  
  if (!$pid || $pid == "C000000000" || $pid == "C000000001") return array();
  
  $result = db_exec("SELECT professorid FROM ProfessorDupeData WHERE pid = '" . addslashes($pid) . "' GROUP BY professorid",$db,__FILE__,__LINE__);
  $professorids = array();
  while($row = mysql_fetch_assoc($result))
    array_push($professorids,$row['professorid']);
  if ($wces_verbose)
  {
    if (count($professorids) >0)
      taskwindow_cprint("Professor with PID #$pid found<br>");
    else
      taskwindow_cprint("Professor with PID #$pid not found<br>");
  }
  return $professorids;
};

// ----------------------------------------------------------------------------
// This function imports data from registrar pid files

function importregpid($filename)
{
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

// ----------------------------------------------------------------------------
// This code was used to populate the wces database with data from the old
// seaseval database.

$import_commentspath = $server_cgiroot . "comments/";
$import_questionpath = $server_cgiroot . "text_quest/";

function readresponses($filename)
{
  if (!is_file($filename))
  {
    taskwindow_cprint("File $filename not found.<br>");
    return "";
  };
  $f = fopen($filename,"r");
  $s = fread($f, 65536);
  fclose($f);
  $s = addcslashes($s,"\t\\");      // tabs are used as separators, escape them away
  $s = strtr($s, "\n","\t");        // convert old-style newline separators to tabs
  $s = str_replace("\r\t","\n",$s); // multiparagraph responses from Windows machines can be put back together 
  $s = trim($s);
  return $s;
};

function addelement(&$thearray,$key,$value)
{
  if ($value) $thearray[$key] = $value;
};

function importseaseval($doclasses,$doprofessors,$docourses,$doresponses)
{
  global $import_commentspath, $import_questionpath;
  taskwindow_start("Progress Window");
  
  taskwindow_cprint("<h2>Importing SEASEVAL Tables</h2>");
  taskwindow_flush();
  
  $i = 0;
  $db = generic_connect();

  if ($doclasses)
  {
    taskwindow_cprint("Transferring Classes...<br>\n");
    mysql_select_db("seaseval",$db);
    $result = mysql_query("SELECT * FROM classinfo",$db);
    mysql_select_db("wces",$db);
    while($row = mysql_fetch_assoc($result))
    {
      $classid = class_update($db,$row["classid"]);
      $firstname = $row["classproffname"];
      $lastname = $row["classproflname"];
      prof_fixname($firstname, $lastname);
      prof_parsemiddlei($firstname,$middlei);
      $full = prof_makefull($firstname,$middlei,$lastname);
      $pids = prof_findwithfirstlast($db,$firstname,$lastname);
      //$pids = array_merge($pids,prof_findwithclassid($db,$classid));
      $pid = prof_merge($db, $pids, Array("name" => $full), Array("first" => $firstname, "middle" => $middlei, "last" => $lastname, "fullname" => $full, "source" => "oldclasses"));
      db_updatevalues($db,"Classes",Array("classid" => $classid),Array("students" => $row["classenrollment"], "professorid" => $pid));
      taskwindow_cprint(" - Added " . $row["classid"] . " as class #$classid with professor #$pid<br>\n");
      if ((++$i) % 10 == 1) taskwindow_flush();
    };
  };
  
  if ($doprofessors)
  {
    taskwindow_cprint("Transferring Professors...<br>\n");
  
    mysql_select_db("seaseval",$db);
    $result = mysql_query("SELECT * FROM profinfo",$db);
    mysql_select_db("wces",$db);
    while($row = mysql_fetch_assoc($result))
    {
      $first = $row["proffname"];
      $last = $row["proflname"];
      prof_fixname($first,$last);
      $full = prof_makefull($first, "", $last);
      $pids = prof_findwithfirstlast($db,$first,$last);
      $pid = prof_merge($db, $pids,
        Array(
          "name" => $full,
          "departmentid" => db_getvalue($db,"Departments",Array("code" => $row["profdept"]),"departmentid"),
          "education" => $row["profeducation"],
          "profile" => $row["profprofile"],
          "statement" => $row["profstatement"],
          "url" => $row["profpageurl"],
          "picname" => $row["profpicurl"],
          "email" => $row["profemail"] ),
        Array(
          "first" => $first,
          "middle" => $middlei,
          "last" => $last,
          "fullname" => $full,
          "source" => "oracle" )
      );

      taskwindow_cprint(" - $last, $first as #$pid<br>\n");
      if ((++$i) % 10 == 1) taskwindow_flush();
    }
  }
  
  if ($docourses)
  {
    taskwindow_cprint("Transferring Courses...<br>\n");
    mysql_select_db("seaseval",$db);
    $result = mysql_query("SELECT * FROM courseinfo",$db);
    mysql_select_db("wces",$db);
    while($row = mysql_fetch_assoc($result))
    {
      $id = course_update($db,$row["courseid"]);
      $update = Array();
      addelement($update,"name",$row["coursename"]);
      addelement($update,"information",$row["coursedesc"]);
      db_updatevalues($db,"Courses",Array("courseid" => $id),$update);
      taskwindow_cprint(" - " . $row["courseid"] . " as #$id<br>\n");
      if ((++$i) % 10 == 1) taskwindow_flush();
    };
  };
  
  if ($doresponses)
  {
    taskwindow_cprint("Transferring Survey Responses...<br>\n");
    mysql_select_db("seaseval",$db);
    $generics = mysql_query("SELECT * FROM generic",$db);
    while($generic = mysql_fetch_assoc($generics))
    {
      $customs = mysql_query("SELECT * FROM evalquest WHERE genID=" . $generic["genID"],$db);
      while($custom = mysql_fetch_assoc($customs))
      {
        $responses = mysql_query("SELECT * FROM classevalresponse WHERE evalid=" . $custom["evalid"],$db);
        while($response = mysql_fetch_assoc($responses))
        {
          mysql_select_db("wces",$db);
          
          $classid = class_update($db,$response["classid"]);
          $classinfo = db_getrow($db,"Classes",Array("classid" => $classid),Array("year","semester"));
          extract($classinfo);

          $periodid = db_replace($db,"QuestionPeriods",Array
          (
            "description" => "SEASEVAL Imported Question Period",
            "year" => $year,
            "semester" => $semester,
            "periodstart" => false,
            "periodend" => false,
          ),0,"questionperiodid");

          if ($generic)
          {
            $genericid = db_replace($db,"QuestionSets",Array
            (
              "displayname" => "General SEAS Questions",
              "MC1" => $generic["Q1Text"],
              "MC2" => $generic["Q2Text"],
              "MC3" => $generic["Q3Text"],
              "MC4" => $generic["Q4Text"],
              "MC5" => $generic["Q5Text"],
              "MC6" => $generic["Q6Text"],
              "MC7" => $generic["Q7Text"],
              "MC8" => $generic["Q8Text"],
              "MC9" => $generic["Q9Text"],
              "MC10" => $generic["Q10Text"],
              "FR1" => "Comments",
              "type" => "public",
            ),0,"questionsetid");
            $generic = 0;
          };

          if ($custom)
          {
            if ($custom["evalQ1"] || $custom["evalQ2"] || $custom["evalQ3"] || $custom["evalQ4"] || $custom["evalQ5"] || $custom["evalQ6"] || $custom["evalQ7"])
            {
              $customid = db_replace($db,"QuestionSets",Array
              (
                "displayname" => "Custom Question Set",
                "MC1" => $custom["evalQ1"],
                "MC2" => $custom["evalQ2"],
                "MC3" => $custom["evalQ3"],
                "MC4" => $custom["evalQ4"],
                "MC5" => $custom["evalQ5"],
                "FR1" => $custom["evalQ6"],
                "FR2" => $custom["evalQ7"],
                "type" => "private"
              ),0,"questionsetid");
            }
            else
              $customid = 0;
            $custom = 0;
          };

          $genericaid = db_replace($db,"AnswerSets", Array("questionsetid" => $genericid,"classid" => $classid, "questionperiodid" => $periodid), Array(
            "responses" => $response["Q1A"] + $response["Q1B"] + $response["Q1C"] + $response["Q1D"] + $response["Q1E"],
            "MC1a" => $response["Q1A"], "MC1b" => $response["Q1B"], "MC1c" => $response["Q1C"], "MC1d" => $response["Q1D"], "MC1e" => $response["Q1E"],
            "MC2a" => $response["Q2A"], "MC2b" => $response["Q2B"], "MC2c" => $response["Q2C"], "MC2d" => $response["Q2D"], "MC2e" => $response["Q2E"],
            "MC3a" => $response["Q3A"], "MC3b" => $response["Q3B"], "MC3c" => $response["Q3C"], "MC3d" => $response["Q3D"], "MC3e" => $response["Q3E"],
            "MC4a" => $response["Q4A"], "MC4b" => $response["Q4B"], "MC4c" => $response["Q4C"], "MC4d" => $response["Q4D"], "MC4e" => $response["Q4E"],
            "MC5a" => $response["Q5A"], "MC5b" => $response["Q5B"], "MC5c" => $response["Q5C"], "MC5d" => $response["Q5D"], "MC5e" => $response["Q5E"],
            "MC6a" => $response["Q6A"], "MC6b" => $response["Q6B"], "MC6c" => $response["Q6C"], "MC6d" => $response["Q6D"], "MC6e" => $response["Q6E"],
            "MC7a" => $response["Q7A"], "MC7b" => $response["Q7B"], "MC7c" => $response["Q7C"], "MC7d" => $response["Q7D"], "MC7e" => $response["Q7E"],
            "MC8a" => $response["Q8A"], "MC8b" => $response["Q8B"], "MC8c" => $response["Q8C"], "MC8d" => $response["Q8D"], "MC8e" => $response["Q8E"],
            "MC9a" => $response["Q9A"], "MC9b" => $response["Q9B"], "MC9c" => $response["Q9C"], "MC9d" => $response["Q9D"], "MC9e" => $response["Q9E"],
            "MC10a" => $response["Q10A"], "MC10b" => $response["Q10B"], "MC10c" => $response["Q10C"], "MC10d" => $response["Q10D"], "MC10e" => $response["Q10E"],
            "FR1" => readresponses($import_commentspath . $response["classid"] . ".txt")
          ),"answersetid");
        
          if ($customid)
          {
            $customaid = db_replace($db,"AnswerSets",Array("questionsetid" => $customid, "classid" => $classid, "questionperiodid" => $periodid), Array(
              "responses" => $response["Q11A"] + $response["Q11B"] + $response["Q11C"] + $response["Q11D"] + $response["Q11E"],
              "MC1a" => $response["Q11A"], "MC1b" => $response["Q11B"], "MC1c" => $response["Q11C"], "MC1d" => $response["Q11D"], "MC1e" => $response["Q11E"],
              "MC2a" => $response["Q12A"], "MC2b" => $response["Q12B"], "MC2c" => $response["Q12C"], "MC2d" => $response["Q12D"], "MC2e" => $response["Q12E"],
              "MC3a" => $response["Q13A"], "MC3b" => $response["Q13B"], "MC3c" => $response["Q13C"], "MC3d" => $response["Q13D"], "MC3e" => $response["Q13E"],
              "MC4a" => $response["Q14A"], "MC4b" => $response["Q14B"], "MC4c" => $response["Q14C"], "MC4d" => $response["Q14D"], "MC4e" => $response["Q14E"],
              "MC5a" => $response["Q15A"], "MC5b" => $response["Q15B"], "MC5c" => $response["Q15C"], "MC5d" => $response["Q15D"], "MC5e" => $response["Q15E"],
              "FR1" => readresponses($import_questionpath . "A1_" . $response["classid"] . ".txt"),
              "FR2" => readresponses($import_questionpath . "A2_" . $response["classid"] . ".txt"),
            ),"answersetid");
          }
          else
            $customaid = "NONE";
          taskwindow_cprint("- " . $response["classid"] . " (Generic responses as " . $genericaid . ", custom responses as " . $customaid . ")<br>\n");
          if ((++$i) % 10 == 1) taskwindow_flush();          
        }
        mysql_select_db("seaseval",$db);
      }  
    }
  }
  taskwindow_end();
};

%>
<%

require_once("server.inc");
require_once("database.inc");
require_once("wces.inc");
require_once("taskwindow.inc");

//$db_debug = true;
//$wces_verbose = true;

// CSV Columns

define ("COURSENAME", 0); // course name
define ("COURSEDESC", 1); // brief course description, optional
define ("INSTRUCTOR", 2); // full name
define ("DEPARTMENT", 3); // department
define ("STUDENTS",   4); // number of students
define ("SUBJECT",    5); // full name
define ("DIVISION",   6); // full name
define ("SCHOOL",     7); // full name
define ("YEAR",       8); // four digit year
define ("SEMESTER",   9); // 'spring', 'fall', 'summer'
define ("SUBJ",      10); // 4 letter subject abbreviation
define ("COURSENO",  11); // course number, 1 letter 4 digits like W1007
define ("SECTION",   12); // 3 digit section ie  '001', '002', 'R01'
define ("DEPT",      13); // 4 letter department abbreviation. Not on page, taken from department hash table deptlist

$import_commentspath = $server_cgiroot . "comments/";
$import_questionpath = $server_cgiroot . "text_quest/";

function addelement(&$thearray,$key,$value)
{
  if ($value) $thearray[$key] = $value;
};

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

function importregcsv($filename)
{
  taskwindow_start("Progress Window");
  taskwindow_cprint("<h2>Importing Registrar Data</h2>\n");
  taskwindow_flush();

  $fp = fopen ($filename,"r");
  $db = wces_connect();
  
  $i = 0;
  while ($data = fgetcsv ($fp, 8192, ","))
  {
    if (count ($data) != 14)
    {
      taskwindow_cprint("This row does not contain the correct number of fields. (14 expected, " . count($data) . " found)<br>\n");
    }
    else
    {
      $schoolid     = db_replace($db, "Schools",     Array("name" => $data[SCHOOL]),0,"schoolid");
      $subjectid    = db_replace($db, "Subjects",    Array("code" => $data[SUBJ]), Array("name" => $data[SUBJECT]),"subjectid");
      $divisionid   = db_replace($db, "Divisions",   Array("name" => $data[DIVISION]), 0, "divisionid");
      $departmentid = db_replace($db, "Departments", Array("code" => $data[DEPT]), Array("name" => $data[DEPARTMENT]), "departmentid");
      $courseid     = db_replace($db, "Courses",     Array("subjectid" => $subjectid, "code" => $data[COURSENO]), Array("departmentid" => $departmentid, "divisionid" => $divisionid, "schoolid" => $schoolid, "name" => $data[COURSENAME], "description" => $data[COURSEDESC]),"courseid");
      $classid      = db_replace($db, "Classes",     Array("courseid" => $courseid, "section" => $data[SECTION], "year" => $data[YEAR], "semester" => $data[SEMESTER]), Array("students" => $data[STUDENTS]), "classid");
  
      prof_parserrname($data[INSTRUCTOR],$full,$email,$url);
      $pids = prof_findwithfull($db,$full);
      $pids = array_merge($pids,prof_findwithclassid($db,$classid));
      $pid = prof_merge($db, $pids,Array("name" => $full, "email" => $email, "url" => $url),Array("fullname" => $full, "source" => "regweb"));
      db_updatevalues($db, "Classes", Array("classid" => $classid), Array("professorid" => $pid));
      taskwindow_cprint($data[SUBJ].$data[COURSENO]." Section ".$data[SECTION]." ".$data[SEMESTER]." ".$data[YEAR]."<br>\n");
    };
    if (((++$i) % 10) == 1) taskwindow_flush();  
  }
  fclose ($fp); 
  taskwindow_end("Progress Window");   
};

function importregpid($filename)
{
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
      $regid = substr($buffer, $m + strlen($middle), $m - strlen($middle) - $b);
      
      prof_parsepidname($name,$first,$last,$middlei,$dept);
      $full = prof_makefull($first,$middlei,$last); 
      
      $pids = prof_findwithfirstlast($db,$first,$last);
      $pids = array_merge($pids,prof_findwithfull($db,$full));
      $pids = array_merge($pids,prof_findwithpid($db,$regid));
      $pid = prof_merge($db,$pids,Array("name" => $full),Array("first" => $first, "middle" => $middlei, "last" => $last, "fullname" => $full, "pid" => $regid, "source" => "regpid"));

      taskwindow_cprint("Adding Professor '$full' as #$pid<br>\n");
    }   
    if (((++$i) % 10) == 1) taskwindow_flush();  
  }
  fclose ($fp);
  taskwindow_end();
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
    print ("Transferring Classes...<br>\n");
    mysql_select_db("seaseval",$db);
    $result = mysql_query("SELECT * FROM classinfo",$db);
    mysql_select_db("wces",$db);
    while($row = mysql_fetch_array($result))
    {
      $classid = class_update($db,$row["classid"]);

      $firstname = $row["classproffname"];
      $lastname = $row["classproflname"];
      
      prof_fixname($firstname, $lastname);
      prof_parsemiddlei($firstname,$middlei);
      $full = prof_makefull($firstname,$middlei,$lastname);
      
      $pids = prof_findwithfirstlast($db,$firstname,$lastname);
      $pids = array_merge($pids,prof_findwithclassid($db,$classid));
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
    while($row = mysql_fetch_array($result))
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
    while($row = mysql_fetch_array($result))
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
    while($generic = mysql_fetch_array($generics))
    {
      $customs = mysql_query("SELECT * FROM evalquest WHERE genID=" . $generic["genID"],$db);
      while($custom = mysql_fetch_array($customs))
      {
        $responses = mysql_query("SELECT * FROM classevalresponse WHERE evalid=" . $custom["evalid"],$db);
        while($response = mysql_fetch_array($responses))
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
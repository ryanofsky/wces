<%

require_once("server.inc");
require_once("database.inc");
require_once("taskwindow.inc");

/*  

- - - - - - - - -  WCES DATABASE - - - - - - - - - -

Schools    Divisions       Departments   Subjects
   |           |                |          /             
   \           \                /         /           
   +------------+--- Courses --+---------+
                        |
        Users           |
         | |            |
         |  \           |
         |  Professors  |             
         |      |       |
         |      |       |
         |    Classes --+
         |     /  
         |    /    
     Enrollments 

 - - - - - - - - - - - - - - - - - - - - - - - - - - 

Classes     Professors     Subjects     Departments
  | |            |             |             |          
  | +------------+------+------+-------------+                                                                                                                                    
  |                     |                  
  |   QuestionPeriods   |          QuestionSets
  |         |           |               |  | 
  |         |           +-- Groupings --+  |
   \        |                             /
    \       |                            /   
     +------+----- AnswerSets ----------+

- - - - - - - - - - - - - - - - - - - - - - - - - - 

LEGEND

A line connecting two tables means that there is a
field in the bottom table that points to a row
in the top table. Example:

 A        C
  \       |
   \      |
    B ----+

Means that B points to A and C

 - - - - - - - - - - - - - - - - - - - - - - - - - - 

*/

$wces_verbose = false;

function wces_connect()
{
  $db = server_sqlinit();
  mysql_select_db("wces",$db);
  return $db;
}

function generic_connect()
{
  return server_sqlinit();
}

function wces_initcurrentclasses($db)
{
  mysql_query("CREATE TEMPORARY TABLE currentclasses (classid INTEGER NOT NULL, PRIMARY KEY(classid))",$db);
  mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' && cl.year = 2000 && cl.semester = 'fall'",$db);
  mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' && cl.year = 2000 && cl.semester = 'fall' && NOT (ASCII(cl.section) IN (82,114,86,118))",$db);
  mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors'  && cl.year = 2000 && cl.semester = 'fall'",$db);
  mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.subjectid INNER JOIN Classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' && cl.year = 2000 && cl.semester = 'fall'",$db);
  mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.departmentid INNER JOIN Classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' && cl.year = 2000 && cl.semester = 'fall'",$db);
};

function prof_fixname(&$first,&$last)
{
  $first = ucwords(strtolower(trim($first)));
  $last = ucwords(strtolower(trim($last)));
  if ($first == ".") $first = "";  
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

function prof_parsepidname($name,&$first,&$last,&$middlei,$dept)
{
  $first = ""; $last = ""; $middlei = ""; $dept = "";
  
  $find = strpos($name,",");
  if ($find === false) return false;
  
  $last = trim(substr($name,0,$find));
  $first = trim(substr($name,$find+1));
  $find = strpos($first," pid=");
  if (!($find === false)) $first = trim(substr($first,0,$find));
  $find = strpos($first," dept=");
  if (!($find === false)) $first = trim(substr($first,0,$find));
  $find = strpos($last," pid=");
  if (!($find === false)) $last = trim(substr($last,$find + 5));
  $find = strpos($name," dept=");
  $dept = ($find === false) ? "" : substr($name,$find+5,4);
  prof_parsemiddlei($first,$middlei); 
  if ($first == ".") $first = "";
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
};

function prof_makefull($first,$middlei,$last)
{
  return trim($first . " " . trim($middlei . " " . $last));
}

function prof_findwithfull($db, $name)
{
  global $wces_verbose,$db_debug;
  
  $sql = "SELECT p.professorid FROM Professors as p, ProfessorDupeData as dd WHERE p.professorid = dd.professorid AND dd.source = 'regweb' AND dd.fullname = '" . addslashes($name) . "'";
  if ($db_debug) taskwindow_cprint("$sql<br>");
  $result = mysql_query($sql,$db);
  $professorids = array();
  
  if ($name)
  {
    while($row = mysql_fetch_array($result))
      array_push($professorids,$row['professorid']);
    $professorids = array_values(array_unique($professorids));
  }
  
  if ($wces_verbose)
  {
    if (count($professorids) >0)
      taskwindow_cprint("Professor '$name' found<br>");
    else
      taskwindow_cprint("Professor '$name' not found<br>");
  }
  return $professorids;
};

function prof_findwithfirstlast($db, $first,$last)
{
  global $wces_verbose,$db_debug;
  
  $sql = "SELECT p.professorid FROM Professors as p, ProfessorDupeData as dd WHERE p.professorid = dd.professorid AND dd.first = '" . addslashes($first) . "' AND dd.last = '" . addslashes($last) . "'";
  if ($db_debug) taskwindow_cprint("$sql<br>");
  $result = mysql_query($sql,$db);
  $pids = Array();
  while($row = mysql_fetch_array($result))
    array_push($pids,$row['professorid']);
  $pids = array_values(array_unique($pids));
  
  if ($wces_verbose)
  {
    if (count($pids) >0)
      taskwindow_cprint("Professor '$first $last' found<br>");
    else
      taskwindow_cprint("Professor '$first $last' not found<br>");
  }
  return $pids;
};

function prof_findwithclassid($db, $classid)
{
  global $wces_verbose,$db_debug;
  $professorids = db_getcolumn($db, "Classes", Array("classid" => $classid),"professorid");
  
  if (is_array($professorids))
  {
    foreach($professorids as $key => $value)
      if ($value == 0) array_splice($professorids,$key,1);
  }
  else
    $professorids = Array();
  
  if ($wces_verbose)
  {
    if (count($professorids) >0)
      taskwindow_cprint("Professor for class #$classid found<br>");
    else
      taskwindow_cprint("Professor for class #$classid not found<br>");
  }
  return $professorids;
};

function prof_findwithpid($db, $pid)
{
  global $wces_verbose,$db_debug;
  $sql = "SELECT p.professorid FROM Professors as p, ProfessorDupeData as dd WHERE p.professorid = dd.professorid AND dd.pid = '" . addslashes($pid) . "'";
  if ($db_debug) taskwindow_cprint("$sql<br>");
  $result = mysql_query($sql,$db);
  $professorids = array();
  while($row = mysql_fetch_array($result))
    array_push($professorids,$row['professorid']);
  $professorids = array_values(array_unique($professorids));
  if ($wces_verbose)
  {
    if (count($professorids) >0)
      taskwindow_cprint("Professor with PID #$pid found<br>");
    else
      taskwindow_cprint("Professor with PID #$pid not found<br>");
  }
  return $professorids;
};

function prof_isdupe(&$newrow,&$oldrows)
{
  if (count($oldrows) > 0)
    foreach($oldrows as $oldrow)
    {
      $dupe = true;
      foreach($newrow as $key => $value)
        if ($value && $value != $oldrow[$key]) $dupe = false;
      if ($dupe) return true;
    }
  return false;
};

function prof_merge($db,$professorids,$newprofessordata,$newdupedata)
{
  global $db_debug;
  
  $professorids = array_values(array_unique($professorids)); // eliminate duplicates in $professorids
  
  if (count($professorids) > 0)
  {
    sort($professorids);

    $profcondition = "(";  // convert $professorids array into SQL conditions
    $first = true;
  
    foreach($professorids as $professorid)
    {
      if ($first) $first = false; else $profcondition .= " OR ";
      $profcondition .= "p.professorid = '" . addslashes($professorid) . "'";
    }
    $profcondition .= ")";

    // if there is a professor entry whose data came from the oracle database
    // then that entry's information is chosen so it will not be overwritten

    $sql = "SELECT p.professorid FROM Professors as p, ProfessorDupeData as dd WHERE p.professorid = dd.professorid AND dd.source = 'oracle' AND $profcondition";
    if ($db_debug) taskwindow_cprint("$sql<br>");
    $result = mysql_query($sql, $db);

    $rows = mysql_num_rows($result);
    if ($rows == 1)
    {
      $prow = mysql_fetch_array($result);
      $chosen = $prow["professorid"];
    }  
    else
      $chosen = $professorids[0];
      
    mysql_free_result($result);  
  
    // get chosen row data 
    $chosenrow = db_getrow($db,"Professors",Array("professorid" => $chosen),0);
    $chosenchanged = false;
    
    // get the other rows
    $sql = "SELECT * FROM Professors as p WHERE $profcondition";
    if ($db_debug) taskwindow_cprint("$sql<br>");
    $result = mysql_query($sql,$db);
  
    // copy data from other rows, point references to chosen row, and finally delete the other rows
    while($professor = mysql_fetch_array($result))
      if ($professor['professorid'] != $chosen)
      {
        foreach($professor as $key => $value)
          if ($value && !$chosenrow[$key])
          {
            $chosenrow[$key] = $value;
            $chosenchanged = true;
          }  
        db_updatevalues($db,"Classes",           Array("professorid" => $professor['professorid']), Array("professorid" => $chosen));
        db_updatevalues($db,"ProfessorDupeData", Array("professorid" => $professor['professorid']), Array("professorid" => $chosen));
        db_updatevalues($db,"Groupings",         Array("linkid" => $professor['professorid'], "linktype" => "professors"), Array("linkid" => $chosen));
        db_delete($db,"Professors", Array("professorid" => $professor['professorid']));
      };    
    
    mysql_free_result($result);

    if (is_array($newprofessordata))
    foreach($newprofessordata as $key => $value)
      if ($value && !$chosenrow[$key])
      {
        $chosenrow[$key] = $value;
        $chosenchanged = true;
      };  

    if ($chosenchanged)
    {
      foreach($chosenrow as $key => $value)   
        if ($key === $key + 0) unset($chosenrow[$key]); // eliminate numerical keys
      db_updatevalues($db,"Professors",Array("professorid" => $chosen),$chosenrow);  
    };  

    // elimininate rows in ProfessorDupeData that are the same
  
    $sql = "SELECT * FROM ProfessorDupeData WHERE professorid = $chosen";
    if ($db_debug) taskwindow_cprint("$sql<br>");
    $result = mysql_query($sql,$db);
  
    $oldrows = Array(); // exists to check against duplicates
    while($newrow = mysql_fetch_array($result))
    {
      if (prof_isdupe($newrow,$oldrows))
        db_delete($db,"ProfessorDupeData",Array("professorid" => $newrow["professorid"]));
      else
        $oldrows[] = $newrow;  
    }
  
    if (is_array($newdupedata))
    {
      $newdupedata["professorid"] = $chosen;
      if (!prof_isdupe($newdupedata,$oldrows)) db_addrow($db,"ProfessorDupeData",$newdupedata);
    };
  
    return $chosen;    
  }  
  else
  {
    $professorid = db_addrow($db,"Professors",$newprofessordata);
    $newdupedata["professorid"] = $professorid;
    db_addrow($db,"ProfessorDupeData",$newdupedata);
    return $professorid;
  };
};




function course_update($db,$code)
{
  $subj = substr($code,0,4);
  $course = substr($code,4,5);
  $subjectid = db_replace($db,"Subjects", Array("code" => $subj), 0, "subjectid");
  $courseid = db_replace($db, "Courses", Array("subjectid" => $subjectid, "code" => $course), 0, "courseid");
  return $courseid;
}

function class_update($db,$code)
{
  $course = substr($code,0,9);
  $section = substr($code,10,3);
  $year = substr($code,14,4);
  $s = substr($code,19,1);
  $semester = ($s == "1") ? "spring" : (($s == "2") ? "summer" : "fall");
  $courseid = course_update($db,$course);  
  $classid = db_replace($db, "Classes", Array("courseid" => $courseid, "section" => $section, "year" => $year, "semester" => $semester), 0, "classid");
  return $classid;
}

%>
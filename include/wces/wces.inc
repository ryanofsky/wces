<?

require_once("wbes/server.inc");
require_once("wces/database.inc");
require_once("wbes/taskwindow.inc");

/*  

- - - - - - - - -  WCES DATABASE - - - - - - - - - -

schools    divisions       departments   subjects
   |           |                |          /             
   \           \                /         /           
   +------------+--- courses --+---------+
                        |
        users           |
         | |            |
         |  \           |
         |  professors  |             
         |      |       |
         |      |       |
         |    classes --+
         |     /  
         |    /    
     enrollments 

 - - - - - - - - - - - - - - - - - - - - - - - - - - 

classes     professors     subjects     departments
  | |            |             |             |          
  | +------------+------+------+-------------+                                                                                                                                    
  |                     |                  
  |   questionperiods   |          questionsets
  |         |           |               |  | 
  |         |           +-- groupings --+  |
   \        |                             /
    \       |                            /   
     +------+----- answersets ----------+

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

$wces = NULL;
$wces_verbose = false;

function wces_connect()
{
  global $wces;
  if (!isset($wces)) $wces = server_pginit("wces");
}

function wces_oldconnect()
{
  $db = server_mysqlinit();
  mysql_select_db("wces",$db);
  return $db;
}

function generic_connect()
{
  return server_mysqlinit();
}

function wces_Findclasses($db,$temptable)
{
  $questionperiods = db_exec("SELECT year, semester FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1", $db, __FILE__, __LINE__);
  $questionperiod = mysql_fetch_assoc($questionperiods);
  $year = "'" . addslashes($questionperiod["year"]) . "'";
  $semester = "'" . addslashes($questionperiod["semester"]) . "'";
  db_exec("CREATE TEMPORARY TABLE $temptable (classid INTEGER NOT NULL, PRIMARY KEY(classid))",$db);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' AND cl.year = $year AND cl.semester = $semester",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (66,98,77,109,78,110,82,114,86,118))",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.subjectid INNER JOIN classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.departmentid INNER JOIN classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester",$db,__FILE__, __LINE__);
};

function wces_Findquestionsets($db,$temptable,$classid = 0)
{
  $questionperiods = db_exec("SELECT year, semester, questionperiodid FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1",$db,__FILE__, __LINE__);
  $questionperiod = mysql_fetch_assoc($questionperiods);
  $year = "'" . addslashes($questionperiod["year"]) . "'";
  $semester = "'" . addslashes($questionperiod["semester"]) . "'";
  $whereclass = $classid > 0 ? " AND cl.classid = '" . addslashes($classid) . "'" : "";
  db_exec("CREATE TEMPORARY TABLE $temptable (classid INTEGER NOT NULL, questionsetid INTEGER NOT NULL, PRIMARY KEY(classid,questionsetid))",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' AND cl.year = $year AND cl.semester = $semester $whereclass",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (66,98,77,109,78,110,82,114,86,118)) $whereclass",$db);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester $whereclass",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.subjectid INNER JOIN classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester $whereclass",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.departmentid INNER JOIN classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester $whereclass",$db,__FILE__, __LINE__);
  return $questionperiod["questionperiodid"];
}

function wces_Findquestionsetsta($db,$temptable,$classid = 0, $topicid = 0)
{
  $questionperiods = db_exec("SELECT year, semester, questionperiodid FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1",$db,__FILE__, __LINE__);
  $questionperiod = mysql_fetch_assoc($questionperiods);
  $year = "'" . addslashes($questionperiod["year"]) . "'";
  $semester = "'" . addslashes($questionperiod["semester"]) . "'";
  $whereclass = $classid > 0 ? " AND cl.classid = '" . addslashes($classid) . "'" : "";
  $wheretopic = $topicid > 0 ? " AND g.topicid = '" . addslashes($topicid) . "'" : "";
  db_exec("CREATE TEMPORARY TABLE $temptable (classid INTEGER NOT NULL, questionsetid INTEGER NOT NULL, tarate enum('no','yes') NOT NULL, topicid INTEGER, PRIMARY KEY(classid,questionsetid))",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid,tarate,topicid) SELECT cl.classid, g.questionsetid, g.tarate, g.topicid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' AND cl.year = $year AND cl.semester = $semester $whereclass $wheretopic",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid,tarate,topicid) SELECT cl.classid, g.questionsetid, g.tarate, g.topicid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (66,98,77,109,78,110,82,114,86,118)) $whereclass $wheretopic",$db);
  db_exec("REPLACE INTO $temptable (classid,questionsetid,tarate,topicid) SELECT cl.classid, g.questionsetid, g.tarate, g.topicid FROM groupings AS g INNER JOIN classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester $whereclass $wheretopic",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid,tarate,topicid) SELECT cl.classid, g.questionsetid, g.tarate, g.topicid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.subjectid INNER JOIN classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester $whereclass $wheretopic",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid,tarate,topicid) SELECT cl.classid, g.questionsetid, g.tarate, g.topicid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.departmentid INNER JOIN classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester $whereclass $wheretopic",$db,__FILE__, __LINE__);
  return $questionperiod["questionperiodid"];
}

function wces_CompleteClasses($db, $temptable, $topicid = 0)
{
  db_exec("CREATE TEMPORARY TABLE $temptable (classid INTEGER, questionperiodid INTEGER PRIMARY KEY (classid, questionperiodid))", $db, __FILE__, __LINE__);
  $where = $criteria["topics"] ?  "WHERE topicid IN (" . implode(", ", $criteria["topics"]) . ")" : "";
  db_exec("REPLACE INTO $temptable(classid, questionperiodid) SELECT classid, questionperiodid FROM answersets $where GROUP BY classid", $db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable(classid, questionperiodid) SELECT classid, questionperiodid FROM taratings  $where GROUP BY classid", $db, __FILE__, __LINE__);
}

function wces_GetQuestionPeriod($db)
{
  $result = db_exec("SELECT questionperiodid, year, semester FROM questionperiods ORDER BY year desc, semester desc, questionperiodid desc LIMIT 1", $db,__FILE__, __LINE__);
  return mysql_result($result,0);
}

function wces_GetCurrentQuestionPeriod($db, &$id, &$description, &$year, &$semester)
{
  $result = db_exec("SELECT questionperiodid, description, year, semester FROM questionperiods ORDER BY year desc, semester desc, questionperiodid desc LIMIT 1", $db,__FILE__, __LINE__);
  $row = mysql_fetch_assoc($result);
  $id = $row["questionperiodid"];
  $description = $row["description"];
  $year = $row["year"];
  $semester = $row["semester"];  
}

function course_update($db,$code)
{
  $subj = substr($code,0,4);
  $course = substr($code,5,4);
  if ($course != $course | 0)
  {
    sendwarning("Invalid Course $code", __FILE__, __LINE__);
    return false;
  }
  $subjectid = db_replace($db,"subjects", Array("code" => $subj), 0, "subjectid");
  $courseid = db_replace($db, "courses", Array("subjectid" => $subjectid, "code" => $course), 0, "courseid");
  return $courseid;
}

function class_update($db, $code, &$courseid)
{
  if(strlen($code) != 20 || $code[9] != '_' || $code[13] != '_' || $code[18] != '_')
  {
    sendwarning("Invalid course format for $code", __FILE__, __LINE__);
    return false;
  }

  $course = substr($code,0,9);
  $div = substr($code,4,1);
  $section = substr($code,10,3);

  $year = substr($code,14,4)|0;
  if (!(1990 < $year && $year < 2020))
  {
    sendwarning("Invalid year for course code $code", __FILE__, __LINE__);
    return false;
  }

  $s = substr($code,19,1)|0;
  if (!(1 <= $s && $s <= 3))
  {
    sendwarning("Invalid semester for course $code", __FILE__, __LINE__);
    return false;
  }
  
  $semester = ($s == 1) ? "spring" : (($s == 2) ? "summer" : "fall");

  if ($courseid = course_update($db,$course))
    return db_replace($db, "classes", Array("courseid" => $courseid, "divisioncode" => $div, "section" => $section, "year" => $year, "semester" => $semester), 0, "classid");
  else
  {
    sendwarning("Class $code not inserted", __FILE__, __LINE__); 
    return false;
  }
}

function course_find($db,$code)
{
  $subj = substr($code,0,4);
  $course = substr($code,5,4);
  if (!($subjectid = db_getvalue($db,"subjects", Array("code" => $subj), "subjectid")))
    return false;
  if (!($courseid = db_getvalue($db, "courses", Array("subjectid" => $subjectid, "code" => $course), "courseid")))
    return false;
  return $courseid;
}

function class_find($db,$code,$courseid = false)
{
  $course = substr($code,0,9);
  $div = substr($code,4,1);
  $section = substr($code,10,3);
  $year = substr($code,14,4);
  $s = substr($code,19,1);
  $semester = ($s == "1") ? "spring" : (($s == "2") ? "summer" : "fall");
  $courseid = course_find($db,$course);  
  $classid = db_getvalue($db, "classes", Array("courseid" => $courseid, "divisioncode" => $div, "section" => $section, "year" => $year, "semester" => $semester), "classid");
  return $classid;
}

?>
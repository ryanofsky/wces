<%

require_once("wces/server.inc");
require_once("wces/database.inc");
require_once("wces/taskwindow.inc");

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
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (66,98,78,110,82,114,86,118))",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.subjectid INNER JOIN classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester",$db, __FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid) SELECT cl.classid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.departmentid INNER JOIN classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester",$db,__FILE__, __LINE__);
};

function wces_Findquestionsets($db,$temptable)
{
  $questionperiods = db_exec("SELECT year, semester FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1",$db,__FILE__, __LINE__);
  $questionperiod = mysql_fetch_array($questionperiods);
  $year = "'" . addslashes($questionperiod["year"]) . "'";
  $semester = "'" . addslashes($questionperiod["semester"]) . "'";
  db_exec("CREATE TEMPORARY TABLE $temptable (classid INTEGER NOT NULL, questionsetid INTEGER NOT NULL, PRIMARY KEY(classid,questionsetid))",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' AND cl.year = $year AND cl.semester = $semester",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (66,98,78,110,82,114,86,118))",$db);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.subjectid INNER JOIN classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester",$db,__FILE__, __LINE__);
  db_exec("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.departmentid INNER JOIN classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester",$db,__FILE__, __LINE__);
}

function wces_GetQuestionPeriod($db)
{
  $result = db_exec("SELECT questionperiodid, year, semester FROM questionperiods ORDER BY year desc, semester desc, questionperiodid desc LIMIT 1", $db,__FILE__, __LINE__);
  return mysql_result($result,0);
}

function course_update($db,$code)
{
  $subj = substr($code,0,4);
  $course = substr($code,5,4);
  $subjectid = db_replace($db,"subjects", Array("code" => $subj), 0, "subjectid");
  $courseid = db_replace($db, "courses", Array("subjectid" => $subjectid, "code" => $course), 0, "courseid");
  return $courseid;
}

function class_update($db,$code,$courseid = false)
{
  $course = substr($code,0,9);
  $div = substr($code,4,1);
  $section = substr($code,10,3);
  $year = substr($code,14,4);
  $s = substr($code,19,1);
  $semester = ($s == "1") ? "spring" : (($s == "2") ? "summer" : "fall");
  $courseid = course_update($db,$course);  
  $classid = db_replace($db, "classes", Array("courseid" => $courseid, "divisioncode" => $div, "section" => $section, "year" => $year, "semester" => $semester), 0, "classid");
  return $classid;
}

%>
<%

require_once("wces/server.inc");
require_once("wces/database.inc");
require_once("wces/taskwindow.inc");

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

function wces_FindClasses($db,$temptable)
{
  $questionperiods = mysql_query("SELECT year, semester FROM QuestionPeriods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1", $db);
  $questionperiod = mysql_fetch_array($questionperiods);
  $year = "'" . addslashes($questionperiod["year"]) . "'";
  $semester = "'" . addslashes($questionperiod["semester"]) . "'";
  mysql_query("CREATE TEMPORARY TABLE $temptable (classid INTEGER NOT NULL, PRIMARY KEY(classid))",$db);
  mysql_query("REPLACE INTO $temptable (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' AND cl.year = $year AND cl.semester = $semester",$db);
  mysql_query("REPLACE INTO $temptable (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (82,114,86,118))",$db);
  mysql_query("REPLACE INTO $temptable (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester",$db);
  mysql_query("REPLACE INTO $temptable (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.subjectid INNER JOIN Classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester",$db);
  mysql_query("REPLACE INTO $temptable (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.departmentid INNER JOIN Classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester",$db);
};

function wces_FindQuestionSets($db,$temptable)
{
  $questionperiods = mysql_query("SELECT year, semester FROM QuestionPeriods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1", $db);
  $questionperiod = mysql_fetch_array($questionperiods);
  $year = "'" . addslashes($questionperiod["year"]) . "'";
  $semester = "'" . addslashes($questionperiod["semester"]) . "'";
  mysql_query("CREATE TEMPORARY TABLE $temptable (classid INTEGER NOT NULL, questionsetid INTEGER NOT NULL, PRIMARY KEY(classid,questionsetid))",$db);
  mysql_query("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' AND cl.year = $year AND cl.semester = $semester",$db);
  mysql_query("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' AND cl.year = $year AND cl.semester = $semester AND NOT (ASCII(cl.section) IN (82,114,86,118))",$db);
  mysql_query("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM Groupings AS g INNER JOIN Classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors' AND cl.year = $year AND cl.semester = $semester",$db);
  mysql_query("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.subjectid INNER JOIN Classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' AND cl.year = $year AND cl.semester = $semester",$db);
  mysql_query("REPLACE INTO $temptable (classid,questionsetid) SELECT cl.classid, g.questionsetid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.departmentid INNER JOIN Classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' AND cl.year = $year AND cl.semester = $semester",$db);
}

function wces_GetQuestionPeriod($db)
{
  $result = mysql_query("SELECT questionperiodid FROM QuestionPeriods ORDER BY year desc, semester desc, questionperiodid desc LIMIT 1", $db);
  return mysql_result($result,0);
}

function course_update($db,$code)
{
  $subj = substr($code,0,4);
  $course = substr($code,5,4);
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
  $div = substr($course,4,1);
  $semester = ($s == "1") ? "spring" : (($s == "2") ? "summer" : "fall");
  $courseid = course_update($db,$course);  
  $classid = db_replace($db, "Classes", Array("courseid" => $courseid, "divisioncode" => $div, "section" => $section, "year" => $year, "semester" => $semester), 0, "classid");
  return $classid;
}

%>
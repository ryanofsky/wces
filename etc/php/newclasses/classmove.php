<?

require_once("wbes/server.inc");
require_once("wbes/postgres.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wbes/general.inc");
require_once("wces/database.inc");
require_once("wbes/survey.inc");
require_once("wces/component_abet.inc");

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");

require_once("widgets/basic.inc");

define("SRC_WWW",      1);
define("SRC_PID",      2);
define("SRC_ORACLE",   3);
define("SRC_REGFILE",  4);

page_top("Import Data", true);

param($do_subject);     
param($do_department);  
param($do_division);   
param($do_course);
param($do_classid);     
param($do_users);       
param($do_enrollments); 
param($do_penrollments);
param($do_profhooks);   
param($do_oldchoice);   
param($do_createsurvey);
param($do_topics);      
param($do_getcustoms);  
param($do_results);     
param($import);

if (!$import)
{
?>
<form method=get>
<FIELDSET>
<LEGEND align="top">Legacy Database Import</LEGEND>
<table>
<tr><td valign=top>Options:</td><td>
<INPUT NAME=do_subject      ID=do_subject      TYPE=checkbox value=1 CHECKED><label for=do_subject>Import Subjects</label><br>
<INPUT NAME=do_department   ID=do_department   TYPE=checkbox value=1 CHECKED><label for=do_department>Import Departments</label><br>
<INPUT NAME=do_division     ID=do_division     TYPE=checkbox value=1 CHECKED><label for=do_division>Import Divisions</label><br>
<INPUT NAME=do_course       ID=do_course       TYPE=checkbox value=1 CHECKED><label for=do_course>Import Divisions</label><br>
<INPUT NAME=do_classid      ID=do_classid      TYPE=checkbox value=1 CHECKED><label for=do_classid>Import Classes</label><br>
<INPUT NAME=do_users        ID=do_users        TYPE=checkbox value=1 CHECKED><label for=do_users>Import Users</label><br>
<INPUT NAME=do_enrollments  ID=do_enrollments  TYPE=checkbox value=1 CHECKED><label for=do_enrollments>Import Enrollments</label><br>
<INPUT NAME=do_penrollments ID=do_penrollments TYPE=checkbox value=1 CHECKED><label for=do_penrollments>Import Professor Enrollments</label><br>
<INPUT NAME=do_profhooks    ID=do_profhooks    TYPE=checkbox value=1 CHECKED><label for=do_profhooks>Import Professor Hooks</label><br>
<INPUT NAME=do_oldchoice    ID=do_oldchoice    TYPE=checkbox value=1 CHECKED><label for=do_oldchoice>Import Old Choices</label><br>
<INPUT NAME=do_createsurvey ID=do_createsurvey TYPE=checkbox value=1 CHECKED><label for=do_createsurvey>Create Base Surveys</label><br>
<INPUT NAME=do_topics       ID=do_topics       TYPE=checkbox value=1 CHECKED><label for=do_topics>Import Topics</label><br>
<INPUT NAME=do_getcustoms   ID=do_getcustoms   TYPE=checkbox value=1 CHECKED><label for=do_getcustoms>Import Custom Questions</label><br>
<INPUT NAME=do_results      ID=do_results      TYPE=checkbox value=1 CHECKED><label for=do_results>Import Results</label><br>
</td></tr>
<tr><td>&nbsp;</td><td><INPUT type="submit" NAME="import" VALUE="Go"></td></tr></table>
</FIELDSET>
</FORM>
<?
}
else
{
  print('<a href="' . $server_url->toString() . '">Back</a>');
  
  $my = wces_oldconnect();
  $pg = server_pginit("russwbes");
  
  $i = 0;
  
  if($do_subject)
  {
    print("<h3>Subjects</h3>\n");
    $s = pg_query("DELETE FROM temp_subj; BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("SELECT subjectid, code, name FROM subjects", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("nullquot", $row);
      pg_query("SELECT temp_subji($row[subjectid], subject_update($row[code], $row[name]))", $pg, __FILE__, __LINE__);
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      if ($first) $first = false; else print(",");
      print($row['code']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_department)
  {
    print("<h3>Departments</h3>\n");
    pg_query("DELETE FROM temp_dept; BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("SELECT departmentid, code, name FROM departments", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("nullquot", $row);
      pg_query("SELECT temp_depti($row[departmentid], department_update($row[code], $row[name]))", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print($row['code']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_division)
  {
    print("<h3>Divisions</h3>\n");
    pg_query("DELETE FROM temp_div; BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("SELECT divisionid, code, shortcode as scode, name FROM divisions", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("quot", $row);
      pg_query("SELECT temp_divi($row[divisionid], division_update($row[code], $row[scode], $row[name]))", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print($row['code']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_course)
  {
    print("<h3>Courses</h3>\n");
    pg_query("DELETE FROM temp_course; BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("
      SELECT c.courseid, c.subjectid, c.code, cl.divisioncode, c.name, c.information
      FROM courses AS c
      INNER JOIN classes AS cl USING (courseid)
      GROUP BY c.courseid, cl.divisioncode
    ", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("nullquot", $row);
      pg_query("SELECT temp_coursei($row[courseid], $row[divisioncode], course_update(temp_subjr($row[subjectid]), $row[code], $row[divisioncode], $row[name], $row[information]))", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print($row['code']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_classid)
  {
    print("<h3>Classes</h3>\n");
    $sem = array("spring" => 0, "summer" => 1, "fall" => 2);
    pg_query("DELETE FROM temp_class; BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("
      SELECT
        cl.classid, cl.courseid, cl.section, cl.year, cl.semester, cl.name,
        cl.time, cl.location, cl.students, cl.callnumber, cl.departmentid,
        cl.divisionid, c.schoolid, cl.divisioncode
      FROM classes AS cl
      INNER JOIN courses AS c USING (courseid)
    ", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      $row["semester"] = $sem[$row["semester"]];
      aarray_map("nullquot", $row);
      pg_query("SELECT temp_classi($row[classid], class_update(temp_courser($row[courseid],$row[divisioncode]), $row[section], $row[year], $row[semester], $row[name], $row[time], $row[location], $row[students], $row[callnumber], temp_deptr($row[departmentid]), temp_divr($row[divisionid]), temp_schr($row[schoolid])))", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print($row['classid']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_users)
  {
    print("<h3>Users</h3>\n");
    pg_query("DELETE FROM temp_user; DELETE FROM temp_prof; BEGIN", $pg, __FILE__, __LINE__);
    $result = array(); // mysql doesn't do full outer joins. or unions...
    $result[] = db_exec("
      SELECT
        u.userid, p.professorid, u.isprofessor, u.isadmin, u.cunix, u.lastlogin, p.email,
        SUBSTRING_INDEX(p.name,' ',-1) AS last,
        SUBSTRING(p.name,1,LENGTH(p.name)-LOCATE(' ',REVERSE(p.name))) AS first,
        p.departmentid, a.departmentid AS adid,
        p.url, p.picname, p.statement, p.profile, p.education
      FROM professors AS p
      LEFT JOIN users AS u ON u.userid = p.userid
      LEFT JOIN admins AS a ON u.userid = a.userid
    ", $my, __FILE__, __LINE__);
  
    $result[] = db_exec("
      SELECT
        u.userid, NULL AS professorid, u.isprofessor, u.isadmin, u.cunix, u.lastlogin, NULL AS email,
        NULL AS last, NULL AS first, NULL AS departmentid, a.departmentid AS adid,
        NULL AS url, NULL AS picname, NULL AS statement, NULL AS profile, NULL AS education
      FROM users AS u
      LEFT JOIN professors AS p USING (userid)
      LEFT JOIN admins AS a ON u.userid = a.userid
      WHERE p.professorid IS NULL
    ", $my, __FILE__, __LINE__);
  
    foreach($result as $res)
    {
      $first = true;
      while($row = mysql_fetch_assoc($res))
      {
        $flags = 0;
        if ($row['isadmin'] == 'true')
          $flags |= 1;
  
        if ($row['adid'])
        {
          $row['departmentid'] = $row['adid'];
          $flags |= 2;
        }
  
        $uid = $row['userid'];
        $pid = $row['professorid'];
  
        if ($row['isprofessor'] == 'true' || $pid)
          $flags |= 4;
  
        aarray_map("nullquot", $row);
        $r = pg_query("SELECT user_update($row[cunix], $row[last], $row[first], $row[email], $flags, $row[lastlogin], temp_deptr($row[departmentid]))", $pg, __FILE__, __LINE__);
        $user_id = pg_result($r,0,0);
  
        if ($uid)
        {
          pg_query("SELECT temp_useri($row[userid], $user_id)", $pg, __FILE__, __LINE__);
        }
  
        if ($pid)
        {
          pg_query("SELECT temp_profi($row[professorid],professor_data_update($user_id, $row[url], $row[picname], $row[statement], $row[profile], $row[education]))", $pg, __FILE__, __LINE__);
        }
  
        if ($first) $first = false; else print(",");
        if (++$i % 100 == 0) { print("<br>\n"); flush(); }
        print("$row[userid]/$row[professorid]");
      }
      print("<br>\n" . mysql_num_rows($res) . " rows<br><br>\n");
    }
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_enrollments)
  {
    pg_query("BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("
      SELECT e.userid, e.classid, u.lastlogin FROM enrollments AS e INNER JOIN users AS u USING (userid)
    ", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("nullquot", $row);
      pg_query("SELECT enrollment_update(temp_userr($row[userid]), temp_classr($row[classid]), 1, $row[lastlogin])", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print("$row[userid]/$row[classid]");
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if($do_penrollments)
  {
    pg_query("BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("
      SELECT professorid, classid FROM classes WHERE professorid IS NOT NULL
    ", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("nullquot", $row);
      pg_query("SELECT enrollment_update(temp_profr($row[professorid]), temp_classr($row[classid]), 3, NULL)", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print("$row[professorid]/$row[classid]");
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if ($do_profhooks)
  {
    pg_query("BEGIN", $pg, __FILE__, __LINE__);
    $result = db_exec("
      SELECT professorid, source, first, middle, last, fullname, pid FROM professordupedata
    ", $my, __FILE__, __LINE__);
    $first = true;
    $sources = array('regweb' => 1,'regpid' => 2,'oracle' => 3,'oldclasses' => 4);
    while($row = mysql_fetch_assoc($result))
    {
      $src = $sources[$row['source']];
      if ($src == 1)
      {
        $row['fullname'] = "'" . addslashes($row['fullname']) . "'";
        $row['first']    = "NULL";
        $row['last']     = "NULL";
        $row['middle']   = "NULL";
        $row['pid']      = "NULL";
      }
      else if ($src == 2)
      {
        $row['fullname'] = "'" . addslashes($row['fullname']) . "'";
        $row['first']    = "'" . addslashes($row['first']) . "'";
        $row['last']     = "'" . addslashes($row['last']) . "'";
        $row['middle']   = "'" . addslashes($row['middle']) . "'";
        $row['pid']      = "'" . addslashes($row['pid']) . "'";
      }
      else if ($src == 3 || $src == 4)
      {
        $row['fullname'] = "NULL";
        $row['first']    = "'" . addslashes($row['first']) . "'";
        $row['last']     = "'" . addslashes($row['last']) . "'";
        $row['middle']   = "NULL";
        $row['pid']      = "NULL";      
      }
      else
        assert(false);
      
      $r = pg_query("SELECT professor_hooks_update(temp_profr($row[professorid]), $src, $row[fullname], $row[first], $row[last], $row[middle], $row[pid])", $pg, __FILE__, __LINE__);
      $r = pg_result($r, 0, 0);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
  
      if (!$r) print("\n\n\n<h1><font color=red>***");
      print("$row[professorid]/$row[source]/$r");
      if (!$r) print("***</font></h1>\n\n\n");
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    pg_query("COMMIT", $pg, __FILE__, __LINE__);
  }
  
  if ($do_createsurvey)
  {
    print("<h3>Base Questions</h3>\n");
  
    // imported revision numbers
  
    // class survey   32 (branch 1)   
    // base questions 11 (branch 14)
    // class comments 12 (branch 14)
    // abet questions 31 (branch 33)
  
    // ta survey      41 (branch 2)
    // ta name        33 (branch 34)
    // ta questions   30 (branch 40)
    // ta comments    40 (branch 41)
  
    pg_query("
      DELETE FROM branch_topics_cache;
      DELETE FROM branch_ancestor_cache;
      DELETE FROM branches;
      DELETE FROM revisions;
      DELETE FROM list_items;
      DELETE FROM topics;
      
      BEGIN;
  
      INSERT INTO wces_topics (topic_id, parent, class_id, category_id) VALUES (1, NULL, NULL, NULL);
  
      INSERT INTO revisions (revision_id,type,parent,branch_id,revision,save_id,merged) VALUES (32,1,NULL,1,1,NULL,NULL);
      INSERT INTO revisions (revision_id,type,parent,branch_id,revision,save_id,merged) VALUES (41,1,NULL,2,1,NULL,NULL);
  
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (1,1,1,NULL,'f',32,32);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (2,2,2,NULL,'f',41,41);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (3,1,3,NULL,'f',1,1);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (4,1,4,NULL,'f',2,2);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (5,1,5,NULL,'f',3,3);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (6,1,6,NULL,'f',4,4);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (7,1,7,NULL,'f',5,5);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (8,1,8,NULL,'f',6,6);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (9,1,9,NULL,'f',7,7);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (10,1,10,NULL,'f',8,8);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (11,1,11,NULL,'f',9,9);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (12,1,12,NULL,'f',10,10);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (13,1,13,NULL,'f',11,11);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (14,1,14,NULL,'f',12,12);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (15,1,15,NULL,'f',13,13);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (16,1,16,NULL,'f',14,14);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (17,1,17,NULL,'f',15,15);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (18,1,18,NULL,'f',16,16);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (19,1,19,NULL,'f',17,17);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (20,1,20,NULL,'f',18,18);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (21,1,21,NULL,'f',19,19);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (22,1,22,NULL,'f',20,20);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (23,1,23,NULL,'f',21,21);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (24,1,24,NULL,'f',22,22);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (25,1,25,NULL,'f',23,23);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (26,1,26,NULL,'f',24,24);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (27,1,27,NULL,'f',25,25);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (28,1,28,NULL,'f',26,26);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (29,1,29,NULL,'f',27,27);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (30,1,30,NULL,'f',28,28);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (31,1,31,NULL,'f',29,29);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (32,1,32,NULL,'f',30,30);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (33,1,33,NULL,'f',31,31);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (34,2,34,NULL,'f',33,33);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (35,2,35,NULL,'f',34,34);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (36,2,36,NULL,'f',35,35);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (37,2,37,NULL,'f',36,36);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (38,2,38,NULL,'f',37,37);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (39,2,39,NULL,'f',38,38);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (40,2,40,NULL,'f',39,39);
      INSERT INTO branches (branch_id,topic_id,base_branch_id,parent,outdated,latest_id,content_id) VALUES (41,2,41,NULL,'f',40,40);
    ", $pg, __FILE__, __LINE__);

    pg_query("
      INSERT INTO choice_components (revision_id,type,parent,branch_id,revision,save_id,merged,ctext,choices,other_choice,first_number,last_number,flags,rows) VALUES (11,2,NULL,13,1,NULL,NULL,'','{excellent,very good,satisfactory,poor,disastrous}','',5,1,16777216,0);
      INSERT INTO choice_components (revision_id,type,parent,branch_id,revision,save_id,merged,ctext,choices,other_choice,first_number,last_number,flags,rows) VALUES (31,2,NULL,33,1,NULL,NULL,'','{not at all,a great deal}','',0,5,16777221,0);
      INSERT INTO choice_components (revision_id,type,parent,branch_id,revision,save_id,merged,ctext,choices,other_choice,first_number,last_number,flags,rows) VALUES (39,2,NULL,40,1,NULL,NULL,'','{excellent,very good,satisfactory,poor,disastrous}','',5,1,16777216,0);
  
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (1,6,NULL,3,1,NULL,NULL,'Instructor: Organization and Preparation');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (2,6,NULL,4,1,NULL,NULL,'Instructor: Classroom Delivery');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (3,6,NULL,5,1,NULL,NULL,'Instructor: Approachability');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (4,6,NULL,6,1,NULL,NULL,'Instructor: Overall Quality');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (5,6,NULL,7,1,NULL,NULL,'Course: Amount Learned');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (6,6,NULL,8,1,NULL,NULL,'Course: Appropriateness of Workload');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (7,6,NULL,9,1,NULL,NULL,'Course: Fairness of Grading Process');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (8,6,NULL,10,1,NULL,NULL,'Course: Quality of Text');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (9,6,NULL,11,1,NULL,NULL,'Course: Quality of Teaching Assistants');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (10,6,NULL,12,1,NULL,NULL,'Course: Overall Quality');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (13,6,NULL,15,1,NULL,NULL,'Design experiments');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (14,6,NULL,16,1,NULL,NULL,'Analyze and interpret data');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (15,6,NULL,17,1,NULL,NULL,'Conduct experiments');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (16,6,NULL,18,1,NULL,NULL,'Analyze and interpret data');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (17,6,NULL,19,1,NULL,NULL,'Design a system, component, or process to meet desired needs');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (18,6,NULL,20,1,NULL,NULL,'Function on multidisciplinary teams');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (19,6,NULL,21,1,NULL,NULL,'Identify or formulate engineering problems');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (20,6,NULL,22,1,NULL,NULL,'Solve engineering problems');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (21,6,NULL,23,1,NULL,NULL,'Understand ethical responsibilities');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (22,6,NULL,24,1,NULL,NULL,'Understand the impact of engineering solutions in a global/societal context');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (23,6,NULL,25,1,NULL,NULL,'Use modern engineering tools');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (34,6,NULL,35,1,NULL,NULL,'Overall Quality');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (24,6,NULL,26,1,NULL,NULL,'Communicate using oral presentations');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (25,6,NULL,27,1,NULL,NULL,'Communicate using written reports');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (26,6,NULL,28,1,NULL,NULL,'Pilot test a component prior to implementation');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (27,6,NULL,29,1,NULL,NULL,'Use text materials to support project design');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (28,6,NULL,30,1,NULL,NULL,'Integrate knowledge of mathematics, science, and engineering in engineering solutions');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (29,6,NULL,31,1,NULL,NULL,'Apply knowledge of contemporary issues to engineering solutions');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (30,6,NULL,32,1,NULL,NULL,'Recognize need to engage in lifelong learning');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (35,6,NULL,36,1,NULL,NULL,'Knowledgeability');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (36,6,NULL,37,1,NULL,NULL,'Approachability');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (37,6,NULL,38,1,NULL,NULL,'Availability');
      INSERT INTO choice_questions (revision_id,type,parent,branch_id,revision,save_id,merged,qtext) VALUES (38,6,NULL,39,1,NULL,NULL,'Communication');
    ", $pg, __FILE__, __LINE__);
  
    pg_query("
      INSERT INTO textresponse_components (revision_id,type,parent,branch_id,revision,save_id,merged,flags,ctext,rows,cols) VALUES (12,3,NULL,14,1,NULL,NULL,16777216,'Comments:',5,60);
      INSERT INTO textresponse_components (revision_id,type,parent,branch_id,revision,save_id,merged,flags,ctext,rows,cols) VALUES (33,3,NULL,34,1,NULL,NULL,50331648,'<strong>Enter a TA name:</strong> <small>(leave blank to rate all TA''s together)</small>',0,25);
      INSERT INTO textresponse_components (revision_id,type,parent,branch_id,revision,save_id,merged,flags,ctext,rows,cols) VALUES (40,3,NULL,41,1,NULL,NULL,16777216,'Comments:',5,40);
  
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (1,11,1,3);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (2,11,2,4);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (3,11,3,5);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (4,11,4,6);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (5,11,5,7);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (6,11,6,8);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (7,11,7,9);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (8,11,8,10);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (9,11,9,11);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (10,11,10,12);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (11,31,1,15);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (12,31,2,16);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (13,31,3,17);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (14,31,4,18);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (15,31,5,19);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (16,31,6,20);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (17,31,7,21);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (18,31,8,22);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (19,31,9,23);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (20,31,10,24);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (21,31,11,25);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (22,31,12,26);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (23,31,13,27);
    ", $pg, __FILE__, __LINE__);
  
    pg_query("    
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (24,31,14,28);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (25,31,15,29);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (26,31,16,30);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (27,31,17,31);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (28,31,18,32);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (29,32,1,13);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (30,32,2,14);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (31,32,3,33);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (32,39,1,35);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (33,39,2,36);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (34,39,3,37);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (35,39,4,38);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (36,39,5,39);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (37,41,1,34);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (38,41,2,40);
      INSERT INTO list_items (list_item_id,revision_id,ordinal,item_id) VALUES (39,41,3,41);
  
      COMMIT;
  
      SELECT branch_generate();
      SELECT branch_ancestor_generate();
      SELECT branch_topics_generate();
    ", $pg, __FILE__, __LINE__);
  
    print("<p>Done.</p>\n");
  }
  
  if ($do_topics)
  {
    //$db_debug = true;
  
    print("<h3>Question Periods</h3>\n");
    flush();
  
    pg_query("
      BEGIN;
      DELETE FROM temp_user;
      DELETE FROM temp_questionperiod;
      DELETE FROM temp_topic;
      DELETE FROM topics;
      DELETE FROM question_periods;
      DELETE FROM survey_categories;
      DELETE FROM survey_responses;
    ", $pg, __FILE__, __LINE__);
  
    $result = db_exec("SELECT questionperiodid, periodstart, periodend, description, year, semester FROM questionperiods", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      $year = (int)$row['year'];
      $semester = $row['semester'] == 'fall' ? 2 : $row['semester'] == 'summer' ? 1 : 0;
      $d = ucwords($row['semester']) . " $row[year] $row[description]";
      aarray_map("mnullquot", $row);
      pg_query("
        INSERT INTO semester_question_periods (displayname, begindate, enddate, year, semester)
        VALUES ('$d', $row[periodstart], $row[periodend]);
        SELECT temp_questionperiodi($row[questionperiodid], currval('question_period_ids'), $year, $semester)
      ", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print($row['questionperiodid']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
  
    print("<h3>Categories</h3>\n");
  
    $result = db_exec("SELECT topicid, name FROM topics", $my, __FILE__, __LINE__);
    $first = true;
    while($row = mysql_fetch_assoc($result))
    {
      aarray_map("nullquot", $row);
      pg_query("
        INSERT INTO survey_categories (name) VALUES ($row[name]);
        SELECT temp_topici($row[topicid], currval('survey_category_ids'))
      ", $pg, __FILE__, __LINE__);
      if ($first) $first = false; else print(",");
      if (++$i % 100 == 0) { print("<br>\n"); flush(); }
      print($row['name']);
    }
    print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
  
    print("<h3>New Topics</h3>\n");
  
    $s = array(
      "SELECT linkid AS classid, topicid FROM groupings WHERE linktype = 'classes'",
      "SELECT classid, NULL AS topicid FROM cheesyclasses GROUP BY classid",
      "SELECT classid, NULL AS topicid FROM cheesyresponses GROUP BY classid"
    );
  
    foreach($s as $sql)
    {
      $result = db_exec($sql, $my, __FILE__, __LINE__);
      $first = true;
      while($row = mysql_fetch_assoc($result))
      {
        aarray_map("nullquot", $row);
        pg_query("SELECT topic_update(1, temp_classr($row[classid]), temp_topicr($row[topicid]))", $pg, __FILE__, __LINE__);
        if ($first) $first = false; else print(",");
        if (++$i % 100 == 0) { print("<br>\n"); flush(); }
        print($row['classid']);
      }
      print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
    }
    
  //  print("<h3>Old Topics</h3>\n");
  //
  //  $result = db_exec("SELECT classid, topicid FROM answersets GROUP BY classid, topicid", $my, __FILE__, __LINE__);
  //  $first = true;
  //  while($row = mysql_fetch_assoc($result))
  //  {
  //    aarray_map("nullquot", $row);
  //    pg_query("SELECT topic_update(1, temp_classr($row[classid]), temp_topicr($row[topicid]))", $pg, __FILE__, __LINE__);
  //    if ($first) $first = false; else print(",");
  //    if (++$i % 100 == 0) { print("<br>\n"); flush(); }
  //    print($row['classid']);
  //  }
  //  print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
  //
  //  $s = pg_query("COMMIT", $pg, __FILE__, __LINE__);
  //
  //  print("<h3>Completion Records</h3>\n");
  //
  //  $result = db_exec("
  //    SELECT a.classid, a.questionperiodid, cs.userid FROM completesurveys AS cs
  //    INNER JOIN answersets AS a USING (answersetid)
  //  ", $my, __FILE__, __LINE__);
  
  
    // // todo: remove this after importing real results
    //
    // $first = true;
    // while($row = mysql_fetch_assoc($result))
    // {
    //   aarray_map("nullquot", $row);
    //   pg_query("
    //     INSERT INTO survey_responses(revision_id, question_period_id, response_id, topic_id, user_id)
    //     VALUES (32, temp_questionperiodr($row[questionperiodid]), NULL,
    //     (SELECT topic_id FROM wces_topics WHERE class_id = temp_classr($row[classid]))
    //     , temp_userr($row[userid]));
    //   ", $pg, __FILE__, __LINE__);
    //   if ($first) $first = false; else print(",");
    //   if (++$i % 100 == 0) { print("<br>\n"); flush(); }
    //   print($row['classid']);
    // }
    // print("<br>\n" . mysql_num_rows($result) . " rows<br><br>\n");
  
    $s = pg_query("COMMIT", $pg, __FILE__, __LINE__);
  
  }
  
  $factories = array
  (
    new ChoiceFactory(),
    new TextResponseFactory(),
    new TextFactory(),
    new HeadingFactory()
  );
  
  if ($do_getcustoms)
  {
    $base = new Survey();
    $base->load(1,1,$factories);
    $baseq = $base->components[0];
    $basec = $base->components[1];
    $abet = $base->components[2];
  
    $customs = db_exec("
      SELECT classid, questionperiodid, dump
      FROM cheesyclasses
      WHERE classid <> 0
    ", $my, __FILE__, __LINE__);
  
    //$db_debug = true;
    while($row = mysql_fetch_assoc($customs))
    {
      global $ABETQUESTIONS;
  
      $result = pg_query("SELECT topic_id FROM wces_topics WHERE class_id = (SELECT temp_classr($row[classid])) AND parent = 1", $pg, __FILE__, __LINE__);
      $topic_id = (int)pg_result($result,0,0);
      if ($topic_id == 0)
      {
        print("<h1>Unable to find topic for class $row[classid]</h1>");
      }
  
      debugout("topic_id = $topic_id");
  
      $survey = unserialize($row['dump']);
      $survey->orig_id = $base->orig_id;
      foreach(array_keys($survey->components) as $key)
      {
        $item = &$survey->components[$key];
        $type = get_class($item);
        if ($type == "choice")
        {
          // add new fields
          $item->revision_id = false;
          $item->question_ids = array_pad(array(),count($item->questions),false);
  
          foreach($item->questions as $qkey => $qtext)
          {
            $i = array_search($qtext, $baseq->questions);
            if (isset($i) && false !== $i)
            {
              $item->question_ids[$qkey] = $baseq->question_ids[$i];
              $item->revision_id = $baseq->revision_id;
              debugout("Item $key is the base question component (question $i matched)");
            }
          }
        }
        else if ($type == "textresponse")
        {
          if ($item->text == $basec->text)
          {
            debugout("Item $key is the comments component");
            $item->revision_id = $basec->revision_id;
          }
        }
        else if ($type == "abet")
        {
          $which = $item->which;
          $item = $abet;
          $item->questions = $item->question_ids = array();
          foreach($ABETQUESTIONS as $k => $v)
          if ($which[$k])
          {
            $i = $k - 1;
            if ($v != $abet->questions[$i])
            {
              print("<h1>mistake</h1>");
              debugout("'$v' != '{$abet->questions[$i]}'");
            }
            $nitem->questions[] = $abet->questions[$i];
            $nitem->question_ids[] = $abet->question_ids[$i];
          }
        }
      }
  
      //debugout(serialize($survey));
      //exit();
      $survey->save($topic_id, 1, "NULL");
      debugout("<hr>");
      //exit();
    }
  }
  
  if ($do_results)
  {
    print("<form name=f>");
    $responses = db_exec("
      SELECT userid, classid, questionperiodid, dump
      FROM cheesyresponses
      WHERE classid > 0
      ORDER BY questionperiodid, classid
    ", $my, __FILE__, __LINE__);
    
    // trouble LIMIT 19, 1
    
    $lc = $lq = 0;
    for(;;)
    {
      $row = mysql_fetch_assoc($responses);
      if (!$row) break;
  
      if ($lq != $row['questionperiodid'])
      {
        $lq == $row['questionperiodid'];
        $question_period_id = pg_result(pg_query("SELECT temp_questionperiodr($row[questionperiodid])", $pg, __FILE__, __LINE__),0,0);
      }
  
      if ($lc != $row['classid'])
      {
        $lc = $row['classid'];
        $result = pg_query("SELECT topic_id FROM wces_topics WHERE class_id = (SELECT temp_classr($row[classid])) AND parent = 1", $pg, __FILE__, __LINE__);
        $topic_id = pg_result($result,0,0);
        if (!$topic_id)
        {
          debugout("topic_id not found for $row[classid]");
        }
        
        $survey = new Survey();
        $survey->load($topic_id,1,$factories);
      }
  
      $HTTP_POST_VARS = unserialize($row['dump']);
  
      // find the row prefix
      $prefix = "";
      foreach($HTTP_POST_VARS as $k => $v)
      {
        if (substr($k,0,8) == "student_")
        {
          $prefix = "student"; // old
          break;
        }
        else if (substr($k,0,7) == "survey_")
        {
          $prefix = "survey"; // new
          break;
        }
      }
      debugout($prefix,"prefix = ");
      print("HTTP_POST_VARS = " . htmlspecialchars(serialize($HTTP_POST_VARS)));
      
      $questionwidgets = array();
      foreach(array_keys($survey->components) as $k)
      {
        $c = $survey->components[$k]->getwidget("{$prefix}_$k","f",WIDGET_POST);
        $c->loadvalues();
        //$c->prefix .= $row['classid'];
        //$c->display();
        $questionwidgets[] = $c;
      }
  
      $sql = "BEGIN;\n";
      $sql .= "INSERT INTO survey_responses(revision_id, question_period_id, response_id, topic_id, user_id) VALUES ({$survey->orig_id}, $question_period_id, NULL, $topic_id, temp_userr($row[userid]));\n";
      $sql .= "INSERT INTO survey_responses(revision_id, question_period_id, date, topic_id) VALUES ({$survey->orig_id}, $question_period_id, NULL, $topic_id);\n";
      $sql .= "SELECT currval('response_ids')";
      $result = pg_query($sql, $wbes, __FILE__, __LINE__);
      $response_id = (int)pg_result($result, 0, 0);
  
      $failure = false;
      
      foreach(array_keys($questionwidgets) as $key)
      {
        $failure = $failure || !$questionwidgets[$key]->save($response_id);
      }
      
      if ($failure)
        pg_query("ROLLBACK", $wbes, __FILE__, __LINE__);
      else
        $failure = $failure || !pg_query("COMMIT", $wbes, __FILE__, __LINE__);    
        
      if ($failure)  
      {
        $this->failure = true;
        print('<p><strong><font size="+2"><font color=red>Warning:</font> Errors were encountered during the save process. Please report this to <a href="mailto:wces@columbia.edu">wces@columbia.edu</a>. Include your user name and text of any error messages that preceded this one.</font></strong></p>' . "\n");
      }
      print("<hr>");
    }
    print("</form>");
  }  
}
page_bottom(true);

?>

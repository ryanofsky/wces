<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wces/taskwindow.inc");
require_once("wces/import.inc");
login_protect(login_administrator);
page_top("New Semester Initialization");

$i = 0;
if (is_uploaded_file($userfile))
{  if (!$questionsetid)
  {
    print("<font color=red>Please go back and select a Question Set</font>");
  }
  else
  {    taskwindow_start("Progress Window");
    taskwindow_cprint("<h2>Importing Classes</h2>\n");
    taskwindow_flush();
    
    $fp = fopen($userfile,"r");
    $db = wces_connect();
  
    $i = 0;
    $row = 0;
  
    while ($data = fgetcsv ($fp, 8192, ","))
    {
      ++$row;
      if (count($data) != 5)
        taskwindow_cprint("<b>Warning:</b> Row $row does not contain the correct number of fields. (5 expected, " . count($data) . " found)<br>\n");
      else
      {
        $classid = class_update($db,$data[0] . "_" . $data[1] . "_2001_1", &$courseid);
        prof_parsepidname($data[4],$first,$last,$middlei,$dept);
        $pids = prof_findwithfirstlast($db, $first, $last);
        //$pids = array_merge($pids,prof_findwithclassid($db,$classid));
        $pid = prof_merge($db, $pids,Array("name" => prof_makefull($first,$middlei,$last)),Array("first" => $first, "middle" => $middlei, "last" => $last, "source" => "oldclasses"));
        $i = Array("professorid" => $pid);
        if ($data[3]) $i["students"] = $data[3];
        db_updatevalues($db, "Classes", Array("classid" => $classid), $i);
        if ($questionsetid) db_replace($db,"Groupings",Array("linkid" => $courseid, "linktype" => "courses", "questionsetid" => $questionsetid), 0, "questionsetid");
        taskwindow_cprint($data[0] . " Section ". $data[1]." added (courseid = $courseid, classid = $classid, professorid = $pid)<br>\n");
      };
      if (((++$i) % 10) == 1) taskwindow_flush();  
    }
    
/*
  original format. will it ever be used again?
  note: db structure has changed since this was written
  
    while (!feof ($fp))
    {
      $buffer = fgets($fp, 4096);      
      $data = explode("\t",$buffer);
      
      if (count($data) != 9)
      {
        print ("<b>Skipping Malformed Row '$buffer'<br></b>");
      }
      else
      {
        $subj = $data[0];
        $course = $data[1] . $data[2];
        $section = $data[3];
        $year = $data[4];
        $s = $data[5];
        $semester = ($s == "1") ? "spring" : (($s == "2") ? "summer" : "fall");
        $name = $data[6];
        if (strlen($name) > 31 && ($name[31] == "Y" || $name[31] =="N"))
          $name = trim(substr($comment,0,31));
        else
          $name = "";
        
        $first = substr($data[7],0,16);
        $last = substr($data[7],16);
        prof_fixname($first,$last);
        $full = prof_makefull($first,"",$last);
        $students = $data[8];       
        $subjectid = db_replace($db,"Subjects", Array("code" => $subj), 0, "subjectid");
        $courseid = db_replace($db, "Courses", Array("subjectid" => $subjectid, "code" => $course), $name ? Array("name" => $name) : 0, "courseid");
        $classid = db_replace($db, "Classes", Array("courseid" => $courseid, "section" => $section, "year" => $year, "semester" => $semester), 0, "classid");
        $pids = prof_findwithfirstlast($db,$first,$last);
        $pids = array_merge($pids,prof_findwithclassid($db,$classid));
        $pid = prof_merge($db, $pids, Array("name" => $full), Array("first" => $first, "last" => $last, "fullname" => $full, "source" => "oldclasses"));
        db_updatevalues($db,"Classes",Array("classid" => $classid),Array("students" => $students, "professorid" => $pid));
        db_replace($db,"Groupings",Array("linkid" => $courseid, "linktype" => "courses", "questionsetid" => $questionsetid), 0, "questionsetid");
        taskwindow_cprint(" - Added " . $data[4] . $data[5] . $data[0] . $data[2] . $data[1] . $data[3] . " as class #$classid with professor #$pid<br>\n");
        if ((++$i) % 10 == 1) taskwindow_flush();
      }
    }
*/  

    fclose ($fp); 
    taskwindow_end();  
  }      
}
else
{
%>
  <FORM ENCTYPE="multipart/form-data" METHOD=POST name=form1>
  <INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="8388608">
  <FIELDSET>
  <LEGEND align="top">New Semester Initialization</LEGEND>
  <table>
  <tr><td>Tabbed registrar file:</td><td><INPUT NAME="userfile" TYPE="file"> <INPUT TYPE="submit" VALUE="Send" name="upload"></td></tr>
  <tr><td valign=top>Choose a Question Set:</td><td><%
  print ('  <SELECT NAME="questionsetid" size="5">');  $db = wces_connect();  $results = db_getrows($db,"QuestionSets",Array("type" => "public"),Array("questionsetid","displayname"));
  foreach($results as $result)
    print('<option value="' . $result["questionsetid"] . '">' . $result["displayname"] . '</option>');
  print('</SELECT><BR>');%>  <INPUT type=button value="Preview..." id=button1 name=button1><INPUT type=button value="New..." id=button2 name=button2></td></tr>
  <tr><td>&nbsp;</td><td><INPUT type=submit value="Submit" id=submit1 name=submit1></td></tr>
  </table>
  </FIELDSET>  </FORM>
<%
};page_bottom();
%>
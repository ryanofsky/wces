<%
require_once("server.inc");
require_once("login.inc");
require_once("page.inc");
require_once("wces.inc");
require_once("taskwindow.inc");
login_protect(login_administrator);
page_top("New Semester Initialization");

$i = 0;

{
  {
    print("<font color=red>Please go back and select a Question Set</font>");
  }
  else
  {
    taskwindow_cprint("<h2>Importing Classes</h2>\n");
    taskwindow_flush();
    
    $fp = fopen($userfile,"r");
    $db = wces_connect();
  
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
  <tr><td valign=top>Choose a Question Set:</td><td>
  print ('  <SELECT NAME="questionsetid" size="5">');
  foreach($results as $result)
    print('<option value="' . $result["questionsetid"] . '">' . $result["displayname"] . '</option>');
  print('</SELECT><BR>');
  <tr><td>&nbsp;</td><td><INPUT type=submit value="Submit" id=submit1 name=submit1></td></tr>
  </table>
  </FIELDSET>
<%
};
%>
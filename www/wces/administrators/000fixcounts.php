<%
require_once("page.inc");
require_once("server.inc");
require_once("login.inc");
login_protect(login_administrator);

function printform()
{ %>

  <FORM ENCTYPE="multipart/form-data" METHOD=POST name=form1>
  <INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="8388608">
  <FIELDSET>
  <LEGEND align="top">File Upload</LEGEND>
  <table>
  <tr><td>Upload this file:</td><td><INPUT NAME="userfile" TYPE="file"> <INPUT TYPE="submit" VALUE="Send" name="upload"></td></tr>
  <tr><td valign=top>&nbsp;</td><td><INPUT type="submit" NAME="import" VALUE="Go"></td></tr>
  </table>
  
  </FIELDSET>
  <%
};


$db_debug = true;

function importcsv($filename)
{
  taskwindow_start("Progress Window");
  taskwindow_cprint("<h2>Importing Registrar Data</h2>\n");
  taskwindow_flush();

  $fp = fopen ($filename,"r");
  $db = wces_connect();
  
  $i = 0;
  while ($data = fgetcsv ($fp, 8192, ","))
  {
    if (count($data) != 4)
    {
      taskwindow_cprint("This row does not contain the correct number of fields. (4 expected, " . count($data) . " found)<br>\n");
    }
    else
    {
      $subj = substr($data[1],0,4);
      $coursecode = substr($data[1],4);
      $subjectid    = db_replace($db, "Subjects",    Array("code" => $subj),0,"subjectid");
      $departmentid = db_replace($db, "Departments", Array("code" => $data[0]), 0, "departmentid");
      $courseid     = db_replace($db, "Courses",     Array("subjectid" => $subjectid, "code" => $coursecode),0, "courseid");
      $classid      = db_replace($db, "Classes",     Array("courseid" => $courseid, "section" => $data[2], "year" => "2000", "semester" => "fall"), Array("students" => $data[3]), "classid");
      taskwindow_cprint($subj.$coursecode." Section ".$data[2]." ".$data[3]." students<hr>\n");
    };
    if (((++$i) % 10) == 1) taskwindow_flush();  
  }
  fclose ($fp); 
  taskwindow_end("Progress Window");   
};  

page_top("WCES Mission");

if ($upload && is_uploaded_file($userfile))
  importcsv($userfile);
else
  printform();

if (count($HTTP_POST_VARS))
  print("<a href=import.php>Back</a><br>\n");

page_bottom();

%>
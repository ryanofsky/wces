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

if (isset($userfile) && is_uploaded_file($userfile))
{
  if (!$questionsetid)
  {
    print("<font color=red>Please go back and select a Question Set</font>");
  }
  else
  {
    importdeancsv($userfile,"2000","3",array("coursecode" => 0, "section" => 1, "professor" => 2, "students" => 3, "cunix" => 4),$questionsetid);
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
<%
  print ('  <SELECT NAME="questionsetid" size="5">');
  $db = wces_connect();
  $results = db_getrows($db,"questionsets",Array("type" => "public"),Array("questionsetid","displayname"));
  foreach($results as $result)
    print('<option value="' . $result["questionsetid"] . '">' . $result["displayname"] . '</option>');
  print('</SELECT><BR>');
%>
  <INPUT type=button value="Preview..." id=button1 name=button1><INPUT type=button value="New..." id=button2 name=button2></td></tr>
  <tr><td>&nbsp;</td><td><INPUT type=submit value="Submit" id=submit1 name=submit1></td></tr>
  </table>
  </FIELDSET>
  </FORM>
<%
};
page_bottom();
%>







<?
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wces/taskwindow.inc");
require_once("wces/import.inc");

$db = wces_connect();

db_exec("CREATE TEMPORARY TABLE keepers (classid INTEGER NOT NULL, PRIMARY KEY(classid))", $db, __FILE__, __LINE__);
db_exec("REPLACE INTO keepers SELECT classid FROM answersets", $db, __FILE__, __LINE__);
db_exec("REPLACE INTO keepers SELECT linkid FROM groupings WHERE linktype = 'classes'", $db, __FILE__, __LINE__);
db_exec("REPLACE INTO keepers SELECT classid FROM enrollments", $db, __FILE__, __LINE__);
$result = db_exec("SELECT classid FROM keepers", $db, __FILE__, __LINE__);
$keepers = array();
while($row = mysql_fetch_assoc($result))
{
  $keepers[$row["classid"]] = true;
}

//print($keepers);

function fixdeancsv($userfile, $year, $semester, $fieldorder = array("coursecode" => 0, "section" => 1, "professor" => 2, "students" => 3), $questionsetid = 0)
{
  global $keepers;
  if (strlen($year) == 4 && strlen($semester) && ($semester == 1 || $semester == 2 || $semester == 3) && is_array($fieldorder) && isset($fieldorder["coursecode"]) && isset($fieldorder["section"]) && isset($fieldorder["professor"]) && isset($fieldorder["students"]))
  {
    taskwindow_start("Progress Window");
    taskwindow_cprint("<h2>Importing classes</h2>\n");
    taskwindow_flush();

    $fp = fopen($userfile,"r");
    $db = wces_connect();
  
    $i = 0;
    $row = 0;
  
    $cache = array();
  
    while ($data = fgetcsv ($fp, 8192, ","))
    {
      ++$row;
      if (count($data) != count($fieldorder))
        taskwindow_cprint("<b>Warning:</b> Row $row does not contain the correct number of fields. (" . count($fieldorder) . "expected, " . count($data) . " found)<br>\n");
      else 
      {
        $classcode = $data[$fieldorder["coursecode"]] . '_' . $data[$fieldorder["section"]];

/* BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
        if (!isset($cache[$classcode]))
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB */        
        {
          if (strlen($classcode) == 13)
          {
            if (isset($cache[$classcode]))
              $classid = $cache[$classcode];
            else
            {
              $classid = class_find($db, $classcode . "_" . $year . "_" . $semester, &$courseid);
              $cache[$classcode] = $classid;
            }  

            if ($classid) 
            {

/* BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
              if (isset($keepers[$classid]))
                taskwindow_cprint("Keeping $classid<br>\n");
              else
              {
                taskwindow_cprint("Deleting $classid<br>\n");  
                db_exec("DELETE FROM classes WHERE classid = '$classid'",$db,__FILE__,__LINE__);
              }  
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB */        

/* AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
              $cunix = $data[$fieldorder["cunix"]];
              if ($cunix)
              {
                $userid = db_getvalue($db, "users", Array("cunix" => $cunix), "userid");
                db_exec("DELETE FROM enrollments WHERE classid = '$classid' AND userid = '$userid'", $db, __FILE__, __LINE__);  
                $row = mysql_affected_rows($db);
                taskwindow_cprint("$row: deleting enrollment $userid ($cunix) in $classid ($classcode)<br>\n");
              }
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA */                    
            }
            else
              taskwindow_cprint("not found class $classcode<br>\n");
          }
          else
            taskwindow_cprint("<b>Warning:</b> Invalid course or section code (class = '$classcode')<br>\n");  
        }
      };
      if (((++$i) % 10) == 1) taskwindow_flush();  
    }
    fclose ($fp); 
    taskwindow_end();  
  }  
};

login_protect(login_administrator);
page_top("datafix");

$i = 0;

if (isset($userfile) && is_uploaded_file($userfile))
{
    fixdeancsv($userfile,"2000","3",array("coursecode" => 0, "section" => 1, "professor" => 2, "students" => 3, "cunix" => 4),0);
}
else
{
?>
  <FORM ENCTYPE="multipart/form-data" METHOD=POST name=form1>
  <INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="8388608">
  <FIELDSET>
  <LEGEND align="top">New Semester Initialization</LEGEND>
  <table>
  <tr><td>Tabbed registrar file:</td><td><INPUT NAME="userfile" TYPE="file"> <INPUT TYPE="submit" VALUE="Send" name="upload"></td></tr>
  <tr><td valign=top>Choose a Question Set:</td><td>
<?
  print ('  <SELECT NAME="questionsetid" size="5">');
  $db = wces_connect();
  $results = db_getrows($db,"questionsets",Array("type" => "public"),Array("questionsetid","displayname"));
  foreach($results as $result)
    print('<option value="' . $result["questionsetid"] . '">' . $result["displayname"] . '</option>');
  print('</SELECT><BR>');
?>
  <INPUT type=button value="Preview..." id=button1 name=button1><INPUT type=button value="New..." id=button2 name=button2></td></tr>
  <tr><td>&nbsp;</td><td><INPUT type=submit value="Submit" id=submit1 name=submit1></td></tr>
  </table>
  </FIELDSET>
  </FORM>
<?
};
page_bottom();
?>
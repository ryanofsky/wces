<?
require_once("wces/page.inc");
require_once("wces/import.inc");

param('upload');
param('import');

function printform()
{ global $WCES_COLUMBIA;

?>

  <FORM ENCTYPE="multipart/form-data" METHOD=POST name=form1>
  <INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="8388608">
  <FIELDSET>
  <LEGEND align="top">File Upload</LEGEND>
  <table>
  <tr><td>Upload this file:</td><td><INPUT NAME="userfile" TYPE="file"> <INPUT TYPE="submit" VALUE="Send" name="upload"></td></tr>
  <tr><td valign=top>File Type:</td><td><INPUT TYPE="radio" name="type" id="type1" value="csv"><label for="type1">CSV Class Listing</label><br>
  <input type="radio" name="type" value="pid" id=type2><label for="type2">PID Professor Listing</label></td></tr>
  </table>
  </FIELDSET>
  
  <br>&nbsp;<br>
<? if ($WCES_COLUMBIA) { ?>
  <FIELDSET>
  <LEGEND align="top">Legacy Database Import</LEGEND>
  <table>
  <tr><td valign="top">Options:</td><td>
  <INPUT NAME="doclasses"    ID="classes"    TYPE="checkbox" value="on" CHECKED><label for="classes">Import Classes</label><br>
  <INPUT NAME="doprofessors" ID="professors" TYPE="checkbox" value="on" CHECKED><label for="professors">Import Professors</label><br>
  <INPUT NAME="docourses"    ID="courses"    TYPE="checkbox" value="on" CHECKED><label for="courses">Import Courses</label><br>
  <INPUT NAME="doresponses"  ID="responses"  TYPE="checkbox" value="on" CHECKED><label for="responses">Import Responses</label>
  </td></tr>
  <tr><td>&nbsp;</td><td><INPUT type="submit" NAME="import" VALUE="Go"></td></tr></table>
  
  </FIELDSET>
<? } ?>
  </FORM>

  <?
};

LoginProtect(LOGIN_ADMIN);

page_top("WCES Mission");

if ($upload && is_uploaded_file($userfile))
{
  if ($type == "csv")
    importregcsv($userfile);
  else if ($type == "pid")
    importregpid($userfile);
  else
    print("Unknown file type. Please try again<br>\n");
}
else if ($import)
  importseaseval($doclasses,$doprofessors,$docourses,$doresponses);
else
  printform();

if (count($HTTP_POST_VARS))
  print("<a href=import.php>Back</a><br>\n");

page_bottom();

?>





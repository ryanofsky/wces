<?
  require_once("wces/general.inc");
  require_once("test/test.inc");

  test_top("Student Home","professor");
  param($classid); param($editclass); param($previewclass);
  $name = $classid == 1 ? "COMS3210 Discrete Math - Professor Jonathan Gross" : "COMS3156 Software Engineering - Professor Gail Kaiser";
?>

<form name=f method=post>
  <table>
    <tr><td valign=top><strong>Display Name:</strong></td><td><input type=text name="info_editor_name" value="Jeffrey Achter" size=50></td></tr>
    
    <tr><td valign=top><strong>CUNIX ID:</strong></td><td><input type=text name="info_editor_cunix" value="jda76" size=50></td></tr>
    
    <tr><td valign=top><strong>Email:</strong></td><td><input type=text name="info_editor_email" value="achter@math.columbia.edu" size=50></td></tr>
    <tr><td valign=top><strong>Department:</strong></td><td>
<SELECT name=info_editor_departmentid">
  <OPTION>APAM - Applied Physics and Applied Math</OPTION>
  <OPTION SELECTED>CEEM - Civil Engineering and Engineering Mechanics</OPTION>
</SELECT>
    </td></tr>
    <tr><td valign=top><strong>Homepage:</strong></td><td><input type=text name="info_editor_url" value="" size=50></td></tr>
    <tr><td valign=top><strong>Statement:</strong></td><td><textarea name="info_editor_statement" rows=6 cols=50></textarea></td></tr>
    <tr><td valign=top><strong>Profile:</strong></td><td><textarea name="info_editor_profile" rows=6 cols=50></textarea></td></tr>
    <tr><td valign=top><strong>Education:</strong></td><td><textarea name="info_editor_education" rows=6 cols=50></textarea></td></tr>
    <tr><td valign=top><strong>Picture Upload: </strong><? test_showhelp("picupload"); ?></td><td>
      <input name="info_editor_picture" type=file>      <input type=submit name="info_editor_action_3_" value="Preview New Picture"><br>
      <div><img src="/oracle/prof_images/default.gif"></div>
    </td></tr>
    <tr><td>&nbsp;</td><td><input type=submit name="info_editor_action_2_" value="Save"> <input type=submit name="info_editor_action_3_" value="Cancel"></td></tr>
  </table>    
  <input type=hidden name="info_editbutton_1606" value="1">
</form>  </div>

<? test_bottom(); ?>
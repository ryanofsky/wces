<?
require_once("wces/page.inc");
require_once("wbes/general.inc");
page_top("Feedback");

param($comments);

if (!$comments)
{
?>
<p>Thank you for visiting the Web-based Course Evaluation System (WCES). In order for us to further improve this site, we would appreciate your feedback.</p>
<form name=mail method=post action="feedback.php">
<?=$ISID?>
<table>
<tr><td valign=top>Email (optional) </td><td><input type=text name=email size=30></td></tr>
<tr><td valign=top>Comments </td><td><textarea name=comments rows=7 cols=40></textarea></td></tr>
<tr><td valign=top>&nbsp;</td><td><input type=submit name=submit value="Submit"></td></tr>
</table>
</form>
<?
}
else
{
  sendfeedback($comments, $email);
  print('Your comments have been sent to <a href="mailto:' . $server_feedback . '">' . $server_admin . '</a>. Thank you for taking the time to help us.');
};
page_bottom();
?>
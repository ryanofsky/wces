<?
  require_once("wces/general.inc");
  require_once("test/test.inc");
  param($status);
  test_top("Feedback",$status);

param($comments);

if (!$comments)
{
?>
<p>Thank You for visiting the SEAS Oracle and WCES. In order for us to further improve this site, we would appreciate your feedback.</p>
<form name=mail method=post action="feedback.php">
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
  if (!$email) $email = "Anonymous";
  mail("wces@columbia.edu","WCES WEB FEEDBACK","Feedback from $email\n\n$comments");
  print('Your comments have been sent to <a href="mailto:wces@columbia.edu">wces@columbia.edu</a>. Thank you for taking the time to help us.');
};
test_bottom();
?>
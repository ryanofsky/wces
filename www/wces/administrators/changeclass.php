<?

require_once("wbes/general.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wces/login.inc");
require_once("wces/oldquestions.inc");

param('class_id');
param('course_id');
param('user_id');
param('uni');
param('surveyid');
param('fake');
param('nofake');
param('delete_enrollment'); 
param('user_find');
param('user_uni');
param('user_last');
param('course_find');
param('course_subject');
param('course_code');
param('course_name');


wces_connect();

LoginProtect(LOGIN_ADMIN);

if ($fake && (LoginValue('status') & LOGIN_ADMIN))
{
  global $QSID;
  $login =& LoginInstance();
  $login->update($fake, LoginValue('user_id'), LoginValue('uni'));
  redirect("{$wces_path}index.php$QSID");  
}

if ($nofake && ($fake = LoginValue('fake_id')))
{
  $login =& LoginInstance();
  $login->update($fake);
}




page_top("Modify Classes");

print count($_POST["id"]);

for ($i = 0; $i < count($_POST["id"]); $i++) {
	$section = $_POST["section"][$i];
	$courseid = $_POST["course"][$i];
	$name = $_POST["title"][$i];
	$id = $_POST["id"][$i];
	pg_go("UPDATE CLASSES SET COURSE_ID=$courseid,
	SECTION='$section', NAME='$name' WHERE CLASS_ID=$id", $wces,
	__FILE__, __LINE__);
//	print "section: $section<br>course: $courseid<br>name: $name";
}

print "<head><script
language=javascript>window.location.href='modifyclass.php?id=$_POST[courseid]';</script></head>";

?>

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

if (isset($_GET['op']) && ($_GET['op'] == '2')) {
	pg_go("DELETE FROM ENROLLMENTS_P WHERE CLASS_ID=$_GET[course]", $wces, __FILE__, __LINE__);
	pg_go("DELETE FROM TEMP_CLASS WHERE NEWID=$_GET[course]", $wces, __FILE__, __LINE__);
	pg_go("DELETE FROM CLASSES WHERE CLASS_ID=$_GET[course]", $wces,
	__FILE__, __LINE__);
} else if (isset($_GET['op']) && ($_GET['op'] == '3')) {

		$proflist = '';
		$profs = pg_go("SELECT * FROM USERS WHERE FLAGS=12 OR FLAGS=4 OR
		FLAGS=13
		ORDER BY LASTNAME, FIRSTNAME",$wces,
		__FILE__, __LINE__);
		while ($profdata = pg_fetch_array($profs)) {
			$proflist = "$proflist<option value=$profdata[user_id]>$profdata[lastname], $profdata[firstname]</option>";
		}

	$query = pg_go("SELECT c.*, u.*, p.* FROM CLASSES as c, USERS as u,
	ENROLLMENTS_p as p WHERE p.class_id = c.class_id AND u.user_id=p.user_id
	AND c.COURSE_ID=$_GET[id] AND c.CLASS_ID=$_GET[course]",$wces, __FILE__,
	__LINE__);
	print "<form method=post action=modifyclass.php?op=5&id=$_GET[id]><font face=arial size=2><br>Course ID: $_GET[id] / Class ID:
	$_GET[course]<br>";
	$counter = 0;
	while ($data = pg_fetch_array($query)) {
		print "<br>Current Professor: $data[lastname], $data[firstname]";
		print "<br>New Professor: <select name=profid[$counter]><option
		value=$data[user_id]>--NO CHANGE--</option>";
		print "$proflist</select><br>";
		$counter++;
	}
	print "<input type=hidden name=class_id value=$_GET[course]><input type=submit value=Change><p>";
} else if (isset($_GET['op']) && ($_GET['op'] == '4')) {

	$proflist = '';
	$profs = pg_go("SELECT * FROM USERS WHERE FLAGS=12 OR FLAGS=4 OR
	FLAGS=13 ORDER BY LASTNAME, FIRSTNAME",$wces, __FILE__, __LINE__);
	while ($profdata = pg_fetch_array($profs)) {
		$proflist = "$proflist<option value=$profdata[user_id]>$profdata[lastname], $profdata[firstname]</option>";
	}

	print "<form method=post action=modifyclass.php?op=6&id=$_GET[id]><font face=arial size=2><br>Course ID: $_GET[id] / Class ID:
	$_GET[course]";
	$counter = 0;
	print "<br>New Professor: <select name=profid><option value=-1>--CANCEL--</option>";
	print "$proflist</select><br>";
	$counter++;
	print "<input type=hidden name=class_id value=$_GET[course]><input type=submit value=Add><p>";	

} else if (isset($_GET['op']) && ($_GET['op'] == 5)) {
	pg_go("DELETE FROM ENROLLMENTS_P WHERE CLASS_ID=$_POST[class_id]",
	$wces, __FILE__, __LINE__);
	for ($i=0; $i < count($_POST['profid']); $i++) {
		$prof = $_POST["profid"][$i];
		pg_go("INSERT INTO ENROLLMENTS_P (USER_ID, CLASS_ID, STATUS)
		VALUES ($prof, $_POST[class_id], 4)", $wces, __FILE__, __LINE__);
	}
	

} else if (isset($_GET['op']) && ($_GET['op'] == 6)) {
		print $_POST["profid"];
		print "<br>";
		pg_go("INSERT INTO ENROLLMENTS_P (USER_ID, CLASS_ID, STATUS) VALUES
		($_POST[profid], $_POST[class_id], 4)", $wces, __FILE__, __LINE__);

}




$query = pg_go("SELECT * FROM CLASSES WHERE COURSE_ID=$_GET[id] ORDER BY SECTION ASC", $wces,
__FILE__, __LINE__);

$num = pg_num_rows($query);

print "<font face=arial size=2>$num Sections<br><form method=post action=changeclass.php><table border=1 cellpadding=2
cellspacing=0 width=98%>";

$counter = 0;
$professors = '';

while ($data = pg_fetch_array($query)) {
		$prof = pg_go("SELECT p.*, u.* FROM USERS as u, ENROLLMENTS_P as
		p WHERE p.class_id=$data[class_id] AND u.user_id = p.user_id
		ORDER BY u.LASTNAME ASC", $wces, __FILE__, __LINE__);
		while ($newdata = pg_fetch_array($prof)) {
			if ($professors == '') {
				$professors = "$newdata[lastname],
				$newdata[firstname]";
			} else {
				$professors = "$professors; $newdata[lastname],
				$newdata[firstname]";
			}
		}
	print "<tr><td><font face=arial size=2><input type=hidden name=courseid value=$data[course_id]><input type=hidden name=\"id[$counter]\" value=$data[class_id]><input type=text size=5
	name=\"course[$counter]\" value=\"$data[course_id]\"> /
	<input type=text size=4 name=\"section[$counter]\" value=\"$data[section]\"></td><td><input type=text name=title[$counter]
	value=\"$data[name]\" size=50></td><td><font face=arial
	size=2>$professors<br><font size=1>[<a
	href=\"modifyclass.php?op=3&id=$data[course_id]&course=$data[class_id]\">Modify Professor</a>][<a
	href=\"modifyclass.php?op=4&id=$data[course_id]&course=$data[class_id]\">Add Professor</a>]<td><font face=arial size=2><a
	href=\"modifyclass.php?course=$data[class_id]&op=2&id=$data[course_id]\">Remove
	Section</a></td></tr>";
	$professors = '';
	$counter++;
}
print "</table><p align=right><input type=submit value=Modify><br><a href=manageclasses.php>Back to class listing</a>"


?>

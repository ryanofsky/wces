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



page_top("Manage Classes");

print("<table width=98% border=1 cellpadding=0 cellspacing=2>");

  $classes = pg_go("SELECT * FROM CLASSES ORDER BY YEAR DESC, SEMESTER DESC, NAME ASC",$wces,__FILE__,__LINE__);
  $year = '';
  $semester = '';
  $professors = '';
  $n = pg_numrows($classes);
  for($i = 0; $i < $n; ++$i)
  {
    	$row = pg_fetch_array($classes,$i,PGSQL_ASSOC);
    	if ($row['name'] != '') {
		$prof = pg_go("SELECT p.*, u.* FROM USERS as u, ENROLLMENTS_P as
		p WHERE p.class_id=$row[class_id] AND u.user_id = p.user_id
		ORDER BY u.LASTNAME ASC", $wces, __FILE__, __LINE__);
		while ($data = pg_fetch_array($prof)) {
			if ($professors == '') {
				$professors = "$data[lastname],
				$data[firstname]";
			} else {
				$professors = "$professors; $data[lastname],
				$data[firstname]";
			}
		}
		if (($row['year'] != $year) || ($row['semester'] != $semester)) {
			$term = $wces_semesters[(int)($row['semester'])];
			print("<tr><td colspan=3>&nbsp;</td></tr><tr><td colspan=3><font face=arial
			size=2><b>$row[year] - $term Semester</td></tr>");
			$year = $row['year'];
			$semester = $row['semester'];
		}
		print("  <tr><td><font face=arial size=2>$row[course_id] /
		$row[section]</td><td><font face=arial
		size=2>$row[name]<br><font size=1>Professor(s): $professors</td><td><font face=arial size=2><a
		href=\"modifyclass.php?id=$row[course_id]\">Modify Class</a></td></tr>");
		$professors='';

	}
  }


print "</table>"
?>

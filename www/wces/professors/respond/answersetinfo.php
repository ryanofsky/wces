<?

require_once("wces/profresponse.inc");
require_once("wces/wces.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");

login_protect(login_administrator);


param($answersetID);

if (!$answersetID)
{
  $url = basename($server_url->path) . "?answersetID=1444";
?>
<h3>Bad Parameters</h3>
Here is an example of a proper link: <a href="<?=$url?>"><?=$url?></a>
<?
exit();
}

page_top("Answerset Info Viewer");

$db = wces_connect();

$result = db_exec("SELECT questionsetid,questionperiodid,classid,topicid,responses FROM answersets WHERE answersetid='$answersetID' LIMIT 0,1", $db, __FILE__, __LINE__);

while ($myrow = mysql_fetch_array($result))
  $classID = $myrow["classid"];
mysql_free_result($result);

$result = db_exec("SELECT * FROM classes WHERE classid='$classID' LIMIT 0,1", $db, __FILE__, __LINE__);
while ($myrow = mysql_fetch_array($result))
{
  $courseID = $myrow["courseid"];
  $year = $myrow["year"];
  $semester = $myrow["semester"];
  $professorID = $myrow["professorid"];
  $section = $myrow["section"];
}
mysql_free_result($result);

echo "<a href=\"${oracle_root}oracle_infopane.php?classid=$classID\">Click</a><p>";

$tmp_array = classinfo($classID);

echo "Name: " . $tmp_array["name"] ."<br>\n";
echo "Code: " . $tmp_array["department_code"] . $tmp_array["code"] . "<br>\n";
echo "Time: " . $tmp_array["year"] . "  " . $tmp_array["sem"] . "<br>\n";
echo "Section: " . $tmp_array["section"] . "<br>\n";
echo "Professor: " . $tmp_array["professor_fullname"] . "<br>\n";

page_bottom();

?>
<?php

require("markfunctions.php");

// pass $answersetID
$db = "wces";
$connection = mysql_connect("localhost","root","------");

$query = "SELECT questionsetid,questionperiodid,classid,topicid,responses FROM answersets WHERE answersetid='$answersetID' LIMIT 0,1";
$result = mysql_db_query($db,$query,$connection);

while ($myrow = mysql_fetch_array($result)) {
  $classID = $myrow["classid"];
}
mysql_free_result($result);


$query = "SELECT * FROM classes WHERE classid='$classID' LIMIT 0,1";
$result = mysql_db_query($db,$query,$connection);
while ($myrow = mysql_fetch_array($result)) {
  $courseID = $myrow["courseid"];
  $year = $myrow["year"];
  $semester = $myrow["semester"];
  $professorID = $myrow["professorid"];
  $section = $myrow["section"];
}
mysql_free_result($result);

echo "<a href=\"http://oracle.seas.columbia.edu/oracle/oracle_infopane.php?classid=$classID\">Click</a><p>";

$tmp_array = classinfo($classID);

echo "Name: " . $tmp_array["name"] ."<br>\n";
echo "Code: " . $tmp_array["department_code"] . $tmp_array["code"] . "<br>\n";
echo "Time: " . $tmp_array["year"] . "  " . $tmp_array["sem"] . "<br>\n";
echo "Section: " . $tmp_array["section"] . "<br>\n";
echo "Professor: " . $tmp_array["professor_fullname"] . "<br>\n";

?>




<?php
// pass $classID
$db = "wces";
$connection = mysql_connect("localhost","root","------");

$class = classinfo($classID);

$STR= "For the class: ".$class["name"]." ". $class["department_code"]. $class["code"]." ". $class["year"]." ". $class["sem"]. " Section ". $class["section"]."<br>";

echo $STR;

$answersetID = array();
$query = "SELECT answersetid FROM answersets WHERE classid='$classID'";
$result = mysql_db_query($db,$query,$connection);
while ($myrow = mysql_fetch_array($result)) {
  $answersetID[] = $myrow["answersetid"];
}
mysql_free_result($result);

foreach ($answersetID as $asID) {
  
  $query = "SELECT * FROM links WHERE answersetid='$asID' LIMIT 0,1";
  $result = mysql_db_query($db,$query,$connection);
  while ($myrow = mysql_fetch_array($result)) {

    $myArray["MC1"] = $myrow["MC1"];
    $myArray["MC2"] = $myrow["MC2"];
    $myArray["MC3"] = $myrow["MC3"];
    $myArray["MC4"] = $myrow["MC4"];
    $myArray["MC5"] = $myrow["MC5"];

    $myArray["MC6"] = $myrow["MC6"];
    $myArray["MC7"] = $myrow["MC7"];
    $myArray["MC8"] = $myrow["MC8"];
    $myArray["MC9"] = $myrow["MC9"];
    $myArray["MC10"] = $myrow["MC10"];

    $myArray["ABET1"] = $myrow["ABET1"];
    $myArray["ABET2"] = $myrow["ABET2"];
    $myArray["ABET3"] = $myrow["ABET3"];
    $myArray["ABET4"] = $myrow["ABET4"];
    $myArray["ABET5"] = $myrow["ABET5"];

    $myArray["ABET6"] = $myrow["ABET6"];
    $myArray["ABET7"] = $myrow["ABET7"];
    $myArray["ABET8"] = $myrow["ABET8"];
    $myArray["ABET9"] = $myrow["ABET9"];
    $myArray["ABET10"] = $myrow["ABET10"];

    $myArray["ABET11"] = $myrow["ABET11"];
    $myArray["ABET12"] = $myrow["ABET12"];
    $myArray["ABET13"] = $myrow["ABET13"];
    $myArray["ABET14"] = $myrow["ABET14"];
    $myArray["ABET15"] = $myrow["ABET15"];

    $myArray["ABET16"] = $myrow["ABET16"];
    $myArray["ABET17"] = $myrow["ABET17"];
    $myArray["ABET18"] = $myrow["ABET18"];
    $myArray["ABET19"] = $myrow["ABET19"];
    $myArray["ABET20"] = $myrow["ABET20"];

   foreach ($myArray as $key => $val) {

      $Qtext = getQuestion($asID,$key);
      if (strlen($Qtext) > 0) echo "<td>$Qtext</td> ";


      // use $val to get old answersetID, old question
      $vals = explode("-",$val);
      $old_answersetID = $vals[0];
      $old_qs = $vals[1];

      echo "<td>" . getQuestion($old_answersetID,$old_qs) . "</td>";

      $query4 = "SELECT classid FROM answersets WHERE answersetid='$old_answersetID' LIMIT 0,1";
      $result4 = mysql_db_query($db,$query4,$connection);
      while ($myrow4 = mysql_fetch_array($result4)) {
        $old_classID = $myrow4["classid"];
      } //while
      mysql_free_result($result4);

      $old_class = classinfo($old_classID);
      $tmp_array = array();
      $tmp_array[] = $old_class["department_code"];
      $tmp_array[] = $old_class["code"];
      $tmp_array[] = $old_class["name"];
      $tmp_array[] = $old_class["sem"];
      $tmp_array[] = $old_class["year"];

       echo implode(" ",$tmp_array) . ": $old_qs<br>";

    } //foreach key->val

  } //while (links)
  mysql_free_result($result);
  
} // for each answersetID





//---------------------------------------------------------------
function classinfo($classID) {


$connection = mysql_connect("localhost", "root", "------");
// pass $classID

$db = "wces";
$class = array();

$query = "SELECT year,semester,section,courseid,students,professorid,departmentid FROM classes WHERE classid = '$classID' LIMIT 0,1";
$result = mysql_db_query($db, $query, $connection);
while ($myrow = mysql_fetch_array($result))
{
  $class["year"] = $myrow["year"];
  $class["sem"] = $myrow["semester"];
  $class["section"] = $myrow["section"];
  $class["courseID"] = $myrow["courseid"];
  $class["total_students"] = $myrow["students"];
  $class["professorID"] = $myrow["professorid"];
  $class["departmentID"] = $myrow["departmentid"];
}
mysql_free_result($result);


$query3 = "SELECT code,name FROM courses WHERE courseid = '".$class["courseID"]."' LIMIT 0,1";
$result3 = mysql_db_query($db, $query3, $connection);
while ($myrow3 = mysql_fetch_array($result3))
{
  $class["name"] = $myrow3["name"];
  $class["code"] = $myrow3["code"];
}
mysql_free_result($result3);


$query2 = "SELECT last,fullname FROM professordupedata WHERE professorid = '" . $class["professorID"] ."' LIMIT 0,1";
$result2 = mysql_db_query($db,$query2,$connection);
while ($myrow2 = mysql_fetch_array($result2))
{
  $class["professor_name"] = $myrow2["last"];
  $class["professor_fullname"] = $myrow2["fullname"];
}
mysql_free_result($result2);

$query3 = "SELECT questionperiodid FROM questionperiods WHERE year = '" .$class["year"]. "' AND semester = '". $class["sem"] ."' AND description = 'Final Evaluations' LIMIT 0,1";
$result3 = mysql_db_query($db,$query3,$connection);
while ($myrow3 = mysql_fetch_array($result3))
{
  $class["questionperiodID"] = $myrow3["questionperiodid"];
}
mysql_free_result($result3);

$query4 = "SELECT code FROM departments WHERE departmentid = '".$class["departmentID"]."' LIMIT 0,1";
$result4 = mysql_db_query($db, $query4, $connection);
while ($myrow4 = mysql_fetch_array($result4))
{
  $class["department_code"] = $myrow4["code"];
}
mysql_free_result($result4);


return $class;

} //classinfo


//---------------------------------------------------------------
function getQuestion($answersetID, $whichQ) {

    global $db,$connection;

    $query = "SELECT questionsetid FROM answersets WHERE answersetid='$answersetID' LIMIT 0,1";
    $result = mysql_db_query($db,$query,$connection);

    while ($myrow = mysql_fetch_array($result)) {
        $qsID = $myrow["questionsetid"];
    }
    mysql_free_result($result);

    $query2 = "SELECT $whichQ FROM questionsets WHERE questionsetid='$qsID' LIMIT 0,1";
    $result2 = mysql_db_query($db,$query2,$connection);

    if ($result2) while ($myrow2 = mysql_fetch_array($result2)) {
        return $myrow2[$whichQ];
    }
    if ($result2) mysql_free_result($result2);

    return $question_text;


} //getQuestion


?>
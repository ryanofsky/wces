<?php
$db = "wces";
$connection = mysql_connect("localhost","root","------");

// BEGIN FUNCTION LIST
//---------------------------------------------------------------
function classinfo($classID) {

global $db, $connection;

// pass $classID


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
// getQuestion(int $answersetID, "MC1" etc)
// will return plain text of question
//
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


//---------------------------------------------------------------
// function getLink(int $answersetID, "MC1" etc)
// will return link in form of answerset-question i.e. 1444-MC1
//
function getLink($answersetID, $whichQ) {

    global $db,$connection;

    $query = "SELECT $whichQ FROM links WHERE answersetid='$answersetID' LIMIT 0,1";
    $result = mysql_db_query($db,$query,$connection);

    if ($result) while ($myrow = mysql_fetch_array($result)) {
        $returnMe = $myrow[$whichQ];
    }
    if ($result) mysql_free_result($result);

    return $returnMe;

}

//---------------------------------------------------------------
// function getRawNums(array $AID, "MC1" etc)
// will return an array of raw numbers
//
function getRawNums($AID, $whichQ) {

    global $db, $connection;

    $temp_str = $whichQ."a,".$whichQ."b,".$whichQ."c,".$whichQ."d,".$whichQ."e";

    foreach ($AID as $answersetID) {
       $as_str .= "or answersetid='$answersetID'";
    }

    $as_str = substr($as_str,3);

    $query = "SELECT $temp_str FROM answersets WHERE ($as_str);";
    $result = mysql_db_query($db,$query,$connection);

    $MCA = $MCB = $MCC = $MCD = $MCE = 0;
 
    while ($myrow = mysql_fetch_array($result)) {
        $MCA += $myrow[$whichQ."a"];
        $MCB += $myrow[$whichQ."b"];
        $MCC += $myrow[$whichQ."c"];
        $MCD += $myrow[$whichQ."d"];
        $MCE += $myrow[$whichQ."e"];
    }
    mysql_free_result($result);

    return array($MCA,$MCB,$MCC,$MCD,$MCE);

} // getRawNums


//-----------------------------------------------------------------
// function get array_of_answersetIDs


function getArrayOfAnswersetIDs($professorID,$last_sem,$last_year) {

global $db,$connection;
$CID = array();
$query0 = "SELECT classid FROM classes WHERE professorid='$professorID' AND semester='$last_sem' AND year='$last_year'";
$result0 = mysql_db_query($db,$query0,$connection);
while ($myrow0 = mysql_fetch_array($result0)) {
    $CID[] = $myrow0["classid"];
}
mysql_free_result($result0);

if (count($CID) != 0) {
foreach ($CID as $classID) {
    $class = array();
    $class = classinfo($classID);
    $temp_str .=  "or classid='$classID'";
    $last_questionperiodid = $class["questionperiodID"];
}

$temp_str = substr($temp_str,3);

$query6= "SELECT answersetid FROM answersets WHERE ($temp_str) and questionsetid='1' and questionperiodid='$last_questionperiodid';";
$AID = array();

$result6 = mysql_db_query($db,$query6,$connection);
while ($myrow6 = mysql_fetch_array($result6)) {
    $AID[] = $myrow6["answersetid"];
}
mysql_free_result($result6);

return $AID;
}
else return array(-1);
}

?>

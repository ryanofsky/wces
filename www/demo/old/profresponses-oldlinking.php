<?php

if (!$submit) {

require_once("wces/oldquestions.inc");
$connection = mysql_connect("localhost", "root", "------");
// pass $classID

if (!$classID) $classID = 8174;
if (!$mode) $mode = "prof-compatible";

$db = "wces";

$class = array();
$class = classinfo($classID);


$query5 = "SELECT responses,questionsetid,answersetid FROM answersets WHERE classid = '$classID' AND questionperiodid = '".$class["questionperiodID"]."'";
$result5 = mysql_db_query($db, $query5, $connection);
$questionsetID = array();
$answersetID = array();
while ($myrow5 = mysql_fetch_array($result5))
{
  $responses = $myrow5["responses"];
  array_push($questionsetID,$myrow5["questionsetid"]);
  array_push($answersetID, $myrow5["answersetid"]);
}
mysql_free_result($result5);


if ($class["sem"] == "fall") {
  $lastsemester1 = "spring";
  $lastsemester2 = $class["year"];
}
if ($class["sem"] == "spring") {
  $lastsemester1 = "fall";
  $lastsemester2 = $class["year"] - 1;
}


$query7 = "SELECT classid FROM classes WHERE courseid = '".$class["courseID"]."' AND semester = '$lastsemester1' AND year = '$lastsemester2' LIMIT 0,1";
$result7 = mysql_db_query($db,$query7,$connection);
$lastyear_classID = "";
while ($myrow7 = mysql_fetch_array($result7))
{
  $lastyear_classID = $myrow7["classid"];
}
mysql_free_result($result7);

if (strlen($lastyear_classID) == 0) $lastyear_asID = -1;

$query = "SELECT questionperiodid FROM questionperiods WHERE year = '$lastsemester2' AND semester = '$lastsemester1' AND description = 'Final Evaluations' LIMIT 0,1";
$result = mysql_db_query($db,$query,$connection);
while ($myrow = mysql_fetch_array($result))
{
  $last_questionperiodID = $myrow["questionperiodid"];
}

$query6 = "SELECT answersetid FROM answersets WHERE classid = '$lastyear_classID' AND questionperiodid = '".$last_questionperiodID."' AND questionsetid = '1' LIMIT 0,1";
$result6 = mysql_db_query($db, $query6, $connection);
while ($myrow6 = mysql_fetch_array($result6))
{
  $lastyear_asID = $myrow6["answersetid"];
}
mysql_free_result($result6);



// BEGIN HTML
//--------------------------------------------------------
echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><HTML><HEAD>
<TITLE>Professor Response Page</TITLE>
<style type=\"text/css\">
<!--
body {font-family: arial,helvetica,sans-serif;font-size: 10pt}
td {font-family: arial,helvetica,sans-serif;font-size: 10pt}
A:HOVER {color: #FF0000}
-->
</style>
</HEAD>
<BODY bgcolor=\"#FFFFFF\">

<strong><font size=\"+1\">";

if ($mode != "admin") {
  echo "Welcome, Professor ";
  echo $class["professor_name"];
  echo "<p>You are responding to the survey results for: <br>";
} else {
  echo "You are viewing Professor " . $class["professor_fullname"] . "'s responses to the survey results for: <br>";
}


echo $class["department_code"];
echo $class["code"];
echo $class["name"];
echo "  Section ".$class["section"]." ".$class["sem"]." ".$class["year"]."</font></strong><p>

<strong>Response Statistics</strong><br>
Total Students: ".$class["total_students"]."<br>
Students Evaluated: $responses <br>

<img src=\"/wces/media/graphs/susagegraph.php?blank=" . ($class["total_students"]-$responses) . "&filled=$responses\" width=200 height=200><img src=\"images/susagelegend.gif\" width=147 height=31>
<p>
(Survey Results Compared Against <a href=\"http://oracle.seas.columbia.edu/oracle/oracle_infopane.php?classid=$lastyear_classID\">This Class</a>)<p>";

if ($mode == "prof") echo "If you are having problems with this form (i.e. your browser does not support Javascript), please use the <a href=\"profresponses.php?classID=$classID&mode=prof-compatible\">non-Javascript page</a>.";

echo "<br><center>";

if ($mode != "admin") echo "<form method=\"post\" action=\"profresponses.php\">";

echo "<table border=1 width=600 bgcolor=\"#0065AD\">
<tr align=\"center\">
<td width=400><font color=\"#FFFFFF\"><strong>Survey Results</strong></font></td>
<td><strong><font color=\"#FFFFFF\">This Year's Avg</font></strong></td>
<td><strong><font color=\"#FFFFFF\">Last Year's Avg</font></strong></td>
<td><img src=\"images/delta.jpg\"></td>";

if ($mode == "prof") echo "<td><strong><font color=\"#FFFFFF\">Respond?</font></strong></td>";

echo "</tr></table><p>";


// generate the GENERAL AND CUSTOM QUESTIONS
$counter = 0;
foreach ($questionsetID as $qID) {
  if ($counter != 0) $lastyear_asID = -1;

  $query = "SELECT displayname FROM questionsets WHERE questionsetid = '$qID' LIMIT 0,1";
  $result = mysql_db_query($db,$query,$connection);
  while ($myrow = mysql_fetch_array($result)) $q_name = $myrow["displayname"];
  mysql_free_result($result);

  echo "<table border=1 width=600 bgcolor=\"#0065AD\"><tr align=\"center\"><td><font color=\"#FFFFFF\"><strong>$q_name</strong></font></td></tr></table><p>";
  generateSetOfResults($qID,$answersetID[$counter],$lastyear_asID,$answersetID[$counter]);
  $counter++;
} //foreach


// generate the ABET QUESTIONS
//
echo "<table border=1 width=600 bgcolor=\"#0065AD\"><tr align=\"center\"><td><font color=\"#FFFFFF\"><strong>ABET Questions</strong></font></td></tr></table><p>";

foreach ($answersetID as $asID) generateABET($asID, $lastyear_asID);

if (($mode == "prof") || ($mode == "prof-compatible")) {
  echo "<p><strong>Additional Comments regarding the course:</strong><br><textarea name=\"comments\" rows=\"3\" cols=\"55\"></textarea><p><input type=\"submit\" value=\"Submit\" name=\"submit\"></form>";
} else if ($mode == "admin") {

// display the ADDITIONAL COMMENTS

  $asID = array_pop($answersetID);

  $query = "SELECT * FROM responses WHERE answersetid = '$asID' LIMIT 0,1";
  $result = mysql_db_query($db,$query,$connection);

  while ($myrow = mysql_fetch_array($result)) {
    $PROF_RESPONSE = $myrow["comments"];
  }
  mysql_free_result($result);

  if (strlen($PROF_RESPONSE) != 0) echo "<p><strong>Professor " . $class["professor_name"] ."'s Comments:</strong> $PROF_RESPONSE<p>";
} //if admin

echo "<!------------------------>
<script type=text/javascript>
// _w : ID of span to hide/show
// _h : (h)ide or (s)how
function toggleT(_w,_h) {
  if (document.all) {// is IE
    if (_h=='s') eval(\"document.all.\"+_w+\".style.visibility='visible';\");
    if (_h=='h') eval(\"document.all.\"+_w+\".style.visibility='hidden';\");
  } else {// is NS? -- just guessing for this simple example...
    if (_h=='s') eval(\"document.layers['\"+_w+\"'].visibility='show';\");
    if (_h=='h') eval(\"document.layers['\"+_w+\"'].visibility='hide';\");
  }
}
</script>
<!----------------------->
</center></body></html>";



} // IF
else {
$db = "wces";
$connection = mysql_connect("localhost","root","------");

$myArray = array();

while (list($var, $value) = each($HTTP_POST_VARS)) {
  if (($var != "submit") && ($var != "comments") && (substr($var,0,3) != "rad")) {
    $temp_array = explode("-",$var);
    if (substr($value,0,20) == "Actions to Improve: ") $value = substr($value,20);
    $myArray[$temp_array[1]][$temp_array[0]] = $value;
  } //if

}//while

ksort($myArray);
reset($myArray);

foreach ($myArray as $thiskey => $inner_array) {
  $inner_keys = array_keys($inner_array);
  $QUERY = "INSERT INTO responses (answersetid," . implode(",",$inner_keys) . ") VALUES ('$thiskey',";
  foreach ($inner_array as $inner_element) $QUERY .= "'$inner_element',";
  $QUERY = substr($QUERY,0,strlen($QUERY)-1) . ")";
  $result = mysql_db_query($db,$QUERY,$connection);
}

$query2 = "UPDATE responses SET comments='$comments' WHERE answersetid = '$thiskey'";
$result = mysql_db_query($db,$query2,$connection);

echo "Information uploaded.";

} // IF


// BEGIN FUNCTION LIST
//---------------------------------------------------------------

function generateSetOfResults($qsID, $asID, $lastyear_asID, $name_of_set)
{

  global $connection, $db;

  $query = "SELECT * FROM questionsets WHERE questionsetid = '$qsID' LIMIT 0,1";
  $result = mysql_db_query($db,$query,$connection);

  while ($myrow = mysql_fetch_array($result)) {

    for ($num = 1; $num < 11; $num++) {
      $theQuestion = "MC$num";
      $A = $theQuestion . "A";
      $B = $theQuestion . "B";
      $C = $theQuestion . "C";
      $D = $theQuestion . "D";
      $E = $theQuestion . "E";

      if ($myrow[$theQuestion] != "") generateResult($myrow[$theQuestion],array($A,$B,$C,$D,$E),$asID,$lastyear_asID,$theQuestion ."-". $name_of_set);

    } //for

  } //while

} //generateSetOfResults


//---------------------------------------------------------------

function generateABET($asID, $lastyear_asID)
{
  
  global $connection, $db, $ABETQUESTIONS;

  for ($num = 1; $num < 21 ; $num++) {
    $temparray = array("ABET".$num."f","ABET".$num."e","ABET".$num."d","ABET".$num."c","ABET".$num."b","ABET".$num."a");

    $query = "SELECT " . implode(",",$temparray) . " FROM answersets WHERE answersetid = '$asID' LIMIT 0,1";
    $result = mysql_db_query($db,$query,$connection);

    while ($myrow = mysql_fetch_array($result)) {
      if (($myrow[$temparray[0]] + $myrow[$temparray[1]] + $myrow[$temparray[2]] + $myrow[$temparray[3]] + $myrow[$temparray[4]] + $myrow[$temparray[5]]) != 0) $go = 1;
      else $go = 0;
    } //while
    mysql_free_result($result);

    if ($go == 1) generateResult($ABETQUESTIONS[$num], $temparray, $asID, $lastyear_asID,"ABET".$num."-".$asID);
  } //for

} //generateABET



//---------------------------------------------------------------
function generateResult($Question,$list_of_choices,$asID,$lastyear_asID,$name_of_set)
{
  global $connection, $db, $responses, $mode;
  $query = "SELECT " . implode(",",$list_of_choices) . " FROM answersets WHERE answersetid = '$asID' LIMIT 0,1";
  $result = mysql_db_query($db,$query,$connection);


// generate QUESTION
  echo "<table width=600 border=1><tr><td colspan=5 bgcolor=\"#CCCCFF\"><strong>$Question</strong></td></tr><tr><td width=400>";


// generate BAR GRAPH
  while ($myrow = mysql_fetch_array($result)) {

    $array_of_percentages = array();
    $array_of_rawnums = array();

    foreach($list_of_choices as $CN) array_push($array_of_rawnums, $myrow[$CN]);

    foreach($array_of_rawnums as $RN) $numresponses += $RN;

    foreach($array_of_rawnums as $RN) {
      $percentage = round(1000 * $RN / $numresponses) / 10;
      array_push($array_of_percentages, $percentage);	
    } //foreach

    $array_of_percentages = array_reverse($array_of_percentages);
    generateBarGraph( count($array_of_percentages), $array_of_percentages);
  } //while

  mysql_free_result($result);

// generate THIS YEAR'S AVG
  $start = 5; $sum = 0;
  foreach($array_of_rawnums as $RAW) $sum += $start-- * $RAW;
  $thisyear_avg = round(100*$sum/$numresponses)/100;


// generate LAST YEAR'S AVG
  if ($lastyear_asID != -1) {
    $query = "SELECT " . implode(",",$list_of_choices) . " FROM answersets WHERE answersetid = '$lastyear_asID' LIMIT 0,1";
    $result = mysql_db_query($db,$query,$connection);

    $array_of_rawnums2 = array();

    while ($myrow = mysql_fetch_array($result)) foreach($list_of_choices as $CN) array_push($array_of_rawnums2, $myrow[$CN]);

    mysql_free_result($result);

    $lastyear_responses = 0;
    foreach($array_of_rawnums2 as $RN) $lastyear_responses += $RN;

    if ($lastyear_responses != 0) {
      $start = 5; $sum = 0;
      foreach($array_of_rawnums2 as $RAW) $sum += $start-- * $RAW;
      $lastyear_avg = round(100*$sum/$lastyear_responses)/100;
    } //if
    else $lastyear_avg = "N/A";
  } //if
  else $lastyear_avg = "N/A";


// generate DIFFERENTIAL

  if ($lastyear_avg != "N/A") {
    $differential = $thisyear_avg - $lastyear_avg;
    if ($differential < 0) $differential = "<font color=\"#FF0000\">$differential</font>";
    if ($differential == "0") $differential = "<font color=\"#CCCCCC\">0</font>";
    if ($differential > 0) $differential = "<font color=\"#006600\">+$differential</font>";
  } //if
  else $differential = "N/A";

// generate RESPOND?

  if ($mode == "prof-compatible") {
    echo "</td><td>$thisyear_avg</td><td>$lastyear_avg</td><td><strong>$differential</strong></td></tr></table>";

    echo "<textarea name=\"$name_of_set\" rows=\"3\" cols=\"55\">Actions to Improve: </textarea><p>";
  }
  else if ($mode == "prof") {
    echo "</td><td>$thisyear_avg</td><td>$lastyear_avg</td><td><strong>$differential</strong></td><td>NO<input type=\"radio\" value=\"\" checked name=\"rad$name_of_set\" onClick=\"toggleT('divt$name_of_set','h')\">&nbsp;&nbsp;YES<input type=\"radio\" value=\"\" name=\"rad$name_of_set\" onClick=\"toggleT('divt$name_of_set','s')\">";

    echo "</td></tr></table><p><span id=\"divt$name_of_set\" style=\"visibility:hidden;position:relative;top:0;left:0\"><textarea name=\"$name_of_set\" rows=\"3\" cols=\"55\">Actions to Improve: </textarea></span><p>";
  }
  else if ($mode == "admin") {
    $query = "SELECT * FROM responses WHERE answersetid = '$asID' LIMIT 0,1";
    $result = mysql_db_query($db,$query,$connection);

    $temp1 = $list_of_choices[0];
    $temp_str = substr($temp1,0,-1);

    while ($myrow = mysql_fetch_array($result)) {
      $PROF_RESPONSE = $myrow[$temp_str];
    }
    mysql_free_result($result);

    echo "</td><td>$thisyear_avg</td><td>$lastyear_avg</td><td><strong>$differential</strong></td>";

    if (strlen($PROF_RESPONSE) != 0) echo "<tr><td colspan=5><strong>Professor's Response:</strong> $PROF_RESPONSE</td></tr></table><p>";
    else echo "</tr></table><p>";

  } //if admin

} //generateResult


//---------------------------------------------------------------
function generateBarGraph($num_choices, $percentages)
{

  $midpoint = intval($num_choices / 2);
  $color_increment_red = intval(255 / $midpoint);
  $color_increment_green = intval(255 / ($num_choices-$midpoint-1));

  $i = 0;

  echo "<table border=0 width=400 bgcolor=\"#000000\"><tr align=\"center\">";

  foreach($percentages as $P) {
    $width = intval($P/100 * 400);
    if ($width != 0) {
      echo "<td bgcolor=\"#";
      if ($i < $midpoint) $color = dechex(($i + 1) * $color_increment_red) . "0000";
      if ($i == $midpoint) $color = "FFFF00";
      if ($i > $midpoint) $color = "00" . dechex(255 - ($i - ($midpoint+1)) * $color_increment_green) . "00";

      $width = intval($P/100 * 400);
      echo "$color\" width=" . $width . ">";

      if ($width > 30) {
        if ((hexdec(substr($color,0,2)) < 256) && (hexdec(substr($color,3,2)) < 153)) echo "<font color=\"#FFFFFF\"><strong>$P%</strong></font></td>";
        else echo "<strong>$P%</strong></td>";
      }
      else if ($width > 20) {
        if ((hexdec(substr($color,0,2)) < 256) && (hexdec(substr($color,3,2)) < 153)) echo "<font color=\"#FFFFFF\"><strong>$P</strong></font></td>";
        else echo "<strong>$P</strong></td>";
      }
      else echo "&nbsp;</td>";

    } // if non zero width

    echo "\n";
    $i++;
  } //foreach

  echo "</tr></table>";

} //generateBarGraph

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


?>
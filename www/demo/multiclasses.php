<?php
require("markfunctions.php");

if (!$submit) {

require_once("wces/oldquestions.inc");
$connection = mysql_connect("localhost", "root", "------");
if (!$mode) $mode = "prof-compatible";

$db = "wces";

$CID = array();
// pass $professorID, $sem and $year, and $mode

$query0 = "SELECT classid FROM classes WHERE professorid='$professorID' AND semester='$sem' AND year='$year'";
$result0 = mysql_db_query($db,$query0,$connection);
while ($myrow0 = mysql_fetch_array($result0)) {
    $CID[] = $myrow0["classid"];
}
mysql_free_result($result0);

$total_students = 0;
$questionperiodid="";
foreach ($CID as $classID) {
    $class = array();
    $class = classinfo($classID);
    $total_students += $class["total_students"];
    $list_of_classes .= $class["department_code"] . $class["code"] . " " . $class["name"] . "  Section ".$class["section"]." ".$class["sem"]." ".$class["year"] . "<br>";
    $temp_str .=  "or classid='$classID'";
    $questionperiodid = $class["questionperiodID"];
}

$temp_str = substr($temp_str,3);
$query6= "SELECT answersetid,responses FROM answersets WHERE ($temp_str) and questionsetid='1' and questionperiodid='$questionperiodid';";
$responses = 0;
$AID = array();

$result6 = mysql_db_query($db,$query6,$connection);
while ($myrow6 = mysql_fetch_array($result6)) {
    $responses += $myrow6["responses"];
    $AID[] = $myrow6["answersetid"];
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

echo "$list_of_classes</font></strong><p>

<strong>Response Statistics</strong><br>
Total Students: $total_students<br>
Students Evaluated: $responses <br>

<img src=\"/wces/media/graphs/susagegraph.php?blank=" . ($total_students-$responses) . "&filled=$responses\" width=200 height=200><img src=\"images/susagelegend.gif\" width=147 height=31>
<p>";

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
  $query = "SELECT displayname FROM questionsets WHERE questionsetid = '1' LIMIT 0,1";
  $result = mysql_db_query($db,$query,$connection);
  while ($myrow = mysql_fetch_array($result)) $q_name = $myrow["displayname"];
  mysql_free_result($result);

  echo "<table border=1 width=600 bgcolor=\"#0065AD\"><tr align=\"center\"><td><font color=\"#FFFFFF\"><strong>$q_name</strong></font></td></tr></table><p>";
  generateSetOfResults(1,$AID);


// ADDITIONAL COMMENTS

if (($mode == "prof") || ($mode == "prof-compatible")) {
  echo "<p><strong>Additional Comments regarding the course:</strong><br><textarea name=\"comments\" rows=\"3\" cols=\"55\"></textarea><p><input type=\"submit\" value=\"Submit\" name=\"submit\"></form>";
} else if ($mode == "admin") {

// display the ADDITIONAL COMMENTS

  $asID = array_pop($AID);

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

//} // foreach ($CID as $classID)

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

function generateSetOfResults($qsID, $asID)
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

      if ($myrow[$theQuestion] != "") generateResult($myrow[$theQuestion],$theQuestion,$asID,$theQuestion ."-". $asID[0]);

    } //for

  } //while

} //generateSetOfResults




//---------------------------------------------------------------
function generateResult($Question,$whichQ,$asID,$name_of_set)
{
  global $connection, $db, $responses, $mode, $professorID, $sem, $year;

// generate QUESTION
  echo "<table width=600 border=1><tr><td colspan=5 bgcolor=\"#CCCCFF\"><strong>$Question</strong></td></tr><tr><td width=400>";


// generate BAR GRAPH
    $array_of_percentages = array();
    $array_of_rawnums = array();

    $array_of_rawnums = getRawNums($asID,$whichQ);

    foreach($array_of_rawnums as $RN) $numresponses += $RN;

    foreach($array_of_rawnums as $RN) {
      $percentage = round(1000 * $RN / $numresponses) / 10;
      array_push($array_of_percentages, $percentage);	
    } //foreach

    $array_of_percentages = array_reverse($array_of_percentages);
    generateBarGraph( count($array_of_percentages), $array_of_percentages);

// generate THIS YEAR'S AVG
  $start = 5; $sum = 0;
  foreach($array_of_rawnums as $RAW) $sum += $start-- * $RAW;
  $thisyear_avg = round(100*$sum/$numresponses)/100;


// generate LAST YEAR'S AVG

if ($sem == "spring") {
    $last_sem = "fall";
    $last_year = $year-1;
} else {
    $last_sem = "spring";
    $last_year = $year;
}

$AID = array();
$AID = getArrayOfAnswersetIDs($professorID,$last_sem,$last_year);

if ($AID[0] != -1) {
$array_of_rawnums2 = getRawNums($AID,$whichQ);

$start = 5; $sum = 0; $numresponses = 0;

  foreach($array_of_rawnums2 as $RAW) $numresponses += $RAW;
  foreach($array_of_rawnums2 as $RAW) $sum += $start-- * $RAW;
  $lastyear_avg = round(100*$sum/$numresponses)/100;

  $lastyear_avg_display = "<a href=\"multiclasses.php?professorID=$professorID&sem=$last_sem&year=$last_year&mode=admin\">$lastyear_avg</a>";
} // if AID !=-1
else {
$lastyear_avg = "N/A";
$lastyear_avg_display = "N/A";
}


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
    echo "</td><td>$thisyear_avg</td><td>$lastyear_avg_display</td><td><strong>$differential</strong></td></tr></table>";

    echo "<textarea name=\"$name_of_set\" rows=\"3\" cols=\"55\">Actions to Improve: </textarea><p>";
  }
  else if ($mode == "prof") {
    echo "</td><td>$thisyear_avg</td><td>$lastyear_avg_display</td><td><strong>$differential</strong></td><td>NO<input type=\"radio\" value=\"\" checked name=\"rad$name_of_set\" onClick=\"toggleT('divt$name_of_set','h')\">&nbsp;&nbsp;YES<input type=\"radio\" value=\"\" name=\"rad$name_of_set\" onClick=\"toggleT('divt$name_of_set','s')\">";

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

    echo "</td><td>$thisyear_avg</td><td>$lastyear_avg_display</td><td><strong>$differential</strong></td>";

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
      if ($i < $midpoint) $color = "FF0000";
      if ($i == $midpoint) $color = "FFFF00";
      if ($i > $midpoint) $color = "00FF00";

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


?>
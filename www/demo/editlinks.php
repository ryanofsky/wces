<?php
require("markfunctions.php");
require_once("wces/oldquestions.inc");

//pass $asID and $whichQ

$db = "wces";
$connection = mysql_connect("localhost","root","------");

if (!$step) $step = 1;

echo "You are linking the following question: " . getQuestion($asID,$whichQ) . "<p>";

echo "<form action=editlinks.php method=put>";


$next_step = $step + 1;
echo "<input type=hidden name=asID value=$asID>";
echo "<input type=hidden name=whichQ value=$whichQ>";
echo "<input type=hidden name=step value=$next_step>";


switch ($step) {
   case 1: // select a time + department
     echo "STEP 1<p>CHOOSE SEMESTER OF CLASS: <p><select name=semester>
           <option value=spring>Spring
           <option value=fall>Fall</select>";
     echo "<input type=text name=year value=2001><p>";

     echo "CHOOSE DEPARTMENT: <p><select name=departmentid>";
     $q = "SELECT * FROM departments ORDER BY code ASC";
     $r = mysql_db_query($db,$q,$connection);
     while ($mr = mysql_fetch_array($r)) {
        echo "<option value=" . $mr["departmentid"] . ">" . $mr["code"] . " -- " . $mr["name"] . "\n";
     }
     mysql_free_result($r);
     echo "</select><p>";
     break;
   case 2:
     echo "STEP 2<p>CHOOSE CLASS: <p><select name=classid>";
     $q = "SELECT * FROM classes WHERE departmentid='$departmentid' AND year='$year' AND semester='$semester'";
     $r = mysql_db_query($db,$q,$connection);
     while ($mr = mysql_fetch_array($r)) {
       $tmp = classinfo($mr["classid"]);
       echo "<option value=" . $mr["classid"] . ">" . $tmp["department_code"].$tmp["code"] . "  " . $tmp["name"] . "\n";
     }
     mysql_free_result($r);
     echo "</select><p>";
     break;
   case 3:
     echo "STEP 3<p>CHOOSE A QUESTION: <p>";
     outputQuestions($classid);
     break;
   case 4:
     echo "STEP 4<P>UPDATING LINK DATABASE...<P>";
     $q = "SELECT * FROM links WHERE answersetid='$asID'";
     $r = mysql_db_query($db,$q,$connection);

     if (count($r) == 0) {echo "1";$r2 = mysql_db_query($db,"UPDATE links SET $whichQ='$link' WHERE answersetid='$asID'",$connection);}
     else {echo "2";$r = mysql_db_query($db,"INSERT INTO links (answersetid,$whichQ) VALUES ('$asID','$link')",$connection);}

//mysql_db_query($db,"INSERT INTO links (answersetid,$whichQ) VALUES ('$asID','$link')",$connection);

     mysql_free_result($r);
     break;
} //switch

if ($step != 4) echo "<input type=submit name=submit value=\"submit\">";

echo "</form>";






//_-----------------------------------------------------------
//FUNCTIONS

function outputQuestions($classID) {
  global $db,$connection,$ABETQUESTIONS;

  $class = classinfo($classID);

  $answersetID = array();
  $query = "SELECT answersetid FROM answersets WHERE classid='$classID' AND questionperiodid='" . $class["questionperiodID"]. "'";
  $result = mysql_db_query($db,$query,$connection);
  while ($myrow = mysql_fetch_array($result)) {
    $answersetID[] = $myrow["answersetid"];
  }
  mysql_free_result($result);

  echo "<select name=link>";

  foreach ($answersetID as $asID) {
    for ($i = 1; $i < 11; $i++) echo "<option value=$asID-MC$i>" . getQuestion($asID,"MC".$i) . "\n";
  } // for each answersetID


  foreach ($answersetID as $asID) {
    for ($num = 1; $num < 21 ; $num++) {
      $temparray = array("ABET".$num."f","ABET".$num."e","ABET".$num."d","ABET".$num."c","ABET".$num."b","ABET".$num."a");

      $query = "SELECT " . implode(",",$temparray) . " FROM answersets WHERE answersetid = '$asID' LIMIT 0,1";
      $result = mysql_db_query($db,$query,$connection);

      while ($myrow = mysql_fetch_array($result)) {
        if (($myrow[$temparray[0]] + $myrow[$temparray[1]] + $myrow[$temparray[2]] + $myrow[$temparray[3]] + $myrow[$temparray[4]] + $myrow[$temparray[5]]) != 0) $go = 1;
        else $go = 0;
      } //while
      mysql_free_result($result);

      if ($go == 1) echo "<option value=$asID-ABET" . $num . ">$ABETQUESTIONS[$num]\n";
    } //for each ABET question

  } //for each answerset ID (ABET)

  echo "<option value=\"\">No Link\n";
  echo "</select>";

} //outputQuestions



?>
<?php
require_once("wces/profresponse.inc");
require_once("wces/page.inc");
require_once("wces/oldquestions.inc");
require_once("wces/login.inc");

login_protect(login_administrator);

param($asID);
param($whichQ);
param($step,1);

if (!$asID || !$whichQ)
{
  $url = basename($server_url->path) . "?asID=1444&whichQ=MC1";
?>
<h3>Bad Parameters</h3>
Here is an example of a proper link: <a href="<?=$url?>"><?=$url?></a>
<?
exit();
}

page_top("Edit Links");

$db = wces_connect();

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
     $r = db_exec("SELECT * FROM departments ORDER BY code ASC", $db, __FILE__, __LINE__);
     while ($mr = mysql_fetch_array($r)) {
        echo "<option value=" . $mr["departmentid"] . ">" . $mr["code"] . " -- " . $mr["name"] . "\n";
     }
     mysql_free_result($r);
     echo "</select><p>";
     break;
   case 2:
     echo "STEP 2<p>CHOOSE CLASS: <p><select name=classid>";
     $r = db_exec("SELECT * FROM classes WHERE departmentid='$departmentid' AND year='$year' AND semester='$semester'", $db, __FILE__, __LINE__);
     while ($mr = mysql_fetch_array($r))
     {
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
     
     $qsID = QSfromAS($asID);
     
     $r = db_exec("SELECT COUNT(*) FROM links WHERE questionsetid='$qsID'", $db, __FILE__, __LINE__);
     if (mysql_result($r,0) > 0)
     {
       echo "(Updating)";
       $r2 = db_exec("UPDATE links SET $whichQ='$link' WHERE questionsetid='$qsID'", $db,__FILE__,__LINE__);
     }
     else 
     {
       echo "(Inserting)";
       $r = db_exec("INSERT INTO links (questionsetid,$whichQ) VALUES ('$qsID','$link')", $db,__FILE__,__LINE__);
     }

     break;
} //switch

if ($step != 4) echo "<input type=submit name=submit value=\"submit\">";

echo "</form>";

page_bottom();

//_-----------------------------------------------------------
//FUNCTIONS

function outputQuestions($classID) {
  global $db,$ABETQUESTIONS;

  $class = classinfo($classID);

  $answersetID = array();

  $result = db_exec("SELECT answersetid FROM answersets WHERE classid='$classID' AND questionperiodid='" . $class["questionperiodID"]. "'", $db, __FILE__, __LINE__);
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

      $result = db_exec("SELECT " . implode(",",$temparray) . " FROM answersets WHERE answersetid = '$asID' LIMIT 0,1", $db,__FILE__,__LINE__);

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
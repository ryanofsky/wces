<?php
require("markfunctions.php");
require_once("wces/oldquestions.inc");

// pass $classID
$db = "wces";
$connection = mysql_connect("localhost","root","------");

$class = classinfo($classID);

$STR= "For the class: ".$class["name"]." ". $class["department_code"]. $class["code"]." ". $class["year"]." ". $class["sem"]. " Section ". $class["section"]."<p>";

echo $STR;

$answersetID = array();
$query = "SELECT answersetid FROM answersets WHERE classid='$classID' AND questionperiodid='" . $class["questionperiodID"]. "'";
$result = mysql_db_query($db,$query,$connection);
while ($myrow = mysql_fetch_array($result)) {
  $answersetID[] = $myrow["answersetid"];
}
mysql_free_result($result);

foreach ($answersetID as $asID) {

    echo "<table border=1 bgcolor=\"#FFFFFF\"><tr bgcolor=\"#CCCCCC\"><td colspan=3>$asID</td></tr>\n";

    for ($i = 1; $i < 11; $i++) {

        echo "<tr><td>" . getQuestion($asID,"MC".$i);

        $old_set = getLink($asID,"MC".$i);

        $tmp_array = array();
        $tmp_array = explode("-",$old_set);

        echo "</td><td>";
        $linkedQ = getQuestion($tmp_array[0],$tmp_array[1]);
        if (strlen($linkedQ) == 0) echo "Not Linked";
        else echo $linkedQ;
        echo "</td><td><a href=\"editlinks.php?asID=$asID&whichQ=MC$i\">Edit Link...</a></td></tr>\n";
     }

     echo "</table><p>\n";
} // for each answersetID


foreach ($answersetID as $asID) {

  echo "<table border=1><tr bgcolor=\"#CCCCCC\"><td colspan=3>Searching $asID for ABET responses</td></tr>";

  for ($num = 1; $num < 21 ; $num++) {
    $temparray = array("ABET".$num."f","ABET".$num."e","ABET".$num."d","ABET".$num."c","ABET".$num."b","ABET".$num."a");

    $query = "SELECT " . implode(",",$temparray) . " FROM answersets WHERE answersetid = '$asID' LIMIT 0,1";
    $result = mysql_db_query($db,$query,$connection);

    while ($myrow = mysql_fetch_array($result)) {
      if (($myrow[$temparray[0]] + $myrow[$temparray[1]] + $myrow[$temparray[2]] + $myrow[$temparray[3]] + $myrow[$temparray[4]] + $myrow[$temparray[5]]) != 0) $go = 1;
      else $go = 0;
    } //while
    mysql_free_result($result);

    if ($go == 1) {
      echo "<tr><td>$ABETQUESTIONS[$num]</td>";

      $ABETarray = array();
      $ABETarray = getLink($asID,"ABET".$num);

      if (strlen($ABETarray[0]) > 0) echo "<td>$ABETQUESTIONS[$num]</td>";
      else echo "<td>Not Linked</td>";
      echo "<td><a href=\"editlinks.php?asID=$asID&whichQ=ABET$num\">Edit Link...</a></td></tr>\n";

    } //if
  } //for each ABET question

  echo "</table>";

} //for each answerset ID (ABET)


?>
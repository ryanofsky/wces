<?php

require("markfunctions.php");

// pass $professorID, $sem, $year

$db = "wces";
$connection = mysql_connect("localhost","root","------");

$query = "SELECT fullname from professordupedata WHERE professorid='$professorID' LIMIT 0,1";
$result = mysql_db_query($db,$query,$connection);
while ($myrow = mysql_fetch_array($result)) {
    echo "<font size=\"+2\"><strong>Viewing ". $myrow["fullname"] . "'s Rating History</strong></font><p>";
}
mysql_free_result($result);

$original_sem = $sem;
$original_year = $year;


for ($i = 1; $i < 11; $i++) {


$whichQ = "MC" . $i;
$array_of_avgs = array();
$array_of_questionperiods = array();

while (($sem != "spring") || ($year != "1999")) {

    $AID = array();
    $AID = getArrayOfAnswersetIDs($professorID,$sem,$year);
    $Question = "";

    if ($AID[0] != -1) {
        $Question = getQuestion($AID[0], $whichQ);
        $array_of_rawnums2 = getRawNums($AID,$whichQ);

        $start = 5; $sum = 0; $numresponses = 0;

        foreach($array_of_rawnums2 as $RAW) $numresponses += $RAW;
        foreach($array_of_rawnums2 as $RAW) $sum += $start-- * $RAW;
        $avg = round(100*$sum/$numresponses)/100;

    } // if AID !=-1
    else {
         $avg = 0;
    }

        $array_of_avgs[] = $avg;
        $array_of_questionperiods[] = substr($sem,0,1) . $year;


    if ($sem == "spring") {
        $sem = "fall";
        $year = $year-1;
    } else {
        $sem = "spring";
        $year = $year;
    }

} //while

    $array_of_avgs = array_reverse($array_of_avgs);
    $array_of_questionperiods = array_reverse($array_of_questionperiods);

    if ($Question != "") echo "$Question<br><img src=\"graph.php?list_of_avgs=" . implode(",",$array_of_avgs) . "&list_of_questionperiods=". implode(",",$array_of_questionperiods) . "\"><p>";


$sem = $original_sem;
$year = $original_year;


} //for




?>
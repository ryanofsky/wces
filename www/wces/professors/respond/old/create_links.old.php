<?php

$db = "wces";
$connection = mysql_connect("localhost","root","------");

$query = "SELECT answersetid,questionperiodid,classid FROM answersets WHERE questionsetid = '1' LIMIT 0,30";
$result = mysql_db_query($db, $query, $connection);

while ($myrow = mysql_fetch_array($result))
{
  $answersetID = $myrow["answersetid"];
  $classID = $myrow["classid"];
  $questionperiodID = $myrow["questionperiodid"];

  $query2 = "SELECT courseid,year,semester FROM classes WHERE classid = '$classID' LIMIT 0,1";
  $result2 = mysql_db_query($db, $query2, $connection);
  while ($myrow = mysql_fetch_array($result2)) {
    $courseID = $myrow["courseid"];
    $semester = $myrow["semester"];
    $year = $myrow["year"];
  }
  mysql_free_result($result2);

  if ($semester == "fall") {
    $last_semester = "spring";
    $last_year = $year;
  } else {
    $last_semester = "fall";
    $last_year = $year - 1;
  }

  $lastyear_classID = "";
  $query7 = "SELECT classid FROM classes WHERE courseid = '$courseID' AND semester = '$last_semester' AND year = '$last_year' LIMIT 0,1";
  $result7 = mysql_db_query($db,$query7,$connection);
  while ($myrow7 = mysql_fetch_array($result7)) {
    $last_classID = $myrow7["classid"];
  }
  mysql_free_result($result7);

  if (strlen($lastyear_classID) == 0) $lastyear_asID = -1;

  $query4 = "SELECT questionperiodid FROM questionperiods WHERE year = '$last_year' AND semester = '$last_semester' AND description = 'Final Evaluations' LIMIT 0,1";
  $result4 = mysql_db_query($db,$query4,$connection);
  while ($myrow4 = mysql_fetch_array($result4)) {
    $last_questionperiodID = $myrow4["questionperiodid"];
  }
  mysql_free_result($result4);

  $lastyear_asID = -1;
  $query6 = "SELECT answersetid FROM answersets WHERE classid = '$last_classID' AND questionperiodid = '$last_questionperiodID' AND questionsetid = '1' LIMIT 0,1";
  $result6 = mysql_db_query($db, $query6, $connection);
  while ($myrow6 = mysql_fetch_array($result6)) {
    $lastyear_asID = $myrow6["answersetid"];
  }
  mysql_free_result($result6);

  //DEBUG
  echo "answersetID=$answersetID classID=$classID questionperiodID=$questionperiodID courseID=$courseID semester=$semester year=$year last_semester=$last_semester last_year=$last_year last_classID=$last_classID last_questionperiodID=$last_questionperiodID lastyear_asID=$lastyear_asID<p>";

  if ($lastyear_asID != -1) {

    $MYQUERY = "INSERT INTO links MEMBERS (answersetid,MC1,MC2,MC3,MC4,MC5,MC6,MC7,MC8,MC9,MC10,ABET1,ABET2,ABET3,ABET4,ABET5,ABET6,ABET7,ABET8,ABET9,ABET10,ABET11,ABET12,ABET13,ABET14,ABET15,ABET16,ABET17,ABET18,ABET19,ABET20) VALUES ('$answersetID','$lastyear_asID-MC1','$lastyear_asID-MC2','$lastyear_asID-MC3','$lastyear_asID-MC4','$lastyear_asID-MC5','$lastyear_asID-MC6','$lastyear_asID-MC7','$lastyear_asID-MC8','$lastyear_asID-MC9','$lastyear_asID-MC10','$lastyear_asID-ABET1','$lastyear_asID-ABET2','$lastyear_asID-ABET3','$lastyear_asID-ABET4','$lastyear_asID-ABET5','$lastyear_asID-ABET6','$lastyear_asID-ABET7','$lastyear_asID-ABET8','$lastyear_asID-ABET9','$lastyear_asID-ABET10','$lastyear_asID-ABET11','$lastyear_asID-ABET12','$lastyear_asID-ABET13','$lastyear_asID-ABET14','$lastyear_asID-ABET15','$lastyear_asID-ABET16','$lastyear_asID-ABET17','$lastyear_asID-ABET18','$lastyear_asID-ABET19','$lastyear_asID-ABET20')";

    $MYRESULT = mysql_db_query($db,$MYQUERY,$connection);
    echo "Linking $answersetID to $lastyear_asID<br>";

  } // if



}



?>

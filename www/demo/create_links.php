<?php



$db = "wces";
$connection = mysql_connect("localhost","root","------");

$query = "SELECT classid,year,semester,courseid FROM classes";
$result = mysql_db_query($db,$query,$connection);
while ($myrow = mysql_fetch_array($result)) {

  $courseID = $myrow["courseid"];
  $classID = $myrow["classid"];
  $semester = $myrow["semester"];
  $year = $myrow["year"];

    $query3 = "SELECT questionperiodid FROM questionperiods WHERE year = '$year' AND semester = '$semester'";
    $result3 = mysql_db_query($db,$query3,$connection);
    while ($myrow3 = mysql_fetch_array($result3)) {
      $questionperiodID = $myrow3["questionperiodid"];
    }
    mysql_free_result($result3);

    $query3 = "SELECT answersetid FROM answersets WHERE classid='$classID' AND questionperiodid='$questionperiodID' AND questionsetid='1'";
    $result3 = mysql_db_query($db,$query3,$connection);
    while ($myrow3 = mysql_fetch_array($result3)) {
      $answersetID = $myrow3["answersetid"];
    }
    mysql_free_result($result3);

    
    if ($semester == "fall") {
      $last_semester = "spring";
      $last_year = $year;
    } else {
      $last_semester = "fall";
      $last_year = $year - 1;
    }

    $last_classID = "";
    $query3 = "SELECT classid FROM classes WHERE courseid='$courseID' AND year='$last_year' AND semester='$last_semester' LIMIT 0,1";
    $result3 = mysql_db_query($db,$query3,$connection);
    while ($myrow3 = mysql_fetch_array($result3)) {
      $last_classID = $myrow3["classid"];
    }
    mysql_free_result($result3);

    $query3 = "SELECT questionperiodid FROM questionperiods WHERE year='$last_year' AND semester='$last_semester' AND description='Final Evaluations' LIMIT 0,1";
    $result3 = mysql_db_query($db,$query3,$connection);
    while ($myrow3 = mysql_fetch_array($result3)) {
      $last_questionperiodID = $myrow3["questionperiodid"];
    }

    $lastyear_asID = -1;

    $query3 = "SELECT answersetid FROM answersets WHERE classid='$last_classID' AND questionperiodid='$last_questionperiodID' AND questionsetid='1'";
    $result3 = mysql_db_query($db,$query3,$connection);
    while ($myrow3 = mysql_fetch_array($result3)) {
      $lastyear_asID = $myrow3["answersetid"];
    }
    mysql_free_result($result3);


    //DEBUG
//    echo "answersetID=$answersetID classID=$classID questionperiodID=$questionperiodID courseID=$courseID semester=$semester year=$year last_semester=$last_semester last_year=$last_year last_classID=$last_classID last_questionperiodID=$last_questionperiodID lastyear_asID=$lastyear_asID<p>";

    if ($lastyear_asID != -1) {

      $MYQUERY = "INSERT INTO links (answersetid,MC1,MC2,MC3,MC4,MC5,MC6,MC7,MC8,MC9,MC10,ABET1,ABET2,ABET3,ABET4,ABET5,ABET6,ABET7,ABET8,ABET9,ABET10,ABET11,ABET12,ABET13,ABET14,ABET15,ABET16,ABET17,ABET18,ABET19,ABET20) VALUES ('$answersetID','$lastyear_asID-MC1','$lastyear_asID-MC2','$lastyear_asID-MC3','$lastyear_asID-MC4','$lastyear_asID-MC5','$lastyear_asID-MC6','$lastyear_asID-MC7','$lastyear_asID-MC8','$lastyear_asID-MC9','$lastyear_asID-MC10','$lastyear_asID-ABET1','$lastyear_asID-ABET2','$lastyear_asID-ABET3','$lastyear_asID-ABET4','$lastyear_asID-ABET5','$lastyear_asID-ABET6','$lastyear_asID-ABET7','$lastyear_asID-ABET8','$lastyear_asID-ABET9','$lastyear_asID-ABET10','$lastyear_asID-ABET11','$lastyear_asID-ABET12','$lastyear_asID-ABET13','$lastyear_asID-ABET14','$lastyear_asID-ABET15','$lastyear_asID-ABET16','$lastyear_asID-ABET17','$lastyear_asID-ABET18','$lastyear_asID-ABET19','$lastyear_asID-ABET20')";

    mysql_db_query($db,$MYQUERY,$connection);
    echo "Linking $answersetID to $lastyear_asID<br>";

    } // if

  } // for each class in each course
  mysql_free_result($result);





?>
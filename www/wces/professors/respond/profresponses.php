<?

require_once("wces/profresponse.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");

login_protect(login_administrator);

param($submit);
param($classID);
param($mode, "prof-compatible");

if (!$classID)
{
  $url = basename($server_url->path) . "?classID=8174";
?>
<h3>Bad Parameters</h3>
Here is an example of a proper link: <a href="<?=$url?>"><?=$url?></a>
<?
exit();
}

$db = wces_connect();

page_top("The Professor. He responds.");

if (!$submit)
{
  require_once("wces/oldquestions.inc");

  $class = array();
  $class = classinfo($classID);
  
  $result5 = db_exec("
    SELECT responses,questionsetid,answersetid
    FROM answersets
    WHERE classid = '$classID' AND questionperiodid = '".$class["questionperiodID"]."'", $db, __FILE__, __LINE__);
  
  $questionsetID = array();
  $answersetID = array();
  while ($myrow5 = mysql_fetch_array($result5))
  {
    $responses = $myrow5["responses"];
    array_push($questionsetID,$myrow5["questionsetid"]);
    array_push($answersetID, $myrow5["answersetid"]);
  }
  mysql_free_result($result5);
  
  $professor_name = $professor_fullname = $department_code = $code = $name = $section = $sem = $year = $total_students = "";
  extract($class);
?>  


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN"><HTML><HEAD>
<TITLE>Professor Response Page</TITLE>
<style type="text/css">
<!--
body {font-family: arial,helvetica,sans-serif;font-size: 10pt}
td {font-family: arial,helvetica,sans-serif;font-size: 10pt}
A:HOVER {color: #FF0000}
-->
</style>
</HEAD>

<BODY bgcolor="#FFFFFF">

<strong><font size="+1">
<? if ($mode != "admin") { ?>
<p>Welcome, Professor <?=$professor_name?></p>
<p>You are responding to the survey results for: <br>
<? } else { ?>
<p>You are viewing Professor <?=$professor_fullname?>'s responses to the survey results for: <br>
<?  } ?>
<?= "$department_code$code $name Section $section $sem $year" ?>
</p></font></strong>

<strong>Response Statistics</strong><br>
Total Students: <?=$total_students?><br>
Students Evaluated: <?=$responses?><br>
<img src="/wces/media/graphs/susagegraph.php?blank=<?=$total_students-$responses?>&filled=<?=$responses?>" width=200 height=200><img src="images/susagelegend.gif" width=147 height=31>

<? if ($mode == "prof") { ?>
<p>If you are having problems with this form (i.e. your browser does not support Javascript), please use the
<a href="profresponses.php?classID=<?=$classID?>&mode=prof-compatible">non-Javascript page</a></p>
<? } ?>

<br><center>

<? if ($mode != "admin") { ?>
<form method="post" action="profresponses.php">
<? } ?>

<table border=1 width=600 bgcolor="#0065AD">
<tr align="center">
  <td width=400><font color="#FFFFFF"><strong>Survey Results</strong></font></td>
  <td><strong><font color="#FFFFFF">This Year's Avg</font></strong></td>
  <td><strong><font color="#FFFFFF">Last Year's Avg</font></strong></td>
  <td><img src="images/delta.jpg"></td>
  
<? if ($mode == "prof") { ?>
  <td><strong><font color="#FFFFFF">Respond?</font></strong></td>
<? } ?>  

</tr>
</table>

<p>
<?
  
  // generate the GENERAL AND CUSTOM QUESTIONS
  $counter = 0;
  foreach ($questionsetID as $qID)
  {
    $result = db_exec("SELECT displayname FROM questionsets WHERE questionsetid = '$qID' LIMIT 1", $db, __FILE__, __LINE__);
    while ($myrow = mysql_fetch_array($result))
      $q_name = $myrow["displayname"];
    mysql_free_result($result);
  
    echo "<table border=1 width=600 bgcolor=\"#0065AD\"><tr align=\"center\"><td><font color=\"#FFFFFF\"><strong>$q_name</strong></font></td></tr></table><p>";
    generateSetOfResults($qID,$answersetID[$counter]);
    $counter++;
  }
 
  // generate the ABET QUESTIONS
  //
  echo "<table border=1 width=600 bgcolor=\"#0065AD\"><tr align=\"center\"><td><font color=\"#FFFFFF\"><strong>ABET Questions</strong></font></td></tr></table><p>";
  
  foreach ($answersetID as $asID) generateABET($asID);
  
  // ADDITIONAL COMMENTS
  
  if (($mode == "prof") || ($mode == "prof-compatible"))
  {
    echo "<p><strong>Additional Comments regarding the course:</strong><br><textarea name=\"comments\" rows=\"3\" cols=\"55\"></textarea><p><input type=\"submit\" value=\"Submit\" name=\"submit\"></form>";
  }
  else if ($mode == "admin")
  {
    // display the ADDITIONAL COMMENTS
    $asID = array_pop($answersetID);
    $result = db_exec("SELECT comments FROM responses WHERE answersetid = '$asID' LIMIT 1", $db,__FILE__, __LINE__);
    $PROF_RESPONSE = mysql_result($result,0);
    mysql_free_result($result);
    if (strlen($PROF_RESPONSE) != 0) echo "<p><strong>Professor " . $class["professor_name"] ."'s Comments:</strong> $PROF_RESPONSE<p>";
  }
  
?>
<script type=text/javascript>
<!--
// _w : ID of span to hide/show
// _h : (h)ide or (s)how
function toggleT(_w,_h)
{
  if (document.all) // IE or Mozilla
  {
    if (_h=='s') document.all[_w].style.visibility='visible';
    if (_h=='h') document.all[_w].style.visibility='hidden';
  }
  else if(document.layers) // NS4
  {
    if (_h=='s') "document.layers[_w'].visibility='show';
    if (_h=='h') "document.layers[_w'].visibility='hide';
  }
}
// -->
</script>
</center></body></html>
<?

} // IF submit
else
{
  $myArray = array();
  
  while (list($var, $value) = each($HTTP_POST_VARS)) {
    if (($var != "submit") && ($var != "comments") && (substr($var,0,3) != "rad"))
    {
      $temp_array = explode("-",$var);
      if (substr($value,0,20) == "Actions to Improve: ") 
        $value = substr($value,20);
      $myArray[$temp_array[1]][$temp_array[0]] = $value;
    } //if
  }//while
  
  ksort($myArray);
  reset($myArray);
  
  foreach ($myArray as $thiskey => $inner_array)
  {
    $inner_keys = array_keys($inner_array);
    $QUERY = "INSERT INTO responses (answersetid," . implode(",",$inner_keys) . ") VALUES ('$thiskey',";
    foreach ($inner_array as $inner_element) $QUERY .= "'$inner_element',";
    $QUERY = substr($QUERY,0,strlen($QUERY)-1) . ")";
    $result = db_exec($QUERY,$db,__FILE__,__LINE__);
  }
  
  $query2 = "UPDATE responses SET comments='$comments' WHERE answersetid = '$thiskey'";
  $result = db_exec($query2,$db,__FILE__,__LINE__);
 
  echo "Information uploaded.";
  
} // IF

page_bottom();

// BEGIN FUNCTION LIST
//---------------------------------------------------------------

function generateSetOfResults($qsID, $asID)
{
  global $db;

  $result = db_exec("SELECT * FROM questionsets WHERE questionsetid = '$qsID' LIMIT 1", $db, __FILE__, __LINE__);
  while ($myrow = mysql_fetch_array($result)) 
  {
    for ($num = 1; $num <= 10; $num++)
    {
      $theQuestion = "MC$num";
      $A = $theQuestion . "A";
      $B = $theQuestion . "B";
      $C = $theQuestion . "C";
      $D = $theQuestion . "D";
      $E = $theQuestion . "E";
      if ($myrow[$theQuestion] != "") generateResult($myrow[$theQuestion],array($A,$B,$C,$D,$E),$asID,$theQuestion ."-". $asID);
    } //for
  } //while
} //generateSetOfResults


//---------------------------------------------------------------

function generateABET($asID)
{
  global $db, $ABETQUESTIONS;

  for ($num = 1; $num < 21 ; $num++)
  {
    $temparray = array("ABET".$num."f","ABET".$num."e","ABET".$num."d","ABET".$num."c","ABET".$num."b","ABET".$num."a");

    $result = db_exec("SELECT " . implode(",",$temparray) . " FROM answersets WHERE answersetid = '$asID' LIMIT 1", $db, __FILE__, __LINE__);

    while ($myrow = mysql_fetch_array($result)) {
      $go = 0 != $myrow[$temparray[0]] + $myrow[$temparray[1]] + $myrow[$temparray[2]] + $myrow[$temparray[3]] + $myrow[$temparray[4]] + $myrow[$temparray[5]];
    } //while
    mysql_free_result($result);

    if ($go) generateResult($ABETQUESTIONS[$num], $temparray, $asID, "ABET".$num."-".$asID);
  } //for

} //generateABET



//---------------------------------------------------------------
function generateResult($Question,$list_of_choices,$asID,$name_of_set)
{
  global $db, $responses, $mode;

  $result = db_exec("SELECT " . implode(",",$list_of_choices) . " FROM answersets WHERE answersetid = '$asID' LIMIT 0,1", $db, __FILE__, __LINE__);

// generate QUESTION
  echo "<table width=600 border=1><tr><td colspan=5 bgcolor=\"#CCCCFF\"><strong>$Question</strong></td></tr><tr><td width=400>";


// generate BAR GRAPH
  while ($myrow = mysql_fetch_array($result)) {

    $array_of_percentages = array();
    $array_of_rawnums = array();

    foreach($list_of_choices as $CN) array_push($array_of_rawnums, $myrow[$CN]);

    $numresponses = 0;
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

  $fooarray = explode("-",$name_of_set);
  $bararray = array();
  $bararray = explode("-",getLink($fooarray[1],$fooarray[0]));
  $lastyear_asID = $bararray[0];
  if ($lastyear_asID == "") $lastyear_asID = -1;
 
  if (substr($bararray[1],0,2) == "MC") $lastyear_choices = array($bararray[1]."a",$bararray[1]."b",$bararray[1]."c",$bararray[1]."d",$bararray[1]."e");
  else $lastyear_choices =  array($bararray[1]."a",$bararray[1]."b",$bararray[1]."c",$bararray[1]."d",$bararray[1]."e",$bararray[1]."f");

  if ($lastyear_asID != -1)
  {
    $result = db_exec("SELECT " . implode(",",$lastyear_choices) . " FROM answersets WHERE answersetid = '$lastyear_asID' LIMIT 0,1", $db, __FILE__, __LINE__);

    $array_of_rawnums2 = array();

    while ($myrow = mysql_fetch_array($result)) foreach($lastyear_choices as $CN) array_push($array_of_rawnums2, $myrow[$CN]);

    mysql_free_result($result);

    $lastyear_responses = 0;
    foreach($array_of_rawnums2 as $RN) $lastyear_responses += $RN;

    if ($lastyear_responses != 0)
    {
      $result = db_exec("SELECT classid FROM answersets WHERE answersetid='$lastyear_asID' LIMIT 0,1", $db, __FILE__, __LINE__);

      while ($myrow = mysql_fetch_array($result)) {
        $lastyear_classID = $myrow["classid"];
      }
      mysql_free_result($result);

      $start = 5; $sum = 0;
      foreach($array_of_rawnums2 as $RAW) $sum += $start-- * $RAW;
      $lastyear_avg = round(100*$sum/$lastyear_responses)/100;
      $lastyear_avg_display = "<a href=\"http://oracle.seas.columbia.edu/oracle/oracle_infopane.php?classid=$lastyear_classID\">$lastyear_avg</a>";
    } //if
    else $lastyear_avg_display = $lastyear_avg = "N/A";
  } //if
  else $lastyear_avg_display = $lastyear_avg = "N/A";


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
    $result = db_exec("SELECT * FROM responses WHERE answersetid = '$asID' LIMIT 0,1", $db, __FILE__, __LINE__);

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

} //generateBarGraph2

?>
<?

require_once("wces/profresponse.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");

login_protect(login_administrator | login_knownprofessor);

if (!isset($professorID) || !(login_getstatus() & login_administrator)) 
  $professorID = login_getprofid();

if (!$professorID)
{
  $url = basename($server_url->path) . "?professorID=165";
?>
<h3>Bad Parameters</h3>
Here is an example of a proper link: <a href="<?=$url?>"><?=$url?></a>
<?
exit();
}

$db = wces_connect();
wces_GetCurrentQuestionPeriod($db, $null, $null, $year, $sem);

page_top("Professor History", true);

print("<p><strong><a href=\"${wces_path}index.php\">Home</a></strong></p>");

$result = db_exec("SELECT name from professors WHERE professorid='$professorID' LIMIT 0,1",$db,__FILE__,__LINE__);
while ($myrow = mysql_fetch_array($result)) {
    echo "<font size=\"+2\"><strong>Viewing ". $myrow["name"] . "'s Rating History</strong></font><p>";
    flush();
}
mysql_free_result($result);

$original_sem = $sem;
$original_year = $year;

for ($i = 1; $i <= 10; $i++) 
{
  $whichQ = "MC" . $i;
  $array_of_avgs = array();
  $array_of_questionperiods = array();

  $Question = "";
  while ($year >= "1999" || $sem == "fall") 
  {
    $AID = getArrayOfAnswersetIDs($professorID,$sem,$year);
    if ($AID[0] != -1)
    {
      if (!$Question) $Question = getQuestion($AID[0], $whichQ);
      $array_of_rawnums2 = getRawNums($AID,$whichQ);
  
      $start = 5; $sum = 0; $numresponses = 0;
      foreach($array_of_rawnums2 as $RAW) $numresponses += $RAW;
      foreach($array_of_rawnums2 as $RAW) $sum += $start-- * $RAW;
      $avg = round($sum/$numresponses,2);
    } // if AID !=-1
    else 
      $avg = 0;
  
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
  
  if ($Question)
  {
    echo "$Question<br><img src=\"graph.php?list_of_avgs=" . implode(",",$array_of_avgs) . "&list_of_questionperiods=" . implode(",",$array_of_questionperiods) . "\" width=400 height=250><p>";
    flush();
  }  
  
  $sem = $original_sem;
  $year = $original_year;
} //for

page_bottom(true);

?>
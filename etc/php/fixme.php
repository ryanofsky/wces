<?

require_once("wces/general.inc");
require_once("wces/wces.inc");

print("you must edit the source to execute this script");
exit();

$db = wces_connect();

$questionperiodid = wces_Findquestionsetsta($db,"qsets");

$names = array("sms117", "jpm53", "ki126");
foreach($names as $name)
{
  $db_debug = true;
  
  $userid = db_getvalue($db, "users", array("cunix" => $name), "userid");

  // delete enrollments in current semester, for classes with surveys that are not completed
  $classes = db_exec(
  "SELECT cl.classid FROM classes AS cl
  INNER JOIN qsets AS qs ON (qs.classid = cl.classid)
  INNER JOIN enrollments AS e ON e.userid = '$userid' AND e.classid = qs.classid
  LEFT JOIN answersets AS a ON a.questionsetid = qs.questionsetid AND a.classid = qs.classid AND a.questionperiodid = '$questionperiodid'
  LEFT JOIN completesurveys AS cs ON cs.answersetid = a.answersetid AND cs.userid = e.userid
  WHERE cs.userid IS NULL
  GROUP BY cl.classid", $db, __FILE__, __LINE__);
  
  $classset = "";
  while($row = mysql_fetch_assoc($classes))
  {
    if ($classset) $classset .= ", ";
    $classset .= $row["classid"];
  }
  print("$name - $classset<br>");
  if ($classset) db_exec("DELETE FROM enrollments WHERE classid IN ($classset) AND userid = '$userid'", $db, __FILE__, __LINE__);
  print("<hr>");
}

?>

<?

require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("wces/wces.inc");
require_once("wces/oldquestions.inc");
  
login_protect(login_student);
page_top("Student Survey","0100");

$db = wces_connect();
$userid = login_getuserid();

if (isset($classid) && $classid)
{
  $q = new OldSurvey($db, $classid, $userid, "survey.php", "survey", "f", WIDGET_POST);
  $q->loadvalues();
  print("<form name=f method=post action=survey.php>\n");
  if (!$q->finished)
  {
    print('<input type=hidden name=classid value="' . htmlspecialchars($classid) . "\">\n");
    $q->display();
  }
  else
    $classid = 0;     
  print("</form>\n");
}
else
  $classid = 0;

if (!$classid)
{

  $questionperiodid = wces_Findquestionsetsta($db,"qsets");
  $classes = db_exec(
  "SELECT IF((COUNT(DISTINCT q.questionsetid)-COUNT(DISTINCT cs.answersetid) = 0) AND (MAX(qs.tarate) = 'no' OR tac.classid IS NOT NULL),1,0) AS surveyed,
  cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode 
  FROM enrollments AS e 
  INNER JOIN qsets AS qs ON e.classid = qs.classid
  INNER JOIN questionsets AS q ON q.questionsetid = qs.questionsetid
  INNER JOIN classes AS cl ON e.classid = cl.classid
  INNER JOIN courses AS c ON cl.courseid = c.courseid
  INNER JOIN subjects AS s ON c.subjectid = s.subjectid
  LEFT JOIN answersets AS a ON a.questionsetid = q.questionsetid AND a.classid = e.classid AND a.questionperiodid = '$questionperiodid'
  LEFT JOIN completesurveys AS cs ON cs.userid = e.userid AND cs.answersetid = a.answersetid
  LEFT JOIN tacomplete AS tac ON tac.classid = e.classid AND tac.userid = e.userid AND tac.questionperiodid = '$questionperiodid'
  WHERE e.userid = '$userid'
  GROUP BY cl.classid
  ORDER BY surveyed, s.code, c.code
  LIMIT 50",$db,__FILE__,__LINE__);

/*
  $classes = db_exec(
  "SELECT IF(COUNT(DISTINCT q.questionsetid)-COUNT(DISTINCT cs.answersetid)>0,1,0) AS surveyed, cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode 
  FROM enrollments AS e 
  INNER JOIN qsets AS qs ON e.classid = qs.classid
  INNER JOIN questionsets AS q ON q.questionsetid = qs.questionsetid
  LEFT JOIN classes AS cl ON e.classid = cl.classid
  LEFT JOIN courses AS c ON cl.courseid = c.courseid
  LEFT JOIN subjects AS s ON c.subjectid = s.subjectid
  LEFT JOIN answersets AS a ON a.questionsetid = q.questionsetid AND a.classid = e.classid AND a.questionperiodid = '$questionperiodid'
  LEFT JOIN completesurveys AS cs ON cs.userid = e.userid AND cs.answersetid = a.answersetid
  WHERE e.userid = '$userid'
  GROUP BY cl.classid
  ORDER BY surveyed DESC, s.code, c.code
  LIMIT 50",$db,__FILE__,__LINE__);
*/

db_exec("DROP TABLE qsets", $db, __FILE__, __LINE__);

  print ("<p>Choose a class link from this list below.</p>");
  print ("<UL>\n");
  $found = false;
  while ($class = mysql_fetch_assoc($classes))
  {
    $found = true;
    $complete = true;
    extract($class);
    if ($surveyed)  
      print ("  <LI>Survey Complete: " . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>\n");
    else
    {
      $complete = false;
      print ("  <LI><A HREF=\"survey.php?classid=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</a></LI>\n");
    }  
  }
  if (!$found) print ("<LI>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, check our <a href=\"evaluations.php\">class evaluation listing</a> and email <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a> so we can update our database with your information.</LI>");
  print ("</UL>");

  print("<p>Remember to <a href=\"${server_wcespath}login/logout.php\">log out</a> when you are done.</p>");
}



page_bottom();
exit();



  
function listclasses()
{

}

function getquestionsets($classid)
{
  global $db, $userid, $questionperiodid;
  $classid = addslashes($classid);
  
  $classes = db_exec(
  "SELECT qs.questionsetid
  FROM enrollments AS e 
  INNER JOIN qsets AS qs ON e.classid = qs.classid
  INNER JOIN questionsets AS q ON q.questionsetid = qs.questionsetid
  LEFT JOIN answersets AS a ON a.questionsetid = q.questionsetid AND a.classid = e.classid AND a.questionperiodid = '$questionperiodid'
  LEFT JOIN completesurveys AS cs ON cs.userid = e.userid AND cs.answersetid = a.answersetid
  WHERE e.userid = '$userid' AND e.classid = '$classid' AND cs.userid IS NULL
  LIMIT 50",$db);
  
  $q = array();
  while($row = mysql_fetch_assoc($classes))
    $q[] = new OldQuestionWidget($db,$questionsetid, $classid, $questionperiodid, "Q$questionperiodid", "form1", WIDGET_POST);
  return $q;
};

function showquestions(&$qsets,$badfields,$classid)
{
  print ('<form method=post action="survey.php" id=form1 name=form1>');
  print ("<input type=hidden name=classid value=\"$classid\">");
  print ("<input type=hidden name=save value=\"true\">");
  foreach($qsets as $qset)
    $qset->display();
  print ('<center><input type=submit name=submit value="Submit Responses">');
  print (' <input type=reset name=reset value="Reset"></center>');
  print ('</form>');  
}

function validatequestions(&$qsets)
{
	global $db,	$HTTP_POST_VARS;
	$errors = array();
	foreach($qsets as	$k => $qset)
	{
		$qset->validate();
	  array_splice($errors, count($errors), 0, array_values($qset->errors));
	}
  return count($errors) ==	0	?	0	:	$errors;
}

function savequestions($qsets,$classid)  //todo make this work then test it
{
	foreach($qsets as	$k => $qset)
	{
		//$qset->save();
	}
};

//-----------------------------------------------------------------------------
// BEGIN PAGE LOGIC

$userid = login_getuserid();
$db = wces_connect();
$questionperiodid = wces_GetQuestionPeriod($db);
wces_Findquestionsets($db,"qsets");

print('<i>The Spring 2001 Final Evaluation period has not yet begun. Check back soon and email any questions to <a href="mailto:wces@columbia.edu">wces@columbia.edu</a>.</i>');
page_bottom();
exit();

if ($classid)
{
  if ($qsets = getquestionsets($classid))
  {
    if (!$save)
    {
      showquestions($qsets,Array(),$classid);
    }
    else
    {
      $badfields = validatequestions($qsets);
      if ($badfields)
      {
        print("<p>The survey is not complete. Please double check the fields which are highlighted in <font color=red>red</font></p>");
        showquestions($qsets,$badfields,$classid);
      }  
      else
      {
        if (savequestions($qsets,$classid))
          print("<p>Your responses have been saved. You can choose another class from the list below, or log out.</p>");
        else
          print("<p><b>An internal error has occurred and your responses have NOT been saved. Please try again or send an email to <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a></b></p>");
        listclasses();
      };  
    };
  }
  else
  {
    print ("<p><b>Invalid class number ($classid)</b></p>");
    listclasses();
  };  
}
else
{
  listclasses();
}


?>









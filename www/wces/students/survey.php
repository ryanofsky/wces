<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  
  login_protect(login_student);
  page_top("List classes","0100");
  
function listclasses()
{
  global $db, $questionperiodid, $userid, $server_wcespath;

  $classes = mysql_query(
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
  LIMIT 50",$db);

  print ("<p>Choose a class link from this list below.</p>");
  print ("<UL>\n");
  $found = false;
  while ($class = mysql_fetch_array($classes))
  {
    $found = true;
    extract($class);
    if ($surveyed)  
      print ("  <LI><A HREF=\"survey.php?classid=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</a></LI>\n");
    else
      print ("  <LI>" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>\n");
  }
  if (!$found) print ("<LI>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, check our <a href=\"evaluations.php\">class evaluation listing</a> and email <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a> so we can update our database with your information.</LI>");
  print ("</UL>");
  print("<p>Remember to <a href=\"${server_wcespath}login/logout.php\">log out</a> when you are done.</p>");
}

function getquestionsets($classid)
{
  global $db, $userid, $db_debug, $questionperiodid;
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
    array_push($q,$row["questionsetid"]);
  
  return $q;
};

function showqset($questionsetid,$badfields)
{
  global $db,$HTTP_POST_VARS;
  $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid), 0);
  extract($questionset);
  print("<h4>$displayname</h4>\n");
  for($i = 1; $i <= 10; ++$i)
  {
    $fieldname = "Q" . $questionsetid . "MC" . $i;
    $index = "MC" . $i;
    $text = $questionset[$index];
    if ($badfields[$fieldname]) $text = "<font color=red>$text</font>";
   
    
    if ($text)
    {
%>
<p><b><%=$text%></b></p>
<blockquote>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>a" value="a" <%=$HTTP_POST_VARS[$fieldname] == "a" ? "checked" : ""%>><label for="<%=$fieldname%>a">excellent</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>b" value="b" <%=$HTTP_POST_VARS[$fieldname] == "b" ? "checked" : ""%>><label for="<%=$fieldname%>b">very good</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>c" value="c" <%=$HTTP_POST_VARS[$fieldname] == "c" ? "checked" : ""%>><label for="<%=$fieldname%>c">satisfactory</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>d" value="d" <%=$HTTP_POST_VARS[$fieldname] == "d" ? "checked" : ""%>><label for="<%=$fieldname%>d">poor</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>e" value="e" <%=$HTTP_POST_VARS[$fieldname] == "e" ? "checked" : ""%>><label for="<%=$fieldname%>e">disastrous</label>
</blockquote>
<%
    }  
  };  
  for($i = 1; $i <= 2; ++$i)
  {
    $fieldname = "Q" . $questionsetid . "FR" . $i;
    
    $index = "FR" . $i;
    $text = $questionset[$index];
    if ($text)
    {
%>
<p><b><%=$text%></b></p>
<blockquote><textarea name="<%=$fieldname%>" rows=8 cols=50 wrap=virtual><%=$HTTP_POST_VARS[$fieldname]%></textarea></blockquote>
<%
    }  
  };
};
function showquestions($qsets,$badfields,$classid)
{
  print ('<form method=post action="survey.php" id=form1 name=form1>');
  print ("<input type=hidden name=classid value=\"$classid\">");
  print ("<input type=hidden name=save value=\"true\">");
  foreach($qsets as $qsetid)
    showqset($qsetid,$badfields);
  print ('<center><input type=submit name=submit value="Submit Responses">');
  print (' <input type=reset name=reset value="Reset"></center>');
  print ('</form>');  
}

function validatequestions($qsets)
{
  global $db, $HTTP_POST_VARS;
  $badfields = Array();
  foreach($qsets as $questionsetid)
  {
    $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid), 0);
    extract($questionset);
    for($i = 1; $i <= 10; ++$i)
    {
      $fieldname = "Q" . $questionsetid . "MC" . $i;
      if ($questionset["MC" . $i])
      {
        $result = $HTTP_POST_VARS[$fieldname];
        if (!($result == "a" || $result == "b" || $result == "c" || $result == "d" || $result == "e"))
        {
          $badfields[$fieldname] = 1;
        }  
      };
    };
  };  
  return count($badfields) == 0 ? 0 : $badfields;
}

function savequestions($qsets,$classid)
{
  global $userid, $questionperiodid, $db, $db_debug, $HTTP_POST_VARS;
  foreach($qsets as $questionsetid)
  {
    $keys = Array("questionsetid" => $questionsetid, "questionperiodid" => $questionperiodid, "classid" => $classid);
    $answersetid = db_getvalue($db,"answersets",$keys,"answersetid");
    if (!$answersetid)
    {
      $values = Array
      (
        "responses" => 0,
        "MC1a" => 0, "MC1b" => 0, "MC1c" => 0, "MC1d" => 0, "MC1e" => 0,
        "MC2a" => 0, "MC2b" => 0, "MC2c" => 0, "MC2d" => 0, "MC2e" => 0,
        "MC3a" => 0, "MC3b" => 0, "MC3c" => 0, "MC3d" => 0, "MC3e" => 0,
        "MC4a" => 0, "MC4b" => 0, "MC4c" => 0, "MC4d" => 0, "MC4e" => 0,
        "MC5a" => 0, "MC5b" => 0, "MC5c" => 0, "MC5d" => 0, "MC5e" => 0,
        "MC6a" => 0, "MC6b" => 0, "MC6c" => 0, "MC6d" => 0, "MC6e" => 0,
        "MC7a" => 0, "MC7b" => 0, "MC7c" => 0, "MC7d" => 0, "MC7e" => 0,
        "MC8a" => 0, "MC8b" => 0, "MC8c" => 0, "MC8d" => 0, "MC8e" => 0,
        "MC9a" => 0, "MC9b" => 0, "MC9c" => 0, "MC9d" => 0, "MC9e" => 0,
        "MC10a" => 0,"MC10b" => 0,"MC10c" => 0,"MC10d" => 0,"MC10e" => 0,
        "FR1" => "", "FR2" => ""
      );
      $answersetid = db_addrow($db,"answersets",array_merge($values,$keys));
    };
    
    $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid), 0);
    extract($questionset);

    $sql = "UPDATE answersets SET responses = responses + 1";
    for($i = 1; $i <= 10; ++$i)
    {
      $fieldname = "Q" . $questionsetid . "MC" . $i;
      if ($questionset["MC" . $i])
      {
        $result = $HTTP_POST_VARS[$fieldname];
        if ($result == "a" || $result == "b" || $result == "c" || $result == "d" || $result == "e")
        {
          $n = "MC" . $i . $result;
          $sql .= ", $n = $n + 1";
        }; 
      };
    };
    for($i = 1; $i <= 2; ++$i)
    {
      $fieldname = "Q" . $questionsetid . "FR" . $i;
      if ($questionset["FR" . $i])
      {
        $result = addslashes(addcslashes(trim($HTTP_POST_VARS[$fieldname]),"\t\\"));
        if ($result)
        {
          $n = "FR" . $i;
          $sql .= ", $n = concat($n,'	$result')";
        }; 
      };
    };
    
    $sql .= " WHERE answersetid = $answersetid";

    if (!db_exec("INSERT INTO completesurveys(userid,answersetid) VALUES ($userid,$answersetid)",$db) || mysql_affected_rows($db) != 1)
      return false;    
    
    if (!db_exec($sql,$db))
      return false;
  }
  return true;
};

//-----------------------------------------------------------------------------
// BEGIN PAGE LOGIC


$userid = login_getuserid();
$db = wces_connect();
$questionperiodid = wces_GetQuestionPeriod($db);
wces_Findquestionsets($db,"qsets");

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

  page_bottom();
%>









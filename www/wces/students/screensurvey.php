<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  
  page_top("List classes","0100");
  
$db = wces_connect();
$userid = login_getuserid();

function listclasses()
{
  global $db, $userid, $server_wcespath;

  $y = mysql_query("CREATE TEMPORARY TABLE currentclasses (classid INTEGER NOT NULL, PRIMARY KEY(classid))",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM groupings AS g INNER JOIN classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors'  && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.subjectid INNER JOIN classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM groupings AS g INNER JOIN courses AS c  ON g.linkid = c.departmentid INNER JOIN classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' && cl.year = 2000 && cl.semester = 'fall'",$db);

  $classes = mysql_query(
  "SELECT e.surveyed, cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode 
  FROM enrollments AS e 
  INNER JOIN currentclasses AS cc ON e.classid = cc.classid
  LEFT JOIN classes AS cl ON cc.classid = cl.classid
  LEFT JOIN courses AS c ON cl.courseid = c.courseid
  LEFT JOIN subjects AS s ON c.subjectid = s.subjectid
  WHERE e.userid = '$userid'
  ORDER BY e.surveyed
  LIMIT 50",$db);

  print ("<p>Choose a class link from this list below.</p>");
  print ("<UL>\n");
  $found = false;
  while ($class = mysql_fetch_array($classes))
  {
    $found = true;
    extract($class);
    if ($surveyed == "no")  
      print ("  <LI><A HREF=\"screensurvey.php?classid=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</a></LI>\n");
    else
      print ("  <LI>" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>\n");    
  }
  if (!$found) print ("<LI>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, check our <a href=\"evaluations.php\">class evaluation listing</a> and email <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a> so we can update our database with your information.</LI>");
  print ("</UL>");
  print("<p>Remember to <a href=\"${server_wcespath}login/logout.php\">log out</a> when you are done.</p>");
}

function getquestionsets($classid)
{
  global $db, $userid, $db_debug;

  if (!(db_getvalue($db,"enrollments",Array("userid" => $userid, "classid" => $classid),"surveyed") === "no"))
    return false;

  $sql = "SELECT p.professorid, c.courseid, c.subjectid, c.departmentid FROM classes as cl LEFT JOIN professors as p USING (professorid) LEFT JOIN courses as c ON cl.courseid = c.courseid WHERE cl.classid = '$classid'";
  if ($db_debug) print ("$sql<br>");
  
  $y = mysql_query($sql,$db);
  extract(mysql_fetch_array($y));

  $qsetids =             db_getcolumn($db,"groupings",Array("linkid" => $classid, "linktype" => "classes"), "questionsetid");
  $qsetids = array_merge(db_getcolumn($db,"groupings",Array("linkid" => $courseid, "linktype" => "courses"), "questionsetid")         ,$qsetids);  
  $qsetids = array_merge(db_getcolumn($db,"groupings",Array("linkid" => $professorid, "linktype" => "professors"), "questionsetid")   ,$qsetids);
  $qsetids = array_merge(db_getcolumn($db,"groupings",Array("linkid" => $departmentid, "linktype" => "departments"), "questionsetid") ,$qsetids);
  $qsetids = array_merge(db_getcolumn($db,"groupings",Array("linkid" => $subjectid, "linktype" => "subjects"), "questionsetid")       ,$qsetids);

  $qsetids = array_values(array_unique($qsetids));
 
  if (is_array($qsetids) && count($qsetids) > 0)
    return $qsetids;
  else
    return false;  
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
<blockquote><textarea name="<%=$fieldname%>" rows=8 cols=50><%=$HTTP_POST_VARS[$fieldname]%></textarea></blockquote>
<%
    }  
  };
};
function showquestions($qsets,$badfields)
{
  global $classid;

  print ('<form method=post action="screensurvey.php" id=form1 name=form1>');
  print ("<input type=hidden name=classid value=\"$classid\">");
  print ("<input type=hidden name=save value=\"true\">");

  foreach($qsets as $qsetid)
  {
    showqset($qsetid,$badfields);
  };
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

function savequestions($qsets)
{
  return false;
};

if ($classid)
{
  if ($qsets = getquestionsets($classid))
  {
    if (!$save)
    {
      showquestions($qsets,Array());
    }
    else
    {
      $badfields = validatequestions($qsets);
      if ($badfields)
      {
        print("<p>The survey is not complete. Please double check the fields which are highlighted in <font color=red>red</font></p>");
        showquestions($qsets,$badfields);
      }  
      else
      {
        if (savequestions($qsets))
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









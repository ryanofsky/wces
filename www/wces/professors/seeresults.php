<%

  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wces/reporting.inc");
  
  login_protect(login_professor);
  $profid = login_getprofid();

  page_top("Survey Results","0010");
  $db = wces_connect();
  $profname = db_getvalue($db,"Professors",Array("professorid" => $profid),"name");

function report_display($db,$answersetids,$showresponses,$listclasses,$showgraph)
{
  global $server_wcespath;
  $first = true;
  
  $list = "(";
  foreach($answersetids as $answersetid)
  {
    if ($first) $first = false; else $list .= ", ";
    $list .= $answersetid;
  }
  $list .= ")";
  
  $questionsetid = mysql_result(mysql_query("SELECT MIN(questionsetid) as questionsetid FROM AnswerSets WHERE answersetid IN $list",$db),0);
  $questions = db_getrow($db,"QuestionSets",Array("questionsetid" => $questionsetid),0);
  
  print("<hr><h3>" . $questions["displayname"] . "</h3>\n");

  if ($listclasses)
  {
   print("<h4>Classes Included In This Report</h4>\n<ul>");
   
   $sql = "SELECT a.classid AS classid, cl.section AS section, s.code AS scode, c.code AS code, c.name AS name, p.name AS pname, p.professorid AS professorid
      FROM AnswerSets AS a
      LEFT JOIN Classes AS cl USING (classid)
      LEFT JOIN Courses AS c USING (courseid)
      LEFT JOIN Subjects AS s USING (subjectid)
      LEFT JOIN Professors AS p ON (cl.professorid = p.professorid)
      WHERE a.answersetid IN $list";
    
    //print("$sql<br>");
    $y = mysql_query($sql,$db);
    while($result = mysql_fetch_array($y))
    {
      extract($result);
      print("<li><a href=\"${server_wcespath}students/classinfo.php?classid=$classid\">$scode$code$section <i>$name</i></a> - Professor <a href=\"${server_wcespath}students/profinfo.php?professorid=$professorid\">$pname</a></li>");
    };
    print("</ul>");
  };

  $choices = array("a","b","c","d","e");
  $sql = "SELECT SUM(a.responses) AS responses, SUM(c.students) AS students";
  for($i = 1; $i <= 10; ++$i)
    foreach($choices as $choice)
    {
      $sql .= ", SUM(a.MC$i$choice) AS MC$i$choice";
    };
  $sql .= " FROM AnswerSets AS a LEFT JOIN Classes as c USING (classid) WHERE answersetid IN $list";
  
  $answers = mysql_fetch_array(mysql_query($sql,$db));
  
%>
<h4>Response Statistics</h4>

<p>Total Students: <b><%=$answers["students"]%></b><br>
Students Evaluated: <b><%=$answers["responses"]%></b><br>
<%if ($showgraph) {%><img src="<%=$server_wcespath%>media/graphs/susagegraph.php?blank=<%=$answers["students"]-$answers["responses"]%>&filled=<%=$answers["responses"]%>" width=200 height=200><img src="<%=$server_wcespath%>media/graphs/susagelegend.gif" width=147 height=31><%}%></p>
<h4>Ratings</h4>

<table border=1 RULES=ALL FRAME=VOID>
<tr>
  <td>&nbsp;</td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/50.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/40.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/30.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/20.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/10.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/avg.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/mode.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/report/sd.gif"></center></td>
  <td width=120><p><b>Average (Graphical)</p></b></td>
</tr>
<%

  for($i = 1; $i <= 10; ++$i)
  {
    if ($questions["MC$i"])
    {
      print("<tr>\n");
      print("  <td>" . $questions["MC$i"] . "</td>\n");
      foreach($choices as $choice)
      {
        print("  <td>" . $answers["MC$i$choice"] . "</td>\n");
      };
    
      $mode = report_mode($answers["MC${i}a"],$answers["MC${i}b"],$answers["MC${i}c"],$answers["MC${i}d"],$answers["MC${i}e"]);
      $avg = report_avg($answers["MC${i}a"],$answers["MC${i}b"],$answers["MC${i}c"],$answers["MC${i}d"],$answers["MC${i}e"]);
      $sd = report_sd($answers["MC${i}a"],$answers["MC${i}b"],$answers["MC${i}c"],$answers["MC${i}d"],$answers["MC${i}e"],$avg);
    
      print("  <td>" . round($avg,2) . "</td>\n");
      print("  <td>" . round($mode,2) . "</td>\n");
      print("  <td>" . round($sd,2) . "</td>\n");
      print("  <td>"); print(report_meter(round($avg * 20))); print("</td>\n");
      print ("</tr>\n");
    };  
  };
  print("</table>\n");
  
  if ($showresponses && ($questions["FR1"] || $questions["FR2"]))
  {
    $sql = "SELECT a.FR1 AS FR1, a.FR2 AS FR2, a.classid AS classid, cl.section AS section, s.code AS scode, c.code AS code, c.name AS name, p.name AS pname, p.professorid AS professorid
      FROM AnswerSets AS a
      LEFT JOIN Classes AS cl USING (classid)
      LEFT JOIN Courses AS c USING (courseid)
      LEFT JOIN Subjects AS s USING (subjectid)
      LEFT JOIN Professors AS p ON (cl.professorid = p.professorid)
      WHERE a.answersetid IN $list";
    
    //print("$sql<br>");
    
    $y = mysql_query($sql,$db);
    $first = true;
    while($result = mysql_fetch_array($y))
    {
      extract($result);
      if ($FR1 || $FR2)
      {
        if ($first) { $first= false; print("<h4>Text Responses</h4>"); }
        print("<h5><a href=\"${server_wcespath}students/classinfo.php?classid=$classid\">$scode$code$section <i>$name</i></a> - Professor <a href=\"${server_wcespath}students/profinfo.php?professorid=$professorid\">$pname</a></h5>");
        if ($FR1)
        {
          print("<h5>" . $questions["FR1"] . "</h5>");
          print("<UL><LI>");
          print(nl2br(stripcslashes(str_replace("\t","</LI><LI>",trim($FR1)))));
          print("</LI></UL>");        
        };
        if ($FR2)
        {
          print("<p>" . $questions["FR2"] . "</p>");
          print("<UL><LI>");
          print(nl2br(stripcslashes(str_replace("\t","</LI><LI>",trim($FR2)))));
          print("</LI></UL>");        
        };
      };
    };
  };  
  print("<hr>");
};

function listclasses()
{
  global $profid,$profname,$db;

  $sql = 
  "SELECT a.answersetid, a.questionperiodid, qp.year, qp.semester, qp.description, cl.classid, s.code as scode, c.code as code, cl.section, c.name
  FROM Classes as cl 
  INNER JOIN AnswerSets AS a USING (classid)
  LEFT JOIN QuestionPeriods as qp USING (questionperiodid)
  LEFT JOIN Courses AS c ON (cl.courseid = c.courseid)
  LEFT JOIN Subjects AS s USING (subjectid)
  WHERE cl.professorid = $profid
  ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, cl.classid";
  
  print("<h3>$profname - Survey Responses</h3>\n");
  
  $y = mysql_query($sql,$db);
  $oldquestionperiodid = 0;
  $oldclassid = 0;
  while($result = mysql_fetch_array($y))
  {
    extract($result);
    if ($oldquestionperiodid != $questionperiodid)
    {
      if ($oldquestionperiodid != 0) print("  <li><a href=\"?questionperiodid=$oldquestionperiodid\">All Classes Combined</a></li>\n</ul>\n");
      $semester = ucwords($semester);
      print("<h4>$semester $year - $description</h4>\n<ul>");    
    }
    if ($classid != $oldclassid || $oldquestionperiodid != $questionperiodid)
    {
      print("  <li><a href=\"?questionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");  
    };
    $oldquestionperiodid = $questionperiodid;
    $oldclassid = $classid;
  };
  if ($first)
    print("<p><b>No Responses Found</b></p>");
  else  
    print("  <li><a href=\"?questionperiodid=$oldquestionperiodid\">All Classes Combined</a></li>\n</ul>");
      
};  
  
function printresults($questionperiodid,$classid)
{
  global $profid,$profname,$db;

  $questionperiodid = addslashes($questionperiodid);
  $classid = addslashes($classid);

  $classcond = $classid ? " AND cl.classid = '$classid'" : "";
  
  $sql = "SELECT a.answersetid, a.questionsetid
  FROM AnswerSets AS a
  INNER JOIN Classes as cl USING (classid)
  LEFT JOIN Courses as c USING (courseid)
  LEFT JOIN Subjects as s USING (subjectid)
  WHERE a.questionperiodid = '$questionperiodid' AND cl.professorid = '$profid'$classcond
  ORDER BY a.questionsetid";

  $y = mysql_query($sql,$db);

  if (mysql_num_rows($y) == 0) { listclasses(); return; };
  
  print("<h3>$profname - Survey Responses (<a href=\"seeresults.php\">Back</a>)</h3>\n");
  
  $oldquestionsetid = 0;
  while($result = mysql_fetch_array($y))
  {
    extract($result);
    if ($questionsetid != $oldquestionsetid)
    {
      if ($ansids)
      {
        report_display($db,$ansids,true,true,true);
      };  
      $ansids = Array($answersetid);  
    }
    else
    {
      array_push($ansids,$answersetid);
    }
    $oldquestionsetid = $questionsetid;
  };
  report_display($db,$ansids,true,true,true);
  
  print("<p><b><a href=\"seeresults.php\">Back</a></b></p>");
  
};


if ($questionperiodid)
  printresults($questionperiodid,$classid);
else
  listclasses();

page_bottom();

%>
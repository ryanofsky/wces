<%

  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wces/reporting.inc");
  
  login_protect(login_professor);
  $profid = login_getprofid();

  page_top("Survey Results","0010");
  $db = wces_connect();
  $profname = db_getvalue($db,"professors",Array("professorid" => $profid),"name");

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
  
  $questionsetid = mysql_result(mysql_query("SELECT MIN(questionsetid) as questionsetid FROM answersets WHERE answersetid IN $list",$db),0);
  $questions = db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid),0);
  
  print("<hr><h3>" . $questions["displayname"] . "</h3>\n");

  if ($listclasses)
  {
   print("<h4>classes Included In This Report</h4>\n<ul>");
   
   $sql = "SELECT a.classid AS classid, cl.section AS section, s.code AS scode, c.code AS code, c.name AS name, p.name AS pname, p.professorid AS professorid
      FROM answersets AS a
      LEFT JOIN classes AS cl USING (classid)
      LEFT JOIN courses AS c USING (courseid)
      LEFT JOIN subjects AS s USING (subjectid)
      LEFT JOIN professors AS p ON (cl.professorid = p.professorid)
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
  $sql .= " FROM answersets AS a LEFT JOIN classes as c USING (classid) WHERE answersetid IN $list";
  
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
      FROM answersets AS a
      LEFT JOIN classes AS cl USING (classid)
      LEFT JOIN courses AS c USING (courseid)
      LEFT JOIN subjects AS s USING (subjectid)
      LEFT JOIN professors AS p ON (cl.professorid = p.professorid)
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

  print("<h3>$profname - Survey Responses</h3>\n");

  $questionperiods = db_exec("SELECT questionperiodid, semester, year, description FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC", $db, __FILE__, __LINE__);
  while($questionperiod = mysql_fetch_assoc($questionperiods))
  {
    $questionperiodid = $semester = $year = $description = "";
    extract($questionperiod);
    print("<h4>" . ucfirst($semester) . " $year - $description</h4>\n");
    print("<ul>\n");

    $classes = db_exec("
    SELECT cl.classid, s.code as scode, c.code, cl.section, c.name, cl.name as clname, IF(a.answersetid IS NULL, 0, 1) AS hasanswers
    FROM classes AS cl
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    LEFT JOIN answersets AS a ON (a.classid = cl.classid AND a.questionperiodid = '$questionperiodid')
    WHERE cl.year = '$year' AND cl.semester = '$semester' AND cl.professorid = '$profid'
    ORDER BY hasanswers DESC, s.code, c.code, cl.section", $db, __FILE__, __LINE__);

    $count = 0;
    while($cl = mysql_fetch_assoc($classes))
    {
      $classid = $scode = $code = $section = $name = $clname = $hasanswers = "";
      extract($cl);
      if ($clname) $name .= " - $clname";
      if ($hasanswers)
      {
        ++$count;
        print("  <li><a href=\"?questionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");  
      }
      else
        print("  <li>$scode$code$section <i>$name</i> (No Responses Available)</li>");
    }
    if ($count > 1) print("  <li><a href=\"?questionperiodid=$questionperiodid\">All classes Combined</a></li>\n</ul>");      
    print("</ul>\n");
  }
};  
  
function printresults($questionperiodid,$classid)
{
  global $profid,$profname,$db;

  $questionperiodid = addslashes($questionperiodid);
  $classid = addslashes($classid);

  $classcond = $classid ? " AND cl.classid = '$classid'" : "";
  
  $sql = "SELECT a.answersetid, a.questionsetid
  FROM answersets AS a
  INNER JOIN classes as cl USING (classid)
  LEFT JOIN courses as c USING (courseid)
  LEFT JOIN subjects as s USING (subjectid)
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









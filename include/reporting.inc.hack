<%

require_once("server.inc");
require_once("wizard.inc");
require_once("database.inc");
require_once("wces.inc");

//---------------------------------------------------------------------------------------------
// Reporting Helper Functions

function report_drawmeter($percent)
{
  global $server_wcespath;
  print("<img src=\"${server_wcespath}media/meter/left.gif\" width=6 height=13>");
  for($i = 0; $i < 5; ++$i)
  {
    $num = $percent - 20 * $i;
    if ($num < 0) $num = 0;
    if ($num < 10) $num = "0" . $num;
    if ($num > 20) $num = 20;
    print("<img src=\"${server_wcespath}media/meter/mid${num}.gif\" width=21 height=13>");
  };
  print("<img src=\"${server_wcespath}media/meter/right.gif\" width=5 height=13>");
};

function report_mode($i1,$i2,$i3,$i4,$i5) // what is the resolution if there are two most popular choices, both chosen the same number of times?
{
  $max = $i1; $maxnum = 5.0;
  if ($i2 > $max) { $max = $i2; $maxnum = 4.0; }
  if ($i3 > $max) { $max = $i3; $maxnum = 3.0; }
  if ($i4 > $max) { $max = $i4; $maxnum = 2.0; }
  if ($i5 > $max) { $max = $i5; $maxnum = 1.0; }
  return $maxnum;
};

function report_avg($i1,$i2,$i3,$i4,$i5)
{
  $sum = $i1 + $i2 + $i3 + $i4 + $i5;
  return $sum != 0 ? ((5.0 * $i1) + (4.0 * $i2) + (3.0 * $i3) + (2.0 * $i4) + (1.0 * i5)) / $sum : 0;
};

function report_sd($i1,$i2,$i3,$i4,$i5,$avg)
{
  $sum = ($i1 + $i2 + $i3 + $i4 + $i5 - 1);
  return ($sum == 0) ? 0 :
         sqrt
          ( 
            ( 
              $i5 * pow((1.0 - $avg),2) + 
              $i4 * pow((2.0 - $avg),2) +
              $i3 * pow((3.0 - $avg),2) + 
              $i2 * pow((4.0 - $avg),2) + 
              $i1 * pow((5.0 - $avg),2) 
            ) / $sum
          );
};

//---------------------------------------------------------------------------------------------
// Reporting Implementation #1 

function report_render($db, &$questionperiods, &$departments, &$professors, &$courses, &$groups, $custom, $textresponses, $showgraph, &$pagezero)
{
  global $server_wcespath;
  
  $nojoin = 0;
  $leftjoin = 1;
  $innerjoin = 2;

  $join_classes = $leftjoin;
  $join_courses = $nojoin;
  $join_professors = $nojoin;
  $join_departments = $innerjoin;
  $join_subjects = $nojoin;
  $join_questionperiods = $nojoin;

  $sql_columns = "COUNT(*) AS rows, SUM(a.responses) AS responses, SUM(cl.students) AS students, qs.displayname as qsname, d.name as departmentname";
  $sql_groupby = "qs.questionsetid";
  $sql_where = "";
  
  if (!$custom)
  {
    $sql_where .= " AND qs.type = 'public'";
  };

  if ($questionperiods)
  {
    $sql_where .= " AND qp.questionperiodid IN " . db_set($questionperiods);
    $join_questionperiods = $innerjoin;
  }; 
  
  if ($departments)
  {
    $sql_where .= " AND d.departmentid IN " . db_set($departments);
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
    $join_departments = $innerjoin; 
  };
  
  if ($professors)
  {
    $sql_where .= " AND p.professorid IN " . db_set($professors);
    $join_classes = $innerjoin;
    $join_professors = $innerjoin;
  };
  
  if ($courses)
  {
    $sql_where .= " AND c.courseid IN " . db_set($courses);
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
  }; 
 
  $nogroups = true;
  if($groups["classes"])
  {
    $nogroups = false;
    $sql_columns .= ", CONCAT('<a href=\"${server_wcespath}students/classinfo.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname";
    $join_classes = $innerjoin;
    $join_courses = max($join_courses,$leftjoin);
    $join_subjects = max($join_subjects,$leftjoin);
    $sql_groupby .= ", cl.classid";
  }; 
  
  if($groups["courses"])
  {
    $nogroups = false;
    $sql_columns .= ", CONCAT(s.code, c.code, ' <i>', c.name, '</i>') AS coursename";
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
    $join_subjects = max($join_subjects, $leftjoin);
    $sql_groupby .= ", c.courseid";
  };
  
  if($groups["professors"])
  {
    $nogroups = false;
    $sql_columns .= ", CONCAT('<a href=\"${server_wcespath}students/profinfo.php?professorid=', p.professorid, '\">', p.name, '</a>') AS professorname";
    $join_classes = $innerjoin;
    $join_professors = $innerjoin;
    $sql_groupby .= ", p.professorid";
  };

  if($groups["departments"])
  {
    $nogroups = false;
    $sql_columns .= ", CONCAT(d.name, ' (', d.code, ')') AS departmentname";
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
    $join_departments = $innerjoin;
    $sql_groupby .= ", d.departmentid";
  };
  
  if ($groups["questionperiods"])
  {
    $nogroups = false;
    $sql_columns .= ", CONCAT(qp.semester, ' ',qp.year, ' ', qp.description) AS qpname";
    $join_questionperiods = $innerjoin;
    $sql_groupby .= ", a.questionperiodid";
  };
  
  $nogroups = true; // slows query, but lists individual classes
  
  if ($nogroups)
  {
    $join_courses = max($join_courses,$leftjoin); 
    $join_subjects = max($join_subjects,$leftjoin); 
  };
  
  if ($textresponses)
  {
    $join_professors = max($join_professors,$leftjoin); 
  };
  
  $choices = array("a","b","c","d","e");
  for($i = 1; $i <= 10; ++$i)
  {
    $sql_columns .= ", qs.MC$i";
    foreach($choices as $choice)
      $sql_columns .= ", SUM(a.MC$i$choice) AS MC$i$choice";
  };

  $sql_joins = ($join_classes ? (($join_classes == $innerjoin ? " INNER" : " LEFT") . " JOIN Classes AS cl USING (classid)") : "") . 
  ($join_courses ? (($join_courses == $innerjoin ? " INNER" : " LEFT") . " JOIN Courses AS c USING (courseid)") : "") . 
  ($join_subjects ? (($join_subjects == $innerjoin ? " INNER" : " LEFT") . " JOIN Subjects AS s USING (subjectid)") : "") . 
  ($join_departments ? (($join_departments == $innerjoin ? " INNER" : " LEFT") . " JOIN Departments AS d ON (c.departmentid = d.departmentid)") : "") . 
  ($join_professors ? (($join_professors == $innerjoin ? " INNER" : " LEFT") . " JOIN Professors AS p ON (cl.professorid = p.professorid)") : "") .
  ($join_questionperiods ? (($join_questionperiods == $innerjoin ? " INNER" : " LEFT") . " JOIN QuestionPeriods AS qp ON (a.questionperiodid = qp.questionperiodid)") : "") .  
  " INNER JOIN QuestionSets AS qs ON (a.questionsetid = qs.questionsetid)";
  
  $sql_where = ($sql_where ? "WHERE" . substr($sql_where,4) : "");
  
  $sql = "SELECT $sql_columns FROM AnswerSets AS a $sql_joins $sql_where GROUP BY $sql_groupby ORDER BY d.name, c.name, c.courseid, cl.section, cl.classid, qs.questionsetid, $sql_groupby";
  
  $y = mysql_query($sql,$db);
  $pages = mysql_num_rows($y);
  $page = 0;
  if ($pagezero) print("<hr>\n<h3>Page 0 of $pages</h3>\n$pagezero");
  if (!$y) print("<textarea rows=20 cols=80 id=textarea1 name=textarea1>SQL ERROR\n---------\n\n$sql</textarea>");
  if ($nogroups)      $y2 = mysql_query("SELECT CONCAT('<a href=\"${server_wcespath}students/classinfo.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname FROM AnswerSets AS a $sql_joins $sql_where GROUP BY qs.questionsetid, cl.classid ORDER BY d.name, c.name, c.courseid, cl.section, cl.classid, qs.questionsetid, $sql_groupby");
  if ($textresponses) $y3 = mysql_query("SELECT CONCAT('<a href=\"${server_wcespath}students/classinfo.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname, CONCAT('<a href=\"${server_wcespath}students/profinfo.php?professorid=', p.professorid, '\">', p.name, '</a>') AS professorname, qs.FR1 as q1, qs.FR2 as q2, a.FR1, a.FR2 FROM AnswerSets AS a $sql_joins $sql_where GROUP BY qs.questionsetid, cl.classid ORDER BY d.name, c.name, c.courseid, cl.section, cl.classid, qs.questionsetid, $sql_groupby");
  $lastdepartmentname = "";
  while($result = mysql_fetch_array($y))
  {
    ++$page;
    
    $qpname = "Unknown";
    $departmentname = "Unknown";
    $professorname = "Unknown";
    $coursename = "Unknown";
    $classname = "Unknown";
    $qsname = "";
    $rows = 0;
    extract($result);
  
    $newdept = ($lastdepartmentname == $departmentname) ? false : true;
    $lastdepartmentname = $departmentname;

//    print("<hr" . ($newdept ? ' STYLE="page-break-before:always"' : ""). ">");

    print('<hr STYLE="page-break-before:always">');
    print("\n<h3>Page $page of $pages</h3>\n");
    print("<h4>On this page</h4>");
    print("<table border=0>\n");
    print("<tr><td>Question Set:</td><td><b>$qsname</b></td></tr>\n");
    if ($groups["questionperiods"]) print("<tr><td>Question Period:</td><td><b>" . ucfirst($qpname) . "</b></td></tr>\n");
//  if ($groups["departments"]) 
    print("<tr><td>Department:</td><td><b>$departmentname</b></td></tr>\n");
    if ($groups["professors"]) print("<tr><td>Professor:</td><td><b>$professorname</b></td></tr>\n");
    if ($groups["courses"]) print("<tr><td>Course:</td><td><b>$coursename</b></td></tr>\n");
//    if ($groups["classes"]) print("<tr><td>Class:</td><td><b>$classname</b></td></tr>\n");
    if ($nogroups)
      for($i = 0; $i < $rows; ++$i)
      {
        $result2 = mysql_fetch_array($y2);      
        if ($result2["classname"]) print("<tr><td>Class:</td><td><b>" . $result2["classname"] ."</b></td></tr>\n");
      };
    print("</table>\n");
        
    print("<h4>Response Statistics</h4>");
    print("<table border=0>\n");
    print("<tr><td>Total Students:</td><td><b>$students</b></td></tr>\n");
    print("<tr><td>Students Evaluated:</td><td><b>$responses</b></td></tr>\n");
    print("</table>\n");
    if ($showgraph) print("<img src=\"${server_wcespath}administrators/susagegraph.php?blank=" . ($students-$responses) . "&filled=$responses\" width=200 height=200><img src=\"${server_wcespath}administrators/susagelegend.gif\" width=147 height=31>");
    print("</p>");

    $first = true;
    for($i = 1; $i <= 10; ++$i)
    {
      if ($result["MC$i"])
      {
      
        if ($first)
        {
          $first = false;
%>
<h4>Ratings</h4>

<table border=1 RULES=ALL FRAME=VOID>
<tr>
  <td>&nbsp;</td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep5.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep4.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep3.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep2.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep1.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/repavg.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/repmode.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/repsd.gif"></center></td>
  <td width=120><p><b>Average (Graphical)</p></b></td>
</tr>
<%
        };
        
        print("<tr>\n");
        print("  <td>" . $result["MC$i"] . "</td>\n");
        foreach($choices as $choice)
        {
          print("  <td>" . $result["MC$i$choice"] . "</td>\n");
        };

        $mode = report_mode($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"]);
        $avg = report_avg($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"]);
        $sd = report_sd($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"],$avg);

        print("  <td>" . round($avg,2) . "</td>\n");
        print("  <td>" . round($mode,2) . "</td>\n");
        print("  <td>" . round($sd,2) . "</td>\n");
        print("  <td>"); report_drawmeter(round($avg * 20)); print("</td>\n");
        print("</tr>\n");
      };  
    };
    if (!$first) print("</table>\n");
    
    if ($textresponses)
    {
      $first = true;
      for($i = 0; $i < $rows; ++$i)
      {
        $result3 = mysql_fetch_array($y3);
        $firstq = true;
        for($j = 1; $j <= 2; ++$j)
        if ($result3["q$j"] && $result3["FR$j"])
        {
            if ($first) { $first= false; print("<h4>Text Responses</h4>"); }
            if ($firstq) { $firstq = false; print("<h5>" . $result3["classname"] . " - Professor " . $result3["professorname"] . "</h5>"); }
            print("<h5>" . $result3["q$j"] . "</h5>");
            print("<UL><LI>");
            print(stripslashes(nl2br(stripcslashes(str_replace("\t","</LI><LI>",trim($result3["FR$j"]))))));
            print("</LI></UL>");        
        };
      };
    };
  };
};

//---------------------------------------------------------------------------------------------
// Reporting Implementation #2 (old, used by professor pages)

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
<%if ($showgraph) {%><img src="<%=$server_wcespath%>administrators/susagegraph.php?blank=<%=$answers["students"]-$answers["responses"]%>&filled=<%=$answers["responses"]%>" width=200 height=200><img src="<%=$server_wcespath%>administrators/susagelegend.gif" width=147 height=31><%}%></p>
<h4>Ratings</h4>

<table border=1 RULES=ALL FRAME=VOID>
<tr>
  <td>&nbsp;</td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep5.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep4.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep3.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep2.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/rep1.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/repavg.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/repmode.gif"></center></td>
  <td width=20><center><img src="<%=$server_wcespath%>media/repsd.gif"></center></td>
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
      print("  <td>"); report_drawmeter(round($avg * 20)); print("</td>\n");
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

function leftalign($text,$width)
{
  $l = strlen($text);
  if ($l < $width)
    return $text . str_repeat(" ",$width - $l);
  else
    return $text;
};

function centeralign($text,$width)
{
  $l = strlen($text);
  $x = floor(($width-$l)/2.0);
  if ($l < $width)
    return $t . str_repeat(" ",$x) . "$text" . str_repeat(" ",$width - $x - $l);
  else
    return $t . $text;
};

function texttable($data,$formats)
{
  $widths = array();
  $heights = array();
  foreach($data as $rowname => $row)
  {
    $heights[$rowname] = 1;
    foreach($row as $colname => $col)
    if ($formats[$colname] === $formats[$colname] + 0)
    {
      $widths[$colname] = $formats[$colname];
      $data[$rowname][$colname] = explode("\n", wordwrap($data[$rowname][$colname],$formats[$colname],"\n"));
      $count = sizeof($data[$rowname][$colname]);
      if ($heights[$rowname] < $count) $heights[$rowname] = $count;
    }  
    else  
    {
      $l = strlen($col);
      if (!$widths[$colname] || ($l > $widths[$colname]))
        $widths[$colname] = $l;
    }
  }
  $line = "+";
  foreach($widths as $width)
  {
    $line .= str_repeat("-", 2 + $width) . "+";
  };
  $line .= "\n";
  
  $table = $line;
  
  foreach($data as $rowname => $row)
  {
    for($i = 0; $i < $heights[$rowname]; ++$i)
    {
      $table .= "|";
      foreach($widths as $colname => $width)
      {   
        if ($formats[$colname] === $formats[$colname] + 0)
          $table .= leftalign(" " . $row[$colname][$i],$width + 2);   
        else if ($i == floor($heights[$rowname] / 2))
        {
          if ($formats[$colname] == "center")
            $table .= centeralign(" " . $row[$colname],$width + 2);   
          else // ($formats[$colname] == "left")
            $table .= leftalign($row[$colname],$width + 2);   
        }
        else
          $table .= str_repeat(" ",$width + 2);
        $table .= "|";
      };
      $table .= "\n";
    }
    $table .= "$line";
  };
  
  return $table; 
};

//---------------------------------------------------------------------------------------------
// Reporting Implementation #3 (another redundant one)

function report_send($db, &$questionperiods, &$departments, &$professors, &$courses, $message, $debug)
{
  global $server_wcespath;
  
  $nojoin = 0;
  $leftjoin = 1;
  $innerjoin = 2;

  $join_classes = $innerjoin;
  $join_courses = $leftjoin;
  $join_professors = $innerjoin;
  $join_departments = $nojoin;
  $join_subjects = $leftjoin;
  $join_questionperiods = $nojoin;

  $sql_columns = "a.responses AS responses, cl.students AS students, qs.displayname as qsname";
  $sql_where = "";
  
  if ($questionperiods)
  {
    $sql_where .= " AND qp.questionperiodid IN " . db_set($questionperiods);
    $join_questionperiods = $innerjoin;
  }; 
  
  if ($departments)
  {
    $sql_where .= " AND d.departmentid IN " . db_set($departments);
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
    $join_departments = $innerjoin; 
  };
  
  if ($professors)
  {
    $sql_where .= " AND p.professorid IN " . db_set($professors);
    $join_classes = $innerjoin;
    $join_professors = $innerjoin;
  };
  
  if ($courses)
  {
    $sql_where .= " AND c.courseid IN " . db_set($courses);
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
  }; 
 
  $sql_columns .= ", CONCAT(s.code, c.code, ' ', c.name, ' ', cl.semester, ' ', cl.year, ' section ', cl.section) as classname";
  
  $choices = array("a","b","c","d","e");
  for($i = 1; $i <= 10; ++$i)
  {
    $sql_columns .= ", qs.MC$i";
    foreach($choices as $choice)
      $sql_columns .= ", a.MC$i$choice AS MC$i$choice";
  };
  $sql_columns .= ", qs.FR1, qs.FR2, a.FR1 AS aFR1, a.FR2 AS aFR2";
  
 
  $sql_joins = ($join_classes ? (($join_classes == $innerjoin ? " INNER" : " LEFT") . " JOIN Classes AS cl USING (classid)") : "") . 
  ($join_courses ? (($join_courses == $innerjoin ? " INNER" : " LEFT") . " JOIN Courses AS c USING (courseid)") : "") . 
  ($join_subjects ? (($join_subjects == $innerjoin ? " INNER" : " LEFT") . " JOIN Subjects AS s USING (subjectid)") : "") . 
  ($join_departments ? (($join_departments == $innerjoin ? " INNER" : " LEFT") . " JOIN Departments AS d ON (c.departmentid = d.departmentid)") : "") . 
  ($join_professors ? (($join_professors == $innerjoin ? " INNER" : " LEFT") . " JOIN Professors AS p ON (cl.professorid = p.professorid)") : "") .
  ($join_questionperiods ? (($join_questionperiods == $innerjoin ? " INNER" : " LEFT") . " JOIN QuestionPeriods AS qp ON (a.questionperiodid = qp.questionperiodid)") : "") .  
  " INNER JOIN QuestionSets AS qs ON (a.questionsetid = qs.questionsetid)" .
  " LEFT JOIN Users AS u ON (p.userid = u.userid)";

  $sql_where = ($sql_where ? "WHERE" . substr($sql_where,4) : "");
  
  $sql = "SELECT COUNT(*) AS pages, IFNULL(p.email,u.email) as email, p.name AS pname, p.professorid FROM AnswerSets AS a $sql_joins $sql_where GROUP BY p.professorid ORDER BY p.professorid";
  $professors = mysql_query($sql,$db);
  $sql = "SELECT $sql_columns FROM AnswerSets AS a $sql_joins $sql_where ORDER BY p.professorid";
  $results = mysql_query($sql,$db);
  //print("<textarea rows=10 cols=80 id=textarea1 name=textarea1>$sql</textarea>");
  
  
  $y = mysql_query($sql,$db);
  $pages = mysql_num_rows($y);

 
  $line = "---------------------------------------------------------------------------\n";

  while($professor = mysql_fetch_array($professors))
  {
    $email = "";
    if ($message) $email .= wordwrap(str_replace("%profname%",$professor["pname"],$message),76) . "\n\n$line\n";
    $email .= "A graphical version of this report is available online at\n";
    $email .= "http://oracle.seas.columbia.edu/wces/professors/\n\n";
 
    $pages = $professor["pages"];
 
    for($page = 1; $page <= $pages; ++$page)
    {
      $result = mysql_fetch_array($results);
      $classname = ""; $qsname = ""; $students = 0; $responses = 0;
      extract($result);

      $email .= "\n${line}Page $page of $pages\n\n";
      
      $email .= "Included on this page:\n\n";
      $email .= " - Class: $classname\n";
      $email .= " - Question Set: $qsname\n\n";
      
      $email .= "Response Statistics:\n\n";
      $email .= " - Total Students: $students\n";
      $email .= " - Students Evaluated: $responses\n\n";

      $email .= "5 = Excellent, 4 = Very Good, 3 = Satisfactory, 2 = Poor, 1 = Disastrous\n\n";
      
      $format =Array(30,"center","center","center","center","center","center","center","center");
      $table = Array(Array("Question Text","5","4","3","2","1","Avg","Mode","SD"));
      
      for ($i = 1; $i <= 10; ++$i)
      if ($result["MC$i"])
      {
        $mode = report_mode($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"]);
        $avg = report_avg($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"]);
        $sd = report_sd($result["MC${i}a"],$result["MC${i}b"],$result["MC${i}c"],$result["MC${i}d"],$result["MC${i}e"],$avg);
        $row = Array($result["MC$i"]);
        foreach($choices as $choice)
          array_push($row,$result["MC$i$choice"]);
        array_push($row,round($avg,1)); 
        array_push($row,$mode);
        array_push($row,round($sd,1));
        array_push($table,$row);
      };

      $email .= texttable($table,$format) . "\n\n";
      
      for ($i = 1; $i <=2; ++$i)
      if ($result["FR$i"] && $result["aFR$i"])
      {
        $email .= wordwrap($result["FR$i"],76) . "\n\n";
        $email .= " - ";
        $email .= wordwrap(stripslashes(stripcslashes(str_replace("\t","\n - ",trim($result["aFR$i"])))),76);
        $email .= "\n\n";
      };
    };

    if (!$professor["email"]) print("<p>No email address found for Professor " . $professor["pname"] . "</p>");
    if ($debug)
      print("<pre>$email</pre><hr>");
    else if ($professor["email"])
      mail($professor["email"],"WCES Evaluation Results", $email, "From: wces@columbia.edu\nReply-To: wces@columbia.edu\nX-Mailer: PHP/" . phpversion());
      
  };
};

function listprofs($db, &$questionperiods, &$departments, &$professors, &$courses)
{
  global $server_wcespath;
  
  $nojoin = 0;
  $leftjoin = 1;
  $innerjoin = 2;

  $join_classes = $innerjoin;
  $join_courses = $leftjoin;
  $join_professors = $innerjoin;
  $join_departments = $nojoin;
  $join_subjects = $leftjoin;
  $join_questionperiods = $nojoin;

  $sql_columns = "a.responses AS responses, cl.students AS students, qs.displayname as qsname";
  $sql_where = "";
  
  if ($questionperiods)
  {
    $sql_where .= " AND qp.questionperiodid IN " . db_set($questionperiods);
    $join_questionperiods = $innerjoin;
  }; 
  
  if ($departments)
  {
    $sql_where .= " AND d.departmentid IN " . db_set($departments);
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
    $join_departments = $innerjoin; 
  };
  
  if ($professors)
  {
    $sql_where .= " AND p.professorid IN " . db_set($professors);
    $join_classes = $innerjoin;
    $join_professors = $innerjoin;
  };
  
  if ($courses)
  {
    $sql_where .= " AND c.courseid IN " . db_set($courses);
    $join_classes = $innerjoin;
    $join_courses = $innerjoin;
  }; 
  
  $sql_joins = ($join_classes ? (($join_classes == $innerjoin ? " INNER" : " LEFT") . " JOIN Classes AS cl USING (classid)") : "") . 
  ($join_courses ? (($join_courses == $innerjoin ? " INNER" : " LEFT") . " JOIN Courses AS c USING (courseid)") : "") . 
  ($join_subjects ? (($join_subjects == $innerjoin ? " INNER" : " LEFT") . " JOIN Subjects AS s USING (subjectid)") : "") . 
  ($join_departments ? (($join_departments == $innerjoin ? " INNER" : " LEFT") . " JOIN Departments AS d ON (c.departmentid = d.departmentid)") : "") . 
  ($join_professors ? (($join_professors == $innerjoin ? " INNER" : " LEFT") . " JOIN Professors AS p ON (cl.professorid = p.professorid)") : "") .
  ($join_questionperiods ? (($join_questionperiods == $innerjoin ? " INNER" : " LEFT") . " JOIN QuestionPeriods AS qp ON (a.questionperiodid = qp.questionperiodid)") : "") .  
  " INNER JOIN QuestionSets AS qs ON (a.questionsetid = qs.questionsetid)" .
  " LEFT JOIN Users AS u ON (p.userid = u.userid)";

  $sql_where = ($sql_where ? "WHERE" . substr($sql_where,4) : "");
  
  $professors = mysql_query("SELECT COUNT(*) AS pages, IFNULL(p.email,u.email) as email, p.name AS pname, p.professorid FROM AnswerSets AS a $sql_joins $sql_where GROUP BY p.professorid ORDER BY p.professorid");
  
  print("<ul>\n");
  while($professor = mysql_fetch_array($professors))
  {
    $professorid = 0; $pname = ""; $email = "";
    extract($professor);
    if (!$pname) $pname = "Unknown";
    if (!$email) $email = "Unknown";
    print("Professor <a href=\"{$server_wcespath}students/profinfo.php?professorid=$professorid\">$pname</a> - email <a href=\"mailto:$email\">$email</a> - $pages classes<br>");
  }
  print("</ul>\n");
};  


//---------------------------------------------------------------------------------------------
// Report Wizard

class ReportOptions extends Wizard
{
  function ReportOptions($prefix,$form,$formmethod)
  {
    global $server_wcespath;
    
    $this->Wizard
    ( 
      array("Start","Output","Question Periods","Departments","Professors","Courses","Aggregation","Finish"),
      $server_wcespath,
      $prefix,$form,$formmethod
    );
  }
  
  function loadvalues()
  {
    $this->skip[6] = ($this->loadattribute("output",$output) == "email") ? 1 : 0;
    Wizard::loadvalues();
  }
  
  function summarize()
  {
    global $db;
    if (!$db) $db = wces_connect();
     
    $report = "------------------------\nREPORT OPTIONS\n\n";
     
    $output = $this->loadattribute("output");
    $text = $this->loadattribute("includetext");
    $custom = $this->loadattribute("includecustom");
    $pies = $this->loadattribute("includepies");
     
    $gcl = $this->loadattribute("groupclasses");
    $gc = $this->loadattribute("groupcourses");
    $gp = $this->loadattribute("groupprofessors");
    $gd = $this->loadattribute("groupdepartments");
    $gqp = $this->loadattribute("groupquestionperiods");
     
    $report .= "Output: " . ($output == "email" ? "Professor Email" : "Web Report") . "\n";
    if ($output == "www")
    {
      $report .= "Text Responses: " . ($text ? "Yes" : "No") . "\n";
      $report .= "Custom Question Sets: " . ($custom ? "Yes" : "No") . "\n";
      $report .= "Pie Graphs: " . ($pies ? "Yes" : "No") . "\n";
       
      $first = true;
      $report .= "Group by: ";
      if ($gcl) { if ($first) $first = false; else $report.=", "; $report.="Classes"; }
      if ($gc)  { if ($first) $first = false; else $report.=", "; $report.="Courses"; }
      if ($gp)  { if ($first) $first = false; else $report.=", "; $report.="Professors"; }
      if ($gd)  { if ($first) $first = false; else $report.=", "; $report.="Departments"; }
      if ($gqp) { if ($first) $first = false; else $report.=", "; $report.="Question Periods"; }
      if ($first) $report .= "None";
      $report .= "\n";       
    }

    $report .= "\n------------------------\nREPORT CRITERIA\n\n";
     
    $list = $this->loadattribute("questionperiods");
    if (is_array($list) && count($list) > 0 && !in_array("all",$list))
    {
      $group = db_set($list);
      $y = mysql_query("SELECT questionperiodid, year, semester, description FROM QuestionPeriods WHERE questionperiodid IN $group ORDER BY year DESC, semester DESC, questionperiodid DESC",$db);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Question Period: " . ucwords($semester) . " $year $description ($questionperiodid)\n";
      };
    }
    else
      $report .= "Question Period: ANY\n";
        
    $list = $this->loadattribute("departments");
    if (is_array($list) && count($list) > 0 && !in_array("all",$list))
    {
      $group = db_set($list);   
      $y = mysql_query("SELECT departmentid, name, code FROM Departments AS d WHERE departmentid IN $group ORDER BY d.code",$db);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Department: $code $name ($departmentid)\n";
      };
    }
    else
      $report .= "Department: ANY\n";
    
    $list = $this->loadattribute("professors");
    if (is_array($list) && count($list) > 0 && !in_array("all",$list))
    {
      $group = db_set($list);
      $y = mysql_query("SELECT professorid, name FROM Professors WHERE professorid IN $group",$db);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Professor: $name ($professorid)\n";
      };  
    }
    else
      $report .= "Professor: ANY\n";
       
    $list = $this->loadattribute("courses");
    if (is_array($list) && count($list) > 0 && !in_array("all",$list))
    {
      $group = db_set($list);
      $y = mysql_query("SELECT c.courseid, concat(s.code, c.code, ' ', IFNULL(c.name,'')) AS coursename FROM Courses AS c LEFT JOIN Subjects as s USING (subjectid) WHERE c.courseid IN $group ORDER BY coursename",$db);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Course: $coursename ($courseid)\n";
      };
    }
    else
      $report .= "Course: ANY\n";
    
    return $report;     
  }
  
  function makereport()
  {
    global $db;
    if (!$db) $db = wces_connect();

    $text = $this->loadattribute("includetext") ? true : false;
    $custom = $this->loadattribute("includecustom") ? true : false;
    $pies = $this->loadattribute("includepies") ? true : false;

    $questionperiods = $this->loadattribute("questionperiods");
    $departments = $this->loadattribute("departments");
    $professors = $this->loadattribute("professors");
    $courses = $this->loadattribute("courses");
      
    if (!is_array($questionperiods) || !(count($questionperiods) > 0) || in_array("all",$questionperiods))
      $questionperiods = false;
    if (!is_array($departments) || !(count($departments) > 0) || in_array("all",$departments))
      $departments = false;
    if (!is_array($professors) || !(count($professors) > 0) || in_array("all",$professors))
      $professors = false;
    if (!is_array($courses) || !(count($courses) > 0) || in_array("all",$courses))
      $courses = false;

    $output = $this->loadattribute("output");
    if ($output == "email")
    {
      $message = $this->loadattribute("message");
      $debug = $this->loadattribute("spamdebug");
      if ($this->loadattribute("sendspam"))
      {
        print("<p><b>Send the stuff</b></p>");
        report_send($db, $questionperiods, $departments, $professors, $courses, $message,$debug);
      }  
      else
      {
        print("<h3>Email Report</h3>\n");
        print("<h4>Professors Included</h4>\n");
        listprofs($db, $questionperiods, $departments, $professors, $courses);
        print("<h4>Message</h4>\n");
        print("<p>This message will appear at the top of the professor email.</p>");
        if (!$message) $message = "Dear %profname%,\n\nThis automated email contains the results of the WCES evaluations for your classes. Send questions or comments to wces@columbia.edu";
        print("<textarea name=\"{$this->prefix}_message\" rows=10 cols=80>$message</textarea><br>\n");
        print("<input type=checkbox name=\"{$this->prefix}_spamdebug\" id=\"{$this->prefix}_spamdebug\"><label for=\"{$this->prefix}_spamdebug\">Debug Mode</label><br>\n");
        print("<input type=submit name=\"{$this->prefix}_sendspam\" value=\"Send Emails!\">\n");
      };
    }
    else
    {
      $groups = Array();
      
      if ($this->loadattribute("groupclasses")) $groups["classes"] = true;
      if ($this->loadattribute("groupcourses")) $groups["courses"] = true;
      if ($this->loadattribute("groupprofessors")) $groups["professors"] = true;
      if ($this->loadattribute("groupdepartments")) $groups["departments"] = true;
      if ($this->loadattribute("groupquestionperiods")) $groups["questionperiods"] = true;
      
      $pagezero = "<h4>Report Information</h4>\n";
      $pagezero .= "<table border=0><tr><td>Administrator ID: </td><td><b>" . login_getuni() . "</b></td></tr><tr><td>Date:</td><td><b>" . date("l, F j, Y") . "</b></td></tr><tr><td>Time:</td><td><b>" . date("g:i:s A") . "</b></td></tr></table>";
      $pagezero .= "<h4>Report Specification</h4><pre>" . htmlspecialchars($this->summarize()) . "</pre>";
   
      report_render($db, $questionperiods, $departments, $professors, $courses, $groups, $custom, $text, $pies, $pagezero);
    };  
  }
  
  function drawstep($stepno,$hidden)
  {  
    global $db;
    
    switch($stepno)
    {
    case 0:
      if (!$hidden)
      {
        %><p>Welcome to the reporting wizard.</p><%
      };
    break;  
    case 1:  
      $output = $this->loadattribute("output");
      if (!$output) $output = "www";
      $text = $this->loadattribute("includetext");
      $custom = $this->loadattribute("includecustom");
      $pies = $this->loadattribute("includepies");
      if (!$pies && !$this->visited[$stepno]) $pies = 1;
      if ($hidden)
      {
        $this->printattribute("output",$output);
        $this->printattribute("includetext",$text);
        $this->printattribute("includecustom",$custom);
        $this->printattribute("includepies",$pies);
      }
      else
      {
%>
<p>Choose a report format:</p>
<p>
<input onclick="show('<%=$this->prefix%>_wwwoptions')" type="radio" name="<%=$this->prefix%>_output" id="<%=$this->prefix%>outprof" value="www"<%=$output=="www" ? " checked" : ""%>><label for="<%=$this->prefix%>outprof">Display Report on a web page</label><br>
<input onclick="hide('<%=$this->prefix%>_wwwoptions')" type="radio" name="<%=$this->prefix%>_output" id="<%=$this->prefix%>outmail" value="email"<%=$output=="email" ? " checked" : ""%>><label for="<%=$this->prefix%>outmail">Email reports to professors</label></p>
<div id="<%=$this->prefix%>_wwwoptions"<%=$output == "email" ? ' style="display:none"': ''%>>
<p>For web reports only, choose which optional items you would like to include.</p>
<p>
<input type="checkbox" name="<%=$this->prefix%>_includetext" id="<%=$this->prefix%>_includetext" value="1"<%=$text ? " checked" : ""%>><label for="<%=$this->prefix%>_includetext">Include Text Responses</label><br>
<input type="checkbox" name="<%=$this->prefix%>_includecustom" id="<%=$this->prefix%>_includecustom" value="1"<%=$custom ? " checked" : ""%>><label for="<%=$this->prefix%>_includecustom">Include Custom Questions</label><br>
<input type="checkbox" name="<%=$this->prefix%>_includepies" id="<%=$this->prefix%>_includepies" value="1"<%=$pies ? " checked" : ""%>><label for="<%=$this->prefix%>_includepies">Include Pie Graphs in Usage Statistics</label></p>
</p>
</div>
<%
      };
    break;  
    case 2:
      $vals = $this->loadattribute("questionperiods");
      if (!is_array($vals)) $vals = array("5"); // bad bad bad
      if ($hidden)
        $this->printarrayattribute("questionperiods[]",$vals);
      else
      {
        $hash = array();
        foreach($vals as $val)
          $hash[$val] = 1;
%>
<p>Surveys included in the report must come from the following question period(s):</p>
<select name="<%=$this->prefix%>_questionperiods[]" size=7 multiple>
<option value="all"<%=$hash["all"] ? " selected" : ""%>>--- Any Question Period ---</option>
<%
if (!$db) $db = wces_connect();
$y = mysql_query("SELECT questionperiodid, year, semester, description FROM QuestionPeriods ORDER BY year DESC, semester DESC, questionperiodid DESC",$db);
while($row = mysql_fetch_array($y))
{
  extract($row);
  $selected = $hash[$questionperiodid] ? " selected" : "";
  print("<option value=\"$questionperiodid\"$selected>" . ucwords($semester) . " $year $description</option>");
}
%>
</select>
<%
      };
    break;
    case 3:
      $vals = $this->loadattribute("departments");
      if (!is_array($vals)) $vals = array("all");
      if ($hidden)
        $this->printarrayattribute("departments[]",$vals);
      else
      {
        $hash = array();
        foreach($vals as $val)
          $hash[$val] = 1;
%>
<p>Surveys included in the report must come from classes in these departments:</p>
<select name="<%=$this->prefix%>_departments[]" size=7 multiple>
<option value="all"<%=$hash["all"] ? " selected" : ""%>>--- Any Department ---</option>
<%
if (!$db) $db = wces_connect();
$y = mysql_query("SELECT d.departmentid, d.name, d.code FROM AnswerSets INNER JOIN Classes as cl USING (classid) INNER JOIN Courses AS c USING (courseid) INNER JOIN Departments AS d USING (departmentid) GROUP BY d.departmentid ORDER BY d.code",$db);
while($row = mysql_fetch_array($y))
{
  extract($row);
  $selected = $hash[$departmentid] ? " selected" : "";
  print("<option value=\"$departmentid\"$selected>$code - $name</option>");
}
%>
</select>
<%
      }
    break;  
    case 4:
        %><%
      $vals = $this->loadattribute("professors");
      if (!is_array($vals)) $vals = array("all");
      if ($hidden)
        $this->printarrayattribute("professors[]",$vals);
      else
      {
        $hash = array();
        foreach($vals as $val)
          $hash[$val] = 1;
%>
<p>Surveys included in the report must come from classes with these professors:</p>
<select name="<%=$this->prefix%>_professors[]" size=7 multiple>
<option value="all"<%=$hash["all"] ? " selected" : ""%>>--- Any Professor ---</option>
<%
if (!$db) $db = wces_connect();
$y = mysql_query("SELECT p.professorid, p.name FROM AnswerSets INNER JOIN Classes as cl USING (classid) INNER JOIN Professors AS p USING (professorid) GROUP BY p.professorid",$db);
$plist = array();
while($row = mysql_fetch_array($y))
  if ($row["name"]) 
  {
    $pos = strrpos($row["name"]," ");
    array_push($plist,array("first" => substr($row["name"],0,$pos), "last" => substr($row["name"],$pos), "professorid" => $row["professorid"]));
  }  
usort($plist,"pcmp");
foreach($plist as $p)
{
  extract($p);
  $selected = $hash[$professorid] ? " selected" : "";
  print("<option value=\"$professorid\"$selected>$last, $first</option>");
}
%>
</select>
<%
      }
    break;  
    case 5:
      $vals = $this->loadattribute("courses");
      if (!is_array($vals)) $vals = array("all");
      if ($hidden)
        $this->printarrayattribute("courses[]",$vals);
      else
      {
        $hash = array();
        foreach($vals as $val)
          $hash[$val] = 1;
%>
<p>Surveys included in the report must come from these courses:</p>
<select name="<%=$this->prefix%>_courses[]" size=7 multiple>
<option value="all"<%=$hash["all"] ? " selected" : ""%>>--- Any Course ---</option>
<%
if (!$db) $db = wces_connect();
$y = mysql_query("SELECT c.courseid, concat(s.code, c.code, ' ', IFNULL(c.name,'')) AS coursename FROM AnswerSets INNER JOIN Classes as cl USING (classid) INNER JOIN Courses AS c USING (courseid) INNER JOIN Subjects AS s USING (subjectid) GROUP BY c.courseid ORDER BY coursename",$db);
while($row = mysql_fetch_array($y))
{
  extract($row);
  $selected = $hash[$courseid] ? " selected" : "";
  print("<option value=\"$courseid\"$selected>$coursename</option>");
}
%>
</select>
<%
      }
    break; 
    case 6:
      $cl = $this->loadattribute("groupclasses");
      $c = $this->loadattribute("groupcourses");
      $p = $this->loadattribute("groupprofessors");
      $d = $this->loadattribute("groupdepartments");
      $qp = $this->loadattribute("groupquestionperiods");
      
      if ($hidden)
      {
        $this->printattribute("groupclasses",$cl);
        $this->printattribute("groupcourses",$c);
        $this->printattribute("groupprofessors",$p);
        $this->printattribute("groupdepartments",$d);
        $this->printattribute("groupquestionperiods",$qp);
      }
      else
      {
        if ($this->loadattribute("output",$output) == "email")
        {
          print("<p>Aggregation is not available in email reports. Choose a web based report to use this feature.</p>");
        }
        else
        {
%><p><font size="-1">Check the survey properties below to determine how results are to be aggregated. Surveys that have the checked properties in common will have their scores added up and averaged together. For example, checking 'Departments' and 'Question Periods' will aggregate surveys from the same departments and question periods. To turn off aggregation, select 'Classes' and 'Question Periods.' To aggregate all surveys together, leave all properties unchecked.</font></p>
<input type=checkbox name="<%=$this->prefix%>_groupclasses" id="<%=$this->prefix%>_groupclasses" value="1"<%=$cl?" checked":""%>><label for="<%=$this->prefix%>_groupclasses">Classes</label><br>
<input type=checkbox name="<%=$this->prefix%>_groupcourses" id="<%=$this->prefix%>_groupcourses" value="1"<%=$c?" checked ":""%>><label for="<%=$this->prefix%>_groupcourses">Courses</label><br>
<input type=checkbox name="<%=$this->prefix%>_groupprofessors" id="<%=$this->prefix%>_groupprofessors" value="1"<%=$p?" checked ":""%>><label for="<%=$this->prefix%>_groupprofessors">Professors</label><br>
<input type=checkbox name="<%=$this->prefix%>_groupdepartments" id="<%=$this->prefix%>_groupdepartments" value="1"<%=$d?" checked ":""%>><label for="<%=$this->prefix%>_groupdepartments">Departments</label><br>
<input type=checkbox name="<%=$this->prefix%>_groupquestionperiods" id="<%=$this->prefix%>_groupquestionperiods" value="1"<%=$qp?" checked ":""%>><label for="<%=$this->prefix%>_groupquestionperiods">Question Periods</label><br>
<%
        };
      };
    break;  
    case 7:
      if (!$hidden)
      {
        %>Click finish to generate the report<br><textarea rows=15 cols=40 wrap=off contenteditable=false><%=$this->summarize()%></textarea><%
      };
    break;
    };
  }
}

function pcmp($a, $b)
{
  return strcmp($a["last"],$b["last"]);
} 

%>
<%
function report_makequery(
           $criteria, // 2d array, Members "questionperiods", "departments", "professors", "courses" contain arrays of IDs
           $groups,   // array, members "classes", "courses", "professors", "departments", "questionperiods" should be grouped if true
           $options,  // array, members "listclasses", "textresponses", "custom", "tas" are active options if true
           $sort,     // numerically indexed array. possible values are "questionperiods", "departments", "professors", "courses", "classes"
           
           &$main, // output, primary SelectMonster query
           &$list, // output, child query returned when "listclasses" option is specified
           &$textresponses, // output, child query returned when "textresponses" option is specified
           &$tas) // output, child query returned when "tas" option is specified
{ 
  global $server_wcespath;
  
  if (!$main) $main = new db_SelectMonster();
  
  $main->settable("answersets AS a");
  $main->addcolumn("SUM(a.responses) AS responses, SUM(cl.students) AS students, qs.displayname as qsname");
  $main->addjoin("questionsets AS qs",db_SelectMonster_inner,"qs.questionsetid","a.questionsetid");
  $main->addjoin("classes AS cl",db_SelectMonster_inner,"cl.classid","a.classid");
  $main->addgroup("qs.questionsetid");

  if (!$options["custom"])
  {
    $main->addwhere("qs.type = 'public'");
  };

  $criteria["classes"] = false;
  report_makecriteria($main,$criteria);
 
  if($groups["classes"])
  {
    $main->addcolumn("CONCAT('<a href=\"${server_wcespath}administrators/info.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname");
    $main->addjoin("courses AS c",db_SelectMonster_left,"c.courseid","cl.courseid");
    $main->addjoin("subjects AS s",db_SelectMonster_left,"s.subjectid","c.subjectid");  
    $main->addgroup("cl.classid");
  }; 
  
  if($groups["courses"])
  {
    $main->addcolumn("CONCAT(s.code, c.code, ' <i>', c.name, '</i>') AS coursename");
    $main->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
    $main->addjoin("subjects AS s",db_SelectMonster_left,"s.subjectid","c.subjectid");  
    $main->addgroup("c.courseid");
  };
  
  $main->addcolumn("CONCAT('<a href=\"${server_wcespath}administrators/info.php?professorid=', p.professorid, '\">', p.name, '</a>') AS professorname");
  $main->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");

  if($groups["professors"])
  {
    $main->addgroup("p.professorid");
  };

  if($groups["departments"])
  {
    $main->addcolumn("CONCAT(d.name, ' (', d.code, ')') AS departmentname");
    $main->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
    $main->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");    
    $main->addgroup("d.departmentid");
  };
  
  if ($groups["questionperiods"])
  {
    $main->addcolumn("CONCAT(qp.semester, ' ',qp.year, ' ', qp.description) AS qpname");
    $main->addjoin("questionperiods AS qp",db_SelectMonster_inner,"qp.questionperiodid","a.questionperiodid");
    $main->addgroup("a.questionperiodid");
  };
  
  $main_columns = "";
  $choices = array("a","b","c","d","e");
  for($i = 1; $i <= 10; ++$i)
  {
    if ($main_columns) $main_columns .= ", "; 
    $main_columns .= "qs.MC$i";
    foreach($choices as $choice)
      $main_columns .= ", SUM(a.MC$i$choice) AS MC$i$choice";
  };
  $main->addcolumn($main_columns);  
  
  foreach($sort as $item)
  {
    switch($item)
    {
      case "questionperiods":
        $main->addjoin("questionperiods AS qp",db_SelectMonster_left,"qp.questionperiodid","a.questionperiodid");
        $main->addordering("qp.year",false,true);
        $main->addordering("qp.semester",false,true);
        $main->addordering("qp.questionperiodid",false,true);
      break;
      case "departments":
        $main->addjoin("courses AS c",db_SelectMonster_left,"c.courseid","cl.courseid");
        $main->addjoin("departments AS d",db_SelectMonster_left,"d.departmentid","c.departmentid");    
        $main->addordering("d.code",false,true);
      break;
      case "professors":
        $main->addjoin("professors AS p",db_SelectMonster_left,"p.professorid","cl.professorid");
        $main->addordering("SUBSTRING_INDEX(p.name,' ',-1)",false,true);
      break;
      case "courses":
        $main->addjoin("courses AS c",db_SelectMonster_left,"c.courseid","cl.courseid");
        $main->addjoin("subjects AS s",db_SelectMonster_left,"s.subjectid","c.subjectid");  
        $main->addordering("s.code",false,true);
        $main->addordering("c.code",false,true);
      break;
      case "classes":
        $main->addordering("cl.section",false,true);
      break;
    }
  }

  if ($options["listclasses"] && !$groups["classes"])
  {
    $main->addjoin("courses AS c",db_SelectMonster_left,"c.courseid","cl.courseid");
    $main->addjoin("subjects AS s",db_SelectMonster_left,"s.subjectid","c.subjectid");  
  }
  
  if ($options["textresponses"])
  {  
    $main->addjoin("professors AS p",db_SelectMonster_left,"p.professorid","cl.professorid");
    $main->addjoin("courses AS c",db_SelectMonster_left,"c.courseid","cl.courseid");
    $main->addjoin("subjects AS s",db_SelectMonster_left,"s.subjectid","c.subjectid"); 
    $main->addjoin("questionperiods AS qp",db_SelectMonster_left,"qp.questionperiodid","a.questionperiodid");
  }  

  if ($options["listclasses"] && !$groups["classes"])
  {
    $list = new db_SelectMonster();
    $list->addgroup("cl.classid");
    $list->addcolumn("CONCAT('<a href=\"${server_wcespath}administrators/info.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname");
    $main->makechild($list);
  }

  if ($options["textresponses"])
  {
    $main->addcolumn("qs.FR1, qs.FR2");
    $textresponses = new db_SelectMonster();
    $textresponses->addgroup("a.answersetid");
    $textresponses->addcolumn("CONCAT('<a href=\"${server_wcespath}administrators/info.php?professorid=', p.professorid, '\">', p.name, '</a>') AS professorname");
    $textresponses->addcolumn("CONCAT('<a href=\"${server_wcespath}administrators/info.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname");
    $textresponses->addcolumn("qp.description");
    $textresponses->addcolumn("a.FR1, a.FR2");
    $main->makechild($textresponses);
  }
};

//---------------------------------------------------------------------------------------------
// This function produces report "pages" from the results of selectmonster query
// 
// The output can be html, plain text or both
// I am generating both types of reports in a single function
// to facilitate situations where both formats of a report are needed at
// once, such as when we want to make multiformat MIME messages.

// possible options: 
// - "pies"
// - "outtext"  
// - "outhtml"
// - "custom"
// - "abet"

function report_makeopage($main,$list,$textresponses,$options,$groups,&$outhtml,&$outtext)
{
  global $server_wcespath;
  
  $qpname = "Unknown";
  $departmentname = "Unknown";
  $professorname = "Unknown";
  $coursename = "Unknown";
  $classname = "Unknown";
  $qsname = "";
  $rows = 0;

  extract($main->row);

  //////////////////////////////// ON THIS PAGE ////////////////////////////////////////

  if ($options["outtext"])
  {
    $outtext .= "Included on this page:\n\n";
    $outtext .= " - Question Set: $qsname\n";
    if ($groups["questionperiods"]) $outtext .= " - Question Period: " . ucfirst($qpname) . "\n";
    if ($groups["departments"]) $outtext .= " - Department: $departmentname\n";
    $outtext .= " - Professor: $professorname\n";
    if ($groups["courses"]) $outtext .= " - Course: $coursename\n";
    if ($groups["classes"]) $outtext .= " - Class: " . strip_tags($classname) . "\n";
  }

  if ($options["outhtml"])
  {
    $outhtml .= "<h4>On this page</h4>\n";
    $outhtml .= "<table border=0>\n";
    $outhtml .= "<tr><td>Question Set:</td><td><b>$qsname</b></td></tr>\n";
    if ($groups["questionperiods"]) $outhtml .= "<tr><td>Question Period:</td><td><b>" . ucfirst($qpname) . "</b></td></tr>\n";
    if ($groups["departments"]) $outhtml .= "<tr><td>Department:</td><td><b>$departmentname</b></td></tr>\n";
    $outhtml .= "<tr><td>Professor:</td><td><b>$professorname</b></td></tr>\n";
    if ($groups["courses"]) $outhtml .= "<tr><td>Course:</td><td><b>$coursename</b></td></tr>\n";
    if ($groups["classes"]) $outhtml .= "<tr><td>Class:</td><td><b>$classname</b></td></tr>\n";
  }
  
  if ($list)
  {
    $listcount = $main->childsize($list);
    for($i = 0; $i < $listcount; ++$i)
    {
      $list->advance();
      if ($list->row["classname"])
      {
         if ($options["outhtml"]) $outhtml .= "<tr><td>Class:</td><td><b>" . $list->row["classname"] ."</b></td></tr>\n";
         if ($options["outtext"]) $outtext .= " - Class: " . strip_tags($list->row["classname"]) ."\n"; 
      }   
    }
  }  
  
  if ($options["outhtml"]) $outhtml .= "</table>\n";

  //////////////////////////////// RESPONSE STATISTICS /////////////////////////////////
  
  if ($options["outtext"])
  {  
    $outtext .= "\nResponse Statistics:\n\n";
    $outtext .= " - Total Students: $students\n";
    $outtext .= " - Students Evaluated: $responses\n\n";
  }

  if ($options["outhtml"])
  {
    $outhtml .= "<h4>Response Statistics</h4>";
    $outhtml .= "<table border=0>\n";
    $outhtml .= "<tr><td>Total Students:</td><td><b>$students</b></td></tr>\n";
    $outhtml .= "<tr><td>Students Evaluated:</td><td><b>$responses</b></td></tr>\n";
    $outhtml .= "</table>\n";
    if ($options["pies"]) $outhtml .= '<img src="' . "${server_wcespath}media/graphs/susagegraph.php" .'?blank=' . ($students-$responses) . "&filled=$responses\" width=200 height=200><img src=\"${server_wcespath}media/graphs/susagelegend.gif\" width=147 height=31>";
  }

  ///////////////////////////////// COURSE RATINGS /////////////////////////////////////

  $choices = array("a","b","c","d","e");

  if ($options["outtext"])
  {
    $outtext .= "5 = Excellent, 4 = Very Good, 3 = Satisfactory, 2 = Poor, 1 = Disastrous\n\n";
      
    $format =Array(30,"center","center","center","center","center","center","center","center");
    $table = Array(Array("Question Text","5","4","3","2","1","Avg","Mode","SD"));

    for ($i = 1; $i <= 10; ++$i)
    if ($main->row["MC$i"])
    {
      $mode = report_mode($main->row["MC${i}a"],$main->row["MC${i}b"],$main->row["MC${i}c"],$main->row["MC${i}d"],$main->row["MC${i}e"]);
      $avg = report_avg($main->row["MC${i}a"],$main->row["MC${i}b"],$main->row["MC${i}c"],$main->row["MC${i}d"],$main->row["MC${i}e"]);
      $sd = report_sd($main->row["MC${i}a"],$main->row["MC${i}b"],$main->row["MC${i}c"],$main->row["MC${i}d"],$main->row["MC${i}e"],$avg);
      $row = Array($main->row["MC$i"]);
      foreach($choices as $choice)
        array_push($row,$main->row["MC$i$choice"]);
      array_push($row,round($avg,1)); 
      array_push($row,$mode);
      array_push($row,round($sd,1));
      array_push($table,$row);
    
    };
    $outtext .= texttable($table,$format) . "\n\n";
  }
  
  if ($options["outhtml"])
  {
    $first = true;
    for($i = 1; $i <= 10; ++$i)
    {
      if ($main->row["MC$i"])
      {
        if ($first)
        {
          $first = false;
          $outhtml .= "<h4>Ratings</h4>\n";
          $outhtml .= "<table border=1 RULES=ALL FRAME=VOID>\n";
          $outhtml .= "<tr>\n";
          $outhtml .= "  <td>&nbsp;</td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/50.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/40.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/30.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/20.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/10.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/avg.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/mode.gif\"></center></td>\n";
          $outhtml .= "  <td width=20><center><img src=\"${server_wcespath}/media/report/sd.gif\"></center></td>\n";
          $outhtml .= "  <td width=120><p><b>Average (Graphical)</p></b></td>\n";
          $outhtml .= "</tr>";
        };
        $outhtml .= "<tr>\n";
        $outhtml .= "  <td>" . $main->row["MC$i"] . "</td>\n";

        foreach($choices as $choice)
          $outhtml .= ("  <td>" . $main->row["MC$i$choice"] . "</td>\n");

        $mode = report_mode($main->row["MC${i}a"],$main->row["MC${i}b"],$main->row["MC${i}c"],$main->row["MC${i}d"],$main->row["MC${i}e"]);
        $avg = report_avg($main->row["MC${i}a"],$main->row["MC${i}b"],$main->row["MC${i}c"],$main->row["MC${i}d"],$main->row["MC${i}e"]);
        $sd = report_sd($main->row["MC${i}a"],$main->row["MC${i}b"],$main->row["MC${i}c"],$main->row["MC${i}d"],$main->row["MC${i}e"],$avg);

        $outhtml .= "  <td>" . round($avg,2) . "</td>\n";
        $outhtml .= "  <td>" . round($mode,2) . "</td>\n";
        $outhtml .= "  <td>" . round($sd,2) . "</td>\n";
        $outhtml .= "  <td>" . report_meter(round($avg * 20)) . "</td>\n";
        $outhtml .= "</tr>\n";
      };  
    };
    if (!$first) $outhtml .= "</table>\n";
  };
  
  ///////////////////////////////// TEXT RESPONSES /////////////////////////////////////
  
  if ($textresponses)
  {
    $responsecount = $main->childsize($textresponses);
    
    $first = true;
    for($i = 0; $i < $responsecount; ++$i)
    {
      $textresponses->advance();
      $firstq = true;
      
      for($j = 1; $j <= 2; ++$j)
      if ($main->row["FR$j"] && $textresponses->row["FR$j"])
      {
        if ($first)
        { 
          $first = false;
          if ($options["outhtml"]) $outhtml .= "<h4>Text Responses</h4>";
          if ($options["outtext"]) $outtext .= "Text Responses\n\n"; 
        }  
        if ($firstq)
        { 
          $firstq = false; 
          if ($options["outhtml"]) $outhtml .= "<h5>" . $textresponses->row["classname"] . " - Professor " . $textresponses->row["professorname"] . "</h5>";
          if ($options["outtext"]) $outtext .= strip_tags($textresponses->row["classname"]) . " - Professor " . strip_tags($textresponses->row["professorname"]) . "\n\n";
        }  
        if ($options["outhtml"])
        {
          $outhtml .= "<h5>" . $main->row["FR$j"] . "</h5>";
          $outhtml .= "<UL><LI>";
          $outhtml .= nl2br(stripcslashes(str_replace("\t","</LI><LI>",trim($textresponses->row["FR$j"]))));
          $outhtml .= "</LI></UL>";        
        }
        if ($options["outtext"])
        {
           $outtext .= wordwrap($main->row["FR$j"],76) . "\n\n";
           $outtext .= " - ";
           $outtext .= wordwrap("    " . stripslashes(stripcslashes(str_replace("\t","\n - ",trim($textresponses->row["FR$j"])))),76);
           $outtext .= "\n\n";
        }
      };
    };  
  };
  return true;    
};
%>
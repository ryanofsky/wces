<%

require_once("wces/server.inc");
require_once("widgets/wizard.inc");
require_once("widgets/basic.inc");
require_once("wces/database.inc");
require_once("wces/wces.inc");
require_once("widgets/dynamiclist.inc");

//---------------------------------------------------------------------------------------------
// Reporting Helper Functions

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
    return str_repeat(" ",$x) . $text . str_repeat(" ",$width - $x - $l);
  else
    return $text;
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
      if (!isset($widths[$colname]) || ($l > $widths[$colname]))
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

function report_meter($percent)
{
  global $server_wcespath;
  $out = "<img src=\"${server_wcespath}/media/meter/left.gif\" width=6 height=13>";
  for($i = 0; $i < 5; ++$i)
  {
    $num = $percent - 20 * $i;
    if ($num < 0) $num = 0;
    if ($num < 10) $num = "0" . $num;
    if ($num > 20) $num = 20;
    $out .= "<img src=\"${server_wcespath}media/meter/mid${num}.gif\" width=21 height=13>";
  };
  $out .= "<img src=\"${server_wcespath}media/meter/right.gif\" width=5 height=13>";
  return $out;
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
  return $sum != 0 ? ((5.0 * $i1) + (4.0 * $i2) + (3.0 * $i3) + (2.0 * $i4) + (1.0 * $i5)) / $sum : 0;
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

// This function adds JOINS and WHERES to a selectmonster query depending on the reporting
// criteria it is given.

function report_makecriteria(&$query,&$criteria)
{
  if ($criteria["questionperiods"])
  {
    $query->addwhere("qp.questionperiodid IN " . db_set($criteria["questionperiods"]));
    $query->addjoin("questionperiods AS qp",db_SelectMonster_inner,"qp.questionperiodid","a.questionperiodid");
  }; 
  
  if ($criteria["departments"])
  {
    $query->addwhere("d.departmentid IN " . db_set($criteria["departments"]));
    $query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
    $query->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");    
  };
  
  if ($criteria["professors"])
  {
    $query->addwhere("p.professorid IN " . db_set($criteria["professors"]));
    $query->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
  };
  
  if ($criteria["courses"])
  {
    $query->addwhere("c.courseid IN " . db_set($criteria["courses"]));
    $query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
  }; 
  
  if ($criteria["classes"])
  {
    $query->addwhere("cl.classid IN " . db_set($criteria["classes"]));
  }
}

//---------------------------------------------------------------------------------------------
// This function turns the reporting options into an SelectMonster query

function report_makequery(
           $criteria, // 2d array, Members "questionperiods", "departments", "professors", "courses" contain arrays of IDs
           $groups,   // array, members "classes", "courses", "professors", "departments", "questionperiods" should be grouped if true
           $options,  // array, members "listclasses", "textresponses", "custom" are active options if true
           $sort,     // numerically indexed array. possible values are "questionperiods", "departments", "professors", "courses", "classes"
           
           &$main, // output, primary SelectMonster query
           &$list, // output, child query returned when "listclasses" option is specifed
           &$textresponses) // output, child query returned when "textresponses" option is specified
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
    $main->addcolumn("CONCAT('<a href=\"${server_wcespath}info/classinfo.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname");
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
  
  if($groups["professors"])
  {
    $main->addcolumn("CONCAT('<a href=\"${server_wcespath}info/profinfo.php?professorid=', p.professorid, '\">', p.name, '</a>') AS professorname");
    $main->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
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
        $main->addordering("p.name",false,true);
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
    $list->addcolumn("CONCAT('<a href=\"${server_wcespath}info/classinfo.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname");
    $main->makechild($list);
  }

  if ($options["textresponses"])
  {
    $main->addcolumn("qs.FR1, qs.FR2");
    $textresponses = new db_SelectMonster();
    $textresponses->addgroup("a.answersetid");
    $textresponses->addcolumn("CONCAT('<a href=\"${server_wcespath}info/profinfo.php?professorid=', p.professorid, '\">', p.name, '</a>') AS professorname");
    $textresponses->addcolumn("CONCAT('<a href=\"${server_wcespath}info/classinfo.php?classid=',cl.classid,'\">',s.code, c.code, ' <i>', c.name, '</i> (', cl.semester, ' ', cl.year, ' section ', cl.section, ')</a>') AS classname");
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

function report_makepage($main,$list,$textresponses,$options,$groups,&$outhtml,&$outtext)
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
    if ($groups["professors"]) $outtext .= " - Professor: $professorname\n";
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
    if ($groups["professors"]) $outhtml .= "<tr><td>Professor:</td><td><b>$professorname</b></td></tr>\n";
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

function makereport($db,&$wizard,$stdout,&$html,&$text)
{
  global $server_wcespath;    
    
  $groups = Array
  (
    "classes" => $wizard->groupclasses->checked,
    "courses" => $wizard->groupcourses->checked,
    "professors" => $wizard->groupprofessors->checked,
    "departments" => $wizard->groupdepartments->checked,
    "questionperiods" => $wizard->groupquestionperiods->checked
  );
    
  $sqloptions = Array
  (
    "listclasses" => $wizard->listclasses->checked,
    "textresponses" => $wizard->includetext->checked,
    "custom" => $wizard->includecustom->checked
  );  
  
  $displayoptions = Array
  (
    "pies" => $wizard->includepies->checked
  );
    
  $sort = array();
  foreach ($wizard->order->items as $order)
    array_push($sort,$wizard->ordernames[$order]);
  
  $main = $list = $textresponses = false;

  report_makequery($wizard->criteria, $groups, $sqloptions, $sort, $main, $list, $textresponses);

  $main->execute($db);
  if ($list) $list->execute($db);
  if ($textresponses) $textresponses->execute($db);

  $pages = $main->size;
  $page = 0;
    
  if ($stdout || $html)
  {
    $outhtml = "<h3>Page $page of $pages</h3>\n";
    $outhtml .= "<h4>Report Information</h4>\n";
    $outhtml .= "<table border=0><tr><td>Administrator ID: </td><td><b>" . login_getuni() . "</b></td></tr><tr><td>Date:</td><td><b>" . date("l, F j, Y") . "</b></td></tr><tr><td>Time:</td><td><b>" . date("g:i:s A") . "</b></td></tr></table>";
    $outhtml .= "<h4>Report Specification</h4><pre>" . htmlspecialchars($wizard->summarize()) . "</pre>";
      
    if ($html) $html = $outhtml;
    if ($stdout) print ($outhtml);
  }
    
  $line = "---------------------------------------------------------------------------\n";
  if ($text)
  {
    $text = "$line\nPage $page of $pages\n\n";
    $text .= "Report Information\n";
    $text .= " - Administrator ID: " . login_getuni() . "\n";
    $text .= " - Date: " . date("l, F j, Y") . "\n";
    $text .= " - Time: " . date("g:i:s A") . "\n";
  }
    
  $options = array();
  $options["pies"] = $wizard->includepies->checked ? true : false;
  $options["outhtml"] = $stdout || $html ? true : false;
  $options["outtext"] = $text ? true : false;

  while($main->advance())
  {
    ++$page;
    $outhtml = '<hr style="page-break-before:always">' . "\n";
    $outhtml .= "<h3>Page $page of $pages</h3>\n";
    $outtext = "\n$line\nPage $page of $pages\n\n";
    report_makepage($main,$list,$textresponses,$options,&$groups,$outhtml,$outtext);
    if ($stdout) print($outhtml);
    if ($html) $html .= $outhtml;
    if ($text) $text .= $outtext;
  };
}

//---------------------------------------------------------------------------------------------
// Report Widget

define("Report_wizard",1); // state
define("Report_results",2); // state
define("Report_profemail",3); // state
define("Report_email",4); // state and action
define("Report_print",5); // action
define("Report_revivewizard",6); // action
define("Report_email_send",1);
define("Report_email_cancel",2);
define("Report_email_topcancel",3);
class Report extends FormWidget
{
  var $wizard;
  var $hidemenus;
  var $state;
  var $action;
  
  var $email_form;
  var $email_to;
  var $email_from;
  var $email_subject;
  var $email_text;
  var $email_action;
  
  var $profemail_form;
  var $profemail_subject;
  var $profemail_message;
  var $profemail_debug;
  var $profemail_action; 
  
  function Report($prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
    $this->action = new ActionButton($this->prefix . "_action",$this->form,$this->formmethod);
    $this->action->loadvalues();
    
    $this->wizard = new ReportWizard($this->prefix . "_wizard",$this->form,$this->formmethod);
    $this->wizard->loadvalues();
    
    if (!$this->wizard->isfinished() || $this->action->action == Report_revivewizard)
      $this->state = Report_wizard;
    else
    {
      if ($this->wizard->output->checked == ReportWizard_output_www)
      {
        $this->email_action = new ActionButton($this->prefix . "_email_action",$this->form,$this->formmethod);
        $this->email_action->loadvalues();
        if ($this->action->action == Report_email && $this->email_action->action != Report_email_cancel && $this->email_action->action != Report_email_topcancel)
          $this->state = Report_email;
        else
          $this->state = Report_results;  
      }  
      else
        $this->state = Report_profemail;  
    }

    if ($this->action->action == Report_print) $this->wizard->printable->checked = true;

    $this->hidemenus = ($this->state == Report_results && ($this->wizard->printable->checked || $this->action->action == Report_print));
    
    if ($this->state == Report_email)
    {
      $this->email_form = new Form($this->prefix . "_email_form", $this->form, $this->formmethod);
      $this->email_to = new TextBox(1,40,"",$this->prefix . "_email_to",$this->form,$this->formmethod);
      $this->email_from = new TextBox(1,40,"",$this->prefix . "_email_from",$this->form,$this->formmethod);
      $this->email_subject = new TextBox(1,80,"",$this->prefix . "_email_subject",$this->form,$this->formmethod);
      $this->email_text = new TextBox(15,80,"",$this->prefix . "_email_text",$this->form,$this->formmethod);
      $this->email_form->loadvalues();
      $this->email_to->loadvalues();
      $this->email_from->loadvalues();
      $this->email_subject->loadvalues();
      $this->email_text->loadvalues();
    }
    else if ($this->state == Report_profemail)
    {
      $this->profemail_form = new Form($this->prefix . "_profemail_form", $this->form, $this->formmethod);
      $this->profemail_subject = new TextBox(1,76,"",$this->prefix . "_profemail_subject", $this->form, $this->formmethod);
      $this->profemail_message = new TextBox(8,76,"",$this->prefix . "_profemail_message", $this->form, $this->formmethod);
      $this->profemail_debug = new CheckBox($this->prefix . "_profemail_debug", $this->form, $this->formmethod);
      $this->profemail_action = new ActionButton($this->prefix . "_profemail_action", $this->form, $this->formmethod);
      $this->profemail_form->loadvalues();
      if ($this->profemail_form->isstale)
      {
        $this->profemail_subject->loadvalues();
        $this->profemail_message->loadvalues();
        $this->profemail_debug->loadvalues();
        $this->profemail_action->loadvalues();
      }
      else
      {
        $this->profemail_subject->text = "WCES Evaluation Results";
        $this->profemail_message->text = "Dear %profname%,\n\nThis automated email contains the results of the WCES evaluations for your classes. Send questions or comments to wces@columbia.edu";
        $this->profemail_debug->checked = false;
      }
    }
  }
  
  function display()
  {
    global $server_wcespath,$db;
  
    if ($this->state == Report_wizard)
      $this->wizard->display(false);
    else
      $this->wizard->display(true);
  
    if ($this->state == Report_email || $this->state == Report_profemail || $this->state == Report_results)
    {
      print("<table><tr><td height=50>\n");
      $this->action->display(false,Report_revivewizard,0);
      %><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<%=$this->action->gethref(Report_revivewizard,0)%>"><img src="<%=$server_wcespath%>media/report/wizard.gif" border=0 width=24 height=22 alt="Back to the Reporting Wizard"> Back to the Reporting Wizard</a></span> <%
      if ($this->state == Report_results)
      {
        $this->action->display(false,Report_print,0);
        $this->action->display(false,Report_email,0);
        %><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<%=$this->wizard->printable->checked ? "javascript:void(window.print())" : $this->action->gethref(Report_print,0) %>"><img src="<%=$server_wcespath%>media/report/print.gif" border=0 width=20 height=20 alt="Printer"><%=$this->wizard->printable->checked ? "Print this report" : "Printable version of this Report"%></a></span> <%
        %><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<%=$this->action->gethref(Report_email,0)%>"><img src="<%=$server_wcespath%>media/report/email.gif" border=0 width=20 height=20 alt="Email a copy of this report"> Email this Report</a></span> <%
      }
      if ($this->state == Report_profemail && $this->profemail_action->action)
      {
        $this->profemail_action->display(false,0,0);
        %><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<%=$this->profemail_action->gethref(0,0)%>"><img src="<%=$server_wcespath%>media/report/back.gif" border=0 width=20 height=20 alt="Back">Back to Professor Email</span> <%
      }
      if ($this->state == Report_email)
      {
        $this->email_action->display(false,Report_email_topcancel,0);
        %><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<%=$this->email_action->gethref(Report_email_topcancel,0)%>"><img src="<%=$server_wcespath%>media/report/back.gif" border=0 width=20 height=20 alt="Back">Back to Results</span> <%
      }
      print("</td></tr></table>\n");
    }

    switch($this->state)
    {
      case Report_results:
        if (!$db) $db = wces_connect();
        $html = $text = false;
        makereport($db,$this->wizard,true,$html,$text);
        break;
      case Report_email:
        $this->emailreport();
      break;
      case Report_profemail:
         $this->profemail();
      break;
    }
  }
  
  function emailreport()
  {
    global $db;
    $this->action->display();
    if ($this->email_action->action == Report_email_send)
    {
       $from = $this->email_from->text;
       $address = $this->email_to->text;
       $subject = $this->email_subject->text;
       $email = $this->email_text->text;
       print("<h3>Sending...</h3>");
       print("<pre>");
       print("<strong>From: $from</strong>\n");
       print("<strong>To: $address</strong>\n");
       print("<strong>Subject: $subject</strong>\n\n");
       print($email);
       print("</pre>");
       mail($address,$subject,$email, "To: $address\nFrom: $from\nReply-To: $from\nX-Mailer: PHP/" . phpversion());
       print("<h3>Done.</h3>"); 
    }
    else
    {
       if (!$this->email_form->isstale)
       {
         if (!$db) $db = wces_connect();
         $html = false; $text = true;
         makereport($db,$this->wizard,false,$html,$text);
         $this->email_text->text = $text;
       }  
%>
<table>
  <tr>
    <td valign=top align=right><STRONG>To:</STRONG></td>
    <td><% $this->email_to->display(); %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>From:</STRONG></td>  
    <td><% $this->email_from->display(); %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Subject:</STRONG></td>
    <td><% $this->email_subject->display(); %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Text:</STRONG></td>
    <td><% $this->email_text->display(); %></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td><% $this->email_action->display("Send",Report_email_send,0); %> <% $this->email_action->display("Cancel",Report_email_cancel,0); %></td>
  </tr>
</table>
<%
    }
  }
  
  function profemail()
  {
    global $server_wcespath,$db;
    if (!$db) $db = wces_connect();

    $this->profemail_form->display();
    
    if ($this->profemail_action->action)
    {
      $this->profemail_subject->display(true); // true means hidden
      $this->profemail_message->display(true); 
      $this->profemail_debug->display(true);
  
      $criteria = $this->wizard->criteria;
      $groups = Array ( "classes" => true, "questionperiods" => true );
      $sqloptions = Array ( "textresponses" => true, "custom" => true );
      $displayoptions = Array ("outtext" => true);
      $sort = array();

      $profs = $list = $textresponses = false;
      
      $main = new db_SelectMonster();
      $main->addjoin("questionperiods AS qp",db_SelectMonster_inner,"qp.questionperiodid","a.questionperiodid");
      $main->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
      $main->addjoin("users AS u",db_SelectMonster_left,"u.userid","p.userid");
      report_makequery($criteria, $groups, $sqloptions, $sort, $main, $list, $textresponses);

      $profs = new db_SelectMonster();
      $profs->addcolumn("p.professorid, p.name AS pname, IFNULL(p.email,u.email) AS email");
      $profs->addgroup("p.professorid");
      $main->makeparent($profs);
      $profs->execute($db); $main->execute($db); $textresponses->execute($db);
      
      $line = "---------------------------------------------------------------------------\n";
      $subject = $this->profemail_subject->text;
      $message = $this->profemail_message->text;
      $debug = $this->profemail_debug->checked;
      
      if (!$debug) print("<h3>Sending Email...</h3>");
      
      while($profs->advance())
      {
        $email = "";
        
        if ($message) $email .= wordwrap(str_replace("%profname%",$profs->row["pname"],$message),76) . "\n\n$line";
        $email .= "\nA graphical version of this report is available online at\n";
        $email .= "http://oracle.seas.columbia.edu/wces/professors/\n";

        $pages = $profs->childsize($main);
        for($page = 1; $page <= $pages; ++$page)
        {
          $email .= "\n${line}Page $page of $pages\n\n";
          $main->advance();
          $outtext = "";
          report_makepage($main,$list,$textresponses,$displayoptions,&$groups,$outhtml,$outtext);
          $email .= $outtext;
        }  
        
        $name = $profs->row["pname"];
        $address = $profs->row["email"];
//        $address = "rey4@columbia.edu"; 
        $from = "wces@columbia.edu";
        
        if (!$address) print("<p><b><font color=red>No email address found for Professor $name</font></b></p>");

        if ($debug)
        {
          print("<h4>Professor $name (<a href=\"mailto:$address\">$address</a>)</h4>\n");
          print("<p><b>$subject</b></p>\n");
          print("<ul><pre>$email</pre></ul>");
        }  
        else
          mail($address,$subject,$email, "To: $address\nFrom: $from\nReply-To: $from\nX-Mailer: PHP/" . phpversion());
      }
      
      print("<h3>Done.</h3>");
    }  
    else
    {
      print("<h3>Email Report</h3>\n");
      print("<h4>professors Included</h4>\n");
      $q = $this->wizard->query;
      $q->addcolumn("p.professorid, p.name AS pname, IFNULL(p.email,u.email) AS email, COUNT(DISTINCT cl.classid, qp.questionperiodid) AS pages");
      $q->addjoin("questionperiods AS qp",db_SelectMonster_inner,"qp.questionperiodid","a.questionperiodid");
      $q->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
      $q->addjoin("users AS u",db_SelectMonster_left,"u.userid","p.userid");
      $q->addgroup("p.professorid");
      $q->execute($db);
      print("<ul>\n");
      while($q->advance())
      {
        $email = "Unknown"; $name="Unknown";
        extract($q->row);
        if (!$email) $email = "Unknown";
        print("Professor <a href=\"{$server_wcespath}info/profinfo.php?professorid=$professorid\">$pname</a> - email <a href=\"mailto:$email\">$email</a> - $pages classes<br>");
      };  
      print("</ul>\n");
%>
<h4>Message</h4>
<table>
<tr>
  <td valign=top align=right><strong>Subject:</strong></td><td valign=top><% $this->profemail_subject->display(); %></td>
</tr>  
<tr>
  <td valign=top align=right><STRONG>Custom Message:</STRONG></td><td valign=top><% $this->profemail_message->display(); %></td>
</tr>
<tr>
  <td>&nbsp;</td><td><% $this->profemail_debug->display(); %> <% $this->profemail_debug->displaylabel("Debug Mode (doesn't actually send emails)"); %></td>
</tr>
<tr>  
  <td>&nbsp;</td><td><% $this->profemail_action->display("Submit",1,0); %></td>
</tr>
</table>  
<%      
    };
  }
};  

//---------------------------------------------------------------------------------------------
// Report Wizard

define("ReportWizard_start",0);
define("ReportWizard_output",1);
define("ReportWizard_qperiods",2);
define("ReportWizard_depts",3);
define("ReportWizard_profs",4);
define("ReportWizard_courses",5);
define("ReportWizard_aggreg",6);
define("ReportWizard_order",7);
define("ReportWizard_finish",8);
define("ReportWizard_output_www",0);
define("ReportWizard_output_email",1);

class ReportWizard extends Wizard
{
  var $output;
  var $includetext;
  var $includepies;
  var $includecustom;
  var $printable;

  var $groupclasses;
  var $groupcourses;
  var $groupprofessors;
  var $groupdepartments;
  var $groupquestionperiods;

  function ReportWizard($prefix,$form,$formmethod)
  {
    global $server_wcespath;
    
    $this->Wizard
    ( 
      array("Start","Output","Question Periods","Departments","Professors","Courses","Aggregation","Order","Finish"),
      $server_wcespath,
      $prefix,$form,$formmethod
    );
  }

  function loadvalues()
  {
    global $db;
    
    //////////////////////////////// WIZARD SETUP ////////////////////////////////////////    
    
    Wizard::loadvalues(false);

    //////////////////////////////// WIDGET SETUP ////////////////////////////////////////    
    
    $this->output = new RadioButton(array("Display Report on Web Page","Email reports to professors"),$this->prefix . "_output",$this->form,$this->formmethod);
    $this->includetext = new CheckBox($this->prefix . "_includetext", $this->form, $this->formmethod);
    $this->includepies = new CheckBox($this->prefix . "_includepies", $this->form, $this->formmethod);
    $this->includecustom = new CheckBox($this->prefix . "_includecustom", $this->form, $this->formmethod);
    $this->listclasses = new CheckBox($this->prefix . "_listclasses",$this->form,$this->formmethod);
    $this->printable = new CheckBox($this->prefix . "_printable",$this->form,$this->formmethod);

    $this->groupclasses = new CheckBox($this->prefix . "_groupclasses", $this->form, $this->formmethod);
    $this->groupcourses = new CheckBox($this->prefix . "_groupcourses", $this->form, $this->formmethod);
    $this->groupprofessors = new CheckBox($this->prefix . "_groupprofessors", $this->form, $this->formmethod);
    $this->groupdepartments = new CheckBox($this->prefix . "_groupdepartments", $this->form, $this->formmethod);
    $this->groupquestionperiods = new CheckBox($this->prefix . "_groupquestionperiods", $this->form, $this->formmethod);
    
    if ($this->visited[ReportWizard_output])
    {
      $this->output->loadvalues();
      $this->includetext->loadvalues();
      $this->includepies->loadvalues();
      $this->includecustom->loadvalues();
      $this->printable->loadvalues();
      $this->listclasses->loadvalues();
    }  
    else
    {
      $this->output->checked = ReportWizard_output_www;
      $this->includetext->checked = true;
      $this->includecustom->checked = true;
      $this->printable->checked = true;
      $this->listclasses->checked = true;
    }  
    
    if ($this->visited[ReportWizard_aggreg])
    {
      $this->groupclasses->loadvalues();
      $this->groupcourses->loadvalues();
      $this->groupprofessors->loadvalues();
      $this->groupdepartments->loadvalues();
      $this->groupquestionperiods->loadvalues();
    }
    else
    {
      $this->groupclasses->checked = true;
      $this->groupquestionperiods->checked = true;
      $this->groupdepartments->checked = true;
    }

    //////////////////////////////// WIZARD SETUP ////////////////////////////////////////    
    
    $this->crawl(array(ReportWizard_aggreg => ($this->output->checked == ReportWizard_output_email) ? true : false, ReportWizard_order => ($this->output->checked == ReportWizard_output_email) ? true : false));

    //////////////////////////////// CRITERIA SETUP //////////////////////////////////////    

    $q = $this->loadattribute("questionperiods");
    $d = $this->loadattribute("departments");
    $p = $this->loadattribute("professors");
    $c = $this->loadattribute("courses");

    if (!is_array($q) || !(count($q) > 0)) 
    {
      if (!$db) $db = wces_connect();
      $q = array(mysql_result(mysql_query("SELECT questionperiodid FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1", $db),0));
    }
    else if (in_array("all",$q)) $q = false;
    
    if (!is_array($p) || !(count($p) > 0) || in_array("all",$p)) $p = false;
    if (!is_array($d) || !(count($d) > 0) || in_array("all",$d)) $d = false;
    if (!is_array($c) || !(count($c) > 0) || in_array("all",$c)) $c = false;
    
    $this->criteria = array("questionperiods" => $q, "departments" => $d, "professors" => $p, "courses" => $c);
    unset($q,$d,$p,$c);

    /////////////////////////////// ORDERING SETUP ///////////////////////////////////////

    $labels = array("Question Period", "Department", "Course", "Class Section", "Professor");
    $this->ordernames = array("questionperiods", "departments", "courses", "classes", "professors");
    
    $this->order = new DynamicList(25,5,$labels, $this->prefix . "_order", $this->form, $this->formmethod);
    $this->order->loadvalues();

    //////////////////////////// SELECTMONSTER SETUP /////////////////////////////////////
    
    $this->query = new db_SelectMonster();
    $this->query->settable("answersets AS a");
    $this->query->addjoin("classes AS cl",db_SelectMonster_inner,"cl.classid","a.classid");

    $c = $this->criteria;
    if ($this->showstep == ReportWizard_qperiods) $c["questionperiods"] = false;
    if ($this->showstep == ReportWizard_depts) $c["departments"] = false;
    if ($this->showstep == ReportWizard_profs) $c["professors"] = false;
    if ($this->showstep == ReportWizard_courses) $c["courses"] = false;
    $c["classes"] = false;
    report_makecriteria($this->query,$c);
    unset($c);
  }

  function drawstep($stepno,$hidden)
  {  
    global $db;
    
    switch($stepno)
    {
      case ReportWizard_start:
        if (!$hidden)
          print("<p>Welcome to the reporting wizard.</p>\n");
      break;  
      case ReportWizard_output:  
        if ($hidden)
        {
          $this->output->display(true);
          $this->includetext->display(true);
          $this->includepies->display(true);
          $this->includecustom->display(true);                
          $this->printable->display(true);
        }
        else
        {
%>
<p>Choose a report format:</p>
<p>
<% $this->output->display(ReportWizard_output_www,   "onclick=\"show('" . $this->prefix . "_wwwoptions" . "')\""); %> <% $this->output->displaylabel(ReportWizard_output_www); %><br>
<% $this->output->display(ReportWizard_output_email, "onclick=\"hide('" . $this->prefix . "_wwwoptions" . "')\""); %> <% $this->output->displaylabel(ReportWizard_output_email); %>
</p>
<div id="<%=$this->prefix%>_wwwoptions"<%=$this->output->checked == ReportWizard_output_email ? ' style="display:none"': ''%>>
<p>For web reports only, choose from these options:</p>
<p>
<% $this->includetext->display(); %> <% $this->includetext->displaylabel("Include Text Responses"); %><br>
<% $this->includecustom->display(); %> <% $this->includecustom->displaylabel("Include professors' Custom Questions"); %><br>
<% $this->includepies->display(); %> <% $this->includepies->displaylabel("Include Pie Graphs in Usage Statistics"); %><br>
<% $this->printable->display(); %> <% $this->printable->displaylabel("Produce a printable report"); %>
</p>
</div>
<%
        };
      break;      
     case ReportWizard_qperiods:
        if ($hidden)
          $this->preservearrayattribute("questionperiods");
        else
        {
          if (!$db) $db = wces_connect();
          $result = mysql_query("SELECT questionperiodid, year, semester, description FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC",$db);
          $criteria = $this->criteria["questionperiods"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;

          print("<p>Surveys included in the report must come from the following question period(s):</p>\n");
          print('<select name="' . $this->prefix . '_questionperiods[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . '>--- Any Question Period ---</option>' . "\n");
          while($row = mysql_fetch_array($result))
          {
            $questionperiodid = ""; $year = ""; $semester = ""; $description = "";
            extract($row);
            $selected = $hash[$questionperiodid] ? " selected" : "";
            print("<option value=\"$questionperiodid\"$selected>" . ucwords($semester) . " $year $description</option>");
          }
          print("</select>\n");
        };
      break;
      case ReportWizard_depts:
        if ($hidden)
          $this->preservearrayattribute("departments");
        else
        {
          if (!$db) $db = wces_connect();
          $this->query->addcolumn("d.departmentid, d.name, d.code");
          $this->query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
          $this->query->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");
          $this->query->addgroup("d.departmentid");
          $this->query->addordering("d.code");
          $this->query->execute($db);
          $criteria = $this->criteria["departments"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;
 
          print("<p>Surveys included in the report must come from classes in these departments:</p>\n");
          print('<select name="' . $this->prefix . '_departments[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . '>--- Any Department ---</option>' . "\n");

          while($this->query->advance())
          {
            extract($this->query->row);
            $selected = $hash[$departmentid] ? " selected" : "";
            print("<option value=\"$departmentid\"$selected>$code - $name</option>\n");
          }
          print("</select>\n");
        }
      break;  
      case ReportWizard_courses:
        if ($hidden)
          $this->preservearrayattribute("courses");
        else
        {
          if (!$db) $db = wces_connect();
          $this->query->addcolumn("c.courseid, concat(s.code, c.code, ' ', IFNULL(c.name,'')) AS coursename");
          $this->query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
          $this->query->addjoin("subjects AS s",db_SelectMonster_inner,"s.subjectid","c.subjectid");
          $this->query->addgroup("c.courseid");
          $this->query->addordering("coursename");
          $this->query->execute($db);
          $criteria = $this->criteria["courses"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;

          print("<p>Surveys included in the report must come from these courses:</p>\n");
          print('<select name="' . $this->prefix . '_courses[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . ">--- Any Course ---</option>\n");
        
          while($this->query->advance())
          {
            extract($this->query->row);
            $selected = $hash[$courseid] ? " selected" : "";
            print("<option value=\"$courseid\"$selected>$coursename</option>");
          }          
          print("</select>\n");
        }
      break; 
      case ReportWizard_profs:
        if ($hidden)
          $this->preservearrayattribute("professors");
        else
        {
          if (!$db) $db = wces_connect();
          $this->query->addcolumn("p.professorid, p.name");
          $this->query->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
          $this->query->addgroup("p.professorid");
          $this->query->execute($db);
          $criteria = $this->criteria["professors"];
          if (!$criteria) $criteria = array("all");
          $hash= array();
          foreach($criteria as $value)
            $hash[$value] = true;
          print("<p>Surveys included in the report must come from classes with these professors:</p>\n");
          print('<select name="' . $this->prefix . '_professors[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . ">--- Any Professor ---</option>\n");
          $plist = array();
          while($this->query->advance())
          if ($name = $this->query->row["name"]) 
          {
            $pos = strrpos($name," ");
            array_push($plist,array("first" => substr($name,0,$pos), "last" => substr($name,$pos), "professorid" => $this->query->row["professorid"]));
          }
          usort($plist,"pcmp");
          foreach($plist as $p)
          {
            extract($p);
            $selected = $hash[$professorid] ? " selected" : "";
            print("<option value=\"$professorid\"$selected>$last, $first</option>");
          }  
          print("</select>\n");  
        }
      break;
      case ReportWizard_aggreg:
        if ($hidden)
        {
          $this->groupclasses->display(true);
          $this->groupcourses->display(true);
          $this->groupprofessors->display(true);
          $this->groupdepartments->display(true);
          $this->groupquestionperiods->display(true);
        }
        else
        {
          if ($this->output->checked == ReportWizard_output_email)
            print("<p>Aggregation is not available in email reports. Choose a web based report to use this feature.</p>");
          else
          {
%>
<p><font size="-1">Check the survey properties below to determine how results are to be aggregated. Surveys that have the checked properties in common will have their scores added up and averaged together. For example, checking 'departments' and 'Question Periods' will aggregate all surveys the come from the same department AND question period. To prevent any aggregation, select 'classes' and 'Question Periods.' To aggregate all surveys together, leave all properties unchecked.</font></p>
<p>
<% $this->groupclasses->display(); %> <% $this->groupclasses->displaylabel("Classes"); %><br>
<% $this->groupcourses->display(); %> <% $this->groupcourses->displaylabel("Courses"); %><br>
<% $this->groupprofessors->display(); %> <% $this->groupprofessors->displaylabel("Professors"); %><br>
<% $this->groupdepartments->display(); %> <% $this->groupdepartments->displaylabel("Departments"); %><br>
<% $this->groupquestionperiods->display(); %> <% $this->groupquestionperiods->displaylabel("Question Periods"); %><br>
</p> 
<%
          };
        };
      break;  
      case ReportWizard_order:
        if ($hidden)
          $this->order->display(true);
        else if ($this->output->checked == ReportWizard_output_email)
          print("<p>Ordering options are not available in email reports. Choose a web based report to use this feature.</p>");
		else
		{
          print("<p>Report pages will be ordered by these properties (highest priority first):</p>");
          $this->order->display();
        }
      break;
      case ReportWizard_finish:
        if (!$hidden)
        {
          print("Click finish to generate the report<br><textarea rows=15 cols=40 wrap=off contenteditable=false>" . $this->summarize() . "</textarea>");
        };
      break;
    };
  }
  
  function summarize()
  {
    global $db;
    if (!$db) $db = wces_connect();
     
    $report = "------------------------\nREPORT OPTIONS\n\n";
     
    $report .= "Output: " . ($this->output->checked == ReportWizard_output_email ? "Professor Email" : "Web Report") . "\n";
    if ($this->output->checked == ReportWizard_output_www)
    {
      $report .= "Text Responses: " . ($this->includetext->checked ? "Yes" : "No") . "\n";
      $report .= "Custom Question Sets: " . ($this->includecustom->checked ? "Yes" : "No") . "\n";
      $report .= "Pie Graphs: " . ($this->includepies->checked ? "Yes" : "No") . "\n";
      $report .= "Printable: " . ($this->printable->checked ? "Yes" : "No") . "\n";
      
      $first = true;
      $report .= "Group by: ";
      if ($this->groupclasses->checked) { if ($first) $first = false; else $report.=", "; $report.="Classes"; }
      if ($this->groupcourses->checked)  { if ($first) $first = false; else $report.=", "; $report.="Courses"; }
      if ($this->groupprofessors->checked)  { if ($first) $first = false; else $report.=", "; $report.="Professors"; }
      if ($this->groupdepartments->checked)  { if ($first) $first = false; else $report.=", "; $report.="Departments"; }
      if ($this->groupquestionperiods->checked) { if ($first) $first = false; else $report.=", "; $report.="Question Periods"; }
      if ($first) $report .= "None";
      $report .= "\n";       
      
      $report .= "Order by: ";
      $first = true;
      foreach($this->order->items as $item)
      {
        if ($first) $first = false; else $report .= ", ";
        $report .= $this->order->orderonlyitems[$item];  
      }
      $report .= "\n";
    }

    $report .= "\n------------------------\nREPORT CRITERIA\n\n";

    $list = $this->criteria["questionperiods"];
    if ($list)
    {
      $group = db_set($list);
      $y = mysql_query("SELECT questionperiodid, year, semester, description FROM questionperiods WHERE questionperiodid IN $group ORDER BY year DESC, semester DESC, questionperiodid DESC",$db);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Question Period: " . ucwords($semester) . " $year $description ($questionperiodid)\n";
      };
    }
    else
      $report .= "Question Period: ANY\n";
        
    $q = $this->query;
    if ($this->criteria["departments"])
    {
      $q->addcolumn("d.departmentid, d.name, d.code");
      $q->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
      $q->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");
      $q->addgroup("d.departmentid");
      $q->addordering("d.code");
      $q->execute($db);
      while($q->advance())
      {
        extract($q->row);
        $report .= "Department: $code $name ($departmentid)\n";
      };
    }
    else
      $report .= "Department: ANY\n";
    
    $q = $this->query;
    if ($this->criteria["professors"])
    {
      $q->addcolumn("p.professorid, p.name");
      $q->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
      $q->addgroup("p.professorid");
      $q->execute($db);
      while($q->advance())
      {
        extract($q->row);
        $report .= "Professor: $name ($professorid)\n";
      };  
    }
    else
      $report .= "Professor: ANY\n";
       
    $q = $this->query;
    if ($this->criteria["courses"])
    {
      $q->addcolumn("c.courseid, concat(s.code, c.code, ' ', IFNULL(c.name,'')) AS coursename");
      $q->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
      $q->addjoin("subjects AS s",db_SelectMonster_inner,"s.subjectid","c.subjectid");
      $q->addgroup("c.courseid");
      $q->addordering("coursename");
      $q->execute($db);
      while($q->advance())
      {
        extract($q->row);
        $report .= "Course: $coursename ($courseid)\n";
      };
    }
    else
      $report .= "Course: ANY\n";
    
    return $report;     
  }  
}

function pcmp($a, $b)
{
  return strcmp($a["last"],$b["last"]);
} 

%>
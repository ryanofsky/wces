<?

require_once("wces/database.inc");
require_once("legacy/legacy.inc");
require_once("legacy/wces/report_help.inc");
require_once("legacy/wces/report_textgrid.inc");

//options is an array of booleans with these keys:
// - "standard"
// - "custom"

$legacy_ABETCONDITION = "qs.abet != 0";
$legacy_STANDARDCONDITION = "qs.type = 'public'";
$legacy_CUSTOMCONDITION = "qs.type = 'private'";
$legacy_RATINGSCONDITION = "(LENGTH(qs.MC1) > 0 OR LENGTH(qs.MC2) > 0 OR LENGTH(qs.MC3) > 0 OR LENGTH(qs.MC4) > 0 OR LENGTH(qs.MC5) > 0 OR LENGTH(qs.MC6) > 0 OR LENGTH(qs.MC7) > 0 OR LENGTH(qs.MC8) > 0 OR LENGTH(qs.MC9) > 0 OR LENGTH(qs.MC10) > 0)";
$legacy_TEXTCONDITION = "(LENGTH(qs.FR1) > 0 OR LENGTH(qs.FR2) > 0)";
 
function legacy_report_makequeries($db, $options, $criteria, $groups, $sort, &$header, &$listclasses, &$listprofessors, &$ratings, &$abet, &$responses, &$tas)
{
  global $legacy_ABETCONDITION, $legacy_STANDARDCONDITION, $legacy_CUSTOMCONDITION, $legacy_RATINGSCONDITION, $legacy_TEXTCONDITION, $legacy_TAQUESTIONS;

  db_exec("
    CREATE TEMPORARY TABLE filledclasses
    (
      classid INTEGER NOT NULL,
      questionperiodid INTEGER NOT NULL,
      questionsetid INTEGER,
      response INTEGER,
      UNIQUE KEY(classid, questionperiodid, questionsetid)
    )
  ", $db, __FILE__, __LINE__);
  
  db_exec("
    CREATE TEMPORARY TABLE mastertable
    (
      masterid INTEGER AUTO_INCREMENT,
      clusterid INTEGER,
      classid INTEGER,
      courseid INTEGER,
      questionperiodid INTEGER,
      professorid INTEGER,
      departmentid INTEGER,
      cname TINYTEXT,
      ccode TINYTEXT,
      section CHAR(3),
      year YEAR,
      semester ENUM('spring','summer','fall'),
      pfname TINYTEXT,
      plname TINYTEXT,
      students INTEGER,
      response INTEGER,
      PRIMARY KEY(masterid),
      UNIQUE KEY(classid,questionperiodid)
    )  
  ", $db, __FILE__, __LINE__);

  $where = "";
    
  if ($criteria["topics"])
    legacy_addwhere($where,"topicid IN (" . implode(", ", $criteria["topics"]) . ")");
  
  if ($criteria["questionperiods"])
    legacy_addwhere($where,"questionperiodid IN (" . implode(", ", $criteria["questionperiods"]) . ")");
  
  if (isset($criteria["classes"]) && $criteria["classes"])
    legacy_addwhere($where,"classid IN (" . implode(", ", $criteria["classes"]) . ")");

  if ($tas)
    db_exec("INSERT INTO filledclasses(classid, questionperiodid) SELECT classid, questionperiodid FROM taratings $where GROUP BY classid, questionperiodid", $db, __FILE__, __LINE__);

  if ($options["custom"] && $options["standard"])
    $typecondition = "";
  else if ($options["custom"])
    $typecondition = $legacy_CUSTOMCONDITION;
  else if ($options["standard"])
    $typecondition = $legacy_STANDARDCONDITION;
  else // defensive programming, this is bad in many other ways as well
    $typecondition = "0";
    
  $formcondition = $responses ? "($legacy_RATINGSCONDITION OR $legacy_TEXTCONDITION)" : "($legacy_RATINGSCONDITION)";
  
  if ($abet || $options["standard"] || $options["custom"])
  {
    $conditions = array();
    if ($abet)
      $conditions[] = "$legacy_ABETCONDITION";
  
    if($options["standard"] || $options["custom"])
      $conditions[] = $typecondition ? "($typecondition AND $formcondition)" : $formcondition;
  
    if (count($conditions) > 0)
      legacy_addwhere($where,'(' . implode(" OR ",$conditions) . ')');
    
    db_exec("
      INSERT INTO filledclasses(classid, questionperiodid, questionsetid, response)
      SELECT a.classid, a.questionperiodid, a.questionsetid, a.responses
      FROM answersets AS a
      INNER JOIN questionsets AS qs ON (qs.questionsetid = a.questionsetid)
      $where", $db, __FILE__, __LINE__);
  }
    
  $where = "";
  
  if ($criteria["departments"])
    legacy_addwhere($where, "d.departmentid IN (" . implode(",", $criteria["departments"]) . ")");
    
  if ($criteria["professors"])
    legacy_addwhere($where, "p.professorid IN (" . implode(",", $criteria["professors"]) . ")");
    
  if ($criteria["courses"])
    legacy_addwhere($where, "c.courseid IN (" . implode(",", $criteria["courses"]) . ")");
    
  if ($groups["classes"])
    $groups["courses"] = $groups["professors"] = $groups["departments"] = true;

  $len = count($sort);
  for($i=0;  $i<$len; ++$i)
  foreach($groups as $group => $checked)
  if(!$checked && isset($sort[$i]) && $sort[$i] == $group)
  {
    array_push($sort,$group);
    unset($sort[$i]);
  }
  $sort = array_values($sort);
 
  $order = "";
  foreach($sort as $item)
  switch($item)
  {
    case "questionperiods":
      legacy_addorder($order, "cl.year, cl.semester, fc.questionperiodid");
    break;
    case "departments":
      legacy_addorder($order, "d.code, d.departmentid");
    break;
    case "professors":
      legacy_addorder($order, "plname, pfname, p.professorid");
    break;
    case "courses":
      legacy_addorder($order, "c.name, cl.courseid");
    break;
    case "classes":
      legacy_addorder($order, "ccode, cl.year, cl.semester, cl.section, cl.classid");
    break;
  }

  $tables = 
    "FROM filledclasses AS fc
    INNER JOIN classes AS cl ON (cl.classid = fc.classid)
    INNER JOIN courses AS c ON (c.courseid = cl.courseid)
    INNER JOIN subjects AS s ON (s.subjectid = c.subjectid)
    LEFT JOIN departments AS d ON (cl.departmentid = d.departmentid)
    LEFT JOIN professors AS p ON (p.professorid = cl.professorid)";
    
  $insert = db_exec("
    INSERT INTO mastertable(classid,courseid,questionperiodid,professorid,departmentid,
      cname, ccode, section, year, semester, plname, pfname, students, response)
    SELECT fc.classid, cl.courseid, fc.questionperiodid, cl.professorid, d.departmentid,
      c.name, CONCAT(s.code, c.code) AS ccode, cl.section, cl.year, cl.semester,
      SUBSTRING_INDEX(p.name,' ',-1) AS plname,
      SUBSTRING(p.name,1,LENGTH(p.name)-LOCATE(' ',REVERSE(p.name))) AS pfname,
      cl.students, MAX(fc.response)
    $tables $where GROUP BY fc.classid, fc.questionperiodid $order
  ",$db,__FILE__,__LINE__);

  $group = "";
  if ($groups["classes"]) legacy_addgroup($group, "classid");
  if ($groups["courses"]) legacy_addgroup($group, "courseid");
  if ($groups["professors"]) legacy_addgroup($group, "professorid");
  if ($groups["departments"]) legacy_addgroup($group, "departmentid");
  if ($groups["questionperiods"]) legacy_addgroup($group, "questionperiodid");
  
  $counts = db_exec("SELECT COUNT(*) AS number FROM mastertable $group ORDER BY masterid", $db, __FILE__, __LINE__);
  
  $masterid = 0;
  $clusterid = 0;
  while($row = mysql_fetch_assoc($counts))
  {
    ++$clusterid;
    $emasterid = $masterid + $row["number"];
    mysql_query("UPDATE mastertable SET clusterid = $clusterid WHERE $masterid < masterid AND masterid <= $emasterid", $db) or die("bad,bad,bad");
    $masterid = $emasterid;
  }

  if ($header)
  {
    $columns = " ";
    if ($groups["departments"])
      legacy_addcolumn($columns, "d.departmentid, d.name AS dname, d.code AS dcode");
    if ($groups["questionperiods"])
      legacy_addcolumn($columns, "CONCAT(qp.semester, ' ' , qp.year, ' ' , qp.description) AS qpname");
    $header = new legacy_SegmentedQuery(db_exec("
      SELECT ml.clusterid, SUM(ml.students) AS students, SUM(ml.response) AS response
        $columns
      FROM mastertable AS ml
      LEFT JOIN departments AS d ON d.departmentid = ml.departmentid
      LEFT JOIN questionperiods AS qp ON qp.questionperiodid = ml.questionperiodid
      GROUP BY ml.clusterid
      ORDER BY ml.clusterid
    ", $db, __FILE__, __LINE__), "clusterid");
    $header->advance();
  }  

  if ($listclasses)
  {
    $listclasses = new legacy_SegmentedQuery(db_exec("
      SELECT clusterid, ccode, section, year, semester, cname, classid FROM mastertable AS ml
      GROUP BY clusterid, classid
      ORDER BY clusterid, ccode, section
    ", $db, __FILE__, __LINE__), "clusterid");
    $listclasses->advance();
  }
  
  if ($listprofessors)
  {
    $listprofessors = new legacy_SegmentedQuery(db_exec("
      SELECT clusterid, plname, pfname, professorid
      FROM mastertable
      GROUP BY clusterid, professorid
      ORDER BY clusterid, plname, pfname
    ", $db, __FILE__, __LINE__), "clusterid");
    $listprofessors->advance();
  }
  
  if ($ratings)
  {
    $columns = "";
    $choices = array("a","b","c","d","e");
    for($i = 1; $i <= 10; ++$i)
    {
      $columns .= ", qs.MC$i";
      foreach($choices as $choice)
        $columns .= ", SUM(a.MC$i$choice) AS MC$i$choice";
    };

    $cnd = $typecondition ? "AND $typecondition" : "";

    $ratings = new legacy_SegmentedQuery(db_exec("
      SELECT ml.clusterid, qs.displayname $columns
      FROM mastertable AS ml
      INNER JOIN filledclasses AS fc ON ml.classid = fc.classid AND ml.questionperiodid = fc.questionperiodid
      INNER JOIN answersets AS a ON a.questionperiodid = fc.questionperiodid AND a.classid = fc.classid AND a.questionsetid = fc.questionsetid
      INNER JOIN questionsets AS qs ON qs.questionsetid = a.questionsetid
      WHERE $legacy_RATINGSCONDITION $cnd      
      GROUP BY clusterid, qs.questionsetid
      ORDER BY clusterid, qs.questionsetid
    ", $db, __FILE__, __LINE__), "clusterid");
    $ratings->advance();
  }  
    
  if ($abet)
  {
    $columns = "";
    $choices = array("a","b","c","d","e","f");
    for($i = 1; $i <= 20; ++$i)
      foreach($choices as $choice)
        $columns .= ", SUM(a.ABET$i$choice) AS ABET$i$choice";
    $abet = new legacy_SegmentedQuery(db_exec("
      SELECT ml.clusterid, qs.displayname $columns
      FROM mastertable AS ml
      INNER JOIN filledclasses AS fc ON ml.classid = fc.classid AND ml.questionperiodid = fc.questionperiodid
      INNER JOIN answersets AS a ON a.questionperiodid = ml.questionperiodid AND a.classid = ml.classid AND a.questionsetid = fc.questionsetid
      INNER JOIN questionsets AS qs ON qs.questionsetid = a.questionsetid
      WHERE $legacy_ABETCONDITION
      GROUP BY ml.clusterid ORDER BY ml.clusterid
    ", $db, __FILE__, __LINE__), "clusterid");
    $abet->advance();
  };

  if ($responses)
  {
    $cnd = $typecondition ? "AND $typecondition" : "";
    $responses = new legacy_SegmentedQuery(db_exec("
      SELECT ml.clusterid, qs.FR1 AS qsFR1, qs.FR2 AS qsFR2, a.FR1, a.FR2, ml.ccode, ml.section, ml.year, ml.semester, ml.cname, ml.classid, ml.plname, ml.pfname, ml.professorid
      FROM mastertable AS ml
      INNER JOIN filledclasses AS fc ON ml.classid = fc.classid AND ml.questionperiodid = fc.questionperiodid
      INNER JOIN answersets AS a ON a.questionperiodid = fc.questionperiodid AND a.classid = fc.classid AND a.questionsetid = fc.questionsetid
      INNER JOIN questionsets AS qs ON qs.questionsetid = a.questionsetid
      WHERE $legacy_TEXTCONDITION $cnd AND (LENGTH(a.FR1) > 0 OR LENGTH(a.FR2) > 0)
      GROUP BY ml.clusterid, a.answersetid ORDER BY ml.clusterid, ml.ccode, ml.year, ml.semester, ml.section
    ", $db, __FILE__, __LINE__), "clusterid");   
    $responses->advance();
  };
  
  if ($tas)
  {
    $columns = "";
    foreach($legacy_TAQUESTIONS as $column => $description)
      $columns .= ", ta.$column";

    $tas = new legacy_SegmentedQuery(db_exec("
      SELECT ml.clusterid, ta.name, ta.tauserid, ta.comments $columns, ml.ccode, ml.section, ml.year, ml.semester, ml.cname, ml.classid
      FROM mastertable AS ml
      INNER JOIN taratings AS ta ON ta.classid = ml.classid AND ta.questionperiodid = ml.questionperiodid
      GROUP BY ml.clusterid, ta.taratingid ORDER BY ml.clusterid, ml.ccode, ml.year, ml.semester, ml.section, ta.name
    ", $db, __FILE__, __LINE__), "clusterid");
    $tas->advance();
  };
  
  db_exec("DROP TABLE filledclasses", $db, __FILE__, __LINE__);
  db_exec("DROP TABLE mastertable", $db, __FILE__, __LINE__);
  
  return $clusterid;
}

//---------------------------------------------------------------------------------------------
// This function produces report "pages" from the results of selectmonster query
// 
// The output can be html, plain text or both
// I am generating both types of reports in a single function
// to facilitate situations where both formats of a report are needed at
// once, such as when we want to make multiformat MIME messages.

// possible options: 
// - "pies"

function legacy_report_makepage(&$outtext, &$outhtml, $options, $groups, &$header, &$listclasses, &$listprofessors, &$ratings, &$abet, &$responses, &$tas, $dataonly = false)
{
  global $wces_path, $legacy_TAQUESTIONS, $TAVALUES, $ABETQUESTIONS;
  
  $pagetext = "";
  $pagehtml = "";

  //////////////////////////////////// PAGE HEADER /////////////////////////////////////
  

  $clusterid = 1;
  if (!$dataonly)
  {
    $students = $response = $clusterid = $dname = $dcode = $semester = $year = $description = $departmentid = "";

    extract($header->row);
    
    if ($outhtml)
    {
      $pagehtml .= "<h4>On this page</h4>\n";
      $pagehtml .= "<table border=0>\n";
      if ($groups["questionperiods"]) $pagehtml .= "<tr><td>Question Period:</td><td><b>" . ucfirst($qpname) . "</b></td></tr>\n";
      if ($groups["departments"] && $departmentid) $pagehtml .= "<tr><td>Department:</td><td><b>$dname ($dcode)</b></td></tr>\n";
    }
    
    if ($outtext)
    {
      $pagetext .= "Included on this page:\n\n";
      if ($groups["questionperiods"]) $pagetext .= " - Question Period: " . ucfirst($qpname) . "\n";
      if ($groups["departments"]) $pagetext .= " - Department: $dname ($dcode)\n";
    }
    
    if ($listprofessors && $clusterid == $listprofessors->row["clusterid"])
    {
      do
      {
        $plname = $pfname = $professorid = "";
        extract($listprofessors->row);
        $profname = ($pfname || $plname) ? "$pfname $plname" : "Unknown";
        if ($outhtml) $pagehtml .= "<tr><td>Professor:</td><td><b>" . ($professorid ? "<a href=\"${wces_path}administrators/info.php?professorid=$professorid\">$profname</a>" : "<i>$profname</i>") . "</b></td></tr>\n";
        if ($outtext) $pagetext .= " - Professor: $profname\n"; 
      }
      while($listprofessors->advance());
    }
    
    if ($listclasses && $clusterid == $listclasses->row["clusterid"])
    {
      do
      {
        $ccode = $cname = $section = $year = $semester = $classid = "";
        extract($listclasses->row);
        $classname = "$ccode $cname Section $section";
        if ($groups["questionperiods"])
          $classname .= " " . ucfirst($semester) . " $year";
        if ($outhtml)
          $pagehtml .= "<tr><td>Class:</td><td><b><a href=\"${wces_path}administrators/info.php?classid=$classid\">$classname</a></b></td></tr>\n";
        if ($outtext)
          $pagetext .= " - Class: $classname\n"; 
      }
      while($listclasses->advance());
    }  
    if ($outhtml) $pagehtml .= "</table>\n";
  }
  
  //////////////////////////////// RESPONSE STATISTICS /////////////////////////////////
  
  if (!$dataonly)
  {
    if ($pagetext)
    {  
      $pagetext .= "\nResponse Statistics:\n\n";
      $pagetext .= " - Total Students: $students\n";
      $pagetext .= " - Students Evaluated: $response\n\n";
    }
    
    if ($outhtml)
    {
      $pagehtml .= "<h4>Response Statistics</h4>";
      $pagehtml .= "<table border=0>\n";
      $pagehtml .= "<tr><td>Total Students:</td><td><b>$students</b></td></tr>\n";
      $pagehtml .= "<tr><td>Students Evaluated:</td><td><b>$response</b></td></tr>\n";
      $pagehtml .= "</table>\n";
      if ($options["pies"]) $pagehtml .= '<img src="' . "${wces_path}media/graphs/susagegraph.php?blank=" . ($students-$response) . "&filled=$response\" width=200 height=200><img src=\"${wces_path}media/graphs/susagelegend.gif\" width=147 height=31>";
    }
    
    $header->advance();
  }

  ///////////////////////////////// COURSE RATINGS /////////////////////////////////////

  $choices = array("a","b","c","d","e");

  if ($ratings && $clusterid == $ratings->row["clusterid"])
  do
  {
    if ($outtext)
    {
      $pagetext .= $ratings->row["displayname"] . "\n\n";

      $pagetext .= "5 = Excellent, 4 = Very Good, 3 = Satisfactory, 2 = Poor, 1 = Disastrous\n\n";
        
      $format =Array(30,"center","center","center","center","center","center","center","center");
      $table = Array(Array("Question Text","5","4","3","2","1","Avg","Mode","SD"));
  
      for ($i = 1; $i <= 10; ++$i)
      if ($ratings->row["MC$i"])
      {
        $datums = array(5 => $ratings->row["MC${i}a"], 4 => $ratings->row["MC${i}b"], 3 => $ratings->row["MC${i}c"], 2 => $ratings->row["MC${i}d"], 1 => $ratings->row["MC${i}e"]);
        
        $mode = legacy_report_mode($datums);
        $avg = legacy_report_avg($datums);
        $sd = legacy_report_sd($datums,$avg);

        $row = Array($ratings->row["MC$i"]);
        foreach($choices as $choice)
          array_push($row,$ratings->row["MC$i$choice"]);
        array_push($row,round($avg,1)); 
        array_push($row,$mode);
        array_push($row,round($sd,1));
        array_push($table,$row);
      };
      $pagetext .= legacy_texttable($table,$format) . "\n\n";
    }
    
    if ($outhtml)
    {
      $first = true;
      for($i = 1; $i <= 10; ++$i)
      {
        if ($ratings->row["MC$i"])
        {
          if ($first)
          {
            $first = false;
            $pagehtml .= "<h4>" . $ratings->row["displayname"] . "</h4>\n";
            $pagehtml .= "<table border=1 cellspacing=0 cellpadding=2 RULES=ALL FRAME=VOID>\n";
            $pagehtml .= "<thead style=\"page-break-inside: avoid\">\n<tr>\n";
            $pagehtml .= "  <td>&nbsp;</td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/50.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/40.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/30.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/20.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/10.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/avg.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/mode.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/sd.gif\"></center></td>\n";
            $pagehtml .= "  <td width=120><p><b>Average (Graphical)</p></b></td>\n";
            $pagehtml .= "</tr>\n</thead>";
          };
          $pagehtml .= "<tr>\n";
          $pagehtml .= "  <td>" . $ratings->row["MC$i"] . "</td>\n";
  
          foreach($choices as $choice)
            $pagehtml .= ("  <td>" . $ratings->row["MC$i$choice"] . "</td>\n");

          $datums = array(5 => $ratings->row["MC${i}a"], 4 => $ratings->row["MC${i}b"], 3 => $ratings->row["MC${i}c"], 2 => $ratings->row["MC${i}d"], 1 => $ratings->row["MC${i}e"]);
  
          $mode = legacy_report_mode($datums);
          $avg = legacy_report_avg($datums);
          $sd = legacy_report_sd($datums,$avg);
  
          $pagehtml .= "  <td>" . round($avg,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($mode,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($sd,2) . "</td>\n";
          $pagehtml .= "  <td>" . legacy_report_meter(round($avg * 20)) . "</td>\n";
          $pagehtml .= "</tr>\n";
        };  
      };
      if (!$first) $pagehtml .= "</table>\n";
    };
  }
  while($ratings->advance());

  ////////////////////////////////// ABET RATINGS /////////////////////////////////////

  global $legacy_TAQUESTIONS;
  $choices = array("a","b","c","d","e","f");
  
  if ($abet && $clusterid == $abet->row["clusterid"])
  do
  {
    if ($outtext)
    {
      $pagetext .= "ABET Questions\n\n";

      $pagetext .= "To what degree did this course enhance your ability to ...\n";
      $pagetext .= "0 = not at all, 5 = a great deal\n\n";
        
      $format =Array(25,"center","center","center","center","center","center","center","center","center");
      $table = Array(Array("Question Text","0","1","2","3","4","5","Avg","Mode","SD"));
  
      for ($i = 1; $i <= 10; ++$i)
      if ($abet->row["ABET${i}a"] || $abet->row["ABET${i}b"] || $abet->row["ABET${i}c"] || $abet->row["ABET${i}d"] || $abet->row["ABET${i}e"] || $abet->row["ABET${i}f"])
      {
        $datums = array(0 => $abet->row["ABET${i}a"], 1 => $abet->row["ABET${i}b"], 2 => $abet->row["ABET${i}c"], 3 => $abet->row["ABET${i}d"], 4 => $abet->row["ABET${i}e"], 5 => $abet->row["ABET${i}f"]);
        
        $mode = legacy_report_mode($datums);
        $avg =  legacy_report_avg ($datums);
        $sd =   legacy_report_sd  ($datums, $avg);

        $row = array($ABETQUESTIONS[$i]);
        foreach($choices as $choice)
          array_push($row,$abet->row["ABET$i$choice"]);
        array_push($row,round($avg,1)); 
        array_push($row,$mode);
        array_push($row,round($sd,1));
        array_push($table,$row);
      
      };
      $pagetext .= legacy_texttable($table,$format) . "\n\n";
    }
    
    if ($outhtml)
    {
      $first = true;
      for($i = 1; $i <= 10; ++$i)
      {
        if ($abet->row["ABET${i}a"] || $abet->row["ABET${i}b"] || $abet->row["ABET${i}c"] || $abet->row["ABET${i}d"] || $abet->row["ABET${i}e"] || $abet->row["ABET${i}f"])
        {
          if ($first)
          {
            $first = false;
            $pagehtml .= "<h4>ABET Questions</h4>\n";
            
            $pagehtml .= "<p><strong>To what degree did this course enhance your ability to ...</strong><br>\n";
            $pagehtml .= "<i>0 = not at all, 5 = a great deal</i></p>\n\n";

            $pagehtml .= "<table border=1 cellspacing=0 cellpadding=2 RULES=ALL FRAME=VOID>\n";
            $pagehtml .= "<thead style=\"page-break-inside: avoid\">\n<tr>\n";
            $pagehtml .= "  <td>&nbsp;</td>\n";
            $pagehtml .= "  <td width=20><center><strong>0</strong></center></td>\n";
            $pagehtml .= "  <td width=20><center><strong>1</strong></center></td>\n";
            $pagehtml .= "  <td width=20><center><strong>2</strong></center></td>\n";
            $pagehtml .= "  <td width=20><center><strong>3</strong></center></td>\n";
            $pagehtml .= "  <td width=20><center><strong>4</strong></center></td>\n";
            $pagehtml .= "  <td width=20><center><strong>5</strong></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/avg.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/mode.gif\"></center></td>\n";
            $pagehtml .= "  <td width=20><center><img src=\"${wces_path}/media/report/sd.gif\"></center></td>\n";
            $pagehtml .= "  <td width=120><p><b>Average (Graphical)</p></b></td>\n";
            $pagehtml .= "</tr>\n</thead>\n";
          };
          $pagehtml .= "<tr>\n";
          $pagehtml .= "  <td>" . $ABETQUESTIONS[$i] . "</td>\n";
  
          foreach($choices as $choice)
            $pagehtml .= ("  <td>" . $abet->row["ABET$i$choice"] . "</td>\n");
  
          $datums = array(0 => $abet->row["ABET${i}a"], 1 => $abet->row["ABET${i}b"], 2 => $abet->row["ABET${i}c"], 3 => $abet->row["ABET${i}d"], 4 => $abet->row["ABET${i}e"], 5 => $abet->row["ABET${i}f"]);
          
          $mode = legacy_report_mode($datums);
          $avg = legacy_report_avg  ($datums);
          $sd = legacy_report_sd    ($datums, $avg);
  
          $pagehtml .= "  <td>" . round($avg,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($mode,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($sd,2) . "</td>\n";
          $pagehtml .= "  <td>" . legacy_report_meter(round($avg * 20)) . "</td>\n";
          $pagehtml .= "</tr>\n";
        };  
      };
      if (!$first) $pagehtml .= "</table>\n";
    };
  }
  while($abet->advance());
  
  ///////////////////////////////// TEXT RESPONSES /////////////////////////////////////
  
  $first = true;
  if ($responses && $clusterid == $responses->row["clusterid"])
  {
    if ($outhtml) $pagehtml .= "<h4>Text Responses</h4>";
    if ($outtext) $pagetext .= "Text Responses\n\n"; 
    do
    {
      $plname = $pfname = $professorid = $ccode = $name = $section = $year = $semester = $classid = "";
      extract($responses->row);
      $profname = $classname = "";
      if (!$groups["classes"])
      {
        $classname = "$ccode $name Section $section";
        if ($groups["questionperiods"])
        $classname .= " " . ucfirst($semester) . " $year";
      }
          
      if (!$groups["classes"] && ! $groups["professors"])
        $profname = "$pfname $plname"; 
          
      if ($outhtml && $classname) $pagehtml .= "<h5><a href=\"${wces_path}administrators/info.php?classid=$classid\">$classname</a>" . ($profname ? " - Professor <a href=\"${wces_path}administrators/info.php?professorid=$professorid\">$profname</a>" : "") . "</h5>";
      if ($outtext && $classname) $pagetext .= "$classname" . ($profname ? " - Professor $profname" : "") . "\n\n";
      
      for($j = 1; $j <= 2; ++$j)
      if ($responses->row["qsFR$j"] && $responses->row["FR$j"])
      {
        if ($outhtml)
        {
          $pagehtml .= "<h5>" . $responses->row["qsFR$j"] . "</h5>";
          $pagehtml .= "<UL><LI>";
          $pagehtml .= nl2br(stripcslashes(str_replace("\t","</LI><LI>",trim($responses->row["FR$j"]))));
          $pagehtml .= "</LI></UL>";        
        }
        if ($outtext)
        {
           $pagetext .= wordwrap($responses->row["qsFR$j"],76) . "\n\n";
           $pagetext .= " - ";
           $pagetext .= stripcslashes(str_replace("\n   \t","\n - ",str_replace("\n","\n   ",wordwrap(str_replace("\t","\n\t",$responses->row["FR$j"]),73))));           
           $pagetext .= "\n\n";
        }
      };
    }
    while($responses->advance());
  }
  /////////////////////////////////// TA RATINGS ///////////////////////////////////////

  if ($tas && $clusterid == $tas->row["clusterid"])
  {
    if ($outhtml)
    {
      $pagehtml .= "<h4>TA Ratings</h4>\n<table border=1 cellspacing=0 cellpadding=2>\n<thead style=\"page-break-inside: avoid\">\n<tr>\n  <td>Name</td>\n";
      foreach($legacy_TAQUESTIONS as $question)
        $pagehtml .= "  <td><div style=\"writing-mode:tb-rl\"><b>" . str_replace(" ","&nbsp;",$question) . "</b></div></td>\n";
      $pagehtml .= "<td><b>Comments</b></td><td><b>Student #</b></td></tr></thead>\n";  
      $avg = array();
      $num = 0;
    }
    if ($outtext) $pagetext .= "TA Ratings\n\n";
    $lclassid = 0;
    do
    {
      if (!$groups["classes"] && $lclassid != $tas->row["classid"])
      {
        $classid = $ccode = $name = $section = "";
        extract($tas->row);
        $classname = "$ccode $name Section $section";
        if ($groups["questionperiods"])
          $classname .= " " . ucfirst($semester) . " $year";
        if ($outhtml) $pagehtml .= "<tr><td colspan=8 align=center><b><a href=\"${wces_path}administrators/info.php?classid=$classid\">$classname</a></b></td></tr>\n";
        if ($outtext) $pagetext .= "$classname\n\n";
        $lclassid = $classid;
      }

      if ($outhtml) 
      {
        $name = $tas->row["name"];
        if (!$name) $name = "<i>All TAs</i>";
        $pagehtml .= "<tr>\n  <td>$name</td>";
        ++$num;
        foreach($legacy_TAQUESTIONS as $field => $question)
        {
          $avg[$field] = (isset($avg[$field]) ? $avg[$field] : 0) + $tas->row[$field];
          $pagehtml .= "  <td>" . $tas->row[$field] . "</td>\n";
        }  
        $pagehtml .= "  <td>" . ($tas->row["comments"] ? $tas->row["comments"] : "&nbsp;") . "</td><td>" . $tas->row["tauserid"] . "</td></tr>\n";
      }
      if ($outtext)
      {
        $name = $tas->row["name"];
        if ($name) $name = "\"$name\""; else $name = "All TAs";
        $pagetext .= " - TA Name: $name\n";
        foreach($legacy_TAQUESTIONS as $field => $question)
          $pagetext .= " - $question: " . $TAVALUES[$tas->row[$field]] . "\n";
        $pagetext .= " - " . str_replace("\n","\n   ",wordwrap("Comments: \"" . $tas->row["comments"],73)) . "\"\n";
        $pagetext .= " - Reviewed by: " . $tas->row["tauserid"] . "\n\n";
      }
    }
    while($tas->advance());
    
    if ($outhtml)
    {
      $pagehtml .= "<tr>\n  <td><b>AVERAGE</b></td>\n";
      foreach($legacy_TAQUESTIONS as $field => $question)
        $pagehtml .= "  <td>" . round($avg[$field]/$num, 2). "</td>\n";
      $pagehtml .= "  <td>&nbsp;</td>\n  <td>---</td>\n  </tr>\n</table>\n";
    }  
  }
  $outhtml .= $pagehtml;
  $outtext .= $pagetext;
};

?>
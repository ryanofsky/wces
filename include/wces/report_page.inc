<?

require_once("wces/database.inc");
require_once("wces/oldquestions.inc");
require_once("wces/report_help.inc");
require_once("wces/report_textgrid.inc");

define("QUESTION_PERIODS", 0x01);
define("CLASSES",          0x02);
define("COURSES",          0x04);
define("DEPARTMENTS",      0x08);
define("PROFESSORS",       0x10);
define("CATEGORIES",       0x20);

//options is an array of booleans with these keys:
// - "standard"
// - "custom"

$ABETCONDITION = "qs.abet != 0";
$STANDARDCONDITION = "qs.type = 'public'";
$CUSTOMCONDITION = "qs.type = 'private'";
$RATINGSCONDITION = "(LENGTH(qs.MC1) > 0 OR LENGTH(qs.MC2) > 0 OR LENGTH(qs.MC3) > 0 OR LENGTH(qs.MC4) > 0 OR LENGTH(qs.MC5) > 0 OR LENGTH(qs.MC6) > 0 OR LENGTH(qs.MC7) > 0 OR LENGTH(qs.MC8) > 0 OR LENGTH(qs.MC9) > 0 OR LENGTH(qs.MC10) > 0)";
$TEXTCONDITION = "(LENGTH(qs.FR1) > 0 OR LENGTH(qs.FR2) > 0)";

function report_findtopics($tempname, $criteria, $needprof = true)
{
  global $wces;
  
  $where = $twhere = "";
  
  if ($criteria[PROFESSORS] || $needprof)
  {
    $pjoin = "LEFT JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = 3\n    ";
    $pgroup = ", e.user_id";
    $pselect = ", e.user_id";
  }
  else 
    $pjoin = $pgroup = $pselect = "";
  
  if ($criteria[QUESTION_PERIODS])
    $twhere = "WHERE question_period_id IN (" . implode($criteria[QUESTION_PERIODS],",") . ") ";

  if ($criteria[CATEGORIES])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.category_id IN (" . implode($criteria[CATEGORIES],",") . ")\n    ";
  }

  if ($criteria[COURSES])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "cl.course_id IN (" . implode($criteria[COURSES],",") . ")\n    ";
  }
  
  if ($criteria[DEPARTMENTS])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "cl.department_id IN (" . implode($criteria[DEPARTMENTS],",") . ")\n    ";
  }
  
  if ($criteria[PROFESSORS])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "e.user_id IN (" . implode($criteria[PROFESSORS],",") . ")\n    ";
  }
  
  // the subselect in the FROM clause is only there as an optimization. ~ 4.5 times faster
  
  pg_query("
    CREATE TEMPORARY TABLE $tempname AS
    SELECT NULL::INTEGER AS ordinal, r.topic_id, r.question_period_id, t.class_id, cl.course_id, cl.department_id$pselect
    FROM (SELECT question_period_id, topic_id FROM survey_responses {$twhere}GROUP BY question_period_id, topic_id) AS r
    INNER JOIN wces_topics AS t USING (topic_id)
    INNER JOIN classes AS cl USING (class_id)
    $pjoin{$where}GROUP BY r.topic_id, r.question_period_id, t.class_id, cl.course_id, cl.department_id$pgroup;
  ", $wces, __FILE__, __LINE__);

  // todo:
  //
  // a major optimization would be to have a materialized view like
  // 
  // CREATE MATERIALIZED VIEW report_topics
  // SELECT r.question_period_id, t.class_id, cl.course_id, cl.department_id, e.user_id
  // FROM survey_responses AS r
  // INNER JOIN wces_topics AS t USING (topic_id)
  // INNER JOIN classes AS cl USING (class_id)
  // LEFT JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = 3
  // GROUP BY t.topic_id, r.question_period_id, t.class_id, cl.course_id, cl.department_id, e.user_id;
  //
  // postgres doesn't currently support materialized views, but it might be
  // possible to set something up with triggers. since reporting isn't typically
  // accessed in the middle of a question period, the triggers wouldn't have to
  // actually maintain data in the view, they could just set a flag that would cause
  // the view to be regenerated before it was used again
  //
  // another option would be to use a view like
  // CREATE MATERIALIZED VIEW professors
  // SELECT user_id, class_id FROM enrollments WHERE status = 3
  //
  // less of an optimization, but less complicated, and could be used to speed up other parts of the site  
};

function report_findgroups($topicstable, $groupings, $orderings)
{
  global $wces;
  
  $cols = array(
    QUESTION_PERIODS => "question_period_id",
    CLASSES          => "class_id",
    COURSES          => "course_id",
    DEPARTMENTS      => "department_id",
    PROFESSORS       => "user_id",
    CATEGORIES       => "survey_category_id"
  );
    
  $order = array(
    QUESTION_PERIODS => "cl.year, cl.semester, t.question_period_id",
    CLASSES          => "s.code, c.code, cl.section",
    DEPARTMENTS      => "d.code",
    PROFESSORS       => "u.lastname, firstname",
    CATEGORIES       => "t.survey_category_id"
  );
  
  $orderfields = array(
    QUESTION_PERIODS => "cl.year, cl.semester",
    CLASSES          => "s.code, c.code, cl.section",
    DEPARTMENTS      => "d.code",
    PROFESSORS       => "u.lastname, firstname",
    CATEGORIES       => ""
  );  

  $gorder = array(
    QUESTION_PERIODS => "MIN(t.question_period_id)",
    CLASSES          => "MIN(s.code)",
    DEPARTMENTS      => "MIN(d.code)",
    PROFESSORS       => "MIN(u.lastname)",
    CATEGORIES       => "MIN(t.survey_category_id)"
  );
  
  $badorder = false;
  
  $fargs = $fcond = $fcols = $group_s = "";
  
  $i = 0;
  foreach($groupings as $k => $v)
  {
    if ($group_s) $group_s .= ", ";
    $group_s .= "t.$cols[$k]";

    ++$i;
    
    if ($fargs) $fargs .= ", ";
    $fargs .= "INTEGER";
    
    if ($fcond) $fcond .= " AND ";
    $fcond .= "((\$$i IS NULL AND $cols[$k] IS NULL) OR $cols[$k] = \$$i)";
    
    if ($fcols) $fcols .= ", ";
    $fcols .= "$cols[$k]";
  }
  
  $joins = $sjoins = 0;
  $order_s = "";
  
  foreach($orderings as $o)
  {
    if ($order_s) $order_s .= ", ";
    $joins |= $o;
    if ($groupings[$o])
    {
      $order_s .= $order[$o];
      $group_s .= ", $orderfields[$o]";
      $sjoins |= $o;
      unset($groupings[$o]);
      if (count($groupings == 0)) break;
    }
    else
    {
      $badorder = true;
      $order_s .= $gorder[$o];
    }
  }
  
  $tempname = substr(md5(uniqid(rand(),1)),0,20);
  
  pg_query("
    CREATE TEMPORARY TABLE tab$tempname AS
    SELECT $group_s
    FROM $topicstable AS t" . ($joins & CLASSES || $sjoins & QUESTION_PERIODS ? "
    LEFT JOIN classes AS cl ON cl.class_id = t.class_id" : "") . ($joins & CLASSES ? "
    LEFT JOIN courses AS c ON c.course_id = t.course_id
    LEFT JOIN subjects AS s ON s.subject_id = c.course_id" : "") . ($joins & PROFESSORS ? "
    LEFT JOIN users AS u ON u.user_id = t.user_id" : "") . ($joins & DEPARTMENTS ? "
    LEFT JOIN departments AS d ON d.department_id = t.department_id" : "") ."
    GROUP BY $group_s
    ORDER BY $order_s;
    
    CREATE FUNCTION fun$tempname($fargs) RETURNS INTEGER AS '
      SELECT oid::integer FROM tab$tempname WHERE $fcond
    ' LANGUAGE 'sql';
    
    UPDATE $topicstable SET ordinal = fun$tempname($fcols);
    
    DROP FUNCTION fun$tempname($fargs);
    DROP TABLE tab$tempname;
  ", $wces, __FILE__, __LINE__); 
  
  return !$badorder;
  
  pg_query("
    UPDATE $topicstable AS t SET ordinal 
  ", $wces, __FILE__, __LINE__);
  
  
}

function report_makequeries($options, $criteria, $groups, $sort, &$header, &$listclasses, &$listprofessors, &$ratings, &$abet, &$responses, &$tas)
{
  global $wces, $ABETCONDITION, $STANDARDCONDITION, $CUSTOMCONDITION, $RATINGSCONDITION, $TEXTCONDITION, $TAQUESTIONS;

  pg_query("
    CREATE TEMPORARY TABLE filledclasses
    (
      classid INTEGER NOT NULL,
      questionperiodid INTEGER NOT NULL,
      questionsetid INTEGER,
      response INTEGER,
      UNIQUE KEY(classid, questionperiodid, questionsetid)
    )
  ", $wces, __FILE__, __LINE__);
  
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
  ", $wces, __FILE__, __LINE__);

  $where = "";
    
  if ($criteria["categories"])
    addwhere($where,"topicid IN (" . implode(", ", $criteria["categories"]) . ")");
  
  if ($criteria["questionperiods"])
    addwhere($where,"questionperiodid IN (" . implode(", ", $criteria["questionperiods"]) . ")");
  
  if (isset($criteria["classes"]) && $criteria["classes"])
    addwhere($where,"classid IN (" . implode(", ", $criteria["classes"]) . ")");

  if ($tas)
    db_exec("INSERT INTO filledclasses(classid, questionperiodid) SELECT classid, questionperiodid FROM taratings $where GROUP BY classid, questionperiodid", $wces, __FILE__, __LINE__);

  if ($options["custom"] && $options["standard"])
    $typecondition = "";
  else if ($options["custom"])
    $typecondition = $CUSTOMCONDITION;
  else if ($options["standard"])
    $typecondition = $STANDARDCONDITION;
  else // defensive programming, this is bad in many other ways as well
    $typecondition = "0";
    
  $formcondition = $responses ? "($RATINGSCONDITION OR $TEXTCONDITION)" : "($RATINGSCONDITION)";
  
  if ($abet || $options["standard"] || $options["custom"])
  {
    $conditions = array();
    if ($abet)
      $conditions[] = "$ABETCONDITION";
  
    if($options["standard"] || $options["custom"])
      $conditions[] = $typecondition ? "($typecondition AND $formcondition)" : $formcondition;
  
    if (count($conditions) > 0)
      addwhere($where,'(' . implode(" OR ",$conditions) . ')');
    
    db_exec("
      INSERT INTO filledclasses(classid, questionperiodid, questionsetid, response)
      SELECT a.classid, a.questionperiodid, a.questionsetid, a.responses
      FROM answersets AS a
      INNER JOIN questionsets AS qs ON (qs.questionsetid = a.questionsetid)
      $where", $wces, __FILE__, __LINE__);
  }
    
  $where = "";
  
  if ($criteria["departments"])
    addwhere($where, "d.departmentid IN (" . implode(",", $criteria["departments"]) . ")");
    
  if ($criteria["professors"])
    addwhere($where, "p.professorid IN (" . implode(",", $criteria["professors"]) . ")");
    
  if ($criteria["courses"])
    addwhere($where, "c.courseid IN (" . implode(",", $criteria["courses"]) . ")");
    
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
      addorder($order, "cl.year, cl.semester, fc.questionperiodid");
    break;
    case "departments":
      addorder($order, "d.code, d.departmentid");
    break;
    case "professors":
      addorder($order, "plname, pfname, p.professorid");
    break;
    case "courses":
      addorder($order, "c.name, cl.courseid");
    break;
    case "classes":
      addorder($order, "ccode, cl.year, cl.semester, cl.section, cl.classid");
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
  ",$wces,__FILE__,__LINE__);

  $group = "";
  if ($groups["classes"]) addgroup($group, "classid");
  if ($groups["courses"]) addgroup($group, "courseid");
  if ($groups["professors"]) addgroup($group, "professorid");
  if ($groups["departments"]) addgroup($group, "departmentid");
  if ($groups["questionperiods"]) addgroup($group, "questionperiodid");
  
  $counts = db_exec("SELECT COUNT(*) AS number FROM mastertable $group ORDER BY masterid", $wces, __FILE__, __LINE__);
  
  $masterid = 0;
  $clusterid = 0;
  while($row = mysql_fetch_assoc($counts))
  {
    ++$clusterid;
    $emasterid = $masterid + $row["number"];
    pg_query("UPDATE mastertable SET clusterid = $clusterid WHERE $masterid < masterid AND masterid <= $emasterid", $db, __FILE__, __LINE__);
    $masterid = $emasterid;
  }
  
  if ($header)
  {
    $columns = " ";
    if ($groups["departments"])
      addcolumn($columns, "d.departmentid, d.name AS dname, d.code AS dcode");
    if ($groups["questionperiods"])
      addcolumn($columns, "CONCAT(qp.semester, ' ' , qp.year, ' ' , qp.description) AS qpname");
    $header = new SegmentedQuery(db_exec("
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
    $listclasses = new SegmentedQuery(db_exec("
      SELECT clusterid, ccode, section, year, semester, cname, classid FROM mastertable AS ml
      GROUP BY clusterid, classid
      ORDER BY clusterid, ccode, section
    ", $db, __FILE__, __LINE__), "clusterid");
    $listclasses->advance();
  }
  
  if ($listprofessors)
  {
    $listprofessors = new SegmentedQuery(db_exec("
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

    $ratings = new SegmentedQuery(db_exec("
      SELECT ml.clusterid, qs.displayname $columns
      FROM mastertable AS ml
      INNER JOIN filledclasses AS fc ON ml.classid = fc.classid AND ml.questionperiodid = fc.questionperiodid
      INNER JOIN answersets AS a ON a.questionperiodid = fc.questionperiodid AND a.classid = fc.classid AND a.questionsetid = fc.questionsetid
      INNER JOIN questionsets AS qs ON qs.questionsetid = a.questionsetid
      WHERE $RATINGSCONDITION $cnd      
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
    $abet = new SegmentedQuery(db_exec("
      SELECT ml.clusterid, qs.displayname $columns
      FROM mastertable AS ml
      INNER JOIN filledclasses AS fc ON ml.classid = fc.classid AND ml.questionperiodid = fc.questionperiodid
      INNER JOIN answersets AS a ON a.questionperiodid = ml.questionperiodid AND a.classid = ml.classid AND a.questionsetid = fc.questionsetid
      INNER JOIN questionsets AS qs ON qs.questionsetid = a.questionsetid
      WHERE $ABETCONDITION
      GROUP BY ml.clusterid ORDER BY ml.clusterid
    ", $db, __FILE__, __LINE__), "clusterid");
    $abet->advance();
  };

  if ($responses)
  {
    $cnd = $typecondition ? "AND $typecondition" : "";
    $responses = new SegmentedQuery(db_exec("
      SELECT ml.clusterid, qs.FR1 AS qsFR1, qs.FR2 AS qsFR2, a.FR1, a.FR2, ml.ccode, ml.section, ml.year, ml.semester, ml.cname, ml.classid, ml.plname, ml.pfname, ml.professorid
      FROM mastertable AS ml
      INNER JOIN filledclasses AS fc ON ml.classid = fc.classid AND ml.questionperiodid = fc.questionperiodid
      INNER JOIN answersets AS a ON a.questionperiodid = fc.questionperiodid AND a.classid = fc.classid AND a.questionsetid = fc.questionsetid
      INNER JOIN questionsets AS qs ON qs.questionsetid = a.questionsetid
      WHERE $TEXTCONDITION $cnd AND (LENGTH(a.FR1) > 0 OR LENGTH(a.FR2) > 0)
      GROUP BY ml.clusterid, a.answersetid ORDER BY ml.clusterid, ml.ccode, ml.year, ml.semester, ml.section
    ", $db, __FILE__, __LINE__), "clusterid");   
    $responses->advance();
  };
  
  if ($tas)
  {
    $columns = "";
    foreach($TAQUESTIONS as $column => $description)
      $columns .= ", ta.$column";

    $tas = new SegmentedQuery(db_exec("
      SELECT ml.clusterid, ta.name, ta.tauserid, ta.comments $columns, ml.ccode, ml.section, ml.year, ml.semester, ml.cname, ml.classid
      FROM mastertable AS ml
      INNER JOIN taratings AS ta ON ta.classid = ml.classid AND ta.questionperiodid = ml.questionperiodid
      GROUP BY ml.clusterid, ta.taratingid ORDER BY ml.clusterid, ml.ccode, ml.year, ml.semester, ml.section, ta.name
    ", $db, __FILE__, __LINE__), "clusterid");
    $tas->advance();
  };
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

function report_makepage(&$outtext, &$outhtml, $options, $groups, &$header, &$listclasses, &$listprofessors, &$ratings, &$abet, &$responses, &$tas)
{
  global $wces_path, $TAQUESTIONS, $TAVALUES, $ABETQUESTIONS;
  
  $pagetext = "";
  $pagehtml = "";

  //////////////////////////////////// PAGE HEADER /////////////////////////////////////
  
  $students = $response = $clusterid = $dname = $dcode = $semester = $year = $description = "";
  extract($header->row);
  
  if ($outhtml)
  {
    $pagehtml .= "<h4>On this page</h4>\n";
    $pagehtml .= "<table border=0>\n";
    if ($groups["questionperiods"]) $pagehtml .= "<tr><td>Question Period:</td><td><b>" . ucfirst($qpname) . "</b></td></tr>\n";
    if ($groups["departments"]) $pagehtml .= "<tr><td>Department:</td><td><b>$dname ($dcode)</b></td></tr>\n";
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

  //////////////////////////////// RESPONSE STATISTICS /////////////////////////////////
  
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
        
        $mode = report_mode($datums);
        $avg = report_avg($datums);
        $sd = report_sd($datums,$avg);

        $row = Array($ratings->row["MC$i"]);
        foreach($choices as $choice)
          array_push($row,$ratings->row["MC$i$choice"]);
        array_push($row,round($avg,1)); 
        array_push($row,$mode);
        array_push($row,round($sd,1));
        array_push($table,$row);
      };
      $pagetext .= texttable($table,$format) . "\n\n";
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
  
          $mode = report_mode($datums);
          $avg = report_avg($datums);
          $sd = report_sd($datums,$avg);
  
          $pagehtml .= "  <td>" . round($avg,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($mode,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($sd,2) . "</td>\n";
          $pagehtml .= "  <td>" . report_meter(round($avg * 20)) . "</td>\n";
          $pagehtml .= "</tr>\n";
        };  
      };
      if (!$first) $pagehtml .= "</table>\n";
    };
  }
  while($ratings->advance());

  ////////////////////////////////// ABET RATINGS /////////////////////////////////////

  global $TAQUESTIONS;
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
        
        $mode = report_mode($datums);
        $avg =  report_avg ($datums);
        $sd =   report_sd  ($datums, $avg);

        $row = array($ABETQUESTIONS[$i]);
        foreach($choices as $choice)
          array_push($row,$abet->row["ABET$i$choice"]);
        array_push($row,round($avg,1)); 
        array_push($row,$mode);
        array_push($row,round($sd,1));
        array_push($table,$row);
      
      };
      $pagetext .= texttable($table,$format) . "\n\n";
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
          
          $mode = report_mode($datums);
          $avg = report_avg  ($datums);
          $sd = report_sd    ($datums, $avg);
  
          $pagehtml .= "  <td>" . round($avg,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($mode,2) . "</td>\n";
          $pagehtml .= "  <td>" . round($sd,2) . "</td>\n";
          $pagehtml .= "  <td>" . report_meter(round($avg * 20)) . "</td>\n";
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
      foreach($TAQUESTIONS as $question)
        $pagehtml .= "  <td><div style=\"writing-mode:tb-rl\"><b>" . str_replace(" ","&nbsp;",$question) . "</b></div></td>\n";
      $pagehtml .= "<td><b>Comments</b></td><td><b>Student #</b></td></thead></tr>\n";  
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
        foreach($TAQUESTIONS as $field => $question)
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
        foreach($TAQUESTIONS as $field => $question)
          $pagetext .= " - $question: " . $TAVALUES[$tas->row[$field]] . "\n";
        $pagetext .= " - " . str_replace("\n","\n   ",wordwrap("Comments: \"" . $tas->row["comments"],73)) . "\"\n";
        $pagetext .= " - Reviewed by: " . $tas->row["tauserid"] . "\n\n";
      }
    }
    while($tas->advance());
    
    if ($outhtml)
    {
      $pagehtml .= "<tr>\n  <td><b>AVERAGE</b></td>\n";
      foreach($TAQUESTIONS as $field => $question)
        $pagehtml .= "  <td>" . round($avg[$field]/$num, 2). "</td>\n";
      $pagehtml .= "  <td>&nbsp;</td>\n  <td>---</td>\n  </tr>\n</table>\n";
    }  
  }
  $outhtml .= $pagehtml;
  $outtext .= $pagetext;
};

?>
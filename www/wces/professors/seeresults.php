<?

  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wbes/component.inc");
  require_once("wbes/component_choice.inc");
  require_once("wbes/component_textresponse.inc");
  require_once("wbes/component_heading.inc");
  require_once("wbes/component_text.inc");
  require_once("wbes/survey.inc");
  require_once("wces/database.inc");
  require_once("widgets/basic.inc");

  require_once("wces/report_page.inc");
  require_once("wces/report_generate.inc");
  login_protect(login_professor);
  $user_id = login_getuserid();
  wces_connect();
  $db = wces_oldconnect();
  $profname = login_getname();

  $result = pg_query("SELECT oldid FROM temp_prof WHERE newid = $user_id", $wces, __FILE__, __LINE__);
  if (pg_numrows($result) == 1)
    $profid = pg_result($result,0,0);
  else
    $profid = 0;

function tshowresults($question_period_id,$class_id)
{
  global $profid, $wces;

  $user_id = login_getuserid();

  print('<h3><a href="seeresults.php">Back</a></h3><hr>');

  $sqloptions = array ("standard" => true, "custom" => true);

  $criteria = array
  (
    PROFESSORS => array($user_id),
    CLASSES => array($class_id),
    COURSES => false,
    DEPARTMENTS => false,
    QUESTION_PERIODS => array($question_period_id),
    CATEGORIES => false
  );

  $sort = array(QUESTION_PERIODS, COURSES, DEPARTMENTS, PROFESSORS, CATEGORIES);
  $groups = array(QUESTION_PERIODS => 1, CLASSES => 1, COURSES => 1, DEPARTMENTS => 1, PROFESSORS => 1, CATEGORIES => 0);

  wces_connect();
  report_findtopics("rwtopics", $criteria);
  //$result = pg_query("SELECT * FROM rwtopics", $wces, __FILE__, __LINE__);
  //pg_show($result);
  report_findgroups("rwtopics", $groups, $sort);

  $html = $text = false;
  makeall(true, $html, $text, $groups);
}

function addwhere(&$where, $clause)
{
  $where .= $where ? " AND " : "WHERE ";
  $where .= $clause;
}

function addorder(&$order, $clause)
{
  $order .= $order ? ", " : "ORDER BY ";
  $order .= $clause;
}

function addgroup(&$group, $clause)
{
  $group .= $group ? ", " : "GROUP BY ";
  $group .= $clause;
}

function addcolumn(&$columns, $clause)
{
  $columns .= $columns ? ", " : "";
  $columns .= $clause;
}

class SegmentedQuery
{
  var $result;
  var $column;
  var $last;

  function SegmentedQuery($result, $column)
  {
    $this->result = $result;
    $this->column = $column;
    $this->last = false;
  }

  function advance()
  {
    $this->row = mysql_fetch_assoc($this->result);
    if ($this->row && $this->last == $this->row[$this->column])
    {
      $this->last = $this->row[$this->column];
      return true;
    }
    else
    {
      $this->last = $this->row[$this->column];
      return false;
    }
  }
}

$TAQUESTIONS = array
(
  "overall" => "Overall Quality",
  "knowledgeability" => "Knowledgeability",
  "approachability" => "Approachability",
  "availability" => "Availability",
  "communication" => "Communication"
);

function old_report_makepage(&$outtext, &$outhtml, $options, $groups, &$header, &$listclasses, &$listprofessors, &$ratings, &$abet, &$responses, &$tas, $dataonly = false)
{
  global $wces_path, $TAQUESTIONS, $TAVALUES, $ABETQUESTIONS;

  $pagetext = "";
  $pagehtml = "";

  //////////////////////////////////// PAGE HEADER /////////////////////////////////////


  $clusterid = 1;
  if (!$dataonly)
  {
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

$ABETCONDITION = "qs.abet != 0";
$STANDARDCONDITION = "qs.type = 'public'";
$CUSTOMCONDITION = "qs.type = 'private'";
$RATINGSCONDITION = "(LENGTH(qs.MC1) > 0 OR LENGTH(qs.MC2) > 0 OR LENGTH(qs.MC3) > 0 OR LENGTH(qs.MC4) > 0 OR LENGTH(qs.MC5) > 0 OR LENGTH(qs.MC6) > 0 OR LENGTH(qs.MC7) > 0 OR LENGTH(qs.MC8) > 0 OR LENGTH(qs.MC9) > 0 OR LENGTH(qs.MC10) > 0)";
$TEXTCONDITION = "(LENGTH(qs.FR1) > 0 OR LENGTH(qs.FR2) > 0)";

function report_makequeries($db, $options, $criteria, $groups, $sort, &$header, &$listclasses, &$listprofessors, &$ratings, &$abet, &$responses, &$tas)
{
  global $ABETCONDITION, $STANDARDCONDITION, $CUSTOMCONDITION, $RATINGSCONDITION, $TEXTCONDITION, $TAQUESTIONS;

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
    addwhere($where,"topicid IN (" . implode(", ", $criteria["topics"]) . ")");

  if ($criteria["questionperiods"])
    addwhere($where,"questionperiodid IN (" . implode(", ", $criteria["questionperiods"]) . ")");

  if (isset($criteria["classes"]) && $criteria["classes"])
    addwhere($where,"classid IN (" . implode(", ", $criteria["classes"]) . ")");

  if ($tas)
    db_exec("INSERT INTO filledclasses(classid, questionperiodid) SELECT classid, questionperiodid FROM taratings $where GROUP BY classid, questionperiodid", $db, __FILE__, __LINE__);

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
        $where", $db, __FILE__, __LINE__);
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
  ",$db,__FILE__,__LINE__);

  $group = "";
  if ($groups["classes"]) addgroup($group, "classid");
  if ($groups["courses"]) addgroup($group, "courseid");
  if ($groups["professors"]) addgroup($group, "professorid");
  if ($groups["departments"]) addgroup($group, "departmentid");
  if ($groups["questionperiods"]) addgroup($group, "questionperiodid");

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

  db_exec("DROP TABLE filledclasses", $db, __FILE__, __LINE__);
  db_exec("DROP TABLE mastertable", $db, __FILE__, __LINE__);

  return $clusterid;
}


function listclasses()
{
  global $profid,$profname,$db, $wces;

  $uid = login_getuserid();
  wces_connect();

  print("<h3>$profname - Survey Responses</h3>\n");

  $result = pg_query("
    SELECT t.topic_id, t.class_id, get_class(t.class_id) AS cl, q.question_period_id, COUNT(r.user_id) AS count, q.displayname
    FROM enrollments AS e
    INNER JOIN wces_topics AS t USING (class_id)
    INNER JOIN classes AS cl ON cl.class_id = e.class_id
    INNER JOIN semester_question_periods AS q ON q.year = cl.year AND q.semester = cl.semester
    LEFT JOIN survey_responses AS r ON r.topic_id = t.topic_id AND r.question_period_id = q.question_period_id
    WHERE e.user_id = $uid AND e.status = 3 AND q.question_period_id <= 2
    GROUP BY t.class_id, q.question_period_id, q.displayname, t.topic_id
    ORDER BY q.question_period_id DESC, cl
  ", $wces, __FILE__, __LINE__);

  $q = new pg_segmented_wrapper($result, array("question_period_id"));
  while($q->row)
  {
    extract($q->row);
    if ($q->split[0]) print("<h4>$displayname</h4>\n<ul>\n");
    if ($count == 0)
      print("  <li>" . format_class($cl) . " (No Responses Found)</li>");
    else
      print("  <li><a href=\"seeresults.php?question_period_id=$question_period_id&class_id=$class_id\">" . format_class($cl) . "</a></li>");

    $q->advance();
    if ($q->split[0]) print("</ul>\n");
  }












  /////////////////////////


  $questionperiods = db_exec("
    SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
    cl.classid, s.code as scode, c.code, cl.section, c.name, cl.name as clname, COUNT(DISTINCT cr.userid, cr.classid) AS hasanswers
    FROM classes AS cl
    INNER JOIN groupings AS g ON cl.classid = g.linkid AND g.linktype = 'classes'
    INNER JOIN questionperiods AS qp ON cl.year = qp.year AND cl.semester = qp.semester
    INNER JOIN courses AS c ON c.courseid = cl.courseid
    INNER JOIN subjects AS s USING (subjectid)
    LEFT JOIN cheesyresponses AS cr ON (cr.classid = cl.classid AND cr.questionperiodid = qp.questionperiodid)
    WHERE 8 <= qp.questionperiodid AND qp.questionperiodid <= 9 AND cl.professorid = '$profid'
    GROUP BY qp.questionperiodid, cl.classid
    ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
  ", $db, __FILE__, __LINE__);

  $count = $lqp = 0; $first = true;
  while($questionperiod = mysql_fetch_assoc($questionperiods))
  {
    $questionperiodid = $semester = $year = $description = "";
    $classid = $scode = $code = $section = $name = $clname = $hasanswers = "";
    extract($questionperiod);

    if ($questionperiodid != $lqp)
    {
      if ($first)
        $first = false;
      else
        print("</ul>");
      $count = 0;
      $lqp = $questionperiodid;
      print("<h4>" . ucfirst($semester) . " $year - $description</h4>\n");
      print("<ul>\n");
    }

    if ($clname) $name .= " - $clname";
    if ($hasanswers)
    {
      ++$count;
      print("  <li><a href=\"seeresults.php?nquestionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");
    }
    else
      print("  <li>$scode$code$section <i>$name</i> (No Responses Found)</li>");
  }

  if (!$first)
  {
    print("</ul>");
  }

  $questionperiods = db_exec("
    SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
    cl.classid, s.code as scode, c.code, cl.section, c.name, cl.name as clname, COUNT(DISTINCT a.answersetid) AS hasanswers
    FROM questionperiods AS qp
    INNER JOIN classes AS cl ON cl.year = qp.year AND cl.semester = qp.semester
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    LEFT JOIN answersets AS a ON (a.classid = cl.classid AND a.questionperiodid = qp.questionperiodid)
    WHERE qp.questionperiodid <= 7 AND cl.professorid = '$profid'
    GROUP BY qp.questionperiodid, cl.classid
    ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
  ", $db, __FILE__, __LINE__);

  $count = $lqp = 0; $first = true;
  while($questionperiod = mysql_fetch_assoc($questionperiods))
  {
    $questionperiodid = $semester = $year = $description = "";
    $classid = $scode = $code = $section = $name = $clname = $hasanswers = "";
    extract($questionperiod);

    if ($questionperiodid != $lqp)
    {
      if ($first)
        $first = false;
      else
      {
        if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");
        print("</ul>");
      }
      $count = 0;
      $lqp = $questionperiodid;
      print("<h4>" . ucfirst($semester) . " $year - $description</h4>\n");
      print("<ul>\n");
    }

    if ($clname) $name .= " - $clname";
    if ($hasanswers)
    {
      ++$count;
      print("  <li><a href=\"seeresults.php?questionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");
    }
    else
      print("  <li>$scode$code$section <i>$name</i> (No Responses Found)</li>");
  }

  if (!$first)
  {
    if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");
    print("</ul>");
  }

};

function showresults($db,$questionperiodid,$classid)
{
  global $profid;

  print('<h3><a href="seeresults.php">Back</a></h3><hr>');

  $sqloptions = array ("standard" => true, "custom" => true);
  $groups = Array("classes" => $classid ? true : false, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $header = $ratings = $listclasses = $listprofessors = $abet = $responses = $tas = true;
  $sort = array("classes","questionperiods","professors","courses","departments");
  $criteria = array("professors" => array($profid), "classes" => $classid ? array($classid) : false, "topics" => false, "questionperiods" => array($questionperiodid), "departments" => false, "courses" => false);

  report_makequeries($db, $sqloptions, $criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet,
    $responses, $tas);

  $displayoptions = array("pies" => true);

  $outhtml = "<br>"; $text = false;

  old_report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
    $listprofessors, $ratings, $abet, $responses, $tas);

  print($outhtml);
}

///////////////////////////////////////////////////////////////////////////////

function happycsv(&$str)
{
  $l = strlen($str);
  $str = str_replace(array("\r", "\n", '"'), array(" ", " ", '""'), $str);
  if (strlen($str) != $l || strpos($str,",") !== false)
    $str = "\"$str\"";
}

function getr(&$arr, $index1, $index2)
{
  if (isset($arr[$index1]))
    return $arr[$index1];
  if (isset($arr[$index2]))
    return $arr[$index2];
  return NULL;
}

function getrating(&$component, $index)
{
  if (!is_numeric($index)) return "";

  $count = (int)count($component->choices);
  if ($component->is_numeric  || $component->first_number || $component->last_number)
  {
    if ($count <= 1)
      return (int)$component->first_number;
    else
      return (double)$index / (double)($count-1.0) * ($component->last_number - $component->first_number) + $component->first_number;
  }
  else if (isset($component->choices[$index]))
    return $component->choices[$index];
  else if ($component->other_choice && $index == $count)
    return "";
  else
    return $index;
}

function printcsv($survey,$ck,$results)
{
  header("Content-Type: application/octet-stream");
  //header("Content-Type: text/plain");
  $first = true;
  foreach($ck as $k)
  {
    $c = &$survey->components[$k];
    if (get_class($c) == "textresponse")
    {
      if ($first) $first = false; else print(",");
      $str = $c->text; happycsv($str);
      print($str);
    }
    else if (get_class($c) == "choice")
    {
      foreach($c->questions as $str)
      {
        if ($first) $first = false; else print(",");
        happycsv($str);
        print($str);
      }
    }
  }
  print("\n");

  while($row = mysql_fetch_assoc($results))
  {
    $result = unserialize($row["dump"]);
    if (is_array($result))
    {
      $first = true;
      foreach($ck as $k)
      {
        $prefix = "survey_${k}_";
        $prefix2 = "student_${k}_";

        $c = &$survey->components[$k];
        if (get_class($c) == "textresponse")
        {
          if ($first) $first = false; else print(",");
          $str = getr($result,"${prefix}response","${prefix2}response");
          happycsv($str);
          print($str);
        }
        else if (get_class($c) == "choice")
        {
          foreach($c->questions as $q => $str)
          {
            if ($first) $first = false; else print(",");
            $v = getr($result,"${prefix}q${q}","${prefix2}q${q}");
            if (is_array($v))
            {
              $str = "";
              foreach($v as $index)
              {
                $rat = getrating($c, $index);
                if ($rat)
                {
                  if ($str) $str .= "|";
                  $str .= $rat;
                }
              }
            }
            else
              $str = getrating($c, $v);

            $other = getr($result,"${prefix}q${q}_" . count($c->choices),"${prefix2}q${q}_" . count($c->choices));

            if ($other)
            {
                if ($str) $str .= "|";
                $str .= $other;
            }

            happycsv($str);
            print($str);
          }
        }
      }
    }
    else
    {
      print("bad result set");
    }
    print("\n");
  }
}

function calc_mode($distribution)
{
  $max = $maxv = false;
  foreach($distribution as $score => $people)
  if ($max === false || $people >= $maxv)
  {
    $max = $score;
    $maxv = $people;
  }
  return $max;
}

function calc_avg($distribution)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $score * $people;
  }
  return $sum == 0 ? 0 : $wsum / $sum;
}

function calc_sd($distribution, $avg)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $people * pow(($score - $avg),2);
  }
  return $sum == 0 ? 0 : sqrt($wsum/$sum);
}

function make_html($is_html, $text)
{
  return $is_html ? $text : htmlspecialchars($text);
};

function nshowresults($db,$questionperiodid,$classid,$showcsv)
{
  global $profid, $wces_path;

  $surveyrow = db_exec("
    SELECT cc.dump
    FROM cheesyclasses AS cc
    WHERE cc.classid IN ($classid,0) AND cc.questionperiodid = $questionperiodid
    ORDER BY cc.classid DESC LIMIT 1
  ", $db, __FILE__, __LINE__);
  if (mysql_num_rows($surveyrow) != 1) die("Survey '$surveyid' not found");

  $surveyarr = mysql_fetch_assoc($surveyrow);
  $survey = unserialize($surveyarr["dump"]);

  if (get_class($survey) != "survey") die("Invalid Survey Data");

  $ck = array_keys($survey->components);

  $results = db_exec("
    SELECT cr.dump
    FROM cheesyresponses AS cr
    INNER JOIN classes AS cl USING (classid)
    WHERE cr.classid = $classid AND cr.questionperiodid = $questionperiodid AND cl.professorid = $profid
    ORDER BY RAND()
  ", $db, __FILE__, __LINE__);

  if ($showcsv)
    return printcsv($survey,$ck,$results);

  print('<h3><a href="seeresults.php">Back</a></h3><hr>');

  $qpi = db_exec("SELECT year, semester, description FROM questionperiods WHERE questionperiodid = $questionperiodid", $db, __FILE__, __LINE__);
  extract(mysql_fetch_assoc($qpi));

  $cli = db_exec("
    SELECT CONCAT(s.code, c.code, ' ', c.name, ' Section ', cl.section) cname, p.name AS pname, cl.students, COUNT(*) AS responses
    FROM classes AS cl
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    INNER JOIN professors AS p ON p.professorid = cl.professorid
    INNER JOIN cheesyresponses AS cr ON cr.classid = cl.classid AND cr.questionperiodid = $questionperiodid
    WHERE cl.classid = $classid
    GROUP BY cl.classid
  ", $db, __FILE__, __LINE__);

  extract(mysql_fetch_assoc($cli));

?><br><h4>On this page</h4>
 <table border=0>
 <tr><td>Question Period:</td><td><b><?=ucwords($semester)?> <?=$year?> <?=$description?></b></td></tr>
 <tr><td>Professor:</td><td><b><a href="<?=$wces_path?>administrators/info.php?professorid=<?=$profid?>"><?=$pname?></a></b></td></tr>
 <tr><td>Class:</td><td><b><a href="<?=$wces_path?>administrators/info.php?classid=<?=$classid?>"><?=$cname?></a></b></td></tr>
 </table>
 <h4>Response Statistics</h4><table border=0>
 <tr><td>Total Students:</td><td><b><?=$students?></b></td></tr>
 <tr><td>Students Evaluated:</td><td><b><?=$responses?></b></td></tr>
 </table>
 <img src="<?=$wces_path?>media/graphs/susagegraph.php?blank=<?=$students-$responses?>&filled=<?=$responses?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/susagelegend.gif" width=147 height=31>
 <h4>Results Summary</h4>
<?

  $resp = array();

  while($row = mysql_fetch_assoc($results))
    $resp[] = unserialize($row['dump']);

  $first = true;
  foreach($ck as $k)
  {
    print("<p>");
    
    $c = &$survey->components[$k];
    $prefix = "survey_${k}_";
    $prefix2 = "student_${k}_";

    if (get_class($c) == "textresponse")
    {
      print("<h5>" . make_html($c->is_html, $c->text) . "</h5>\n");
      print("<ul>");
      reset($resp);
      $first = true;
      while(list($rk) = each($resp))
      {
        $v = isset($resp[$rk]["{$prefix}response"]) ? $resp[$rk]["{$prefix}response"] : $resp[$rk]["{$prefix2}response"];
        if ($v)
        {
          if ($first) {$first = false; print("  <li>"); } else print("</li>\n  <li>");
          print(nl2br(htmlspecialchars($v)));
        }
      }
      if (!$first) print("</li>\n"); else print("<blockquote><i>None</i></blockquote>");
      print("</ul>");
    }
    else if (get_class($c) == "choice")
    {
      if ($c->is_numeric)
      {
        print("($c->first_number = " . make_html($c->is_html, $c->choices[0]) . ", $c->last_number = " . make_html($c->is_html, $c->choices[1]) . ")<br>");
      }
      print("<table border=1 cellspacing=0 cellpadding=2>\n");
      print("<thead style=\"page-break-inside: avoid\">\n");
      print("<tr>\n");
      print("<td>&nbsp;</td>\n");
      $values = array();
      if ($c->is_numeric)
      {
        $showstats = true;
        $d = $c->first_number < $c->last_number ? 1 : -1;
        $r = abs($c->last_number - $c->first_number);
        for($i=0; $i <= $r; ++$i)
        {
          $values[$i] = $c->first_number + $d * $i;
          printf("<td>%.1f</td>", $values[$i]);
        }
      }
      else
      {
        $showstats = is_numeric($c->first_number) && is_numeric($c->last_number) ? true : false;
        foreach($c->choices as $ci => $ct)
        {
          $str = make_html($c->is_html, $ct);
          if ($showstats)
          {
            $values[$ci] = $c->first_number + ($ci / (count($c->choices)-1)) * ($c->last_number - $c->first_number);
            $str .= sprintf(" (%.1f)",$values[$ci]);
          }
          else
            $values[$ci] == false;

          print("  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$str</b></div></td>\n");
        }
        if ($c->other_choice)
        {
          $str = make_html($c->is_html, $c->other_choice);
          $values[] = false;
          print("  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$str</b></div></td>\n");
        }
        if ($showstats)
        {
          print("  <td><b>Avg</b></td>\n  <td><b>Mode</b></td>\n  <td><b>SD</b></td>\n");
        }
      }
      print("</tr>\n");


      foreach($c->questions as $q => $str)
      {
        print("<tr>\n");
        print("  <td>" . make_html($c->is_html, $str) . "</td>\n");

        // array_fill function apparently missing on oracle (?)
        $sums = array_pad(array(),count($c->choices),0);

        reset($resp);
        while(list($rk) = each($resp))
        {
          $result = &$resp[$rk];
          $v = getr($result, "${prefix}q${q}", "${prefix2}q${q}");
          if (is_array($v))
          {
            foreach($v as $vi)
              $sums[$vi] += 1;
          }
          else if (isset($v))
            @$sums[$v] += 1;
        }

        $dist = false;
        foreach($values as $vi => $vk)
        {
          if ($showstats) $dist[$vk] = $sums[$vi];
          print("  <td>$sums[$vi]</td>\n");
        }

        if ($showstats)
        {
          $a = report_avg($dist);
          printf("  <td>%.1f</td>\n  <td>%.1f</td>\n  <td>%.1f</td>\n",$a,report_mode($dist),report_sd($dist,$a));
        }

        print("</tr>\n");
      }
      print("</table>\n");
    }
    print("</p>");
  }
  print("\n");

  /////////////////////////////////////////////////////////////////////////////
  // TA SECTION

  $sqloptions = array ("standard" => false, "custom" => false);
  $groups = Array("classes" => $classid ? true : false, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $ratings = $listclasses = $listprofessors = $abet = $responses = false;
  $tas = $header = true;

  $sort = array("classes","questionperiods","professors","courses","departments");
  $criteria = array("professors" => array($profid), "classes" => $classid ? array($classid) : false, "topics" => false, "questionperiods" => array($questionperiodid), "departments" => false, "courses" => false);

  report_makequeries($db, $sqloptions, $criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet,
    $responses, $tas);

  $displayoptions = array("pies" => false);

  $outhtml = "<br>"; $text = false;

  old_report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
    $listprofessors, $ratings, $abet, $responses, $tas, true);

  print($outhtml);

  /////////////////////////////////////////////////////////////////////////////

  $url = "seeresults.php/results.csv?nquestionperiodid=$questionperiodid&classid=$classid";

?>
 <!--
 <h4>Results Download</h4>
 <p>Download the raw survey responses as a spreadsheet.</p>
 <blockquote><a href="<?=$url?>"><img src="<?=$wces_path?>media/report/download.gif" width=16 height=16 border=0 alt="results.csv" align=absmiddle></a> <a href="<?=$url?>">results.csv</a></blockquote>
 -->
<?



}



///////////////////////////////////////////////////////////////////////////////

param($question_period_id);
param($nquestionperiodid);
param($questionperiodid);
param($classid);
param($topic_id);

$question_period_id = (int) $question_period_id;
$nquestionperiodid = (int) $nquestionperiodid;
$questionperiodid = (int) $questionperiodid;
$classid = (int) $classid;

$showcsv = $server_url->xpath ? true : false;

if (!$showcsv)
  page_top("Survey Results");



if ($questionperiodid)
  showresults($db,$questionperiodid,$classid);
else if ($nquestionperiodid && $classid)
  nshowresults($db,$nquestionperiodid,$classid,$showcsv);
else if ($question_period_id && $class_id)
  tshowresults($question_period_id,$class_id);
else
  listclasses();

if (!$showcsv)
  page_bottom();

?>

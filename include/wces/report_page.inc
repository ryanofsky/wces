<?

require_once("wces/database.inc");
require_once("wces/oldquestions.inc");
require_once("wces/report_help.inc");
require_once("wces/report_textgrid.inc");

require_once("wbes/component_choice.inc");
require_once("jpgraph/jpgraph.php");

define("QUESTION_PERIODS", 0x01);
define("CLASSES",          0x02);
define("COURSES",          0x04);
define("DEPARTMENTS",      0x08);
define("PROFESSORS",       0x10);
define("CATEGORIES",       0x20);

$report_debug = false;
$report_find_wces_topics = 'wces_topics';

function report_topics_cache()
{
  global $wces;
  wces_connect();
  pg_go("
    INSERT INTO report_topics (topic_id, question_period_id, class_id, category_id, course_id, department_id, user_id)
    SELECT DISTINCT t.topic_id, t.question_period_id, t.class_id, t.category_id, cl.course_id, cl.department_id, e.user_id
    FROM survey_responses AS r
    INNER JOIN wces_topics AS t USING (topic_id)
    INNER JOIN classes AS cl USING (class_id)
    LEFT JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = 3;
  ", $wces, __FILE__, __LINE__);
}

function report_findtopics($tempname, $criteria, $needprof = true)
{
  global $wces, $report_find_wces_topics;

  $where = "";

  if ($criteria[QUESTION_PERIODS])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.question_period_id IN (" . implode($criteria[QUESTION_PERIODS],",") . ")\n    ";
  }
  
  if ($criteria[CATEGORIES])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.category_id IN (" . implode($criteria[CATEGORIES],",") . ")\n    ";
  }

  if ($criteria[CLASSES])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.class_id IN (" . implode($criteria[CLASSES],",") . ")\n    ";
  }

  if ($criteria[COURSES])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.course_id IN (" . implode($criteria[COURSES],",") . ")\n    ";
  }

  if ($criteria[DEPARTMENTS])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.department_id IN (" . implode($criteria[DEPARTMENTS],",") . ")\n    ";
  }

  if ($criteria[PROFESSORS])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "t.user_id IN (" . implode($criteria[PROFESSORS],",") . ")\n    ";
  }

  wces_connect();

  pg_go("
    CREATE TEMPORARY TABLE $tempname AS
    SELECT NULL::INTEGER AS ordinal, t.*
    FROM report_topics AS t
    $where
  ", $wces, __FILE__, __LINE__);
};

function report_findgroups($topicstable, $groupings, $orderings)
{
  global $wces;

  if (isset($groupings[CLASSES]) && $groupings[CLASSES])
    $groupings[COURSES] = true;

  if (isset($groupings[COURSES]) && $groupings[COURSES])
    $groupings[DEPARTMENTS] = true;

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
    COURSES          => "s.code, c.code, cl.section",
    DEPARTMENTS      => "d.code",
    PROFESSORS       => "u.lastname, firstname",
    CATEGORIES       => "t.survey_category_id"
  );

  $orderfields = array(
    QUESTION_PERIODS => "cl.year, cl.semester",
    COURSES          => "s.code, c.code, cl.section",
    DEPARTMENTS      => "d.code",
    PROFESSORS       => "u.lastname, firstname",
    CATEGORIES       => ""
  );

  $gorder = array(
    QUESTION_PERIODS => "MIN(t.question_period_id)",
    COURSES          => "MIN(s.code)",
    DEPARTMENTS      => "MIN(d.code)",
    PROFESSORS       => "MIN(u.lastname)",
    CATEGORIES       => NULL
    //XXX: no category field in rwtopics CATEGORIES       => "MIN(t.survey_category_id)"
    //CATEGORIES       => "MIN(t.survey_category_id)"
  );

  $badorder = false;

  $fargs = $fcond = $fcols = $tcols = $group_s = "";

  $i = 0;
  foreach($groupings as $k => $v)
  if ($v)
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

    if ($tcols) $tcols .= ", ";
    $tcols .= "t.$cols[$k]";
  }
  if (!$group_s)
  {
    pg_go("UPDATE $topicstable SET ordinal = 1", $wces, __FILE__, __LINE__);
    return true;
  }
  else
  {
    $joins = $sjoins = 0;
    $order_s = "";

    foreach($orderings as $o)
    {
      $joins |= $o;
      if ($groupings[$o])
      {
        if ($order_s) $order_s .= ", ";
        $order_s .= $order[$o];
        $group_s .= ", $orderfields[$o]";
        $sjoins |= $o;
        unset($groupings[$o]);
        if (count($groupings) == 0) break;
      }
      else if ($gorder[$o])
      {
        if ($order_s) $order_s .= ", ";
        $badorder = true;
        $order_s .= $gorder[$o];
      }
    }

    $tabname = substr(uniqid('tab'),0,31);
    $funname = substr(uniqid('fun'),0,31);

    pg_go("
      CREATE TEMPORARY TABLE $tabname AS
      SELECT $tcols
      FROM $topicstable AS t" . ($joins & CLASSES || $sjoins & QUESTION_PERIODS ? "
      LEFT JOIN classes AS cl ON cl.class_id = t.class_id" : "") . ($joins & COURSES ? "
      LEFT JOIN courses AS c ON c.course_id = t.course_id
      LEFT JOIN subjects AS s ON s.subject_id = c.course_id" : "") . ($joins & PROFESSORS ? "
      LEFT JOIN users AS u ON u.user_id = t.user_id" : "") . ($joins & DEPARTMENTS ? "
      LEFT JOIN departments AS d ON d.department_id = t.department_id" : "") ."
      GROUP BY $group_s
      ORDER BY $order_s;

      CREATE FUNCTION $funname($fargs) RETURNS INTEGER AS '
        SELECT oid::integer FROM $tabname WHERE $fcond
      ' LANGUAGE 'sql';

      UPDATE $topicstable SET ordinal = $funname($fcols);

      DROP FUNCTION $funname($fargs);
      DROP TABLE $tabname;
    ", $wces, __FILE__, __LINE__);

    return !$badorder;
  }
}

class ReportSection
{
  var $text = "";
  var $html = "";
  function ReportSection($text, $html)
  {
    $this->text = $text;
    $this->html = $html;
  }
};

class Sections
{
  var $branchorder;
  var $sections = array();

  function Sections($branchorder)
  {
    $this->branchorder = $branchorder;
  }

  function addSection($item_id, $text, $html)
  {
    $ordinal = isset($this->branchorder[$item_id]) ? $this->branchorder[$item_id] : 99999999 ;
    $r = new ReportSection($text, $html);
    if (isset($this->sections[$ordinal]))
      $this->sections[$ordinal][] = $r;
    else
      $this->sections[$ordinal] = array($r);
  }

  function flatten(&$pagetext, &$pagehtml)
  {
    $outtext = is_string($pagetext);
    $outhtml = is_string($pagehtml);

    $s = array_keys($this->sections);
    sort($s);

    foreach($s as $k1)
    {
      foreach(array_keys($this->sections[$k1]) as $k2)
      {
        $rs = &$this->sections[$k1][$k2];
        if ($outtext) $pagetext .= $rs->text;
        if ($outhtml) $pagehtml .= $rs->html;
      }
    }
  }
};

//---------------------------------------------------------------------------------------------
// This function produces report "pages" from the results of selectmonster query
//
// The output can be html, plain text or both
// I am generating both types of reports in a single function
// to facilitate situations where both formats of a report are needed at
// once, such as when we want to make multiformat MIME messages.

function report_makepage(&$pacer, &$pagetext, &$pagehtml, &$stats, &$classes, &$professors, &$departments, &$questionperiods, &$textresponses, &$choiceComponents, &$choiceQuestions, &$choiceResponses, &$ords, &$taresponses, $options)
{
  global $wces_path, $TAQUESTIONS, $TAVALUES, $report_debug;

  $outhtml = is_string($pagehtml);
  $outtext = is_string($pagetext);

  $ordinal = (int)$pacer->row['ordinal'];
  $pacer->advance();

  //////////////////////////////////// TOPIC INFO //////////////////////////////////////

  if (!isset($options['topicInfo']) || $options['topicInfo'])
  {
    if ($outhtml)
    {
      $pagehtml .= "<h4>On this page</h4>\n";
      $pagehtml .= "<table border=0 summary=\"Survey Information\">\n";
    }
  
    if ($outtext)
      $pagetext .= "Included on this page\n\n";
  
    while($questionperiods->row && $questionperiods->row['ordinal'] == $ordinal)
    {
      if ($outhtml) $pagehtml .= "<tr><td>Question Period:</td><td><b>{$questionperiods->row['displayname']}</b></td></tr>\n";
      if ($outtext) $pagetext .= " - Question Period: {$questionperiods->row['displayname']}\n";
      $questionperiods->advance();
    }
  
    while(isset($departments->row) && $departments->row['ordinal'] == $ordinal)
    {
      extract($departments->row);
      if ($outhtml) $pagehtml .= "<tr><td>Department:</td><td><b>$name ($code)</b></td></tr>\n";
      if ($outtext) $pagetext .= " - Department: $name ($code)\n";
      $departments->advance();
    }
  
    if ($professors)
    {
      while(isset($professors->row) && $professors->row['ordinal'] == $ordinal)
      {
        extract($professors->row);
        $profname = ($firstname || $lastname) ? "$firstname $lastname" : "Unknown";
        if ($outhtml) $pagehtml .= "<tr><td>Professor:</td><td><b>" . ($user_id ? "<a href=\"${wces_path}administrators/info.php?user_id=$user_id\">$profname</a>" : "<i>$profname</i>") . "</b></td></tr>\n";
        if ($outtext) $pagetext .= " - Professor: $profname\n";
        $professors->advance();
      }
  
      while(isset($classes->row) && $classes->row['ordinal'] == $ordinal)
      {
        extract($classes->row);
        if ($outhtml) $pagehtml .= "<tr><td>Class:</td><td><b><a href=\"${wces_path}administrators/info.php?class_id=$class_id\">$scode $ccode $name Section $section</a></b></td></tr>\n";
        if ($outtext) $pagetext .= " - Class: $scode $ccode $name Section $section\n";
        $classes->advance();
      }
    }
    else
    {
      while(isset($classes->row) && $classes->row['ordinal'] == $ordinal)
      {
        extract($classes->row);
        $pname = $firstname || $lastname ? " - Professor $firstname $lastname" : "";
        if ($outhtml) $pagehtml .= "<tr><td>Class:</td><td><b>$scode $ccode $name Section $section$pname</b></td></tr>\n";
        if ($outtext) $pagetext .= " - Class: $scode $ccode $name Section $section$pname\n";
        $classes->advance();
      }
    }
    if ($outhtml) $pagehtml .= "</table>\n";
  }

  ////////////////////////////////// COMPONENT ORDER ///////////////////////////////////

  $branchorder = array();
  while(isset($ords->row) && $ords->row['ordinal'] == $ordinal)
  {
    extract($ords->row);
    $branchorder[$item_id] = $iordinal;
    $ords->advance();
  }

  //////////////////////////////// RESPONSE STATISTICS /////////////////////////////////
  if (!isset($options['responseStatistics']) || $options['responseStatistics'])
  {
    assert($ordinal == (int)$stats->row['ordinal']);
    $students = (int)$stats->row['students'];
    $responses = (int)$stats->row['responses'];
    $stats->advance();
    
    if ($outtext)
    {
      $pagetext .= "\nResponse Statistics\n\n";
      $pagetext .= " - Total Surveys: $students\n";
      $pagetext .= " - Evaluated Surveys: $responses\n\n";
    }
    
    if ($outhtml)
    {
      $pagehtml .= "<h4>Response Statistics</h4>";
      $pagehtml .= "<table border=0 summary=\"number of responses\">\n";
      $pagehtml .= "<tr><td>Total Surveys:</td><td><b>$students</b></td></tr>\n";
      $pagehtml .= "<tr><td>Evaluated Surveys:</td><td><b>$responses</b></td></tr>\n";
      $pagehtml .= "</table>\n";
      if (!isset($options['pies']) || $options['pies'])
        $pagehtml .= '<img src="' . "${wces_path}media/graphs/susagegraph.php?blank=" . ($students-$responses) . "&filled=$responses\" width=200 height=200><img src=\"${wces_path}media/graphs/susagelegend.gif\" width=147 height=31>";
      $pagehtml .= "<h4>Responses</h4>\n";
    }
  }

  ///////////////////////////////// COURSE RATINGS /////////////////////////////////////

  $sections = new Sections($branchorder);

  while(isset($choiceResponses->row) && $choiceResponses->row['ordinal'] == $ordinal)
  {
    $ctext = '';

    if ($choiceResponses->split[2])
    {
      $is_single = $choiceResponses->peek(0) || $choiceResponses->peek(1)
        || $choiceResponses->peek(2);      
      $ag = isset($options['alwaysGraph']) && $options['alwaysGraph'];
      $sp = $choiceResponses->row['qrevision_id'] == 221;
      $show_dist = $ag || ($is_single && !$sp);
      
      $sectionhtml = $sectiontext = "";
      $s = '; page-break-before: always';
      $sectionhtml .= "<div style=\"page-break-inside: avoid$s\">\n";

      $citem_id = $choiceResponses->row['citem_id'];
      $crevision_id = $choiceResponses->row['crevision_id'];
      
      assert($choiceComponents->row['ordinal'] == $ordinal);
      assert($choiceComponents->row['citem_id'] == $citem_id);      
      assert($choiceComponents->row['crevision_id'] == $crevision_id);      
      $choices = pg_explode($choiceComponents->row['choices']);
      $ctext = $choiceComponents->row['ctext'];
      $flags = $choiceComponents->row['flags'];
      $first_number = $choiceComponents->row['first_number'];
      $last_number = $choiceComponents->row['last_number'];
      $other_choice = $choiceComponents->row['other_choice'];
      
      // set variables which will be used to interpret data and print headers:
      //
      // arrays indexed by choice keys: 
      //   $headings - choice text
      //   $values - choice numeric values
      //   $vheadings - choice text with numeric values in parentheses if applicable

      $values = array(); // 

      if ($flags & FLAG_ISNUMERIC)
      {
        $showstats = true; // show avg, std dev, mode columns
        $d = $first_number < $last_number ? 1 : -1;
        $r = abs($last_number - $first_number);
        for($i=0; $i <= $r; ++$i)
          $values[] = $first_number + $d * $i;
        $headings = $values;
      }
      else
      {
        $showstats = is_numeric($first_number) && is_numeric($last_number) && $first_number != $last_number;
        $headings = $choices;

        foreach($choices as $ci => $ct)
        {
          $values[] = $showstats ? $first_number + ($ci
            / (count($choices)-1)) * ($last_number - $first_number) : false;
        }
      }

      if ($other_choice)
      {
        $headings[] = $other_choice;
        $values[] = false;
      }

      if ($flags & FLAG_NACHOICE)
      {
        $headings[] = "N/A";
        $values[] = false;
      }

      $vheadings = array();
      foreach ($headings as $hk => $hv)
      {
        $str = $hv;
        if ($showstats && $values[$hk] !== false)
          $str .= sprintf(" (%.1f)",$values[$hk]);
        $vheadings[$hk] = $str;
      }

      // information about the question set is now available, so
      // begin doing html / text formatting

      // do html header
      if ($outhtml)
      {
        if ($report_debug) 
        {
          $sectionhtml .= "<h5><font color=red><i>Component item_id = $citem_id, revision_id = $crevision_id</i></font></h5>\n";        
        }

        if ($ctext) $sectionhtml .= "$ctext<br>";
        if (!$show_dist)
        {
          if ($flags & FLAG_ISNUMERIC)
            $sectionhtml .= "($first_number = $choices[0], $last_number = $choices[1])<br>";

          $sectionhtml .= "<table border=1 cellspacing=0 cellpadding=2 summary=\"multiple choice response distributions\">\n";
          $sectionhtml .= "<thead style=\"page-break-inside: avoid\">\n";
          $sectionhtml .= "<tr>\n";
          $sectionhtml .= "<td>&nbsp;</td>\n";

          if ($flags & FLAG_ISNUMERIC)
          {
            foreach ($headings as $hk => $hv)
              $sectionhtml .= "  <td><b>$hv</b></td>\n";
          }
          else
          {
            foreach ($vheadings as $hk => $hv)
              $sectionhtml .= "  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$hv</b></div></td>\n";
          }

          if ($showstats)
            $sectionhtml .= "  <td><b>Avg</b></td>\n  <td><b>Mode</b></td>\n  <td><b>SD</b></td>\n";

          $sectionhtml .= "</tr>\n";
        }
      }

      // do text header
      if ($outtext)
      {
        if ($flags & FLAG_ISNUMERIC)
          $sectiontext .= "($first_number = $choices[0], $last_number = $choices[1])\n";
        else
        {
          $key = "";
          foreach ($choices as $ci => $ct)
          {
             // don't print numeric value for columns that don't have, like N/A
            if ($showstats && $values[$ci] === false) continue;

            if ($key) $key .= ", ";
            $key .= ($showstats ? $values[$ci] : $ci) . " = $ct";
          }
          $sectiontext .= "($key)\n";
        }
        $key = "";

        $columnWidths = array(null);
        $textrows = array();
        $columnFormats = array(0);

        $header = array("Question Text");

        foreach ($headings as $ci => $ct)
        {
          $textformat[] = "center";
          if ($showstats)
          {
            if ($values[$ci] === false)
              $label = $ct;
            else
              $label = sprintf("%.1f", $values[$ci]);
          }
          else
            $label = $ci;
          $header[] = $label;
          $columnWidths[] = 4;
          $columnFormats[] = ALIGN_CENTER;
        }

        if ($showstats)
        {
          array_push($columnFormats, ALIGN_CENTER, ALIGN_CENTER, ALIGN_CENTER);
          array_push($columnWidths, 4, 4, 4);
          array_push($header, "Avg", "Mode", "SD");
        }
        $textrows[] = $header;
      }
    } // if ($choicesResponses->split[2])

    $qitem_id = $choiceResponses->row['qitem_id'];
    $qrevision_id = $choiceResponses->row['qrevision_id'];
    $dist = pg_explode($choiceResponses->row['dist']);
    $dist = array_pad($dist, count($values), 0);

    assert($choiceQuestions->row['ordinal'] == $ordinal);
    assert($choiceQuestions->row['citem_id'] == $citem_id);    
    assert($choiceQuestions->row['crevision_id'] == $crevision_id);      
    assert($choiceQuestions->row['qitem_id'] == $qitem_id);        
    assert($choiceQuestions->row['qrevision_id'] == $qrevision_id);  

    $qtext = $choiceQuestions->row['qtext'];

    if ($report_debug)
    {
      $qtext = "<font color=red><i>item_id = $qitem_id, revision_id = $qrevision_id</i></font> $qtext";
    }

    // done with header, format individual questions within the set

    if ($showstats)
    {
      $s = array();
      foreach($values as $vi => $vk)
        if ($vk !== false) $s[$vk] = $dist[$vi];

      $avg = report_avg($s);
      $mode = report_mode($s);
      $sd = report_sd($s, $avg);
      $md = report_median($s);
    }

    if ($outhtml)
    {
      if ($show_dist)
      {
        $sectionhtml .= "<h5>$qtext</h5>";
        $sectionhtml .= "<p>";
        $sectionhtml .= MakeGraph($headings, $dist);
        $sectionhtml .= "</p>\n<table>\n";

        foreach($vheadings as $k => $v)
          $sectionhtml .= "<tr><td><b>$v</b>:</td><td>$dist[$k]</td></tr>\n";

        if ($showstats)
        {
          $sectionhtml .= sprintf("<tr><td><b><i>Average</i></b>:</td><td>%.1f</td></tr>\n"
                            . "<tr><td><b><i>Median</i></b>:</td><td>%.1f</td></tr>\n"
                            . "<tr><td><b><i>Mode</i></b>:</td><td>%.1f</td></tr>\n"
                            . "<tr><td><b><i>Standard Deviation</i></b>:</td><td>%.1f</td></tr>\n"
                            ,$avg, $md, $mode, $sd);
        }
        $sectionhtml .= "</table>\n";
      }
      else
      {
        $sectionhtml .= "<tr>\n  <td>$qtext</td>\n";
        foreach($values as $vi => $vk)
          $sectionhtml .= "  <td>$dist[$vi]</td>\n";
        if ($showstats)
          $sectionhtml .= sprintf("  <td>%.1f</td>\n  <td>%.1f</td>\n  <td>%.1f</td>\n", $avg, $mode, $sd);
        $sectionhtml .= "</tr>\n";
      }
    }

    if ($outtext)
    {
      $row = array($qtext);
      foreach($values as $vi => $vk)
        $row[] = $dist[$vi];
      if ($showstats)
        array_push($row, sprintf("%.1f",$avg), sprintf("%.1f",$mode), sprintf("%.1f",$sd));
      $textrows[] = $row;
    }

    $choiceResponses->advance();
    $choiceQuestions->advance();

    if ($choiceResponses->split[2])
    {
      if ($outhtml && !$show_dist) $sectionhtml .= "</table>\n";
      if ($outhtml) $sectionhtml .= "</div>\n<br>\n";
      if ($outtext)
      {
        $sectiontext .= TextTable($textrows, $columnFormats, 76, $columnWidths);
        $sectiontext .= "\n\n";
      }
      $sections->addSection($citem_id, $sectiontext, $sectionhtml);
      $choiceComponents->advance();
    }
  } // while($choices->row ...)


  ///////////////////////////////// TEXT RESPONSES /////////////////////////////////////

  while(isset($textresponses->row) && $textresponses->row['ordinal'] == $ordinal)
  {
    extract($textresponses->row);

    if ($textresponses->split[1])
    {
      if ($report_debug)
      {
        $ctext = "<font color=red><i>item_id = $item_id, revision_id = $revision_id</i></font> $ctext";
      }
      $sectionhtml = $sectiontext = "";
      if ($outhtml) $sectionhtml .= "<h5>$ctext</h5>\n<ul>\n";
      if ($outtext) $sectiontext .= "$ctext\n\n";
    }

    if ($outhtml) $sectionhtml .= "  <li>" . nl2br($rtext) . "</li>";
    if ($outtext) $sectiontext  .= " - " . str_replace("\n","\n   ",wordwrap("$rtext",73)) . "\n";

    $textresponses->advance();

    if ($textresponses->split[1])
    {
      if ($outhtml) $sectionhtml .= "\n</ul>";
      if ($outtext) $sectiontext .= "\n\n";
      $sections->addSection($item_id, $sectiontext, $sectionhtml);
    }
  } // while($textresponses->row ...)

  $sections->flatten($pagetext, $pagehtml);

  ///////////////////////////////// TA RATINGS /////////////////////////////////////

/*MAGIC*/
$TAQUESTIONS = array
(
  "overall" => "Overall Quality",
  "knowledgeability" => "Knowledgeability",
  "approachability" => "Approachability",
  "availability" => "Availability",
  "communication" => "Communication"
);
/*MAGIC*/
$TAVALUES = array
(
  "5" => "excellent",
  "4" => "very good",
  "3" => "satisfactory",
  "2" => "poor",
  "1" => "disastrous",
  "0" => "n/a"
);

  while (isset($taresponses->row) && $taresponses->row['ordinal'] == $ordinal)
  {
    extract($taresponses->row);

    if ($taresponses->split[0])
    {
      if ($outhtml)
      {
        $pagehtml .= "<h4>TA Ratings</h4>\n";

        $first = true;
        foreach($TAVALUES as $k => $t)
        {
          if ($first) $first = false; else $pagehtml .= ", ";
          $pagehtml .= "$k = $t";
        }

        $pagehtml .= "<br><table border=1 cellspacing=0 cellpadding=2>\n<thead style=\"page-break-inside: avoid\">\n<tr>\n  <td>Name</td>\n";

        foreach($TAQUESTIONS as $question)
          $pagehtml .= "  <td><div style=\"writing-mode:tb-rl\"><b>" . str_replace(" ","&nbsp;",$question) . "</b></div></td>\n";
        $pagehtml .= "<td><b>Comments</b></td><td><b>Student #</b></td></tr></thead>\n";
        $avg = array();
        $num = array();
      }
      if ($outtext)
        $pagetext .= "TA Ratings\n\n";
    }

    if ($outhtml)
    {
      if ($user_id)
        $name = "$firstname $lastname";
      else if (!$name)
        $name = "<i>All TAs</i>";
      $pagehtml .= "<tr>\n  <td>$name</td>";
      foreach($TAQUESTIONS as $field => $question)
      {
        if ($$field > 0)
        {
          $avg[$field] = (isset($avg[$field]) ? $avg[$field] : 0) + $$field;
          if (isset($num[$field])) ++$num[$field]; else $num[$field] = 1;
        }
        $pagehtml .= "  <td>" . $$field . "</td>\n";
      }
      $pagehtml .= "  <td>" . ($comments ? $comments : "&nbsp;") . "</td><td>" . $parent . "</td></tr>\n";
    }

    if ($outtext)
    {
      if ($name) $name = "\"$name\""; else $name = "All TAs";
      $pagetext .= " - TA Name: $name\n";
      foreach($TAQUESTIONS as $field => $question)
        $pagetext .= " - $question: " . $TAVALUES[$$field] . "\n";
      $pagetext .= " - " . str_replace("\n","\n   ",wordwrap("Comments: \"" . $comments,73)) . "\"\n";
      $pagetext .= " - Reviewed by: " . $response_id . "\n\n";
    }

    $taresponses->advance();

    if ($taresponses->split[0])
    {
      if ($outhtml)
      {
        $pagehtml .= "<tr>\n  <td><b>AVERAGE</b></td>\n";
        foreach($TAQUESTIONS as $field => $question)
          if (isset($avg[$field]) && isset($num[$field]))
            $pagehtml .= "  <td>" . round($avg[$field]/$num[$field], 2). "</td>\n";
          else
            $pagehtml .= "<td>0</td>";

        $pagehtml .= "  <td>&nbsp;</td>\n  <td>---</td>\n  </tr>\n</table>\n";
      }
      if ($outtext) $pagetext .= "\n\n";
    }
  } // while($taresponses->row ...)
};

?>

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

  if ($criteria[CLASSES])
  {
    $where .= $where ? "  AND " : "WHERE ";
    $where .= "cl.class_id IN (" . implode($criteria[CLASSES],",") . ")\n    ";
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
  wces_connect();

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
    pg_query("UPDATE $topicstable SET ordinal = 1", $wces, __FILE__, __LINE__);
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
    
    $tempname = substr(md5(uniqid(rand(),1)),0,20);
    
    pg_query("
      CREATE TEMPORARY TABLE tab$tempname AS
      SELECT $tcols
      FROM $topicstable AS t" . ($joins & CLASSES || $sjoins & QUESTION_PERIODS ? "
      LEFT JOIN classes AS cl ON cl.class_id = t.class_id" : "") . ($joins & COURSES ? "
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
  
  function addSection($branch_id, $text, $html)
  {
    $ordinal = isset($this->branchorder[$branch_id]) ? $this->branchorder[$branch_id] : 99999999 ;
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

// possible options:
// - "pies"

function report_makepage(&$pagetext, &$pagehtml, &$stats, &$classes, &$professors, &$departments, &$questionperiods, &$textresponses, &$choices, &$cords, &$ords, &$taresponses)
{
  global $wces_path, $TAQUESTIONS, $TAVALUES;

  $outhtml = is_string($pagehtml);
  $outtext = is_string($pagetext);

  $ordinal = (int)$stats->row['ordinal'];
  $students = (int)$stats->row['students'];
  $responses = (int)$stats->row['responses'];
  $stats->advance();

  //////////////////////////////////// PAGE HEADER /////////////////////////////////////

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
      if ($outtext) $pagetext .= " - Class: $scode $ccode $cname Section $section$pname\n";
      $classes->advance();
    }
  }

  if ($outhtml) $pagehtml .= "</table>\n";

  //////////////////////////////// RESPONSE STATISTICS /////////////////////////////////

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
    $pagehtml .= '<img src="' . "${wces_path}media/graphs/susagegraph.php?blank=" . ($students-$responses) . "&filled=$responses\" width=200 height=200><img src=\"${wces_path}media/graphs/susagelegend.gif\" width=147 height=31>";
  }

  ///////////////////////////////// COURSE RATINGS /////////////////////////////////////

  $branchorder = array();
  while(isset($ords->row) && $ords->row['ordinal'] == $ordinal)
  {
    extract($ords->row);
    $branchorder[$branch_id] = $iordinal;
    $ords->advance();
  }
  $sections = new Sections($branchorder);

  if (isset($choices->row) && $choices->row['ordinal'] == $ordinal)
    $is_single_question = $choices->peek(0) || $choices->peek(1);
  
  while(isset($choices->row) && $choices->row['ordinal'] == $ordinal)
  {
    extract($choices->row);

    if ($choices->split[1])
    {

      $cbranchorder = array();
      while(isset($cords->row) && $cords->row['ordinal'] == $ordinal && $cords->row['crevision_id'] == $crevision_id)
      {
        $cbranchorder[$cords->row['branch_id']] = $cords->row['iordinal'];
        $cords->advance();
      }
      $csections = new Sections($cbranchorder);

      $sectionhtml = $sectiontext = "";
      
      // set variables which will be used to interpret data and print headers:
      // $headings, $cchoices, $values, $showstats, $textformat, $textrows

      $cchoices = pg_explode($cchoices);

      $values = array(); // indexed by choice keys, holds numeric values of choices

      if ($flags & FLAG_ISNUMERIC)
      {
        $showstats = true; // show avg, std dev, mode columns
        $d = $first_number < $last_number ? 1 : -1;
        $r = abs($last_number - $first_number);
        for($i=0; $i <= $r; ++$i)
          $values[$i] = $first_number + $d * $i;
        $headings = $values;
      }
      else
      {
        $showstats = is_numeric($first_number) && is_numeric($last_number) && $first_number != $last_number;
        $headings = $cchoices;

        foreach($cchoices as $ci => $ct)
        {
          $values[$ci] = $showstats ? $values[$ci] = $first_number + ($ci
            / (count($cchoices)-1)) * ($last_number - $first_number) : false;
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
        $values[] = NULL;
      }

      $vheadings = array();
      foreach ($headings as $hk => $hv)
      {
        $str = $hv;
        if ($showstats && isset($values[$hk]))
          $str .= sprintf(" (%.1f)",$values[$hk]);
        $vheadings[$hk] = $str;
      }

      // information about the question set is now readily available, so
      // begin doing html / text formatting
      
      // do html header
      if ($outhtml)
      {
        if ($ctext) $sectionhtml .= "<p><strong>$ctext</strong></p>\n";
        
        if (!$is_single_question)
        {
          if ($flags & FLAG_ISNUMERIC)
            $sectionhtml .= "($first_number = $cchoices[0], $last_number = $cchoices[1])<br>";
  
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
              $sectionhtml .= "  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$str</b></div></td>\n";
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
          $sectiontext .= "($first_number = $cchoices[0], $last_number = $cchoices[1])\n";
        else
        {
          $key = "";
          foreach ($cchoices as $ci => $ct)
          {
             // don't print numeric value for columns that don't have, like N/A
            if ($showstats && $values[$ci] === false) continue;

            if ($key) $key .= ", ";
            $key .= ($showstats ? $values[$ci] : $ci) . " = $ct";
          }
          $sectiontext .= "($key)\n";
        }
        $key = "";

        $textformat = array(25);
        $textrows = array();

        $header = array("Question Text");

        foreach ($cchoices as $ci => $ct)
        {
          $textformat[] = "center";
          if ($showstats)
          {
            if ($values[$ci] === false)
              $header[] = $ct;
            else
              $header[] = sprintf("%.1f", $values[$ci]);
          }
          else
            $header[] = $ci;
        }

        if ($showstats)
        {
          array_push($textformat, "center", "center", "center");
          array_push($header, "Avg", "Mode", "SD");
          $textrows[] = $header;
        }
      } // if (!$is_single_question)
    } // if ($choices->split[1])
    
    // done with header, format individual questions within the set

    $csectiontext = $csectionhtml = ""; 

    // get the numbers
    $sums = array_pad(pg_explode($dist), count($values), 0);
    
    $dist = false;
    if ($showstats)
    {
      foreach($values as $vi => $vk)
        $dist[$vk] = $sums[$vi];

      // don't include the N/A in the computation
      if ($flags & FLAG_NACHOICE)
        unset($dist[end(array_keys($dist))]);

      $avg = report_avg($dist);
      $mode = report_mode($dist);
      $sd = report_sd($dist, $avg);
      $md = report_median($dist);
    }

    if ($outhtml)
    {
      if ($is_single_question)
      {
        $csectionhtml .= "<h5>$qtext</h5>";
        $csectionhtml .= "<p>";
        $csectionhtml .= MakeGraph($headings, $sums);
        $csectionhtml .= "</p>\n<table>\n";

        foreach($vheadings as $k => $v)
          $csectionhtml .= "<tr><td><b>$v</b>:</td><td>$sums[$k]</td></tr>\n";

        if ($showstats)
        {
          $csectionhtml .= sprintf("<tr><td><b><i>Average</i></b>:</td><td>%.1f</td></tr>\n"
                            . "<tr><td><b><i>Median</i></b>:</td><td>%.1f</td></tr>\n"
                            . "<tr><td><b><i>Mode</i></b>:</td><td>%.1f</td></tr>\n"
                            . "<tr><td><b><i>Standard Deviation</i></b>:</td><td>%.1f</td></tr>\n"
                            ,$avg, $md, $mode, $sd);
        }
        $csectionhtml .= "</table>\n";
      }
      else
      {
        $csectionhtml .= "<tr>\n  <td>$qtext</td>\n";
        foreach($values as $vi => $vk)
          $csectionhtml .= "  <td>$sums[$vi]</td>\n";
        if ($showstats)
          $csectionhtml .= sprintf("  <td>%.1f</td>\n  <td>%.1f</td>\n  <td>%.1f</td>\n", $avg, $mode, $sd);
        $csectionhtml .= "</tr>\n";
      }
    }
    
    if ($outtext)
    {
      $row = array($qtext);
      foreach($values as $vi => $vk)
        $row[] = $sums[$vi];
      if ($showstats)
        array_push($row, sprintf("%.1f",$avg), sprintf("%.1f",$mode), sprintf("%.1f",$sd));
      $textrows[] = $row;
    }
  
    
    $csections->addSection($qbranch_id, $csectiontext, $csectionhtml);
    
    $choices->advance();

    if ($choices->split[1])
    {
      if ($outhtml && !$is_single_question) $sectionhtml .= "</table>\n";
      if ($outtext)
      {
        $sectiontext .= texttable($textrows,$textformat);
        $sectiontext .= "\n\n";
      }
      $csections->flatten($sectiontext, $sectionhtml);
      unset($csections);
      $sections->addSection($cbranch_id, $sectiontext, $sectionhtml);
    }
  } // while($choices->row ...)


  ///////////////////////////////// TEXT RESPONSES /////////////////////////////////////

  while(isset($textresponses->row) && $textresponses->row['ordinal'] == $ordinal)
  {
    extract($textresponses->row);

    if ($textresponses->split[1])
    {
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
      $sections->addSection($branch_id, $sectiontext, $sectionhtml);
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

  while(isset($taresponses->row) && $taresponses->row['ordinal'] == $ordinal)
  {
    extract($taresponses->row);

    if ($taresponses->split[1])
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
      if ($outtext) $pagetext .= "TA Ratings\n\n";
    }

    if ($outhtml)
    {
      if (!$name) $name = "<i>All TAs</i>";
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
      $pagehtml .= "  <td>" . ($comments ? $comments : "&nbsp;") . "</td><td>" . $response_id . "</td></tr>\n";
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

    if ($taresponses->split[1])
    {
      if ($outhtml)
      {
        $pagehtml .= "<tr>\n  <td><b>AVERAGE</b></td>\n";
        foreach($TAQUESTIONS as $field => $question)
          $pagehtml .= "  <td>" . round($avg[$field]/$num[$field], 2). "</td>\n";
        $pagehtml .= "  <td>&nbsp;</td>\n  <td>---</td>\n  </tr>\n</table>\n";
      }
      if ($outtext) $pagetext .= "\n\n";
    }
  } // while($taresponses->row ...)
};

?>

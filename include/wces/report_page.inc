<?

require_once("wces/database.inc");
require_once("wces/oldquestions.inc");
require_once("wces/report_help.inc");
require_once("wces/report_textgrid.inc");

require_once("wbes/component_choice.inc");

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

//---------------------------------------------------------------------------------------------
// This function produces report "pages" from the results of selectmonster query
//
// The output can be html, plain text or both
// I am generating both types of reports in a single function
// to facilitate situations where both formats of a report are needed at
// once, such as when we want to make multiformat MIME messages.

// possible options:
// - "pies"

function report_makepage(&$pagetext, &$pagehtml, &$stats, &$classes, &$professors, &$departments, &$questionperiods, &$textresponses, &$choices)
{
  global $wces_path, $TAQUESTIONS, $TAVALUES, $ABETQUESTIONS;

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
    $pagehtml .= "<table border=0>\n";
  }

  if ($outtext)
    $pagetext .= "Included on this page:\n\n";

  while($questionperiods->row && $questionperiods->row['ordinal'] == $ordinal)
  {
    if ($outhtml) $pagehtml .= "<tr><td>Question Period:</td><td><b>{$questionperiods->row['displayname']}</b></td></tr>\n";
    if ($outtext) $pagetext .= " - Question Period: {$questionperiods->row['displayname']}\n";
    $questionperiods->advance();
  }

  while($departments->row && $departments->row['ordinal'] == $ordinal)
  {
    extract($departments->row);
    if ($outhtml) $pagehtml .= "<tr><td>Department:</td><td><b>$name ($code)</b></td></tr>\n";
    if ($outtext) $pagetext .= " - Department: $name ($code)\n";
    $departments->advance();
  }

  if ($professors)
  {
    while($professors->row && $professors->row['ordinal'] == $ordinal)
    {
      extract($professors->row);
      $profname = ($firstname || $lastname) ? "$firstname $lastname" : "Unknown";
      if ($outhtml) $pagehtml .= "<tr><td>Professor:</td><td><b>" . ($professorid ? "<a href=\"${wces_path}administrators/info.php?user_id=$user_id\">$profname</a>" : "<i>$profname</i>") . "</b></td></tr>\n";
      if ($outtext) $pagetext .= " - Professor: $profname\n";
      $professors->advance();
    }

    while($classes->row && $classes->row['ordinal'] == $ordinal)
    {
      extract($classes->row);
      if ($outhtml) $pagehtml .= "<tr><td>Class:</td><td><b><a href=\"${wces_path}administrators/info.php?class_id=$class_id\">$scode $ccode $name Section $section</a></b></td></tr>\n";
      if ($outtext) $pagetext .= " - Class: $scode $ccode $name Section $section\n";
      $classes->advance();
    }
  }
  else
  {
    while($classes->row && $classes->row['ordinal'] == $ordinal)
    {
      extract($classes->row);
      $pname = $firstname || $lastname ? " - Professor $firstname $lastname" : "";
      if ($outhtml) $pagehtml .= "<tr><td>Class:</td><td><b>$scode $ccode $cname Section $section$pname</b></td></tr>\n";
      if ($outtext) $pagetext .= " - Class: $scode $ccode $cname Section $section$pname\n";
      $classes->advance();
    }
  }

  if ($outhtml) $pagehtml .= "</table>\n";

  //////////////////////////////// RESPONSE STATISTICS /////////////////////////////////

  if ($pagetext)
  {
    $pagetext .= "\nResponse Statistics:\n\n";
    $pagetext .= " - Total Students: $students\n";
    $pagetext .= " - Students Evaluated: $responses\n\n";
  }

  if ($outhtml)
  {
    $pagehtml .= "<h4>Response Statistics</h4>";
    $pagehtml .= "<table border=0>\n";
    $pagehtml .= "<tr><td>Total Students:</td><td><b>$students</b></td></tr>\n";
    $pagehtml .= "<tr><td>Students Evaluated:</td><td><b>$responses</b></td></tr>\n";
    $pagehtml .= "</table>\n";
    $pagehtml .= '<img src="' . "${wces_path}media/graphs/susagegraph.php?blank=" . ($students-$responses) . "&filled=$responses\" width=200 height=200><img src=\"${wces_path}media/graphs/susagelegend.gif\" width=147 height=31>";
  }

  ///////////////////////////////// COURSE RATINGS /////////////////////////////////////

  while($choices->row && $choices->row['ordinal'] == $ordinal)
  {
    extract($choices->row);

    if ($choices->split[1])
    {
      // set variables which will be used to interpret data and print headers:
      // $headings, $choices, $values, $showstats, $textformat, $textrows

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
        $showstats = is_numeric($first_number) && is_numeric($last_number);
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
        $values[] = false;
      }

      // do html header
      if ($outhtml)
      {
        if ($flags & FLAG_ISNUMERIC)
          $pagehtml .= "($first_number = $cchoices[0], $last_number = $cchoices[1])<br>";

        $pagehtml .= "<table border=1 cellspacing=0 cellpadding=2>\n";
        $pagehtml .= "<thead style=\"page-break-inside: avoid\">\n";
        $pagehtml .= "<tr>\n";
        $pagehtml .= "<td>&nbsp;</td>\n";

        if ($flags & FLAG_ISNUMERIC)
        {
          foreach ($headings as $hk => $hv)
            $pagehtml .= "  <td><b>$hv</b></td>\n";
        }
        else
        {
          foreach ($headings as $hk => $hv)
          {
            $str = $hv;
            if ($showstats)
              $str .= sprintf(" (%.1f)",$values[$hk]);
            $pagehtml .= "  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$str</b></div></td>\n";
          }
        }

        if ($showstats)
          $pagehtml .= "  <td><b>Avg</b></td>\n  <td><b>Mode</b></td>\n  <td><b>SD</b></td>\n";

        $pagehtml .= "</tr>\n";
      }

      // do text header
      if ($outtext)
      {
        if ($flags & FLAG_ISNUMERIC)
          $pagetext .= "($first_number = $cchoices[0], $last_number = $cchoices[1])\n";
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
          $pagetext .= "($key)";
        }
        $key = "";

        $textformat = array(30);
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
      }
    } // if ($choices->split[0])

    // done with header, print a row

    if ($outhtml) $pagehtml .= "<tr>\n  <td>$qtext</td>\n";
    if ($outtext) $row = array($qtext);

    $sums = array_pad(pg_explode($dist),count($choices),0);
    $dist = false;
    foreach($values as $vi => $vk)
    {
      if ($showstats) $dist[$vk] = $sums[$vi];
      if ($outhtml) $pagehtml .= "  <td>$sums[$vi]</td>\n";
      if ($outtext) $row[] = $sums[$vi];
    }

    if ($showstats)
    {
      $avg = report_avg($dist);
      $mode = report_mode($dist);
      $sd = report_sd($dist, $avg);
      if ($outhtml) $pagehtml .= sprintf("  <td>%.1f</td>\n  <td>%.1f</td>\n  <td>%.1f</td>\n",$avg, $mode, $sd);
      if ($outtext) array_push($row, sprintf("%.1f",$avg), sprintf("%.1f",$mode), sprintf("%.1f",$sd));
    }

    if ($outhtml) $pagehtml .= "</tr>\n";
    if ($outtext) $textrows[] = $row;

    $choices->advance();

    if ($choices->split[1])
    {
      if ($outhtml) $pagehtml .= "</table>\n";
      if ($outtext)
      {
        $pagetext .= texttable($textrows,$textformat);
        $pagetext .= "\n\n";
      }
    }
  } // while($choices->row ...)


  ///////////////////////////////// TEXT RESPONSES /////////////////////////////////////

  while($textresponses->row && $textresponses->row['ordinal'] == $ordinal)
  {
    extract($textresponses->row);

    if ($textresponses->split[1])
    {
      if ($outhtml) $pagehtml .= "<h5>$ctext</h5>\n<ul>\n";
      if ($outtext) $pagetext .= "$ctext\n\n";
    }

    if ($outhtml) $pagehtml .= "  <li>" . nl2br($rtext) . "</li>";
    if ($outtext) $pagetext  .= " - " . str_replace("\n","\n   ",wordwrap("$rtext",73)) . "\n";

    $textresponses->advance();

    if ($textresponses->split[1])
    {
      if ($outhtml) $pagehtml .= "\n</ul>";
      if ($outtext) $pagetext .= "\n\n";
    }
  } // while($textresponses->row ...)
};

?>
<?

require_once("wces/report_page.inc");

$line = "---------------------------------------------------------------------------\n";

function makereport(&$wizard,$stdout,&$html,&$text, $options = array())
{
  global $wces_path, $wces, $line;

  $outhtml = is_string($html);
  $outtext = is_string($text);

  $groups = Array
  (
    CLASSES => $wizard->groupClasses->checked,
    COURSES => $wizard->groupCourses->checked,
    PROFESSORS => $wizard->groupProfessors->checked,
    DEPARTMENTS => $wizard->groupDepartments->checked,
    QUESTION_PERIODS => $wizard->groupQuestionPeriods->checked
  );

  $header = true;
  $ratings = true;
  $listclasses = ($wizard->groupClasses->checked || $wizard->output->bigHead->checked) ? true : false;
  $listprofessors = ($wizard->groupProfessors->checked || $wizard->output->bigHead->checked) ? true : false;

  $abet = true;
  $responses = $wizard->output->includeText->checked ? true : false;
  $tas = $wizard->output->includeTa->checked ? true : false;
  $sort = $wizard->order->keys;

  report_findtopics("rwtopics", $wizard->criteria);
  report_findgroups("rwtopics", $groups, $sort);

  $pages = pg_result(pg_go("SELECT COUNT(DISTINCT ordinal) FROM rwtopics", $wces, __FILE__,__LINE__),0,0);

  $displayoptions = array("pies" => $wizard->output->includePies->checked);

  $page = 0;
  if ($stdout || $outhtml)
  {
    $head = "<font size=1>[ $page / $pages ]</font>\n";
    $head .= "<h4>Report Information</h4>\n";
    $head .= "<table border=0><tr><td>Administrator ID: </td><td><b>" . login_getuni() . "</b></td></tr><tr><td>Date:</td><td><b>" . date("l, F j, Y") . "</b></td></tr><tr><td>Time:</td><td><b>" . date("g:i:s A") . "</b></td></tr></table>";
    $head .= "<h4>Report Specification</h4><pre>" . htmlspecialchars($wizard->summarize(true)) . "</pre>";

    if ($outhtml) $html = $head;
    if ($stdout) print ($head);
  }

  if ($outtext)
  {
    $text = "{$line}[ $page / $pages ]\n\n";
    $text .= "Report Information\n\n";
    $text .= " - Administrator ID: " . login_getuni() . "\n";
    $text .= " - Date: " . date("l, F j, Y") . "\n";
    $text .= " - Time: " . date("g:i:s A") . "\n";
  }
  makeall($stdout, $html, $text, $groups, $options);
}

// values in options array         
//   responseStatistics: boolean, default true. 
//     find number of responses and max possible number
//   pies: boolean, default true
//     for html reports draw pie graphs of response statistics
//   multipleChoice: boolean, default true
//     include multiple choice responses
//   textResponses: boolean, default true
//     include text responses
//   taResponses: boolean, default true
//     include results from TA section
//   topicInfo: boolean, default true
//     include information about survey topics (classes,
//       professors, departments, question periods etc)
//     this information appears in headers in the report
//   pageNumbers: boolean, default true
//     display page numbers
//   skipClassList: boolean, default false
//     don't display list of classes
//   surveyResponses: string, default 'survey_responses'
//     table to select responses from
//     $makeall_survey_responses
//   textResponses: string, default 'textresponse_responses'
//     table to select text responses from
//     $makeall_text_responses
//   choiceResponses: string, default 'choice_responses'
//     table to select choice responses from
//     $makeall_choice_responses
//   skipLines: boolean, default false
//      skip horizontal lines between report pages
//      $report_skip_lines
//   alwaysGraph: boolean, 
//      always show graphical distributions even 
//      for multiple questionsin a single choice components
//      $report_always_graph


function makeall($stdout,&$html,&$text,&$groups, $options = array())
{
  global $wces_path, $wces, $line;

  if (!isset($options['textResponses']))
    $options['textResponses'] = 'textresponse_responses';
  if (!isset($options['choiceResponses']))
    $options['choiceResponses'] = 'choice_responses';
  if (!isset($options['surveyResponses']))
    $options['surveyResponses'] = 'survey_responses';

  if (!isset($options['responseStatistics']) || $options['responseStatistics'])
  {
    $result = pg_go("
      SELECT s.ordinal, s.students, r.responses
      FROM
      (
        -- this subselect contains the count of students for each ordinal
        SELECT cc.ordinal, sum(cc.students) AS students
        FROM
        ( -- this subselect is needed to eliminate
          -- duplicate class entries in the rwtopics
          -- table that occur when a class has more than
          -- one professor
          SELECT t1.ordinal, cl1.students
          FROM rwtopics AS t1
          INNER JOIN classes AS cl1 USING (class_id)
          GROUP BY t1.ordinal, t1.class_id, cl1.students
        ) AS cc
        GROUP BY cc.ordinal
      ) AS s
      INNER JOIN
      (
        -- this subselect contains the number of responses for each ordinal
        SELECT t2.ordinal, COUNT(DISTINCT r2.user_id) AS responses
        FROM (SELECT DISTINCT ordinal, topic_id, question_period_id FROM rwtopics) AS t2
        LEFT JOIN $options[surveyResponses] AS r2 ON
          r2.topic_id = t2.topic_id
          AND r2.question_period_id = t2.question_period_id
          AND r2.user_id IS NOT NULL
        GROUP BY t2.ordinal
      ) AS r ON s.ordinal = r.ordinal
      ORDER BY s.ordinal
    ", $wces, __FILE__, __LINE__);
    
    $stats =& new pg_segmented_wrapper($result, "ordinal");
  }
  else
    $stats = NULL;
  
  if (!isset($options['topicInfo']) || $options['topicInfo'])
  {
    $result = pg_go("
      SELECT DISTINCT t.ordinal, d.name, d.code
      FROM rwtopics AS t
      INNER JOIN departments AS d USING (department_id)
      ORDER BY t.ordinal
    ", $wces, __FILE__, __LINE__);
  
    $departments =& new pg_segmented_wrapper($result, "ordinal");
  
    $result = pg_go("
      SELECT DISTINCT t.ordinal, t.question_period_id, qp.displayname
      FROM rwtopics AS t
      INNER JOIN question_periods AS qp USING (question_period_id)
      ORDER BY t.ordinal
    ", $wces, __FILE__, __LINE__);
  
    $questionperiods =& new pg_segmented_wrapper($result, "ordinal");

    $classes = $professors = null;
    if (!isset($options['skipClassList']) || !$options['skipClassList'])
    if ($groups[CLASSES] || $groups[PROFESSORS])
    {
      $result = pg_go("
        SELECT DISTINCT t.ordinal, t.class_id, s.code AS scode, c.code AS ccode, c.name, cl.section
        FROM rwtopics AS t
        INNER JOIN classes AS cl USING (class_id)
        INNER JOIN courses AS c ON c.course_id = cl.course_id
        INNER JOIN subjects AS s USING (subject_id)
        ORDER BY t.ordinal, s.code, c.code, cl.section
      ", $wces, __FILE__, __LINE__);
  
      $classes =& new pg_segmented_wrapper($result, "ordinal");
  
      $result = pg_go("
        SELECT DISTINCT t.ordinal, t.user_id, u.firstname, u.lastname
        FROM rwtopics AS t
        INNER JOIN users AS u USING (user_id)
        ORDER BY t.ordinal
      ", $wces, __FILE__, __LINE__);
  
      $professors =& new pg_segmented_wrapper($result, "ordinal");
    }
    else
    {
      $result = pg_go("
        SELECT t.ordinal, t.class_id, u.user_id, u.firstname, u.lastname, s.code AS scode, c.code AS ccode, c.name, cl.section
        FROM rwtopics AS t
        INNER JOIN classes AS cl USING (class_id)
        INNER JOIN courses AS c ON c.course_id = cl.course_id
        INNER JOIN subjects AS s USING (subject_id)
        INNER JOIN users AS u USING (user_id)
        ORDER BY t.ordinal
      ", $wces, __FILE__, __LINE__);
  
      $classes =& new pg_segmented_wrapper($result, "ordinal");
      $professors = NULL;
    }
  }
  else
    $departments = $questionperiods = $professors = $classes = NULL;

  if (!isset($options['textResponses']) || $options['textResponses'])
  {
    $result = pg_go("
      SELECT DISTINCT t.ordinal, tr.revision_id, c.ctext, c.flags, tr.rtext, c.branch_id
      FROM rwtopics AS t
      INNER JOIN $options[surveyResponses] AS r USING (topic_id, question_period_id)
      INNER JOIN $options[textResponses] AS tr ON tr.parent = r.response_id
      INNER JOIN textresponse_components AS c ON c.revision_id = tr.revision_id
      ORDER BY t.ordinal, tr.revision_id
    ", $wces, __FILE__, __LINE__);

    $textresponses =& new pg_segmented_wrapper($result, array("ordinal", "revision_id"));
  }
  else
    $textresponses = NULL;

  if (!isset($options['taResponses']) || $options['taResponses'])
  {
    $result = pg_go("
      SELECT DISTINCT t.ordinal, tr.parent, tr.response_id, tr.name, tr.overall, tr. knowledgeability,
        tr.approachability, tr.availability, tr.communication, tr.comments
      FROM rwtopics AS t
      INNER JOIN $options[surveyResponses] AS r USING (topic_id, question_period_id)
      INNER JOIN ta_ratings AS tr ON tr.parent = r.response_id
      ORDER BY t.ordinal, name, tr.response_id
    ", $wces, __FILE__, __LINE__);

    $taresponses =& new pg_segmented_wrapper($result, array("ordinal"));  
  }
  else
    $taresponses = NULL;

  if (!isset($options['multipleChoice']) || $options['multipleChoice'])
  {
    $result = pg_go("
      SELECT t.ordinal, c.revision_id AS crevision_id, first(c.choices) AS component_choices,
        c.flags, c.first_number, c.last_number, c.other_choice,
        q.revision_id AS qrevision_id, q.qtext, choice_dist(qr.answer) AS dist, c.branch_id
      FROM (SELECT DISTINCT ordinal, topic_id, question_period_id FROM rwtopics) AS t
      INNER JOIN $options[surveyResponses] AS r USING (topic_id, question_period_id)
      INNER JOIN $options[choiceResponses] AS cr ON cr.parent = r.response_id
      INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
      INNER JOIN choice_questions AS q ON q.revision_id = qr.revision_id
      INNER JOIN choice_components AS c ON c.revision_id = cr.revision_id
      GROUP BY t.ordinal, c.revision_id, q.revision_id, c.flags, c.first_number,
        c.last_number, c.other_choice, c.branch_id, q.qtext
      ORDER BY t.ordinal, c.revision_id, q.revision_id
    ", $wces, __FILE__, __LINE__);

    $choices =& new pg_segmented_wrapper($result, array("ordinal", "crevision_id"));
  }
  else
    $choices = NULL;

  //
  
  $result = pg_go("
    SELECT ordinal FROM rwtopics GROUP BY ordinal ORDER BY ordinal
  ", $wces, __FILE__, __LINE__);
  
  $pacer =& new pg_wrapper($result);

  // XXX: what is the meaning of *magic* branch_id 1
  $result = pg_go("  
    SELECT t.ordinal, ibtc.branch_id, l.ordinal AS iordinal
    FROM
    ( 
      SELECT DISTINCT ordinal, topic_id, topic_contents(topic_id) AS ctopic_id
      FROM rwtopics
    ) AS t
    INNER JOIN branch_topics_cache AS btc ON (btc.base_branch_id = 1) AND (btc.topic_id = t.ctopic_id)
    INNER JOIN branches AS b ON b.branch_id = btc.branch_id
    INNER JOIN list_items AS l ON l.revision_id = b.content_id
    INNER JOIN branch_topics_cache AS ibtc ON (ibtc.base_branch_id = l.item_id) AND (ibtc.topic_id = t.ctopic_id)
    ORDER BY t.ordinal;
  ", $wces, __FILE__, __LINE__);
  
  $ords =& new pg_segmented_wrapper($result, array("ordinal"));
  
  $pages = pg_result(pg_go("SELECT COUNT(DISTINCT ordinal) FROM rwtopics", $wces, __FILE__,__LINE__),0,0);

  $outhtml = is_string($html);
  $outtext = is_string($text);
  
  $pageNumbers = !isset($options['pageNumbers']) || $options['pageNumbers'];

  for($page = 1; $page <= $pages; ++$page)
  {
    if ($pageNumbers)
    {
      if ($outtext) $text .= "\n{$line}[ $page / $pages ]\n\n";

      if ($outhtml || $stdout)
      {
        $pagehtml = '';
        if (!isset($options['skipLines'] || !$options['skipLines'])
          $pagehtml .= '<hr style="page-break-before:always">' . "\n";
        $pagehtml .= "<font size=1>[ $page / $pages ]</font>\n";
      }
      else
        $pagehtml = false;
    }
    else
      $pagehtml = ($outhtml || $stdout) ? '' : false;

    report_makepage($pacer, $text, $pagehtml, $stats, $classes, $professors, $departments, $questionperiods, $textresponses, $choices, $ords, $taresponses, $options);

    if ($stdout) print($pagehtml);
    if ($outhtml) $html .= $pagehtml;
  }
}

function report_profemail($db,&$wizard,$subject,$message,$debug)
{
  global $server_feedback, $line;
  $sqloptions = array();
  $displayoptions = array("pies" => false);
  $groups = array("classes" => true, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $sort = array("professors","questionperiods","classes");

  $header = true;
  $ratings = ($wizard->includeStandard->checked || $wizard->includeCustom->checked) ? true : false;
  $listclasses = true;
  $listprofessors = true;
  $abet = $wizard->includeAbet->checked ? true : false;
  $responses = $wizard->includeText->checked ? true : false;
  $tas = $wizard->includeTa->checked ? true : false;

  report_makequeries($db, $sqloptions, $wizard->criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet,
    $responses, $tas);

  $professors = db_exec("
    SELECT COUNT(DISTINCT ml.masterid) AS number, ml.professorid, p.name, IFNULL(p.email,IFNULL(u.email,'')) AS address
    FROM mastertable AS ml
    LEFT JOIN professors AS p ON p.professorid = ml.professorid
    LEFT JOIN users AS u ON u.userid = p.userid
    GROUP BY ml.professorid
    ORDER BY ml.masterid", $db, __FILE__, __LINE__);

  $html = false;
  if (!$debug) print("<h3>Sending Email...</h3>");
  print("<p>");
  while($professor = mysql_fetch_assoc($professors))
  {
    $name = $professor["name"];
    $address = $professor["address"];
    $email = "";
    if ($message) $email .= wordwrap(str_replace("%profname%", $name, $message),76) . "\n\n$line";
    $email .= "\nA graphical version of this report is available online at\n";
    $email .= "http://oracle.seas.columbia.edu/wces/professors/\n";

    $pages = $professor["number"];
    $professorid = $professor["professorid"];

    for($page=1; $page <= $pages; ++$page)
    {
      if ($professorid != $listprofessors->row["professorid"])
      {
        printarray($listprofessors->row,"listprofessors");
        debugout("<b>SYNCHRONIZATION ERROR (" . $professor["professorid"] . " != ". $listprofessors->row["professorid"] . ")</b>");
        exit();
      }
      $email .= "\n${line}Page $page of $pages\n\n";
      report_makepage($email, $html, $displayoptions, $groups, $header, $listclasses,
        $listprofessors, $ratings, $abet, $responses, $tas);
    }

    $from = $server_feedback;

    if (!$address) print("<b><font color=red>No email address found for Professor $name</font></b><br>\n");

    if ($debug)
    {
      print("<h4>Professor $name (<a href=\"mailto:$address\">$address</a>)</h4>\n");
      print("<p><b>$subject</b></p>\n");
      print("<ul><pre>$email</pre></ul>");
    }
    else
    {
      $address = "rey4@columbia.edu"; //FOR DEBUGGING
      if (mail($address,$subject,$email, "To: $address\nFrom: $from\nReply-To: $from\nX-Mailer: PHP/" . phpversion()))
      {
        print("<font size=\"-1\">Successfully sent report to Professor $name ($address, $professorid)</font><br>\n");
      }
      else
      {
      	print("<font size=\"-1\" color=red><b>Problem sending report to Professor $name ($address, $professorid)</b></font><br>\n");
      }
    }
  }
  print("</p>\n<h3>Done.</h3>\n");

}

?>
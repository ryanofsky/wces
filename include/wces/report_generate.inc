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
//   questionPeriods: boolean, default true
//     display question period name
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
//   choiceCache: boolean, default true
//     make use of cached_choice_responses table
//   skipLines: boolean, default false
//      skip horizontal lines between report pages
//      $report_skip_lines
//   alwaysGraph: boolean, 
//      always show graphical distributions even 
//      for multiple questionsin a single choice components
//      $report_always_graph
//   topicsTable: string
//     name of table that maps a topic_id to an item_id and specialization_id
//   uglyHackTa, uglyHackText

function makeall($stdout,&$html,&$text,&$groups, $options = array())
{
  global $wces_path, $wces, $line;

  if (!isset($options['textResponses']))
    $options['textResponses'] = 'textresponse_responses';
  if (!isset($options['choiceResponses']))
    $options['choiceResponses'] = 'choice_responses';
  if (!isset($options['surveyResponses']))
    $options['surveyResponses'] = 'survey_responses';
  if (!isset($options['topicsTable']))
    $options['topicsTable'] = 'wces_topics';

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
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t2
        LEFT JOIN $options[surveyResponses] AS r2 ON r2.topic_id = t2.topic_id AND r2.user_id IS NOT NULL
        GROUP BY t2.ordinal
      ) AS r USING (ordinal)
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
  
    $questionperiods = null;
    if (!isset($options['questionPeriods']) || $options['questionPeriods'])
    {
      $result = pg_go("
        SELECT DISTINCT t.ordinal, t.question_period_id, qp.displayname
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t
        INNER JOIN question_periods AS qp USING (question_period_id)
        ORDER BY t.ordinal
      ", $wces, __FILE__, __LINE__);
    
      $questionperiods =& new pg_segmented_wrapper($result, "ordinal");
    }

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
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t
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

  $taresponses = $textresponses = NULL;

  if (!isset($options['textResponses']) || $options['textResponses'])
  {
    if (isset($options['uglyHackText']))
      $result = $options['uglyHackText'];
    else    
    {
      $result = pg_go("
        SELECT t.ordinal, tr.item_id, tr.revision_id, c.ctext, c.flags, tr.rtext, tr.response_id
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t
        INNER JOIN $options[surveyResponses] AS r USING (topic_id)
        INNER JOIN $options[textResponses] AS tr ON tr.parent = r.response_id
        INNER JOIN revisions AS rv ON rv.revision_id = tr.revision_id
        INNER JOIN textresponse_components AS c ON c.component_id = rv.component_id
        ORDER BY t.ordinal, tr.item_id, tr.revision_id
      ", $wces, __FILE__, __LINE__);
    }

    $textresponses =& new pg_segmented_wrapper($result, array("ordinal", "item_id", "revision_id"));
  }

  if (!isset($options['taResponses']) || $options['taResponses'])
  {
    if (isset($options['uglyHackTa']))
      $result = $options['uglyHackTa'];
    else
      $result = pg_go("
        SELECT t.ordinal, tr.parent, tr.response_id, tr.user_id, tr.name, tr.overall, tr. knowledgeability,
          tr.approachability, tr.availability, tr.communication, tr.comments, u.firstname, u.lastname
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t
        INNER JOIN $options[surveyResponses] AS r USING (topic_id)
        INNER JOIN ta_ratings AS tr ON tr.parent = r.response_id
        LEFT JOIN users AS u ON u.user_id = tr.user_id
        ORDER BY t.ordinal, u.lastname, u.firstname, u.user_id, name, tr.response_id
      ", $wces, __FILE__, __LINE__);

    $taresponses =& new pg_segmented_wrapper($result, array('ordinal', 'user_id'));  
  }

  if (!isset($options['multipleChoice']) || $options['multipleChoice'])
  {
    // this is the most expensive query made during reporting. It could be
    // vastly sped up with a cache table (materialized view) that contained:
    //  CREATE TABLE cached_choice_responses AS                                    
    //  SELECT t.topic_id, cr.item_id AS citem_id,                                 
    //    cr.revision_id AS crevision_id, qr.item_id AS qitem_id,                  
    //    qr.revision_id AS qrevision_id, choice_dist(qr.answer) AS dist           
    //  FROM report_topics AS t                                                    
    //  INNER JOIN survey_responses AS r USING (topic_id)                          
    //  INNER JOIN choice_responses AS cr ON cr.parent = r.response_id             
    //  INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id   
    //  GROUP BY t.topic_id, cr.item_id, cr.revision_id, qr.item_id, qr.revision_id, 
    
    $tabname = substr(uniqid('tab'),0,31);

    //$options['choiceCache'] = false;    
    if (isset($options['choiceCache']) && !$options['choiceCache'])
    {
      pg_go("
        CREATE TEMPORARY TABLE $tabname AS
        SELECT t.ordinal, cr.item_id AS citem_id, cr.revision_id AS crevision_id,
          qr.item_id AS qitem_id, qr.revision_id AS qrevision_id,
          choice_dist(qr.answer) AS dist
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t
        INNER JOIN $options[surveyResponses] AS r USING (topic_id)
        INNER JOIN $options[choiceResponses] AS cr ON cr.parent = r.response_id
        INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
        GROUP BY t.ordinal, cr.item_id, cr.revision_id, qr.item_id, qr.revision_id;
      ", $wces, __FILE__, __LINE__);
    }
    else
    {
      pg_go("
        CREATE TEMPORARY TABLE $tabname AS
        SELECT t.ordinal, r.citem_id, r.crevision_id, r.qrevision_id, r.qitem_id,
          dist_sum(r.dist) AS dist
        FROM (SELECT DISTINCT ordinal, topic_id FROM rwtopics) AS t
        INNER JOIN cached_choice_responses AS r USING (topic_id)
        GROUP BY t.ordinal, r.citem_id, r.crevision_id, r.qrevision_id, r.qitem_id
      ", $wces, __FILE__, __LINE__);
    }
    
    $result = pg_go("
      SELECT DISTINCT ON (t.ordinal, t.citem_id, t.crevision_id) t.ordinal, 
        t.citem_id, t.crevision_id, c.choices, c.flags,
        c.first_number, c.last_number, c.other_choice, c.ctext
      FROM $tabname AS t
      INNER JOIN revisions AS r ON r.revision_id = t.crevision_id
      INNER JOIN choice_components AS c USING (component_id)
      ORDER BY t.ordinal, t.citem_id, t.crevision_id, t.qitem_id, t.qrevision_id;
    ", $wces, __FILE__, __LINE__);

    $choiceComponents = new pg_wrapper($result);
    
    $result = pg_go("
      SELECT t.ordinal, t.citem_id, t.crevision_id, t.qitem_id, 
        t.qrevision_id, q.qtext
      FROM $tabname AS t
      INNER JOIN revisions AS r ON r.revision_id = t.qrevision_id
      INNER JOIN choice_questions AS q USING (component_id)
      ORDER BY t.ordinal, t.citem_id, t.crevision_id, t.qitem_id, t.qrevision_id;
    ", $wces, __FILE__, __LINE__);
    
    $choiceQuestions = new pg_wrapper($result);    

    $result = pg_go("
      SELECT * 
      FROM $tabname AS t
      ORDER BY t.ordinal, t.citem_id, t.crevision_id, t.qitem_id, t.qrevision_id;
    ", $wces, __FILE__, __LINE__);

    $choiceResponses = new pg_segmented_wrapper($result, array('ordinal', 'citem_id', 'crevision_id', 'qitem_id', 'qrevision_id'));
  }
  else
    $choiceComponents = $choiceQuestions = $choiceResponses = NULL;

  $result = pg_go("
    SELECT ordinal FROM rwtopics GROUP BY ordinal ORDER BY ordinal
  ", $wces, __FILE__, __LINE__);
  
  $pacer =& new pg_wrapper($result);

  $result = pg_go("  
    SELECT tp.ordinal, l.item_id, l.ordinal AS iordinal
    FROM
    ( 
      SELECT DISTINCT rw.ordinal, rw.topic_id, branch_latest(t.item_id, t.specialization_id, 1000000) AS revision_id
      FROM rwtopics AS rw
      INNER JOIN $options[topicsTable] AS t USING (topic_id)      
    ) AS tp
    INNER JOIN revisions AS r USING (revision_id)
    INNER JOIN list_items AS l USING (component_id)
    ORDER BY tp.ordinal;
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
        if (!isset($options['skipLines']) || !$options['skipLines'])
          $pagehtml .= '<hr style="page-break-before:always">' . "\n";
        $pagehtml .= "<font size=1>[ $page / $pages ]</font>\n";
      }
      else
        $pagehtml = false;
    }
    else
      $pagehtml = ($outhtml || $stdout) ? '' : false;

    report_makepage($pacer, $text, $pagehtml, $stats, $classes, $professors, $departments, $questionperiods, $textresponses, $choiceComponents, $choiceQuestions, $choiceResponses, $ords, $taresponses, $options);

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
        print("<b>SYNCHRONIZATION ERROR (" . $professor["professorid"] . " != ". $listprofessors->row["professorid"] . ")</b>");
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
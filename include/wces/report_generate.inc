<?

require_once("wces/report_page.inc");

function makereport(&$wizard,$stdout,&$html,&$text)
{
  global $wces_path, $wces;

  $outhtml = is_string($html);
  $outtext = is_string($text);

  $sqloptions = array
  (
    "standard" => $wizard->includestandard->checked || ($wizard->includetext->checked && !$wizard->includecustom->checked),
    "custom" => $wizard->includecustom->checked
  );

  $groups = Array
  (
    CLASSES => $wizard->groupclasses->checked,
    COURSES => $wizard->groupcourses->checked,
    PROFESSORS => $wizard->groupprofessors->checked,
    DEPARTMENTS => $wizard->groupdepartments->checked,
    QUESTION_PERIODS => $wizard->groupquestionperiods->checked
  );

  $header = true;
  $ratings = ($wizard->includestandard->checked || $wizard->includecustom->checked) ? true : false;
  $listclasses = ($wizard->groupclasses->checked || $wizard->bighead->checked) ? true : false;
  $listprofessors = ($wizard->groupprofessors->checked || $wizard->bighead->checked) ? true : false;

  $abet = $wizard->includeabet->checked ? true : false;
  $responses = $wizard->includetext->checked ? true : false;
  $tas = $wizard->includeta->checked ? true : false;
  $sort = $wizard->order->keys;

  report_findtopics("rwtopics", $wizard->criteria);
  report_findgroups("rwtopics", $groups, $sort);

  $pages = pg_result(pg_query("SELECT COUNT(DISTINCT ordinal) FROM rwtopics", $wces, __FILE__,__LINE__),0,0);

  $displayoptions = array("pies" => $wizard->includepies->checked);

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

  $line = "---------------------------------------------------------------------------\n";
  if ($outtext)
  {
    $text = "${line}[ $page / $pages ]\n\n";
    $text .= "Report Information\n\n";
    $text .= " - Administrator ID: " . login_getuni() . "\n";
    $text .= " - Date: " . date("l, F j, Y") . "\n";
    $text .= " - Time: " . date("g:i:s A") . "\n";
  }
  makeall($stdout, $html, $text, $groups);
}

function makeall($stdout,&$html,&$text,&$groups)
{
  global $wces_path, $wces;
  
  $result = pg_query("
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
      SELECT t2.ordinal, COUNT(*) AS responses
      FROM rwtopics AS t2
      LEFT JOIN survey_responses AS r2 ON
        r2.topic_id = t2.topic_id
        AND r2.question_period_id = t2.question_period_id
        AND r2.user_id IS NOT NULL
      GROUP BY t2.ordinal
    ) AS r ON s.ordinal = r.ordinal
    ORDER BY s.ordinal
  ", $wces, __FILE__, __LINE__);             

  $stats = new pg_segmented_wrapper($result, "ordinal");

  $result = pg_query("
    SELECT t.ordinal, d.name, d.code
    FROM rwtopics AS t
    INNER JOIN departments AS d USING (department_id)
    ORDER BY t.ordinal
  ", $wces, __FILE__, __LINE__);

  $departments = new pg_segmented_wrapper($result, "ordinal");

  $result = pg_query("
    SELECT t.ordinal, t.question_period_id, qp.displayname
    FROM rwtopics AS t
    INNER JOIN question_periods AS qp USING (question_period_id)
    ORDER BY t.ordinal
  ", $wces, __FILE__, __LINE__);

  $questionperiods = new pg_segmented_wrapper($result, "ordinal");

  if ($groups[CLASSES] || $groups[PROFESSORS])
  {
    $result = pg_query("
      SELECT t.ordinal, t.class_id, s.code AS scode, c.code AS ccode, c.name, cl.section
      FROM rwtopics AS t
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN courses AS c ON c.course_id = cl.course_id
      INNER JOIN subjects AS s USING (subject_id)
      ORDER BY t.ordinal
    ", $wces, __FILE__, __LINE__);

    $classes = new pg_segmented_wrapper($result, "ordinal");

    $result = pg_query("
      SELECT t.ordinal, u.user_id, u.firstname, u.lastname
      FROM rwtopics AS t
      INNER JOIN users AS u USING (user_id)
      ORDER BY t.ordinal
    ", $wces, __FILE__, __LINE__);

    $professors = new pg_segmented_wrapper($result, "ordinal");
  }
  else
  {
    $result = pg_query("
      SELECT t.ordinal, t.class_id, u.user_id, u.firstname, u.lastname, s.code AS scode, c.code AS ccode, c.name, cl.section
      FROM rwtopics AS t
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN courses AS c ON c.course_id = cl.course_id
      INNER JOIN subjects AS s USING (subject_id)
      INNER JOIN users AS u USING (user_id)
      ORDER BY t.ordinal
    ", $wces, __FILE__, __LINE__);

    $classes = new pg_segmented_wrapper($result, "ordinal");

    $professors = NULL;
  }

  $result = pg_query("
    SELECT t.ordinal, tr.revision_id, c.ctext, c.flags, tr.rtext
    FROM rwtopics AS t
    INNER JOIN survey_responses AS r USING (topic_id, question_period_id)
    INNER JOIN textresponse_responses AS tr ON tr.parent = r.response_id
    INNER JOIN textresponse_components AS c ON c.revision_id = tr.revision_id
    ORDER BY t.ordinal, tr.revision_id
  ", $wces, __FILE__, __LINE__);
  
  $textresponses = new pg_segmented_wrapper($result, array("ordinal", "revision_id"));

  $result = pg_query("
    SELECT t.ordinal, c.revision_id AS crevision_id, first(c.choices) AS cchoices,
      c.flags, c.first_number, c.last_number, c.other_choice,
      q.revision_id AS qrevision_id, q.qtext, choice_dist(qr.answer) AS dist
    FROM rwtopics AS t
    INNER JOIN survey_responses AS r USING (topic_id, question_period_id)
    INNER JOIN choice_responses AS cr ON cr.parent = r.response_id
    INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
    INNER JOIN choice_questions AS q ON q.revision_id = qr.revision_id
    INNER JOIN choice_components AS c ON c.revision_id = cr.revision_id
    GROUP BY t.ordinal, c.revision_id, q.revision_id, c.flags, c.first_number,
      c.last_number, c.other_choice, q.qtext
    ORDER BY t.ordinal, c.revision_id, q.revision_id
  ", $wces, __FILE__, __LINE__);

  $choices = new pg_segmented_wrapper($result, array("ordinal", "crevision_id"));
  $pages = pg_result(pg_query("SELECT COUNT(DISTINCT ordinal) FROM rwtopics", $wces, __FILE__,__LINE__),0,0);

  $outhtml = is_string($html);
  $outtext = is_string($text);

  for($page = 1; $page <= $pages; ++$page)
  {
    if ($outtext) $text .= "\n${line}[ $page / $pages ]\n\n";

    if ($outhtml || $stdout)
    {
      $pagehtml = '<hr style="page-break-before:always">' . "\n";
      $pagehtml .= "<font size=1>[ $page / $pages ]</font>\n";
    }
    else
      $pagehtml = false;

    report_makepage($text, $pagehtml, $stats, $classes, $professors, $departments, $questionperiods, $textresponses, $choices);

    if ($stdout) print($pagehtml);
    if ($outhtml) $html .= $pagehtml;
  }
}

function report_profemail($db,&$wizard,$subject,$message,$debug)
{
  $sqloptions = array
  (
    "standard" => $wizard->includestandard->checked || ($wizard->includetext->checked && !$wizard->includecustom->checked),
    "custom" => $wizard->includecustom->checked
  );
  $displayoptions = array("pies" => false);
  $groups = array("classes" => true, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $sort = array("professors","questionperiods","classes");

  $header = true;
  $ratings = ($wizard->includestandard->checked || $wizard->includecustom->checked) ? true : false;
  $listclasses = true;
  $listprofessors = true;
  $abet = $wizard->includeabet->checked ? true : false;
  $responses = $wizard->includetext->checked ? true : false;
  $tas = $wizard->includeta->checked ? true : false;

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

  $line = "---------------------------------------------------------------------------\n";
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

    $from = "admin@thayer.dartmouth.edu";

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
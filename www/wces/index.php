<?
  require_once("wces/login.inc");
  login_protect(login_any);
  
  require_once("wces/page.inc");
  page_top("The WCES");
  
  print("<h3>Welcome " . login_getname() . ",</h3>\n");
  $status = login_getstatus();

  $onemenu = true;

  if ($status & login_administrator)
  {
    if ($onemenu) $onemenu = false; else print("<h4>Administrator Options</h4>");
?>
<p><img align=right src="<?=$wces_path?>media/admin.jpg" width=125 height=125>Here is a list of the options currently available for administrators:</p>
<ul>
  <li><a href="<?=$wces_path?>administrators/surveys.php<?= ($wces_ns4 ? "/ns4?auto=1$ASID" : $QSID) ?>">Edit Surveys</a> - Edit base questions and custom questions.</li>
  <li><a href="<?=$wces_path?>administrators/susage.php<?=$QSID?>">Student Usage</a> - See a report on student usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/pusage.php<?=$QSID?>">Professor Usage</a> - See a detailed report of professor usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/info.php<?=$QSID?>">Course Database</a> - See courses and enrollments.</li>
  <li><a href="<?=$wces_path?>administrators/report.php<?=$QSID?>">Reporting Wizard</a> - View and print past results.</li>
  <li><a href="<?=$wces_path?>administrators/massmail.php<?=$QSID?>">Mass Mail</a> - Send reminder and thank-you emails to students.</li>
  <li><a href="<?=$wces_path?>administrators/import.php<?=$QSID?>">Data Import</a> - Upload data into the WCES course database.</li>
  
</ul>
<?
  }
  
  if ($status & login_professor)
  {
    if ($onemenu) $onemenu = false; else print("<h4>Professor Options</h4>");
    
    $pimg = "<img align=right src=\"${wces_path}media/professor_small.gif\" width=98 height=110>";
    
    if (!($status & login_knownprofessor))
    {
?>
<p><?=$pimg?>Before you will be able to use WCES, your CUNIX account has to be associated
with your listing in the SEAS course database. You can use this 
<a href="<?=$wces_path?>login/profsearch.php<?=$QSID?>">search form</a> to find
your listing and save the association.</p><p>&nbsp;</p>
<?
    }
    else
    {
?>
<p><?=$pimg?>Here is a list of the options currently available for professors:</p>
<ul>
  <li><a href="<?=$wces_path?>professors/surveys.php<?= ($wces_ns4 ? "/ns4?auto=1$ASID" : $QSID) ?>">Upcoming Surveys</a> - Edit or preview the questions your students will see in the upcoming question period.</li>
  <li><a href="<?=$wces_path?>professors/seeresults.php<?=$QSID?>">Survey Results</a> - View the results of past surveys</li>
<? /*
  <li><a href="<?=$wces_path?>professors/respond/multiclasses.php<?=$QSID?>">Survey Responses</a> - Post responses to your survey results</li>
  <li><a href="<?=$wces_path?>professors/respond/historygraph.php<?=$QSID?>">Past Averages</a> - View graphs of past survey averages.</li>
  <li><a href="<?=$wces_path?>professors/infoedit.php<?=$QSID?>">Edit Profile</a> - Edit the information that displayed in the SEAS Oracle. You can update your personal statement, add a link to your homepage, or even upload a new photo.</li>
*/ ?>  
</ul>
<?
    }
  }

  if ($status & login_student)
  {
    if (!isset($db)) $db = wces_connect();
    $userid = login_getuserid();
    
    if ($onemenu) $onemenu = false; else print("<h4>Student Options</h4>");
?>
<p><img align=right src="<?=$wces_path?>media/student.gif" width=99 height=99>
<p>Spring 2002 Midterm Evaluations have not yet begun.</p>
<?
    // wces_connect();
    // $result = pg_query("
    //   SELECT cl.class_id, (s.code || to_char(c.code::int4,'000') || ' ' || c.name) AS name, CASE WHEN sr.user_id IS NULL THEN 0 ELSE 1 END AS surveyed
    //   FROM wces_topics AS t
    //   INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = t.class_id AND e.status = 1
    //   LEFT JOIN survey_responses AS sr ON sr.topic_id = t.topic_id AND sr.question_period_id = (SELECT get_question_period()) AND sr.user_id = $user_id
    //   INNER JOIN classes AS cl ON cl.class_id = e.class_id
    //   INNER JOIN courses AS c USING (course_id)
    //   INNER JOIN subjects AS s USING (subject_id)
    //   GROUP BY cl.class_id, c.code, s.code, c.name, sr.user_id
    //   ORDER BY surveyed, s.code, c.code
    // ", $wces, __FILE__, __LINE__);
    // 
    // print ("Choose a class to evaluate from the list below.</p>");
    // print ("<UL>\n");
    // 
    // $n = pg_numrows($result);
    // 
    // 
    // $found = false;
    // for($i = 0; $i < $n; ++$i)
    // {
    //   extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    //   $found = true;
    //   $complete = true;
    //   if ($surveyed)  
    //     print ("  <LI>Survey Complete: $name</LI>\n");
    //   else
    //   {
    //     $complete = false;
    //     print ("  <LI><A HREF=\"students/survey.php?class_id=$class_id\">$name</a></LI>\n");
    //   }  
    // }
    // if ($n == 0) print ("<LI>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, please <a href=\"{$wces_path}about/feedback.php{$QSID}\">contact us</a>.</LI>");
    // print ("</UL>");
    // 
    // print("<p>Remember to <a href=\"${wces_path}login/logout.php\">log out</a> when you are done.</p>");
  }  
  
  page_bottom();
?>
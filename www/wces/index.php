<?

  require_once("wces/login.inc");
  login_protect(0);
  
  require_once("wces/page.inc");
  page_top("The WCES");
  
  print("<h3>Welcome " . login_getname() . ",</h3>\n");
  $status = login_getstatus();

  $onemenu = true;

  if ($status == 0)
  {
    print("<p>You are not a known student, professor, or administrator. To get access to this site, please <a href=\"{$wces_path}about/feedback.php\">contact us</a>.</p>");
  }

  if ($status & login_administrator)
  {
    if ($onemenu) $onemenu = false; else print("<h4>Administrator Options</h4>");
?>
<p><img align=right src="<?=$wces_path?>media/admin.jpg" width=125 height=125>Here is a list of the options currently available for administrators:</p>
<ul>
  <li><a href="<?=$wces_path?>administrators/susage.php<?=$QSID?>">Student Usage</a> - See a report on student usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/pusage.php<?=$QSID?>">Professor Usage</a> - See a detailed report of professor usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/pstats.php<?=$QSID?>">Professor Listing</a> - View and edit information about professors.</li>
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
  <li><a href="<?=$wces_path?>professors/editsurveys.php<?= ($wces_ns4 ? "/ns4?auto=1$ASID" : $QSID) ?>">Upcoming Surveys</a> - Edit or preview the questions your students will see in the upcoming question period.</li>
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
The midterm evalution period ended on 10/18/01.</p>
<?
//    $classes = db_exec("
//      SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode, if(cr.userid IS NULL, 0, 1) AS surveyed
//      FROM enrollments AS e
//      INNER JOIN groupings AS g ON e.classid = g.linkid AND g.linktype = 'classes'
//      INNER JOIN classes AS cl ON g.linkid = cl.classid AND year = '$year' AND semester = '$semester'
//      INNER JOIN courses AS c USING (courseid)
//      INNER JOIN subjects AS s USING (subjectid)
//      LEFT JOIN cheesyresponses AS cr ON cr.userid = e.userid AND cr.classid = e.classid AND cr.questionperiodid = '$questionperiodid'
//      WHERE e.userid = '$userid'
//      ORDER BY surveyed, s.code, c.code
//      ", $db, __FILE__, __LINE__);
//    
//    print ("<p>Choose a class link from this list below.</p>");
//    print ("<UL>\n");
//    $found = false;
//    while ($class = mysql_fetch_assoc($classes))
//    {
//      $found = true;
//      $complete = true;
//      extract($class);
//      if ($surveyed)  
//        print ("  <LI>Survey Complete: " . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>\n");
//      else
//      {
//        $complete = false;
//        print ("  <LI><A HREF=\"students/survey.php?classid=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</a></LI>\n");
//      }  
//    }
//    if (!$found) print ("<LI>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, check our <a href=\"evaluations.php\">class evaluation listing</a> and email <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a> so we can update our database with your information.</LI>");
//    print ("</UL>");
//    
//    print("<p>Remember to <a href=\"${wces_path}login/logout.php\">log out</a> when you are done.</p>");
  }  
  
  page_bottom();
?>
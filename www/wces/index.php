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
  <li><a href="<?=$wces_path?>administrators/susage.php<?=$QSID?>">Student Usage</a> - See a report on student usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/pusage.php<?=$QSID?>">Professor Usage</a> - See a detailed report of professor usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/pstats.php<?=$QSID?>">Professor Listing</a> - View and edit information about professors.</li>
  <li><a href="<?=$wces_path?>administrators/massmail.php<?=$QSID?>">Mass Mail</a> - Send reminder and thank-you emails to students.</li>
  <li><a href="<?=$wces_path?>administrators/fakelogin.php<?=$QSID?>">Fake Login</a> - Log onto WCES as another user.</li>
  <li><a href="<?=$wces_path?>administrators/enrollment.php<?=$QSID?>">Enrollment Viewer</a> - See a student's class enrollments.</li>
  <li><a href="<?=$wces_path?>administrators/import.php<?=$QSID?>">Data Upload</a> - Load text files into the database.</li>
  <li><a href="<?=$wces_path?>administrators/semester.php<?=$QSID?>">New Semester Initialization</a> - Upload registrar data for a new question period.</li>
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
<p><?=$pimg?>
Here is a list of the options currently available for professors:</p>
<ul>
  <li><a href="<?=$wces_path?>professors/editsurveys.php<?= ($wces_ns4 ? "/ns4?auto=1$ASID" : $QSID) ?>">Upcoming Surveys</a> - Edit or preview the questions your students will see in the upcoming question period.</li>
  <li><a href="<?=$wces_path?>professors/seeresults.php<?=$QSID?>">Survey Results</a> - View the results of past surveys</li>
<? /*
  <li><a href="<?=$wces_path?>professors/respond/multiclasses.php<?=$QSID?>">Survey Responses</a> - Post responses to your survey results</li>
  <li><a href="<?=$wces_path?>professors/respond/historygraph.php<?=$QSID?>">Past Averages</a> - View graphs of past survey averages.</li>
  <li><a href="<?=$wces_path?>professors/infoedit.php<?=$QSID?>">Edit Profile</a> - Edit the information that displayed in the SEAS Oracle. You can update your personal statement, add a link to your homepage, or even upload a new photo.</li>
*/ ?>  
</ul>

<p><i>Note:</i> The results of the Fall 2001 Final Evaluations will be available on Tuesday, January 15, 2002.

<?
    }
  }

  if ($status & login_student)
  {
    if (!isset($db)) $db = wces_connect();
    $userid = login_getuserid();
    wces_GetCurrentQuestionPeriod($db, $questionperiodid, $description, $year, $semester);
    
    if ($onemenu) $onemenu = false; else print("<h4>Student Options</h4>");
    
    if (!$student_open)
    {
?>
<p><img align=right src="<?=$wces_path?>media/student.gif" width=99 height=99>
The Fall 2001 Final Evaluation Period has ended.</p>
<?
    }
    else
    {
      $classes = db_exec("
        SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode, if(cr.userid IS NULL, 0, 1) AS surveyed
        FROM enrollments AS e
        INNER JOIN groupings AS g ON e.classid = g.linkid AND g.linktype = 'classes'
        INNER JOIN classes AS cl ON g.linkid = cl.classid AND year = '$year' AND semester = '$semester'
        INNER JOIN courses AS c USING (courseid)
        INNER JOIN subjects AS s USING (subjectid)
        LEFT JOIN professors AS p ON p.professorid = cl.professorid
        LEFT JOIN cheesyresponses AS cr ON cr.userid = e.userid AND cr.classid = e.classid AND cr.questionperiodid = '$questionperiodid'
        WHERE e.userid = '$userid' AND (p.userid IS NULL OR p.userid <> e.userid)
        ORDER BY surveyed, s.code, c.code
        ", $db, __FILE__, __LINE__);
      
      print ("<p><img align=right src=\"{$wces_path}media/student.gif\" width=99 height=99>Choose a class link from this list below.</p>");
      print ("<UL>\n");
      $found = false;
      while ($class = mysql_fetch_assoc($classes))
      {
        $found = true;
        $complete = true;
        extract($class);
        if ($surveyed)  
          print ("  <LI>Survey Complete: " . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>\n");
        else
        {
          $complete = false;
          print ("  <LI><A HREF=\"students/survey.php?classid=$classid$ASID\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</a></LI>\n");
        }  
      }
      if (!$found) print ("<LI>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, email <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a> so we can update our database with your information.</LI>");
      print ("</UL>");
      
      print("<p>Remember to <a href=\"${wces_path}login/logout.php$QSID\">log out</a> when you are done.</p>");
    }  
  }  
?>
<font size=-1>
<p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
<p align=center>Please send questions or comments to:&nbsp;<a href="mailto:jackie@columbia.edu"></a>
Jackie O. Pavlik&nbsp;&nbsp;
<br>Staff Associate for Information Technology
<br>Columbia University&nbsp;
<br>Fu Foundation School of Engineering and Applied Science
<br>
Voice (212) 854-6444
<br>e-mail <a href="<?=$wces_path?>about/feedback.php<?=$QSID?>">jackie@columbia.edu</a>
<br></p>
</font>
<?  
  page_bottom();
?>
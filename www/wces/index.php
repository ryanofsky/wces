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
    if ($onemenu) $onemenu = false; else print("<h4>Student Options</h4>");
?>
<p><img align=right src="<?=$wces_path?>media/student.gif" width=99 height=99>
The WCES will open for Midterm Evaluations on Friday, October 6.<p>
<!-- Choose a class to evaluate from the list below.</p>
<ul>
  <li><a href="<?=$wces_path?>students/survey.php?surveyid=1<?=$ASID?>">Survey 1</a></li>
  <li><a href="<?=$wces_path?>students/survey.php?surveyid=2<?=$ASID?>">Survey 2</a></li>
</ul>
<p>Sick of automated emails? Adjust your <a href="<?=$wces_path?>students/optout.php<?=$QSID?>">email settings</a>.</p> -->
<?
  }  
  
  page_bottom();
?>
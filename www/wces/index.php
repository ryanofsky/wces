<?
  require_once("wces/login.inc");
  
  login_protect(0);

  require_once("wces/page.inc");  
  page_top("The WCES");
  
  $login =& LoginInstance();
  
  print("<h3>Welcome " . $login->get('name') . ",</h3>\n");
  $status = $login->get('status');

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
  <li><a href="<?=$wces_path?>administrators/surveys.php<?= ($wces_ns4 ? "/ns4?auto=1$ASID" : $QSID) ?>">Edit Surveys</a> - Edit base questions and custom questions.</li>
  <li><a href="<?=$wces_path?>administrators/susage.php<?=$QSID?>">Student Usage</a> - See a report on student usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/pusage.php<?=$QSID?>">Professor Usage</a> - See a detailed report of professor usage of WCES during the current question period.</li>
  <li><a href="<?=$wces_path?>administrators/info.php<?=$QSID?>">Enrollment Database</a> - See information about users and classes.</li>
  <li><a href="<?=$wces_path?>administrators/report_choose.php<?=$QSID?>">Reporting Wizard</a> - View and print past results.</li>
  <li><a href="<?=$wces_path?>administrators/massmail.php<?=$QSID?>">Mass Mail</a> - Send reminder and thank-you emails to students.</li>
  <li><a href="<?=$wces_path?>administrators/import.php<?=$QSID?>">Data Import</a> - Upload data into the WCES course database.</li>  
</ul>
<?
  }

  if ($status & login_deptadmin)
  {
    if ($onemenu) $onemenu = false; else print("<h4>Department Administrator Options</h4>");
?>
<p><img align=right src="<?=$wces_path?>media/admin.jpg" width=125 height=125>Here is a list of the options currently available for department administrators:</p>
<ul>
  <li><a href="<?=$wces_path?>administrators/choose.php<?=$QSID?>">Reporting Wizard</a> - View and print past survey results for courses within your department.</li>
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
<p><?=$pimg?>According to the AcIS affiliation listings, you are a professor
but are not currently teaching any classes. If you need professor access to
WCES (the ability to see survey results and create survey questions), you
should <a href="<?=$wces_path?>about/feedback.php<?=$QSID?>">send us</a>
your name, CUNIX ID, and the list of classes you teach.
<? if (!($status & login_student)) { ?>
If you just need student access to the site (to evaluate a class you are
taking) also <a href="<?=$wces_path?>about/feedback.php<?=$QSID?>">let us
know</a>. In both cases, we can quickly add your information to the database
and make the rest of the site accessible to you.
<? } ?>
<?
    }
    else
    {
?>
<p><?=$pimg?>Here is a list of the options currently available for professors:</p>
<ul>
  <li><a href="<?=$wces_path?>professors/surveys.php<?= ($wces_ns4 ? "/ns4?auto=1$ASID" : $QSID) ?>">Upcoming Surveys</a> - Edit or preview the questions your students will see in the upcoming question period.</li>
  <li><a href="<?=$wces_path?>professors/seeresults.php<?=$QSID?>">Survey Results</a> - View the results of past surveys</li>
  <li><a href="<?=$wces_path?>professors/profile.php<?=$QSID?>">Edit Profile</a> - Update the biographical information that appears in Oracle</li>
  <li><a href="<?=$wces_path?>professors/new_prof.php<?=$QSID?>">Comment on Results</a> - Review your courses' longitudinal data</li>
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
<?
    $survey_listing = get_surveys();
    if (!$survey_listing)
    {
      print("<p>The evaluation period has closed.</p>");	
    }
    else
    {
      if (!isset($db)) $db = wces_connect();
      $userid = LoginValue("user_id");
      
      $n = pg_numrows($survey_listing);

      if ($n == 0)
      {
        print("<p>None of the classes you are enrolled in have evaluations available at this time. If you think this is an error, please <a href=\"{$wces_path}about/feedback.php{$QSID}\">contact us</a>.</p>");
      }
      else
      {
        print ("Choose a class to evaluate from the list below.</p>");
        print ("<UL>\n");

        $found = false;
        for($i = 0; $i < $n; ++$i)
        {
          extract(pg_fetch_array($survey_listing,$i,PGSQL_ASSOC));
          $name = format_class($name);
          $found = true;
          $complete = true;
          if ($surveyed)  
            print ("  <LI>Survey Complete: $name</LI>\n");
          else
          {
            $complete = false;
            print ("  <LI><A HREF=\"students/survey.php?topic_id=$topic_id$ASID\">$name</a></LI>\n");
          }  
        }

        print ("</UL>");

        print("<p>Remember to <a href=\"${wces_path}login/logout.php\">log out</a> when you are done.</p>");
      }
    }
  }
  
  page_bottom();
?>
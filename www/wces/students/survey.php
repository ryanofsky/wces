<?

require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("wces/wces.inc");
require_once("wbes/surveyeditor.inc");
login_protect(login_student);

param($classid);
param($save);

if (!isset($db)) $db = wces_connect();
$userid = login_getuserid();
wces_GetCurrentQuestionPeriod($db, $questionperiodid, $description, $year, $semester);

if ($classid)
{
  $classid = (int)$classid;
  $result = db_exec("
    SELECT 1
    FROM enrollments AS e
    INNER JOIN groupings AS g ON e.classid = g.linkid AND g.linktype = 'classes'
    INNER JOIN classes AS cl ON g.linkid = cl.classid AND year = '$year' AND semester = '$semester'
    LEFT JOIN cheesyresponses AS cr ON cr.userid = e.userid AND cr.classid = e.classid AND cr.questionperiodid = '$questionperiodid'
    WHERE e.classid = '$classid' AND e.userid = '$userid' AND cr.userid IS NULL
  ", $db, __FILE__, __LINE__);
  
  if(mysql_num_rows($result) == 1)
  {
    $e = new SurveyEditor("prefix","f",WIDGET_POST);
    $e->editclass = $classid;
    $e->load();
    
    foreach(array_keys($e->survey->components) as $i)
    {
      $c = &$e->survey->components[$i];
      $w = $c->getwidget("student_$i","f", WIDGET_POST);
      $w->loadvalues();
      $widgets[] = $w;
    }
    
    if ($save)
    {
      db_exec("
        INSERT INTO cheesyresponses (userid, classid, questionperiodid, dump) VALUES
        ('$userid', '$classid', '$questionperiodid', '" . addslashes(serialize($HTTP_POST_VARS)) . "')", $db, __FILE__, __LINE__);
        redirect($wces_path);
        exit();
    }
  }
  else
    $classid = 0;
}

page_top("Student Survey");

if ($classid)
{
  $result = db_exec("
    SELECT CONCAT(s.code, c.code, ' ', c.name) AS name
    FROM classes AS cl
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    WHERE cl.classid = '$classid'
  ", $db, __FILE__, __LINE__);
  print("<h3>" . mysql_result($result,0) . "</h3>");  
  print("<form name=f method=post action=survey.php>\n");
  print("<input type=hidden name=classid value=$classid>\n");
  foreach(array_keys($widgets) as $i)
  {
    print("<div>");
    $widgets[$i]->display();
    print("</div>\n<br>\n");
  }
  print('<input type=submit name=save value="Save Responses" class=tinybutton>');
  print("</form>\n");
}
else
  print("Invalid Class ID.");

page_bottom();

?>
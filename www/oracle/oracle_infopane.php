<?

require_once("wces/oracle.inc");
require_once("wces/wces.inc");
require_once("wces/report_help.inc");
require_once("wbes/general.inc");
require_once("wbes/postgres.inc");
require_once("wbes/component_choice.inc");
require_once("wces/report_page.inc");

if (isset($show_distributions))
{
  $show_distributions = $show_distributions ? 1 : 0;
  setcookie('cookie_show_distributions', $show_distributions);
}
else if (isset($HTTP_COOKIE_VARS['cookie_show_distributions']))
{
  $show_distributions = $HTTP_COOKIE_VARS['cookie_show_distributions'] ? 1 : 0;
}
else
{
  $show_distributions = 0;
};

?>

<style type="text/css">
<!--
body                    { font-family: Arial, Helvetica, sans-serif; }
p                       { font-family: Arial, Helvetica, sans-serif; }
h3                      { font-family: Arial, Helvetica, sans-serif; }
h4                      { font-family: Arial, Helvetica, sans-serif; }
-->
</style>

<body bgcolor="#6699CC"><table vspace=20 hspace=20 width=100% height=100% bordercolor=black border=1 cellpadding=5 cellspacing=0><tr>
<?

param($user_id);
param($course_id);
param($class_id);


$show_distributions = (int) $show_distributions;

wces_connect();

function ShowClass($class_id)
{
  global $oracle_branch_id, $show_distributions, $wces, $select_classes, $set_question_periods, $wces_path, $oracle_root, $set_question_periods, $base_base_branch_id;

  $class_id = (int)$class_id;

  $sections = pg_query("
    SELECT cl2.class_id AS section_id, get_class(cl2.class_id) AS section_info
    FROM classes AS cl1
    INNER JOIN courses AS c USING (course_id)
    INNER JOIN classes AS cl2 ON cl2.course_id = c.course_id
    INNER JOIN ($select_classes) AS l ON l.class_id = cl2.class_id
    WHERE cl1.class_id = $class_id
    ORDER BY cl2.year DESC, cl2.semester DESC, cl2.section
  ", $wces, __FILE__, __LINE__);
  
  // after this test it is safe to assume that this class is in $selected_classes
  if (pg_numrows($sections) == 0) return false;

  $infoq = pg_query("SELECT get_class($class_id) AS class_info, get_profs($class_id) AS profs_info", $wces, __FILE__, __LINE__);
  extract(pg_fetch_array($infoq,0,PGSQL_ASSOC));
  
  print("<h3>" . format_class($class_info, "%N") . "</h3>\n");

?>
<table width=100% border=0 cellpadding=0 cellspacing=0>
<tr>
  <td width=50% valign=top>
    <form method=get name=classes>
    <h4>Section</h4>
    <p><select name=class_id onchange="this.form.submit()" size=1>
<?

  $n = pg_numrows($sections);
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($sections, $i, PGSQL_ASSOC));
    $selected = $section_id == $class_id ? " selected" : "";
    print("      <option value=$section_id$selected>" . format_class($section_info, "%t - Section %s") . "</option>\n");
  }
  DebugBreak();
?>    </select></p>
    <p><input type=image name=go src="<?=$oracle_root?>media/go.gif"></p>
    </form>
  </td>
  <td width=50% valign=top>
    <h4>Professor</h4>
    <div><?= format_profs($profs_info, 'oracle_infopane.php?user_id=', '', '', '<br>'); ?></div>
  </td>
</tr>
</table>    
<?    
  $results = pg_query("
    SELECT c.revision_id AS crevision_id, first(c.choices) AS cchoices,
      c.flags, c.first_number, c.last_number, c.other_choice,
      q.revision_id AS qrevision_id, q.qtext, choice_dist(qr.answer) AS dist
    FROM wces_topics AS t
    INNER JOIN survey_responses AS r ON r.topic_id = t.topic_id
    INNER JOIN choice_responses AS cr ON cr.parent = r.response_id
    INNER JOIN choice_components AS c ON c.revision_id = cr.revision_id
    INNER JOIN branches AS b ON c.branch_id = b.branch_id
    INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
    INNER JOIN choice_questions AS q ON q.revision_id = qr.revision_id
    WHERE t.class_id = $class_id AND r.question_period_id IN $set_question_periods AND c.branch_id = $oracle_branch_id
    GROUP BY c.revision_id, q.revision_id, c.flags, c.first_number,
      c.last_number, c.other_choice, c.branch_id, q.qtext
    ORDER BY q.revision_id
  ", $wces, __FILE__, __LINE__);
  
  $rs = pg_query("
    SELECT cl.students, COUNT(DISTINCT r.user_id) AS responses
    FROM classes AS cl
    INNER JOIN wces_topics AS t USING (class_id)
    INNER JOIN survey_responses AS r ON r.topic_id = t.topic_id AND r.user_id IS NOT NULL
    WHERE cl.class_id = $class_id AND r.question_period_id IN $set_question_periods
    GROUP BY cl.class_id, cl.students
  ", $wces, __FILE__, __LINE__);

  $n = pg_numrows($results);
  
  $db = NULL;
  if ($n == 0)
  {
    $abc = pg_query("SELECT oldid FROM temp_class WHERE newid = $class_id", $wces, __FILE__, __LINE__);
    $classid = pg_numrows($abc) == 1 ? (int)pg_result($abc,0,0) : 0;
    
    if ($classid)
    {
      $db = wces_oldconnect();
      $answer_criteria = "a.questionperiodid IN (1,2,4,5,7,9) AND a.topicid IN (1,2,4) AND a.questionsetid = '1'";
      
      $sql_columns = "c.students, a.responses"; 
       
      $choices = array("a","b","c","d","e"); 
      for($i = 1; $i <= 10; ++$i) 
      { 
        $sql_columns .= ", q.MC$i"; 
        foreach($choices as $choice) 
          $sql_columns .= ", a.MC$i$choice"; 
      }; 
       
      $n = db_exec(" 
        SELECT $sql_columns 
        FROM answersets AS a 
        INNER JOIN classes as c USING (classid) 
        LEFT JOIN questionsets as q ON (a.questionsetid = q.questionsetid) 
        WHERE a.classid = '$classid' AND $answer_criteria 
        ORDER BY a.responses DESC LIMIT 1
      ", $db, __FILE__, __LINE__); 
      
      if (mysql_numrows($n) > 0)
      {
        assert(mysql_numrows($n) == 1);
        $dfg = mysql_fetch_array($n); 
        $responses = (int)$dfg['responses'];
        $students = (int)$dfg['students'];
        $n = 10;        
      }
      else
        $n = 0;
    }
  };
  
  if ($n > 0)
  {
    print("<h4>Survey Results</h4>");
    
    if (!$show_distributions)
      print("<p>Show Averages | <a href=\"oracle_infopane.php?class_id=$class_id&show_distributions=1\">Show Distributions</a></p>"); 
    else
      print("<p><a href=\"oracle_infopane.php?class_id=$class_id&show_distributions=0\">Show Averages</a> | Show Distributions</p>"); 

    if (!$db)
    {
      assert(pg_numrows($rs) == 1);
      extract(pg_fetch_array($rs, 0, PGSQL_ASSOC));
    }

    print('<table border=0 cellpadding=0 cellspacing=5');
    print("<tr><td colspan=3><p><i>$responses of $students students responded</i></p></td></tr>\n");

    for($i = 0; $i < $n; ++$i)
    {
      if (!$db)
      {
        extract(pg_fetch_array($results,$i,PGSQL_ASSOC));      
        $cchoices = pg_explode($cchoices);
        
        $values = array(); // indexed by choice keys, holds numeric values of choices
        foreach($cchoices as $ci => $ct)
        {
          $values[$ci] = $values[$ci] = $first_number + ($ci
            / (count($cchoices)-1)) * ($last_number - $first_number);
        }
           
        $sums = array_pad(pg_explode($dist),count($cchoices),0);
      }
      else
      {
        $j = $i + 1;
        $qtext = $dfg["MC$j"];
        if (!$qtext) continue;
        $cchoices = array("excellent", "very good", "satisfactory", "poor", "disastrous");
        $values = array(5,4,3,2,1);
        $flags = 0;
        $sums = array($dfg["MC{$j}a"], $dfg["MC{$j}b"], $dfg["MC{$j}c"], $dfg["MC{$j}d"], $dfg["MC{$j}e"]); 
      };
      
      $dist = false;
      foreach($values as $vi => $vk)
        $dist[$vk] = $sums[$vi];
      
      if ($flags & FLAG_NACHOICE) 
        unset($dist[end(array_keys($dist))]);
      $avg = report_avg($dist);


      if ($show_distributions)
      {
        print("<tr><td>$qtext<br>");
        print(MakeGraph($cchoices, $sums));
        print("</td></tr>\n");
      }
      else
      {
        printf("<tr><td>$qtext</td><td>%.2f</td><td nowrap>", $avg);
        print(report_meter(round($avg * 20)));
        print("</td></tr>\n");
      }
    }
    print("</table>");
  }
  else
    print("<p><strong><i>No Survey Responses Available</i></strong></p>");

  $result = pg_query("
    SELECT cl.name AS clname, cl.section, cl.year, cl.semester, cl.students, c.code, c.name, c.information, d.code as dcode, d.name as dname, s.code as scode, s.name as sname, dv.name as dvname, sc.name as scname, c.course_id, d.department_id, s.subject_id, sc.school_id, dv.code AS dvcode, dv.division_id, cl.time, cl.location, cl.callnumber
    FROM classes as cl
    INNER JOIN courses as c USING (course_id)
    INNER JOIN subjects as s USING (subject_id)
    LEFT JOIN departments as d ON d.department_id = cl.department_id
    LEFT JOIN divisions as dv ON (dv.division_id = cl.division_id)
    LEFT JOIN schools as sc ON (sc.school_id = cl.school_id)
    WHERE cl.class_id = $class_id
  ", $wces, __FILE__, __LINE__); 
  
  assert(pg_numrows($result) == 1);
  
  extract(pg_fetch_array($result, 0, PGSQL_ASSOC));
    
  print ("<h4>Course Information</h4>\n");
  if ($information) print("<p>$information</p>\n");
  if ($dcode) print ("<p><i>Department:</i> $dname($dcode)</p>");
  if ($scode) print ("<p><i>Subject:</i> $sname($scode)</p>");
  if ($scname) print ("<p><i>School:</i> $scname</p>");
  if ($dvname) print ("<p><i>Division:</i> $dvname</p>");
  if ($code) print ("<p><i>Course ID:</i> $code</p>");
}

function ShowProfessor($user_id)
{
  global $wces, $select_classes, $wces_path, $oracle_root;
  
  $user_id = (int)$user_id;
  
  $result = pg_query("SELECT uni, firstname, lastname, email FROM users WHERE user_id = $user_id AND flags & 4 = 4", $wces, __FILE__, __LINE__);
  if (pg_numrows($result) != 1)
  {
    print("<h1>User not found</h1>");
    return false;
  }
  
  extract(pg_fetch_array($result, 0, PGSQL_ASSOC));
  
  print("<h3>$firstname $lastname</h3>");
  
  $result = pg_query("SELECT url, picname, statement, profile, education FROM professor_data WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
  if (!pg_numrows($result) != 1)
    extract(pg_fetch_array($result,0,PGSQL_ASSOC));
  else
    $url = $picname = $statement = $profile = $education = NULL;

  if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>");
  
  $classes = pg_query("
    SELECT class_id, get_class(e.class_id) AS class_info
    FROM enrollments AS e
    INNER JOIN ($select_classes) AS l USING (class_id)
    WHERE e.user_id = $user_id AND e.status = 3
    GROUP BY e.class_id
    ORDER BY class_info
  ", $wces, __FILE__, __LINE__);

  $n = pg_numrows($classes);
?>
<h4>Courses Taught</h4>
<form method=get name=classes>
<p><select name=class_id onchange="this.form.submit()" size=1>
<?
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($classes, $i, PGSQL_ASSOC));
    print ("  <option value=$class_id>" . htmlspecialchars(format_class($class_info)) . "</option>\n");
  }
?>  
</select></p>
<p><input type=image name=go src="<?=$oracle_root?>media/go.gif"></p>
</form>
<?
  if ($statement) print("<h4>Statement</h4>\n<p>$statement</p>\n");
  if ($profile) print("<h4>Profile</h4>\n<p>$profile</p>\n");
  if ($education) print("<h4>Education</h4>\n<p>$education</p>\n");
  if ($email || $url ||$uni) print("<h4>Contact Information</h4>\n");
  if ($email) print ("<p><a href=\"mailto:$email\">$email</a></p>\n");
  if ($url) print ("<p><a href=\"$url\">$url</a></p>\n");
  if ($uni) print ("<p>CUNIX ID: <a href=\"{$wces_path}administrators/info.php?uni=$uni#ldap\">$uni</a></p>\n");
  print("</td>");
}

if ($user_id)
{
  print("<td bgcolor=\"#B5CFE8\" valign=top>");
  ShowProfessor($user_id);
}
else if ($class_id || $course_id)
{
  print('<td bgcolor="#B5CFE8" valign=top>');
  
  if (!$class_id)
  {
    $r = pg_query("
      SELECT cl.class_id
      FROM classes AS cl
      INNER JOIN ($select_classes) AS l USING (class_id)
      WHERE cl.course_id = $course_id
      ORDER BY cl.section
      LIMIT 1
    ", $wces, __FILE__, __LINE__);
    if (pg_numrows($r) == 1)
      $class_id = (int)pg_result($r, 0, 0);
  }
  
  if ($class_id)
  {
    ShowClass($class_id); 
  }
}
else
{
  ?>
  <td bgcolor="B5CFE8" valign=center>
  <table align=center width=200 height=200 border=1 cellpadding=0 cellspacing=0 bordercolor=black><tr><td bgcolor="#FFFFFF" valign=middle>
  <p align=center><img src="<?=$oracle_root?>media/seas_anim.gif" width=100 height=100></p>
  <p align=center>Choose a course or professor from the list in the left pane.</p>
  </td></tr>
  </table>
  </td>
  <?
}
?>
</tr></table></body>












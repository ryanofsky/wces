<?

require_once("wces/oracle.inc");
require_once("wces/wces.inc");
require_once("wces/report_help.inc");
require_once("wbes/general.inc");
require_once("wbes/postgres.inc");
require_once("wbes/component_choice.inc");
require_once("wces/report_page.inc");
require_once("wces/SimpleResults.inc");

param('user_id');
param('course_id');
param('class_id');
param('show_distributions');

if (isset($show_distributions))
{
  $show_distributions = $show_distributions ? 1 : 0;
  setcookie('cookie_show_distributions', $show_distributions);
}
else if (isset($_COOKIE['cookie_show_distributions']))
{
  $show_distributions = $_COOKIE['cookie_show_distributions'] ? 1 : 0;
}
else
{
  $show_distributions = 0;
};

?>
<head>
<title>Results</title>
<style type="text/css">
<!--
body
{
  font-family: Arial, Helvetica, sans-serif;
  scrollbar-3dlight-color: #6699CC; 
  scrollbar-darkshadow-color:black;
  scrollbar-face-color: #6699CC;
  scrollbar-arrow-color:black;
  scrollbar-highlight-color:#AFD2F5;
  scrollbar-shadow-color: #3D5C7A;
  scrollbar-base-color:tomato;
  scrollbar-track-color: #B5CFE8; 
}
p  { font-family: Arial, Helvetica, sans-serif; }
h3 { font-family: Arial, Helvetica, sans-serif; }
h4 { font-family: Arial, Helvetica, sans-serif; }

-->
</style>
</head>

<body bgcolor="#6699CC" link="#000000" alink="#444444" vlink="#444444"><table width=100% height=100% bordercolor=black border=1 cellpadding=5 cellspacing=0><tr>
<?

wces_connect();

function ShowClass($class_id)
{
  global $oracle_branch_id, $show_distributions, $wces, $select_classes, $set_question_periods, $wces_path, $oracle_root, $set_question_periods, $base_base_branch_id;

  $class_id = (int)$class_id;

  $sections = pg_go("
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

  $infoq = pg_go("SELECT get_class($class_id) AS class_info, get_profs($class_id) AS profs_info", $wces, __FILE__, __LINE__);
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
    print("      <option value=$section_id$selected>" . format_class(
      $section_info, "%t - Section %s") . "</option>\n");
  }
?>    </select></p>
    <noscript><p><input type=image name=go src="<?=$oracle_root?>media/go.gif"></p></noscript>
    </form>
  </td>
  <td width=50% valign=top>
    <h4>Professor</h4>
    <div><?= format_profs($profs_info, 'oracle_infopane.php?user_id=', '', '', '<br>'); ?></div>
  </td>
</tr>
</table>    
<?    
  $sr =& new SimpleResults(array($class_id), false);
  
  print("<h4>Survey Results</h4>");
    
  if (!$show_distributions)
    print("<p>Show Averages | <a href=\"oracle_infopane.php?class_id=$class_id&show_distributions=1\">Show Distributions</a></p>\n"); 
  else
    print("<p><a href=\"oracle_infopane.php?class_id=$class_id&show_distributions=0\">Show Averages</a> | Show Distributions</p>\n"); 

  print("<table border=0 cellpadding=0 cellspacing=5>\n");
  print("<tr><td colspan=3><p><i>{$sr->responses[$class_id]} of {$sr->students[$class_id]} students responded</i></p></td></tr>\n");

  if (false) 
    print("<p><strong><i>No Survey Responses Available</i></strong></p>\n");
  else
  {  
    foreach($sr->base_questions as $i => $qtext)
    {
      if ($show_distributions)
      {
        print("<tr><td>$qtext<br>");
        print(MakeGraph($sr->base_choices, $sr->distributions[$class_id][$i]));
        print("</td></tr>\n");
      }
      else
      {
        $dist = &$sr->distributions[$class_id][$i];
        $vdist = array();
        foreach($sr->base_values as $k => $v)
          $vdist[$v] = $dist[$k];
        $avg = report_avg($vdist);
        printf("<tr><td>$qtext</td><td>%.2f</td><td nowrap>", $avg);
        print(report_meter(round($avg * 20)));
        print("</td></tr>\n");
      }
    }
  }

  print("</table>\n");

  $result = pg_go("
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
  if ($dcode) print ("<p><i>Department:</i> $dname($dcode)</p>\n");
  if ($scode) print ("<p><i>Subject:</i> $sname($scode)</p>\n");
  if ($scname) print ("<p><i>School:</i> $scname</p>\n");
  if ($dvname) print ("<p><i>Division:</i> $dvname</p>\n");
  if ($code) print ("<p><i>Course ID:</i> $code</p>\n");
}

function ShowProfessor($user_id)
{
  global $wces, $select_classes, $wces_path, $oracle_root;
  
  $user_id = (int)$user_id;
  
  $result = pg_go("SELECT uni, firstname, lastname, email FROM users WHERE user_id = $user_id AND flags & 4 = 4", $wces, __FILE__, __LINE__);
  if (pg_numrows($result) != 1)
  {
    print("<h1>User not found</h1>\n");
    return false;
  }
  
  extract(pg_fetch_array($result, 0, PGSQL_ASSOC));
  
  print("<h3>$firstname $lastname</h3>\n");
  
  $result = pg_go("SELECT url, picname, statement, profile, education FROM professor_data WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
  if (!pg_numrows($result) != 1)
    extract(pg_fetch_array($result,0,PGSQL_ASSOC));
  else
    $url = $picname = $statement = $profile = $education = NULL;

  if ($picname) print("<p><img src=\"/oracle/prof_images/$picname\"></p>\n");
  
  $classes = pg_go("
    SELECT e.class_id, get_class(e.class_id) AS class_info
    FROM enrollments AS e
    INNER JOIN ($select_classes) AS l USING (class_id)
    INNER JOIN classes AS cl ON cl.class_id = l.class_id
    WHERE e.user_id = $user_id AND e.status = 3
    GROUP BY e.class_id, cl.semester, cl.year
    ORDER BY cl.year DESC, cl.semester DESC, class_info
  ", $wces, __FILE__, __LINE__);

  $n = pg_numrows($classes);
?>
<h4>Courses Taught</h4>
<form method=get name=classes>
<input type=hidden name=user_id value=<?=$user_id?>>
<p><select name=class_id onchange="this.form.submit()" size=1>
<script>
<!--
document.write('<option value=0 selected>Choose One...</option>');
// -->
</script>
<?
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($classes, $i, PGSQL_ASSOC));
    print ("  <option value=$class_id>" . htmlspecialchars(
      format_class($class_info, "%t %c %n Section %s"))
       . "</option>\n");
  }
?>  
</select></p>
<noscript><p><input type=image name=go src="<?=$oracle_root?>media/go.gif"></p></noscript>
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

if ($class_id || $course_id)
{
  print('<td bgcolor="#B5CFE8" valign=top>');
  
  if (!$class_id)
  {
    $r = pg_go("
      SELECT cl.class_id
      FROM classes AS cl
      INNER JOIN ($select_classes) AS l USING (class_id)
      WHERE cl.course_id = $course_id
      ORDER BY cl.year DESC, cl.semester DESC, cl.section
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
else if ($user_id)
{
  print("<td bgcolor=\"#B5CFE8\" valign=top>");
  ShowProfessor($user_id);
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
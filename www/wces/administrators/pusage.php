<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/usage.inc");
login_protect(login_administrator);

param('question_period_id');
param('category_id');

wces_connect();

// generate lists of options
$questionPeriodChoices = usage_sql_array("
  SELECT q.question_period_id, q.displayname
  FROM semester_question_periods AS q
  WHERE EXISTS (SELECT * FROM wces_topics AS t WHERE t.question_period_id = q.question_period_id)
  ORDER BY begindate
", $wces, __FILE__, __LINE__);

$categoryChoices = usage_sql_array("
  SELECT 0 AS id, 'None'
  UNION
  SELECT c.survey_category_id, c.name
  FROM survey_categories AS c
  ORDER BY id
", $wces, __FILE__, __LINE__);

// figure out current values
if (!isset($category_id) || !isset($categoryChoices[$category_id])) $category_id = 0;
if (isset($question_period_id) && isset($questionPeriodChoices[$question_period_id]))
  set_question_period($question_period_id);
else
  $question_period_id = get_question_period();

$currentValues = array('question_period_id' => $question_period_id,
  'category_id' => $category_id);

// get information about selected question period
$result = pg_go("
  SELECT question_period_id, displayname, year, semester
  FROM semester_question_periods
  WHERE question_period_id = $question_period_id
", $wces, __FILE__, __LINE__);

extract(pg_fetch_array($result,0,PGSQL_ASSOC));

page_top("Professor Usage Data for $displayname");

print("<p>Question Period: "); 
usage_menu($questionPeriodChoices, 'question_period_id', $currentValues);
print("<br>\n");

print("Filtering: "); 
usage_menu($categoryChoices, 'category_id', $currentValues);
print("<br>\n");

$cat = $category_id ? " AND t.category_id = $category_id" : '';
pg_go("
  CREATE TEMPORARY TABLE tcl AS
  SELECT t.class_id, cl.students, u.user_id, u.lastname, u.uni, 
    u.firstname, u.lastlogin, get_class(t.class_id) AS class_info,
    CASE WHEN specialization_modified(t.item_id, t.specialization_id)
    THEN 1 ELSE 0 END AS customized,
    CASE WHEN (select timestamp 'now'  - interval '30 days') < u.lastlogin
    THEN 1 ELSE 0 END AS loggedin
  FROM wces_topics AS t
  INNER JOIN classes AS cl USING (class_id)
  INNER JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = 3
  INNER JOIN users AS u USING (user_id)
  WHERE t.question_period_id = $question_period_id$cat;  
",$wces,__FILE__, __LINE__);

pg_go("
  CREATE TEMPORARY TABLE tst AS
  SELECT class_id, CASE WHEN customized <> 0 THEN 0 WHEN sum(loggedin) > 0 THEN 1 
    ELSE 2 END AS status 
  FROM tcl
  GROUP BY class_id, customized;
",$wces,__FILE__, __LINE__);

pg_go("
  CREATE TEMPORARY TABLE tpr AS
  SELECT user_id, CASE WHEN SUM(customized) > 0 THEN 0 WHEN loggedin <> 0 THEN 1
    ELSE 2 END AS status
  FROM tcl
  GROUP BY user_id, loggedin;
",$wces,__FILE__, __LINE__);

$profcount = pg_result(pg_go("SELECT COUNT(*) FROM tpr",$wces,__FILE__,__LINE__), 0, 0);
$profcustomized = pg_result(pg_go("SELECT COUNT(*) FROM tpr WHERE status = 0", $wces, __FILE__, __LINE__), 0, 0);
$profloggedin = pg_result(pg_go("SELECT COUNT(*) FROM tpr WHERE status = 1", $wces, __FILE__, __LINE__), 0, 0);

?>

<h3>Aggregate Usage</h3>

Number of professors: <b><?=(int)$profcount?></b><br>
Number of professors who logged in during the customization period: <b><?=(int)$profloggedin?></b><br>
Number of professors who created custom surveys: <b><?=(int)$profcustomized?></b><br>
<img src="<?=$wces_path?>media/graphs/pusagegraph.php?neverloggedin=<?=(int)$profcount-(int)$profloggedin?>&custom=<?=(int)$profcustomized?>&nocustom=<?=(int)$profloggedin-(int)$profcustomized?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/pusagelegend.gif" width=133 height=49><br>
<p>&nbsp;</p>

<h3>Individual Class Usage</h3>

<?

$result = pg_go("
  SELECT tst.status, tcl.class_id, tcl.students, tcl.lastname, tcl.firstname, tcl.class_info, tcl.user_id,
    to_char(tcl.lastlogin,'YYYY-MM-DD') AS lastlogin
  FROM tcl INNER JOIN tst USING (class_id)
  ORDER BY tst.status, tcl.class_info, tcl.class_id 
", $wces, __FILE__, __LINE__);

pg_go("DROP TABLE tcl; DROP TABLE tst; DROP TABLE tpr", $wces, __FILE__, __LINE__);

$r =& new pg_segmented_wrapper($result, array('status', 'class_id'));

print("<table cellspacing=1 cellpadding=1 border=1>\n"); 
print("<tr><td><b>Code</b></td><td><b>Name</b></td><td><b>Size</b></td><td><b>Professor</b></td><td><b>Last Login</b></td></tr>\n");
while ($r->row)
{
  if ($r->split[0])
  {
    print("<tr><td colspan=5><strong><font color=\"#0030E7\">");
    if ($r->row['status'] == 0)
      print("Classes with custom surveys:");
    else if ($r->row['status'] == 1)
      print("Classes with professors who have recently logged in:");
    else
      print("Classes without professors who have recently logged in:");    
    print("</font></strong></td></tr>\n");
  }
  
  if ($r->split[1])
  {
    $rows = 0;
    $pr = '';
  }
  else 
    $pr .= '<tr>';
  
  ++$rows;
  
  extract($r->row);
  $pr .= "<td><a href=\"{$wces_path}administrators?user_id=$user_id\">$firstname $lastname</a></td>";
  $pr .= $lastlogin ? "<td>$lastlogin</td>" : "<td><i>never</i></td>";
  $pr .= "</tr>\n";
  
  $r->advance();

  if ($r->split[1])
  {
    $rows = $rows == 1 ? '' : " rowspan=$rows";
    print("<tr><td$rows>");
    print(format_class($class_info, "%c %s"));
    print("</td><td$rows>");
    print(format_class($class_info, "%n"));
    print("</td><td$rows>$students</td>");
    print($pr);
  }
}

print("</table>\n");

page_bottom();
?>

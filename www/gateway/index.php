<?

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_pagebreak.inc");

require_once("wbes/server.inc");
require_once("wbes/surveywidget.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

LoginProtect(LOGIN_ANY);

$user_id = LoginValue('user_id');

param('topic_id');
$topic_id = (int)$topic_id;

if ($topic_id)
{
  wces_connect();

  $r = pg_go("
    SELECT t.item_id, t.specialization_id, d.department_name, d.department_people
    FROM gateway_topics AS t
    NATURAL JOIN gateway_departments AS d
    WHERE topic_id = $topic_id", $wces, __FILE__, __LINE__);
  if (pg_numrows($r) != 1)
    $topic_id = 0;
  else
    $topic_info = pg_fetch_row($r, 0, PGSQL_ASSOC);      
}


$q = $msg = false;
if ($topic_id)
{
  $r = pg_result(pg_go("SELECT EXISTS(SELECT * FROM survey_responses WHERE topic_id = $topic_id AND user_id = $user_id)", $wces, __FILE__, __LINE__),0,0);
  if ($r == 'f')
  {
    $f =& new Form("f");
    $q =& new SurveyWidget($topic_info['item_id'], $topic_info['specialization_id'], $user_id, $topic_id, 'editor', $f);
    $q->ta = false;
    $q->replacements = array
    (  '%DEPARTMENT%' => $topic_info['department_name'],
       '%DEPARTMENT_PEOPLE%' => $topic_info['department_people']
    );
    $f->loadState();    
  }
  else
  {
    $msg = "<p><font color=red>You have already filled out this survey.</font></p>";
    unset($s); $s = null;
  }
}

page_top("Survey");

$main = true;

if ($q)
{
  if ($q->done)
    $msg = $q->message;
  else
  {
    print("<form name=f method=post>");
    $f->display();
    $q->display();
    print("</form>");
    $main = false;
  }
}

if ($main)
{
  if ($msg) print($msg);
  print("<p>Choose a survey to complete:</p>");
 
  wces_connect();
  $r = pg_go("
    SELECT d.gateway_department_id, s.parent, t.topic_id, d.department_name, EXISTS (SELECT * FROM survey_responses WHERE topic_id = t.topic_id AND user_id = $user_id) AS done,
      t.end_date < (SELECT NOW()) AS expired
    FROM gateway_topics AS t
    NATURAL JOIN gateway_departments AS d
    INNER JOIN specializations AS s ON s.specialization_id = t.specialization_id
    WHERE t.year = $year AND t.semester = $semester AND NOT t.hidden AND t.begin_date < (SELECT NOW())
    ORDER BY d.ordinal, d.gateway_department_id, s.parent
  ", $wces, __FILE__, __LINE__);
  
  $result = new pg_segmented_wrapper($r, 'gateway_department_id');
  
  while ($result->row)
  {
    if ($result->split)
    {
      print("<h4>{$result->row['department_name']}</h4>\n<ul>\n");
    }

    $sname = $topics[$result->row['parent']];

    if ($result->row['done'] == 't')
      print("  <li>$sname (Already Completed)</li>\n");
    else if ($result->row['expired'] == 't')
      print("  <li>$sname (No longer available)</li>\n");
    else  
      print("  <li><a href=\"$_SERVER[PHP_SELF]?topic_id={$result->row['topic_id']}$ASID\">$sname</a></li>\n");

    $result->advance();
    
    if ($result->split)
    {
      print("</ul>\n");
    }    
  }
  if ($result->rows == 0)
    print("<p><i>No surveys are currently available.</i></p>\n"); 

}  

page_bottom();

?>

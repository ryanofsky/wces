<?

require_once("wbes/server.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("wces/wces.inc");
require_once("wbes/surveywidget.inc");
require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_pagebreak.inc");
require_once("wces/component_abet.inc");
require_once("wces/oldquestions.inc");

login_protect(login_student);

param('class_id');

wces_connect();

$user_id = login_getuserid();
$class_id = (int)$class_id;

list($question_period_id, $survey_listing) = get_surveys();
$n = pg_numrows($survey_listing);
$notFound = true;
for ($i = 0; $i < $n; ++$i)
{
  $data = pg_fetch_row($survey_listing, $i, PGSQL_ASSOC);
  if ($data['class_id'] == $class_id)
  {
    if ($data['surveyed'])
      die ("You already filled out a survey for " . format_class($data['name']));
    $notFound = false;
    break; 
  }
}
if ($notFound) die ("You can't fill out a survey for class #$class_id");

$topic_id = (int)$data['topic_id'];
$base_branch_id = get_base($topic_id);

$factories = array
(
  new ChoiceFactory,
  new TextResponseFactory,
  new TextFactory,
  new HeadingFactory,
  new PageBreakFactory,
  new AbetFactory
);
$f =& new Form();
$q =& new SurveyWidget($topic_id, $base_branch_id, $user_id, $question_period_id, $factories, 'survey', $f);
$q->ta =& new TASurvey('ta', $q);
$f->loadState();

if ($q->done)
{
  if ($q->failure) exit();
  redirect("{$wces_path}index.php$QSID");
}
else
{
  page_top("Student Survey");
  $r = pg_go("SELECT get_profs($class_id)", $wces, __FILE__, __LINE__);
  $class = format_class($data['name']);
  $prof = format_profs(pg_result($r,0,0), false, "<br>Professor ");

  print("<h3>$class$prof</h3>");

  print("<form name=f method=post>");
?>
<script>
<!--
  
  function handleSubmit()
  {
    window.onbeforeunload = null;
  }
  
  function handleBeforeUnload()
  {
    return "Your survey responses will not saved."; 
  }
  
  window.onbeforeunload = handleBeforeUnload
  document.forms.f.onsubmit = handleSubmit;
// -->
</script>
<?
  print($ISID);
  $f->display();
  $q->display();
  print("</form>");
  page_bottom();
}

?>
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
require_once("config/survey.inc");

LoginProtect(LOGIN_STUDENT);

param('topic_id');

wces_connect();

$user_id = LoginValue('user_id');

$topic_id = (int)$topic_id;

$survey_listing = get_surveys();

$n = pg_numrows($survey_listing);
for ($i = 0; $i < $n; ++$i)
{
  $row = pg_fetch_row($survey_listing, $i, PGSQL_ASSOC);
  if ($row['topic_id'] == $topic_id)
  {
    $data = $row;
    if ($data['surveyed'])
      die ("You already filled out a survey for " . format_class($data['name']));
    break; 
  }
}
if (!isset($data)) die ("You can't fill out a survey for class #$topic_id");

$specialization_id = (int)$data['specialization_id'];
$item_id = (int)$data['item_id'];
$topic_id = (int)$data['topic_id'];

$f =& new Form();

$q =& new SurveyWidget($item_id, $specialization_id, $user_id, $topic_id, 'survey', $f);
//$q->ta =& new TASurvey('ta', $q);
$f->loadState();

if ($q->done)
{
  if ($q->failure) exit();
  redirect("{$wces_path}index.php$QSID");
  exit();
}
else
{
  $seconds = 3600;
  session_cache_limiter('public');
  header('Cache-Control: public');
  header('Expires: ' . gmdate('D, d M Y H:i:s', time() + $seconds) . ' GMT');
  header('Last-Modified: ' . gmdate('D, d M Y H:i:s', getlastmod()) . ' GMT');

  page_top("Student Survey");
  $r = pg_go("SELECT get_profs($data[class_id])", $wces, __FILE__, __LINE__);
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
    return "If you do, your survey responses will not be saved."; 
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

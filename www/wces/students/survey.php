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

param($class_id);
param($save);
param($question_period_id);
wces_connect();

if ($class_id && $question_period_id)
{
  $user_id = login_getuserid();
  $class_id = (int)$class_id;
  $question_period_id = (int)$question_period_id;

  $result = pg_go("
    SELECT t.topic_id, EXISTS (SELECT * FROM survey_responses AS sr WHERE sr.topic_id = t.topic_id AND sr.question_period_id = qt.question_period_id AND sr.user_id = $user_id)
    FROM enrollments AS e
    INNER JOIN wces_topics AS t USING (class_id)
    INNER JOIN question_periods_topics AS qt ON qt.question_period_id = $question_period_id AND qt.topic_id = t.topic_id
    WHERE e.class_id = $class_id AND e.user_id = $user_id AND e.status = 1
  ", $wces, __FILE__, __LINE__);

  if (pg_numrows($result) == 1 && pg_result($result,0,1) == 'f')
  {
    $topic_id = (int)pg_result($result,0,0);

    $factories = array
    (
      new ChoiceFactory(),
      new TextResponseFactory(),
      new TextFactory(),
      new HeadingFactory(),
      new PageBreakFactory(),
      new AbetFactory(),
      new NewAbetFactory(),
      new BioAbetFactory()      
    );
    $q = new SurveyWidget($topic_id, get_base($topic_id), $user_id, $question_period_id, $factories, "prefix","f",WIDGET_POST);
    $q->anonymous = true;
    if ($class_id != 33802 && $class_id != 22753 && $class_id != 33732)
    {
      $ta =& new TASurvey($class_id, "prefix-ta", "f", WIDGET_POST);
      $q->ta =& $ta;
    }
    $q->loadvalues();
  }
  else
    $class_id = 0;
}

if ($class_id)
{
  if ($q->finished)
  {
    if ($q->failure) exit();
    redirect("{$wces_path}index.php$QSID");
  }
  else
  {
    $seconds = 3600;
    session_cache_limiter('public');
    header('Cache-Control: public');
    header('Expires: ' . gmdate('D, d M Y H:i:s', time() + $seconds) . ' GMT');
    header('Last-Modified: ' . gmdate('D, d M Y H:i:s', getlastmod()) . ' GMT');

    page_top("Student Survey");
    $r = pg_go("SELECT get_class($class_id), get_profs($class_id)", $wces, __FILE__, __LINE__);

    $class = format_class(pg_result($r,0,0));
    $prof = format_profs(pg_result($r,0,1), false, "<br>Professor ");

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
    $q->display();
    print("</form>");
    page_bottom();
  }
}
else
  print("Invalid Class ID.");

page_bottom();

?>
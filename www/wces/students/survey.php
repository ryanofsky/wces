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

login_protect(login_student);

param($class_id);
param($save);

wces_connect();

if ($class_id)
{
  $question_period_id = (int) pg_result(pg_query("SELECT get_question_period()", $wces, __FILE__, __LINE__),0,0);
  $user_id = login_getuserid();
  $class_id = (int)$class_id;
  $result = pg_query("
      SELECT t.topic_id, EXISTS (SELECT * FROM survey_responses AS sr WHERE sr.topic_id = t.topic_id AND sr.question_period_id = $question_period_id AND sr.user_id = $user_id)
      FROM wces_topics AS t
      INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = t.class_id AND e.status = 1
      WHERE e.class_id = $class_id
  ", $wces, __FILE__, __LINE__);

  if(pg_numrows($result) == 1 && pg_result($result,0,1) == 'f')
  {
    $topic_id = (int)pg_result($result,0,0);
    $factories = array
    (
      new ChoiceFactory(),
      new TextResponseFactory(),
      new TextFactory(),
      new HeadingFactory(),
      new PageBreakFactory(),
      new AbetFactory()
    );
    $q = new SurveyWidget($topic_id, 1, $user_id, $question_period_id, $factories, "prefix","f",WIDGET_POST);
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
    page_top("Student Survey");
    $r = pg_query("SELECT get_class($class_id), get_profs($class_id)", $wces, __FILE__, __LINE__);

    $class = format_class(pg_result($r,0,0));
    $prof = format_profs(pg_result($r,0,1), false, "<br>Professor ");

    print("<h3>$class$prof</h3>");

    print("<form name=f method=post>");
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
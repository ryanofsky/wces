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
      new PageBreakFactory()
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
    
    $result = pg_query(" 
      SELECT (s.code || to_char(c.code::int4,'000') || ' ' || c.name) AS classname, p.firstname, p.lastname 
      FROM classes AS cl 
      INNER JOIN courses AS c USING (course_id) 
      INNER JOIN subjects AS s USING (subject_id) 
      LEFT JOIN enrollments AS e ON e.status = 3 AND e.class_id = cl.class_id 
      LEFT JOIN users AS p ON e.user_id = p.user_id 
      WHERE cl.class_id = $class_id 
    ", $wces, __FILE__, __LINE__); 
     
    $n = pg_numrows($result); 
    $class = $prof = ""; 

    for($i = 0; $i < $n; ++$i) 
    { 
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC)); 
      $class = $classname; 
      if ($firstname || $lastname) 
      { 
         $prof .= "<br>Professor " . trim("$firstname $lastname"); 
      } 
    } 
    print("<h3>$class$prof</h3>"); 

    print("<form name=f method=post>");
    $q->display();
    print("</form>");
    page_bottom();
  }
}
else
  print("Invalid Class ID.");

page_bottom();

?>

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

// this file has functions to delete responses
// and to load responses from the safe_backup
// table into the main database

//DebugBreak();

//$db_debug = true;

function delete_response($response_id, $depth)
{
  global $wbes;

  print("$response_id ");

  pg_go("DELETE FROM responses WHERE response_id = $response_id", $wbes, __FILE__, __LINE__);
  $r = pg_go("SELECT DISTINCT response_id FROM responses WHERE parent = $response_id", $wbes, __FILE__, __LINE__);
  
  $n = pg_numrows($r);
  
  for ($i = 0; $i < $n; ++$i)
  {
    $child = (int)pg_result($r, $i, 0);
    delete_response($child, $depth + 1); 
  }
}

function cleanRuined($question_period_id, $category_id)
{
  global $wbes;
  wbes_connect();
  
  pg_go("
    DELETE FROM survey_responses WHERE question_period_id = $question_period_id
    AND response_id IS NULL
    AND topic_id IN (SELECT topic_id FROM wces_topics WHERE category_id = $category_id)
  ", $wbes, __FILE__, __LINE__);
  
  $r = pg_go("
    SELECT DISTINCT r.response_id
    FROM wces_topics AS t
    INNER JOIN survey_responses AS r ON r.topic_id = t.topic_id AND r.question_period_id = $question_period_id
    WHERE t.category_id = $category_id
  ", $wbes, __FILE__, __LINE__);
  
  $n = pg_numrows($r);
  for ($i = 0; $i < $n; ++$i)
  {
    $response_id = (int)pg_result($r, $i, 0);
    print("<h5>Deleting base response $response_id</h5>");
    flush();
    delete_response($response_id, 0);
  }
}

function loadRuined($question_period_id, $category_id, &$factories)
{
  global $wbes;
  wbes_connect();
  
  $r = pg_go("
    SELECT b.user_id, b.topic_id, b.date, b.form_vals
    FROM safe_backup AS b
    INNER JOIN wces_topics AS t ON t.topic_id = b.topic_id AND t.category_id = $category_id
    WHERE question_period_id = $question_period_id
    ORDER BY b.topic_id
  ", $wbes, __FILE__, __LINE__);

  $q = new SurveyWidget(0, 1, 0, $question_period_id, $factories, "prefix", "f", WIDGET_POST);
  $q->backup = false;
  $GLOBALS['db_debug'] = true;
  $n = pg_numrows($r);
  for ($i = 4; $i < $n; ++$i)
  {
    extract(pg_fetch_row($r, $i, PGSQL_ASSOC));
    $GLOBALS['HTTP_POST_VARS'] = unserialize($form_vals);

    $p = &$GLOBALS['HTTP_POST_VARS'];
    foreach(array_keys($p) as $k)
      $p[$k] = stripslashes($p[$k]);

    $q->survey = unserialize($GLOBALS['HTTP_POST_VARS']['prefix_survey']);
    $q->topic_id = $topic_id;
    $q->user_id = $user_id;
    print("orig_id = " . $q->survey->orig_id . "<br>");


  $q->loadValues();
//  $q->display();    

    print("[$i / $n] user_id = $user_id, topic_id = $topic_id<br>\n");  
    flush();
  }
}

$factories = array
(
  new ChoiceFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory(),
  new PageBreakFactory(),
  new AbetFactory()
);

wbes_connect();

//$db_debug = true;
wbes_connect();
//pg_go("BEGIN", $wbes, __FILE__, __LINE__);
//cleanRuined(1, 100);
loadRuined(1, 100, $factories);
//pg_go("COMMIT", $wbes, __FILE__, __LINE__);

?>
<?

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_pagebreak.inc");
require_once("wces/component_abet.inc");

require_once("wbes/server.inc");
require_once("wbes/surveyeditor.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

login_protect(login_administrator);

$pagename = $server_url->toString(false, true, false);

param($topic_id);
param($bbid);
if ($bbid) $bbid = (int)$bbid; else $bbid = 1;
$question_period_id = 23;

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

if($topic_id)
{
  $q = new SurveyEditor($topic_id, $bbid, login_getuserid(), $factories, "prefix","f",WIDGET_POST);
  $q->loadvalues();
}
else 
  $q = false;

$survey_debug = 0;

if ($q && $q->barepage) 
{
?>
<head>
<title>Preview Window</title>
<LINK REL="stylesheet" type="text/css" href="<?=$server_media?>/style.css">
</head>
<?
}
else
{
  if ($q) $q->dumpscript();
  page_top("Survey Builder");
}  

if ($q)
{
  print("<form name=f method=post>");
  $q->display();
  print("</form>");

}

function topics_link($result, $class = false)
{
  global $pagename, $ASID;
  $n = pg_numrows($result);
  print("<ul>\n");
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    if ($class) $name = format_class($name);
    print("  <li><a href=\"$pagename?topic_id=$topic_id$ASID\">$name</a></li>\n");
  }
  print("</ul>\n");
}

if (!$q || $q->state == SurveyEditor_done)
{
  if ($q) print($q->message);
  
  wces_connect();
  
  print("<p>Choose a survey to edit.</p>");
  
  $result = pg_go("
    --SELECT topic_id, 'Base Questions' AS name 
    --FROM wces_topics WHERE category_id IS NULL AND class_id IS NULL
    SELECT * FROM
    (
      SELECT 1 AS topic_id, 'Base Questions' AS name, 1 AS ordinal
      UNION
      SELECT 4396, 'Base SEAS Questions', 2      
      UNION      
      SELECT 3736, 'Base Engineering Questions', 3
      UNION
      SELECT 3733, 'Base Bioengineering Questions', 4
    ) AS t ORDER BY ordinal
   ", $wces, __FILE__, __LINE__);
    
    
  topics_link($result);
  
  print("<h5>Individual Classes</h5>\n");

  $result = pg_go("
    SELECT t.topic_id, get_class(t.class_id) AS name
    FROM question_periods AS q
    INNER JOIN question_periods_topics AS qt USING (question_period_id)
    INNER JOIN wces_topics AS t USING (topic_id)
    WHERE q.enddate > (select now()) AND t.class_id IS NOT NULL AND t.category_id IS NOT NULL AND t.category_id <> 103
    ORDER BY name
  ", $wces, __FILE__, __LINE__);
  topics_link($result, true);

}

if (!$q || !$q->barepage)
  page_bottom();

print("<pre>")


?>

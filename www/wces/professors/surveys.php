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

login_protect(login_professor);

$pagename = $server_url->toString(false, true, false);

param($topic_id);

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
  $q = new SurveyEditor($topic_id, get_base($topic_id), login_getuserid(), $factories, "prefix","f",WIDGET_POST);
  $q->loadvalues();
}
else 
  $q = false;


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
  global $pagename;
  $n = pg_numrows($result);
  print("<ul>\n");
  if ($n > 0)
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    if ($class) $name = format_class($name);
    print("  <li><a href=\"$pagename?topic_id=$topic_id\">$name</a></li>\n");
  }
  else
    print("<li><i>No classes found</i></li>");
  print("</ul>\n");
}

if (!$q || $q->state == SurveyEditor_done)
{
  if ($q) print($q->message);
  
  $user_id = login_getuserid();
  wces_connect();
  
  $result = pg_go("
    SELECT t.topic_id, get_class(t.class_id) AS name
    FROM question_periods AS q
    INNER JOIN question_periods_topics AS qt USING (question_period_id)
    INNER JOIN wces_topics AS t USING (topic_id)
    INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = t.class_id AND e.status = 3
    INNER JOIN classes AS cl ON cl.class_id = e.class_id
    WHERE q.enddate > (select now()) AND t.category_id IS NOT NULL
    ORDER BY name
  ", $wces, __FILE__, __LINE__);

  print("<p>Choose a survey to edit:</p>");
  topics_link($result, true);
}

if (!$q || !$q->barepage)
  page_bottom();

print("<pre>")


?>
<?

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_pagebreak.inc");

require_once("wbes/server.inc");
require_once("wbes/surveyeditor.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

login_protect(login_administrator);

$pagename = $server_url->toString(false, true, false);

param($topic_id);

$factories = array
(
  new ChoiceFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory(),
  new PageBreakFactory()
);

if($topic_id)
{
  $q = new SurveyEditor($topic_id, 1, $factories, "prefix","f",WIDGET_POST);
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
  page_top("Survey Builder");
  if ($q) $q->dumpscript();
}  

if ($q)
{
  print("<form name=f method=post>");
  $q->display();
  print("</form>");

}

function topics_link($result)
{
  global $pagename;
  $n = pg_numrows($result);
  print("<ul>\n");
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    print("  <li><a href=\"$pagename?topic_id=$topic_id\">$name</a></li>\n");
  }
  print("</ul>\n");
}

if (!$q || $q->state == SurveyEditor_done)
{
  if ($q) print($q->message);
  
  wces_connect();
  
  print("<p>Choose a survey to edit. Changes to surveys made in general categories propogate downward to surveys beneath them. For example, changes made to the base questions show up in the surveys for groups of classes and for individual classes. Changes made to a survey for a group of classes will show up in the surveys for all the classes in that group.</p>");
  
  $result = pg_query("SELECT topic_id, 'Base Questions' AS name FROM wces_topics WHERE category_id IS NULL AND class_id IS NULL", $wces, __FILE__, __LINE__);
  topics_link($result);
  
  print("<h5>Class Groupings</h5>\n");
  
  $result = pg_query("
    SELECT t.topic_id, c.name
    FROM wces_topics AS t
    INNER JOIN survey_categories AS c ON c.survey_category_id = t.category_id
    WHERE t.category_id IS NOT NULL AND t.class_id IS NULL
  ", $wces, __FILE__, __LINE__);
  topics_link($result);
  
  print("<h5>Individual Classes</h5>\n");
  
  $result = pg_query("
    SELECT t.topic_id, (s.code || to_char(c.code::int4,'000') || ' ' || c.name) AS name
    FROM wces_topics AS t
    INNER JOIN classes AS cl USING (class_id)
    INNER JOIN courses AS c USING (course_id)
    INNER JOIN subjects AS s USING (subject_id)
    WHERE t.category_id IS NOT NULL AND t.class_id IS NOT NULL
    ORDER BY s.code, c.code
  ", $wces, __FILE__, __LINE__);
  topics_link($result);

}

if (!$q || !$q->barepage)
  page_bottom();

print("<pre>")


?>

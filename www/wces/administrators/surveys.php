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
  $q = new SurveyEditor($topic_id, 1, login_getuserid(), $factories, "prefix","f",WIDGET_POST);
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

define("LINK_LITERAL", 0);
define("LINK_CLASS", 1);
define("LINK_COURSE", 2);

function topics_link($result, $type = LINK_LITERAL)
{
  global $pagename;
  $n = pg_numrows($result);
  print("<ul>\n");
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
    if ($type == LINK_CLASS)
      $name = format_class($name);
    else if ($type == LINK_COURSE)
      $name = format_course($name);
    $style = $modified == 't' ? 'style="list-style-type: circle"' : 'style="list-style-type: disc"';
    print("  <li $style><a href=\"$pagename?topic_id=$topic_id\">$name</a></li>\n");
  }
  print("</ul>\n");
}

if (!$q || $q->state == SurveyEditor_done)
{
  if ($q) print($q->message);
  
  wces_connect();
  
  print("<p>Choose a survey to edit. Changes to surveys made in general categories propogate downward to surveys beneath them. For example, changes made to the base questions show up in the surveys for courses and for individual class sections. Changes made to a survey for a course will show up in surveys for that course's class sections.</p>");
  
  $result = pg_query("SELECT topic_id, 'Base Questions' AS name, 't' AS modified FROM wces_topics WHERE course_id IS NULL AND class_id IS NULL", $wces, __FILE__, __LINE__);
  topics_link($result, LINK_LITERAL);
  
  print("<h5>Courses</h5>\n");
  
  $result = pg_query("
    SELECT t.topic_id, get_course(t.course_id) as name, 
      topic_modified(t.topic_id) AS modified
    FROM wces_topics AS t
    WHERE t.class_id IS NULL and t.course_id IS NOT NULL
    ORDER BY name
  ", $wces, __FILE__, __LINE__);
  topics_link($result, LINK_COURSE);

  print("<h5>Class Sections</h5>\n");

  $result = pg_query("
    SELECT question_period_id, displayname, year, semester
    FROM semester_question_periods
    WHERE question_period_id = (SELECT get_anext_question_period())
  ", $wces, __FILE__, __LINE__);
  extract(pg_fetch_array($result,0,PGSQL_ASSOC));
  
  $result = pg_query("
    SELECT t.topic_id, get_class(t.class_id) AS name, topic_modified(t.topic_id) AS modified
    FROM wces_topics AS t
    INNER JOIN classes AS cl USING (class_id)
    WHERE t.category_id IS NOT NULL AND cl.year = $year AND cl.semester = $semester
    ORDER BY name
  ", $wces, __FILE__, __LINE__);
  topics_link($result, LINK_CLASS);

}

if (!$q || !$q->barepage)
  page_bottom();

print("<pre>")


?>

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
  new AbetFactory()
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
  if ($n > 0)
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
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
  
  $result = pg_query("
    SELECT question_period_id, displayname, year, semester
    FROM semester_question_periods
    WHERE question_period_id = (SELECT get_question_period())
  ", $wces, __FILE__, __LINE__);
  extract(pg_fetch_array($result,0,PGSQL_ASSOC));
  
  
  $result = pg_query("
    SELECT t.topic_id, (s.code || ' ' || c.divisioncode || c.code || ' ' || cl.section || ' ' || c.name) AS name
    FROM wces_topics AS t
    INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = t.class_id AND e.status = 3
    INNER JOIN classes AS cl ON cl.class_id = e.class_id
    INNER JOIN courses AS c USING (course_id)
    INNER JOIN subjects AS s USING (subject_id)
    WHERE t.category_id IS NOT NULL AND t.class_id IS NOT NULL AND cl.year = $year AND cl.semester = $semester
    ORDER BY s.code, c.code
  ", $wces, __FILE__, __LINE__);
  
  print("<p>Choose a survey to edit:</p>");
  topics_link($result);
}

if (!$q || !$q->barepage)
  page_bottom();

print("<pre>")


?>
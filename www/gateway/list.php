<?

require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/database.inc");
require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/survey.inc");
require_once("wces/database.inc");
require_once("widgets/basic.inc");

require_once("wces/report_page.inc");
require_once("wces/report_generate.inc");

wces_connect();

$r = pg_go("
    SELECT s.response_id
    FROM survey_responses AS s
    INNER JOIN dp_response_saves AS v USING (response_id)
    WHERE s.question_period_id IN (19, 99902) AND v.save_id = 302 AND s.topic_id = 3570
    ORDER BY s.date
", $wces, __FILE__, __LINE__);

$n = pg_numrows($r);

for ($i = 0; $i < $n; ++$i)
{
  $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
  print("UPDATE survey_responses SET topic_id = 3569 WHERE topic_id = 3570 AND response_id = $row[response_id];<br>");
}

?>

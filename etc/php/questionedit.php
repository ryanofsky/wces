<?

/*

put this stuff in the database:

TRUNCATE branches;
TRUNCATE revisions;
INSERT INTO branches (branch_id, branch) VALUES (1,1);
INSERT INTO revisions (revision_id, type, branch_id, revision) VALUES (1,1,1,1);
SELECT branch_generate();

*/

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");

require_once("wbes/server.inc");
require_once("wbes/surveyeditor.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

$factories = array
(
  new ChoiceFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory()
);

$q = new SurveyEditor(1, $factories, "prefix","f",WIDGET_POST);
$q->loadvalues();

if ($q->barepage) 
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
  $q->dumpscript();
}  

print("<form name=f method=post>");
$q->display();
print("</form>");

if ($q->state == SurveyEditor_done)
{
  print($q->message);
  print("<p align=center><a href=questionedit.php>Edit Survey Again</a></p>\n");
}

if (!$q->barepage) page_bottom();
print("<pre>");

?>
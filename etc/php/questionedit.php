<?

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");

require_once("wbes/server.inc");
require_once("wbes/surveyeditor.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

print("<center><a href=questionedit.php>Reset</a></center><hr>\n");

$q = new SurveyEditor(1, array("Choice","TextResponse","Text","Heading"), "prefix","f",WIDGET_POST);
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
}

if (!$q->barepage) page_bottom();

?>
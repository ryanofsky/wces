<?

require_once("wbes/surveyeditor.inc");
require_once("gk12.inc");

$q = new SurveyEditor("prefix","f",WIDGET_POST);
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

if (!$q->barepage) page_bottom();

?>
<?

require_once("wbes/surveyeditor.inc");
require_once("page.inc");
require_once("wces/wces.inc");

$q = new SurveyEditor("prefix","f",WIDGET_POST);
$q->topic_id = $topic_id;
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
  print("Return to the <a href=\"{$topic_path}index.php\">Home Page</a> or the <a href=\"{$topic_path}builder.php\">Survey Builder</a>");
}

if (!$q->barepage) page_bottom();

?>
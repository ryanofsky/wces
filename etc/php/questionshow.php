<?

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_pagebreak.inc");

require_once("wbes/server.inc");
require_once("wbes/surveywidget.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

$factories = array
(
  new ChoiceFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory(),
  new PageBreakFactory()
);

$q = new SurveyWidget(1, 1, 1, 9, $factories, "prefix","f",WIDGET_POST);
$q->loadvalues();

page_top("Survey");
print("<form name=f method=post>");
if ($q->finished)
  print('<a href="questionshow.php">Show survey again</a>');
else
  $q->display();
print("</form>");  

page_bottom();    

?>
<?

require_once("wces/page.inc");
require_once("wces/questions.inc");
require_once("wces/questioneditor.inc");

page_top("hello joe");
print("<form name=f method=post>");

$q = new QuestionSetEditor(0,0,"prefix","f",WIDGET_POST);
$q->loadvalues();
$q->display();

print("</form>");

page_bottom();

?>
<?

require_once("wces/general.inc");
require_once("test/test.inc");
require_once("wces/questions.inc");
require_once("wces/questioneditor.inc");

test_top("Student Home","professor");

print("<form name=f method=post>");

$q = new QuestionSetEditor(0,0,"prefix","f",WIDGET_POST);
$q->dumpscript();
$q->loadvalues();
$q->display();

print("</form>");

test_bottom();

?>

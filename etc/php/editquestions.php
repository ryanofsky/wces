<%

require_once("page.inc");
require_once("questions.inc");

page_top("Question Editor");

$questionsetid = 1;

$q = new QuestionSet();

$q->addquestionbefore(false,"welcomme");
$q->addquestionbefore(false,"ok");
$q->addquestionbefore(false,"ha haw");
$q->addquestionafter(false,"last");

$q->deletequestion(2);

print("<table>\n");
foreach($q->questions as $key => $value)
  print("<tr><td>$key</td><td>$value</td></tr>");
print("</table>");  

page_bottom()


%>
<?

require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("wces/ProfessorResults.inc");

login_protect(login_professor);

$f =& new Form("f");
$r =& new ProfessorResults("results", $f);
$f->loadState();

if (!$r->csv) page_top("Survey Results", $r->printable);
print("<form name=$f->formName method=post action=\"$f->pageName\">");
$f->display();
$r->display();
print("</form>");
if (!$r->csv) page_bottom($r->printable);

?>
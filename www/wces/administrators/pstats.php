<?
require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/profinfo.inc");
login_protect(login_administrator);

$db = wces_connect();
$pi = new ProfessorInfo($db,"info","f",WIDGET_POST);
$pi->loadvalues();
page_top("Professor Information",$pi->printable ? true : false);
print('<form name=f method=post>');
$pi->display();
print('</form>');
page_bottom($pi->printable ? true : false);
?>
<?

require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("legacy/wces/report_widget.inc");

LoginProtect(LOGIN_ADMIN | LOGIN_DEPT_ADMIN);

$db = wces_oldconnect();
 
$report = new legacy_Report("report","wiz",WIDGET_POST);
$report->loadvalues();

page_top("Administrative Report", $report->hidemenus);

print("<form name=wiz method=post>\n");
$report->display();
print("</form>\n");

page_bottom($report->hidemenus); 

?>
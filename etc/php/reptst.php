<%
require_once("page.inc");
require_once("reporting.inc");
require_once("wces.inc");

page_top("test page");

$db = wces_connect();

report_display($db,Array(369),true);

page_bottom();
%>
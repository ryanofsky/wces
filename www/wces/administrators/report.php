<%
 require_once("wces/page.inc");
 require_once("wces/login.inc");
 require_once("wces/reporting.inc");
 login_protect(login_administrator);
 $report = new Report("report","wiz",WIDGET_POST);
 $report->loadvalues();
 page_top("Administrative Report","001",$report->hidemenus);
%>

<form name="wiz" method="post">
<% $report->display(); %>
</form>

<% page_bottom($report->hidemenus); %>
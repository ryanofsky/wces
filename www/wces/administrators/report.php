<%
 require_once("widgets.inc");
 require_once("page.inc");
 require_once("login.inc");
 require_once("reporting.inc");
 login_protect(login_administrator);
 page_top("Administrative Report","001");

$wiz = new ReportOptions("wizard","wiz",WIDGET_POST);
$wiz->loadvalues();

%>
<form name="wiz" method="post">
<input type=hidden name=revivewizard value=0>
<% 
 if (!$wiz->isfinished() || $revivewizard)
   $wiz->display(false);
 else
 {
   print("<p><b><a href=\"javascript:document.forms['wiz']['revivewizard'].value = 1; void(document.forms['wiz'].submit())\">Back to wizard</a></b></p>");
   $wiz->display(true);
   $wiz->makereport();
 }
%></form><%
  page_bottom();
%>
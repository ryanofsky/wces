<%
 require_once("widgets.inc");
 require_once("page.inc");
 require_once("login.inc");
 require_once("reporting.inc");
 login_protect(login_administrator);
 page_top("Administrative Report","001");


print("<pre>\n");
print(texttable(
  
  Array
  (
    Array("Question Text","5","4","3","2","1","Avg","Mode","SD"),
    Array("Magician: Talent",2,4,67,4,5,4.3,2.5,.6),
    Array("Midgets: akjh kjashdf kjhk kjhk kjhk Rotation",2,4,67,4,5,4.3,2.5,.6)
  ),
  Array(20,"center","center","center","center","center","center","center","center")
  
  ));
    
    
  
      


print("</pre>\n");


$wiz = new ReportOptions("wizard","wiz",WIDGET_GET);
$wiz->loadvalues();

%>
<form name="wiz" method="get">
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
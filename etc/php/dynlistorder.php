<%
 require_once("wces/page.inc");
 require_once("widgets/widgets.inc");
 require_once("widgets/dynamiclist.inc");
 require_once("widgets/basic.inc");
 
 page_top("hello?");

 $form = new Form("form", "f", WIDGET_GET);
 $form->loadvalues();
 
 $mc1 = new DynamicList(30,7,array("check","the","cards","at","the","table"),"mc1",f,WIDGET_GET);
 $mc1->loadvalues();
 
%>
<form name="f" method=get>
<% $form->display(); %>
<% $mc1->display(); %>
<input type=submit name=submit value="Submit"><br><br>
</form>
<%
  page_bottom();
%>
<%
 require_once("widgets.inc");
 require_once("page.inc");
 
 page_top("hello?");

 DynamicList::dumpscript();
 $ok = new DynamicList(50,5,0,"test","f",WIDGET_GET);
 $ok->loadvalues();
%>

<h2>But does it work?</h2>

<form name="f" method=get>
<% $ok->display(); %>
<input type=submit name=submit value="Submit">
</form>
<%
  page_bottom();
%>
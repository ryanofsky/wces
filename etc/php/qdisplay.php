<%
 require_once("widgets.inc");
 require_once("page.inc");
 require_once("questions.inc");
 page_top("hello?");
 $mc1 = new MultipleChoice("Which is the best?",Array("letters","numbers","carrots","labels"),1,"mc1","f",WIDGET_GET);
 $mc1->loadvalues();
%>
<form name="f" method=get>
<% $mc1->display(); %>


<input type=submit name=submit value="Submit">
</form>
<%
  page_bottom();
%>
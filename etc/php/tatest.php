<%

require_once("wces/page.inc");
require_once("wces/oldquestions.inc");

page_top("hello joe");
print("<form name=f method=post>");

%>

print("<table>\n<tr>\n");
print("  <td><p><strong>Enter a TA Name:</strong><br>\n<font size=-1>(Or leave blank to rate all TA's together)</font></p></td>");
print("  <td><input name=taname size=25></td>\n");
print("</tr>\n</table>\n");

<% foreach($TAQUESTIONS AS $fieldname => $text) { %>

<p><b><%=$text%></b></p>
<blockquote>
  <input type=radio name="q<%=$fieldname%>" id="q<%=$fieldname%>a" value="a" <%=$val == "a" ? "checked" : ""%>><label for="q<%=$fieldname%>a">excellent</label>
  <input type=radio name="q<%=$fieldname%>" id="q<%=$fieldname%>b" value="b" <%=$val == "b" ? "checked" : ""%>><label for="q<%=$fieldname%>b">very good</label>
  <input type=radio name="q<%=$fieldname%>" id="q<%=$fieldname%>c" value="c" <%=$val == "c" ? "checked" : ""%>><label for="q<%=$fieldname%>c">satisfactory</label>
  <input type=radio name="q<%=$fieldname%>" id="q<%=$fieldname%>d" value="d" <%=$val == "d" ? "checked" : ""%>><label for="q<%=$fieldname%>d">poor</label>
  <input type=radio name="q<%=$fieldname%>" id="q<%=$fieldname%>e" value="e" <%=$val == "e" ? "checked" : ""%>><label for="q<%=$fieldname%>e">disastrous</label>
  <input type=radio name="q<%=$fieldname%>" id="q<%=$fieldname%>f" value="f" <%=$val == "e" ? "checked" : ""%>><label for="q<%=$fieldname%>f">n/a</label>
</blockquote>

<% } %>

<p><b>Comments</b></p>
<blockquote><textarea name="comments" rows=8 cols=50 wrap=virtual></textarea></blockquote>


<%

print("</form>");


page_bottom();

%>
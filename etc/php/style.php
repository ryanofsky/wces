<%
 require_once("widgets.inc");
 require_once("page.inc");
 
 page_top("title");
%>

<h2>Heading of type 2</h2>
<h3>Heading of type number three</h3>
<h4>Heading the fourth</h4>
<h5>Heading which is the one after the fourth</h5>
<p>and a paragraph</p>
<blockquote>indented text</blockquote>
<p>it is a good idea</p>
<ul>
<li>that i decided</li>
<li>to list these items</li>
</ul>

<table bgcolor="#CCCCFF" width="100%"><tr><td><strong>Old School Table</strong></td></tr></table>

<%
  page_bottom();
%>
<%

require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_professor);

page_top("Professor Search Page");

if (!$lastname)
{
%>
<p>WCES needs to associate your CUNIX id, '<b><%=login_getuni()%></b>' with a professor listing from our database.</p>
<p>After this has been done once, WCES will remember the association for future logins. Associations can be updated at any time by choosing '<b>Update CUNIX Association</b>' from the Professors menu on the left side of WCES pages.</p>
<p>Type in your last name in the form below to search for your listing.</p>
<%
}
%>
<form name=profsearch action=profsearch.php method=get>
Last Name: <input name="lastname" type="text" size="20" value="<%=htmlspecialchars($lastname)%>"> <input name="search" type="submit" value="Search">
<input type="hidden" name="destination" value="<%=htmlspecialchars($destination)%>">
</form>
<%

if ($lastname)
{
  %>
  <hr><h3>Results</h3>
  <p>Find the listing below that displays your name and classes and follow its 'Use This Listing' link.</p>
  <p><font size=-1">Note: If you see your name listed twice, you should contact an administrator so the listings can be merged. In the meantime, choose the listing with the classes you need to access.</font></p>
  <%

  $db = wces_connect();
  $professors = mysql_query("SELECT professorid, name FROM Professors WHERE name LIKE '%" . addslashes($lastname) . "%' LIMIT 500",$db);
  if (mysql_num_rows($professors) > 0)
  {
    print("<TABLE cellSpacing=0 cellPadding=2 border=1>\n");
    while ($professor = mysql_fetch_array($professors))
    {
      extract($professor);
      print("<TR><TD>$name [<a href=\"profbounce.php?profid=$professorid&destination=$destination\">Use This Listing</a>]<UL>");
      $classes = mysql_query("SELECT cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode FROM Classes as cl LEFT JOIN Courses AS c USING (courseid) LEFT JOIN Subjects AS s USING (subjectid) WHERE cl.professorid = '$professorid' ORDER BY cl.year DESC, cl.semester DESC LIMIT 50",$db);
      while ($class = mysql_fetch_array($classes))
      {
        extract($class);
        print ("<LI>" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</LI>");
      }
      print("</UL><P>&nbsp;</P></TD></TR>\n");
    }
    print("</TABLE>\n");
  }
  else
  {
%>
<p><i>No matches found. Please contact an administrator if you need to have your name added to the database.</i></p>
    
<%
    
    
  };  
};

page_bottom();

%>
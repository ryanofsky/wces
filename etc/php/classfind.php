<%
require_once("page.inc");
page_top("Class Search");
%>


<BODY>
<form method=post name=form1>
<fieldset>
<legend align="top">Find a Class</legend>
  <table>
  <tr><td valign=top><label for="coursename">Course Name:</label></td><td><INPUT type=text name="coursename" id="coursename" size="20" style="width:100%"></td></tr>
  <tr><td>&nbsp;</td><td><font size="-1">Example: Discrete Math</font></td></tr>
  <tr valign=top><td><label for="professor">Professor Name:</label></td><td><input type=text name="professor" id="professor" size="20" style="width:100%">
  <tr><td>&nbsp;</td><td><font size="-1">Example: Ackerman</font></td></tr>
  <tr valign=top><td><label for="sectionkey">Section Key</label></td><td><input type=text name="sectionkey" id="sectionkey" size="20" style="width:100%">
  <tr><td>&nbsp;</td><td><font size="-1">Example: 20003CHEM1405C001</font></td></tr>
  <tr valign=top><td><label for="semester">Semester:</label></td><td><select name="semester" id="semester" style="width:100%"><option>Any Semester</option></select></td></tr>
  <tr valign=top><td><label for="subject">Subject:</label></td><td><select name="subject" id="subject" style="width:100%"><option>Any Subject</option></select></td></tr>
  <tr valign=top><td><label for="department">Department:</label></td><td><select name="department" id="department" style="width:100%"><option>Any Department</option></select></td></tr>
  <tr valign=top><td>&nbsp;</td><td><center><INPUT TYPE="submit" VALUE="Search" name="search"></center></td></tr>
  </table>
  </fieldset>
  </form>
<hr>
<p>

<%
if (isset($coursename) || isset($professor) || isset($sectionkey) || isset($semester) || isset($subject) || isset($department))
{
  print ("<h3>Results:</h3>");
  if ($coursename) $conditions = "";
}
%>    
   
<p><pre>

SELECT cl.classid, cl.professorid, cl.section, cl.year, cl.semester, c.name, p.firstname, p.lastname, s.name
FROM classes AS cl
LEFT JOIN courses AS c ON cl.courseid = c.courseid
LEFT JOIN professors as p ON cl.professorid = p.professorid
LEFT JOIN subjects as s ON s.subjectid = c.subjectid
WHERE c.name LIKE "%discrete math%";
  
</pre></p>

<%
page_bottom();
%></BODY></HTML>

<%

require_once("wces/page.inc");
page_top("ABET");

$p = array(
  "Design experiments",
  "Analyze and interpret data",
  "Conduct experiments",
  "Analyze and interpret data",
  "Design a system, component, or process to meet desired needs",
  "Function on multidisciplinary teams",
  "Identify or formulate engineering problems",
  "Solve engineering problems",
  "Understand ethical responsibilities",
  "Understand the impact of engineering solutions in a global/societal context",
  "Use modern engineering tools",
  "Communicate using oral presentations",
  "Communicate using written reports",
  "Pilot test a component prior to implementation",
  "Use text materials to support project design",
  "Integrate knowledge of mathematics, science, and engineering in engineering solutions",
  "Apply knowledge of contemporary issues to engineering solutions",
  "Recognize need to engage in lifelong learning"
);

%>

<form>

<p>The ABET questions accept a rating from 1-5. You should check off any questions that are applicable to your class so they can be included in the surveys students fill out.</p>
<p>Students will be asked, "<b>To what degree did this course enhance your ability to:</b>"</p>


<table cellpadding=0 cellspacing=2>
<%
  foreach($p as $k => $v)
    print('  <tr><td bgcolor="#DDDDDD" background="' . $server_wcespath . 'media/0xDDDDDD.gif" width="100%"><p style="margin-left: 30px; text-indent: -30px"><input id="q' . $k . '" name="q' . $k . '" type=checkbox style="width: 30px"><label for="q' . $k . '"><b>' . $v . "</b></label></p></td></tr>\n");
%>
</table>

<hr>

<% $choices = array(0 => "a", 1 => "b", 2 => "c", 3 => "d", 4 => "e", 5 => "f"); %>

<table bordercolor=black cellspacing=0 cellpadding=3 RULES="groups" FRAME=box STYLE="border: none">
<thead>
<tr>
<td colspan=2 align=left STYLE="border: none"><b>To what degree did this course enhance your ability to ...</b></td>
<td colspan=9 align=right STYLE="border: none"><b>(0 = <i>not at all</i>, 5 = <i>a great deal</i>)</b></td>
</tr>
</thead>
<tbody>
<tr>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>0</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>1</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>2</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>3</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>4</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>5</STRONG></font></td>
</tr>
<% 
  $row = true;
  foreach($p as $k => $v)
  {
    $row = !$row;
    $color = $row ? 'bgcolor="#FFFFFF" background="' . $server_wcespath . 'media/0xFFFFFF.gif"' : 'bgcolor="#EEEEEE" background="' . $server_wcespath . 'media/0xEEEEEE.gif"';
    print("<tr><td colspan=5 $color>$v</td>");
    foreach($choices as $choice)
      print("<td align=center $color><input name=\"q$k\" value=\"$choice\" type=radio></td>");
    print("</tr>\n");
  }
%>
</tbody>
</table>
</div>
</form>



<%
page_bottom();
%>
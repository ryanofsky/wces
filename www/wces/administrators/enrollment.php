<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");

login_protect(login_administrator);
page_top("Enrollment Viewer"); 
$db = wces_connect();
if ($unilist) $uni = $unilist;
if ($uni)
  $data = db_getrow($db,"Users",Array("cunix" => $uni),0);
  {
    $y = mysql_query("SELECT cl.classid, cl.year, cl.semester, concat(s.code, c.code) AS code, c.name, cl.section, IF(cc.classid,'yes','no') AS survey, e.surveyed FROM Users as u INNER JOIN Enrollments as e USING (userid) INNER JOIN Classes as cl USING (classid) INNER JOIN Courses as c using (courseid) LEFT JOIN Subjects as s using (subjectid) LEFT JOIN currentclasses AS cc ON (cl.classid = cc.classid) WHERE u.cunix = '$uni' ORDER BY cl.year, cl.semester",$db);
    $count = mysql_num_rows($y);
    if ($count > 0)
    {
      print ("<p><b>Classes for '$uni'</b></p>");
      print("<table border=1 cellspacing=0 cellpadding=2>\n");
    else
  }
  else
  {
    print("<p>The user '$uni' does not exist.</p>");
  }
}
%>
<!--
{
  var listtext;
  for (var i = 0; i < list.options.length; ++i)
    if (listtext.length > boxtext.length) listtext = listtext.slice(0,boxtext.length);
      list.selectedIndex = i;
    }
    else if (listtext > boxtext)
    {
      if (i>0) list.selectedIndex = i - 1;
    }

function listchange()

{
  var code;
  if (e && e.which)
    code = e.which;
  else if (window.event)
  else 
    return true;
  if (code > 0) textchange(list,this.value + String.fromCharCode(code));
};

function textonkeydown(e)
{
  if (e && e.which)
    code = e.which;
  else if (window.event)
  else 
    return true;
  if (code == 8)
  {
    var boxtext = this.value.length > 1 ? this.value.slice(0,this.value.length-1) : "";
    textchange(list,boxtext);
  };
};

</script>


<form name=fakelogin method=get>
<table>
<tr>
<td valign=top>Username:</td>
<td>
  print('<select name=unilist size=7 style="width:90px">');
  $cunixes = db_getcolumn($db,"Users",0,"cunix");
  sort($cunixes);
  foreach($cunixes as $cunix)
    print('<option value="' . htmlspecialchars($cunix) . '">' . htmlspecialchars($cunix). '</option>');
  print('</select>');  
%></div>
<tr><td>&nbsp;</td><td><input type=submit value=Submit></td></tr>
</table>
</form>

<script>
  var list = document.forms.fakelogin.unilist;
  list.options[list.options.length - 1] = null;
  box.onkeypress = textonkeypress;
  box.onchange = new Function("return textchange(this.form.unilist,this.value);");
};
<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");

login_protect(login_administrator);
page_top("Enrollment Viewer"); 
$db = wces_connect();

param($unilist);
param($uni);

if ($unilist) $uni = $unilist;
if ($uni)
{
  $data = db_getrow($db,"users",Array("cunix" => $uni),0);
  if ($data)
  {
    $questionperiodid = wces_Findquestionsetsta($db,"qsets");
    $y = db_exec(
    
    "SELECT c.courseid, cl.classid, cl.year, cl.semester, concat(s.code, c.code) AS code, c.name, cl.section, IF(qs.classid,'yes','no') AS survey, IF(cs.userid,'yes','no') AS surveyed
    FROM users as u
    INNER JOIN enrollments as e USING (userid)
    INNER JOIN classes as cl USING (classid)
    INNER JOIN courses as c using (courseid)
    INNER JOIN subjects as s using (subjectid)
    LEFT JOIN qsets AS qs ON (qs.classid = cl.classid)
    LEFT JOIN answersets AS a ON (a.classid = e.classid AND a.questionperiodid = '$questionperiodid' AND a.questionsetid = qs.questionsetid)
    LEFT JOIN completesurveys AS cs ON (cs.userid = u.userid AND cs.answersetid = a.answersetid)
    WHERE u.cunix = '$uni'
    GROUP BY classid
    ORDER BY cl.year DESC, cl.semester DESC",$db,__FILE__,__LINE__);
    
    $count = mysql_num_rows($y);
    if ($count > 0)
    {
      print ("<p><b>Classes for '<a href=\"${server_wcespath}info/ldap.php?uni=$uni\">$uni</a>'</b></p>");
      print("<table border=1 cellspacing=0 cellpadding=2>\n");
      print("  <tr><td><b>Year</b></td><td><b>Semester</b></td><td><b>Course Code</b></td><td><b>Course Name</b></td><td><b>Section</b></td><td><b>Survey Available?</b></td><td><b>Survey Complete?</b></td></tr>\n");
      while($result = mysql_fetch_array($y))
      {
        $courseid = $classid = ""; $year = ""; $semester = ""; $code = ""; $name = "??????"; $section = ""; $survey = ""; $surveyed = "";
        extract($result);
        print("  <tr><td>$year</td><td>$semester</td><td><a href=\"${server_wcespath}info/courseinfo.php?courseid=$courseid\">$code</a></td><td>$name</td><td><a href=\"${server_wcespath}info/classinfo.php?classid=$classid\">$section</a></td><td>$survey</td><td>$surveyed</td></tr>\n");
      };
      print("</table>\n<p><a href=\"?\">Back</a></p>");    
    }  
    else
      print("<p><i>No classes found for '$uni.'</i></p>");
  }
  else
  {
    print("<p>The user '$uni' does not exist.</p>");
  }
}
else
{ %>
<script>
<!--
function textchange(list,boxtext)
{
  var listtext;
  for (var i = 0; i < list.options.length; ++i)
  {
    listtext = list.options[i].text;
    if (listtext.length > boxtext.length) listtext = listtext.slice(0,boxtext.length);
    if (listtext == boxtext)
    {
      list.selectedIndex = i;
      break;
    }
    else if (listtext > boxtext)
    {
      if (i>0) list.selectedIndex = i - 1;
      break;
    }
  }
  return true;
};

function listchange()
{
  this.form.uni.value = this.options[this.selectedIndex].text;
};

function textonkeypress(e)
{
  var code;
  if (e && e.which)
    code = e.which;
  else if (e && e.keyCode)
    code = e.keyCode;
  else if (window.event)
    code = window.event.keyCode;
  else 
    return true;
  var list = this.form.unilist;
  if (code > 0) textchange(list,this.value + String.fromCharCode(code));
};

function textonkeydown(e)
{
  var code;
  if (e && e.which)
    code = e.which;
  else if (e && e.keyCode)
    code = e.keyCode;
  else if (window.event)
    code = window.event.keyCode;
  else 
    return true;

  var list = this.form.unilist;
  if (code == 8)
  {
    var boxtext = this.value.length > 1 ? this.value.slice(0,this.value.length-1) : "";
    textchange(list,boxtext);
  };
};

// -->
</script>

<p>This form allows you to view the class enrollments of a person who has logged in previously.</p>

<form name=fakelogin method=get>
<table>
<tr>
<td valign=top>Username:</td>
<td>
<div><input type=text size=10 name=uni value="" style="width:90px"></div>
<div><%
  print('<select name=unilist size=7 style="width:90px">');
  $cunixes = db_getcolumn($db,"users",0,"cunix");
  sort($cunixes);
  array_push($cunixes,"--------------");
  foreach($cunixes as $cunix)
    print('<option value="' . htmlspecialchars($cunix) . '">' . htmlspecialchars($cunix). '</option>');
  print('</select>');  
%></div>
</td>
</tr>
<tr><td>&nbsp;</td><td><input type=submit value=Submit></td></tr>
</table>
</form>

<script>
<!--
  var list = document.forms.fakelogin.unilist;
  list.onchange = listchange;
  list.options[list.options.length - 1] = null;
  var box = document.forms.fakelogin.uni;
  box.onkeydown = textonkeydown;
  box.onkeypress = textonkeypress;
  box.onchange = new Function("return textchange(this.form.unilist,this.value);");
// -->
</script>

<%
};
page_bottom();
%>







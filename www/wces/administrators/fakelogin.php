<%
require_once("server.inc");
require_once("login.inc");
require_once("page.inc");

login_protect(login_administrator);
$db = wces_connect();if ($uni){
  if ($adduser)
  {
    db_replace($db,"Users",Array("cunix" => $uni),Array
    (
      "isprofessor" => $isprofessor == 1 ? "true" : "false",      "isadmin" => $isadministrator == 1 ? "true" : "false",
    ),"userid");  }  $data = db_getrow($db,"Users",Array("cunix" => $uni),Array("userid","isprofessor","isadmin"));  if ($data)
  {    login_setup(      $db,
      $data["userid"],
      $uni,
      true,
      $data["isprofessor"] == "true" ? true : false,
      $data["isadmin"] == "true" ? true : false
    );
    page_top("Fake Logon Success!");
    print("You are now logged in as '" . login_getuni() . ",' as a " . login_getstatus() . ".<br>Use the links at the left to navigate the site as this user.");
  }  else  {
    page_top("Fake Login Page");
    print("The user '$uni' does not exist.");
    %>    
    <form name=adduser>
    <p>The user '$uni' does not exist.</p>
    <p>Permissions:</p>    <blockquote>
      <input type=checkbox name=isstudent id=isstudent value=1 checked><label for=isstudent>Student</label><br>
      <input type=checkbox name=isprofessor id=isprofessor value=1 checked><label for=isprofessor>Professor</label><br>
      <input type=checkbox name=isadministrator id=isadministrator value=1 checked><label for=isadministrator>Administrator</label>    </blockquote>  
    <input type=hidden name=uni value="<%=$uni%>">    <p><input type=submit name=adduser value="Add '<%=$uni%>' to the database"></p>
    </form>
    <p><a href="fakelogin.php">Back</a></p>
    <%      };
}else{page_top("Fake Login Page"); 
%><script>
<!--function textchange(list,boxtext)
{
  var listtext;
  for (var i = 0; i < list.options.length; ++i)  {    listtext = list.options[i].text;
    if (listtext.length > boxtext.length) listtext = listtext.slice(0,boxtext.length);    if (listtext == boxtext)    {
      list.selectedIndex = i;      break;
    }
    else if (listtext > boxtext)
    {
      if (i>0) list.selectedIndex = i - 1;      break;
    }  }  if (listtext != boxtext) list.selectedIndex = -1;
  return true;};

function listchange(){  this.form.uni.value = this.options[this.selectedIndex].text;};
function textonkeypress(e)
{
  var code;
  if (e && e.which)
    code = e.which;  else if (e && e.keyCode)    code = e.keyCode;
  else if (window.event)    code = window.event.keyCode;
  else 
    return true;  var list = this.form.unilist;
  if (code > 0) textchange(list,this.value + String.fromCharCode(code));
};

function textonkeydown(e)
{  var code;
  if (e && e.which)
    code = e.which;  else if (e && e.keyCode)    code = e.keyCode;
  else if (window.event)    code = window.event.keyCode;
  else 
    return true;  var list = this.form.unilist;
  if (code == 8)
  {
    var boxtext = this.value.length > 1 ? this.value.slice(0,this.value.length-1) : "";
    textchange(list,boxtext);
  };
};
// -->
</script>
This form allows you bypass ACIS authentication and log on as any user. If you type in a username that is unknown, you will have the option of adding that username to the database.

<form name=fakelogin action="fakelogin.php" method=get>
<table>
<tr>
<td valign=top>Username:</td>
<td><div><input type=text size=10 name=uni value="" style="width:90px"></div><div><%
  print('<select name=unilist size=7 style="width:90px">');
  $cunixes = db_getcolumn($db,"Users",0,"cunix");
  sort($cunixes);  array_push($cunixes,"--------------");
  foreach($cunixes as $cunix)
    print('<option value="' . htmlspecialchars($cunix) . '">' . htmlspecialchars($cunix). '</option>');
  print('</select>');  
%></div></td></tr>
<tr><td>&nbsp;</td><td><input type=submit value=Submit></td></tr>
</table>
</form>

<script><!--
  var list = document.forms.fakelogin.unilist;  list.onchange = listchange;
  list.options[list.options.length - 1] = null;  var box = document.forms.fakelogin.uni;  box.onkeydown = textonkeydown;
  box.onkeypress = textonkeypress;
  box.onchange = new Function("return textchange(this.form.unilist,this.value);");// --></script><%
};page_bottom();%>
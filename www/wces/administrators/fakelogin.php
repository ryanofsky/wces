<%
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");

login_protect(login_administrator);

  if ($adduser)
  {
    db_replace($db,"Users",Array("cunix" => $uni),Array
    (
      "isprofessor" => $isprofessor == 1 ? "true" : "false",
    ),"userid");
  {
      $data["userid"],
      $uni,
      true,
      $data["isprofessor"] == "true" ? true : false,
      $data["isadmin"] == "true" ? true : false
    );
    page_top("Fake Logon Success!");
    print("You are now logged in as '" . login_getuni() . ",' as a " . login_getstatus() . ".<br>Use the links at the left to navigate the site as this user.");
  }
    page_top("Fake Login Page");
    print("The user '$uni' does not exist.");
    %>    
    <form name=adduser>
    <p>The user '$uni' does not exist.</p>
    <p>Permissions:</p>
      <input type=checkbox name=isstudent id=isstudent value=1 checked><label for=isstudent>Student</label><br>
      <input type=checkbox name=isprofessor id=isprofessor value=1 checked><label for=isprofessor>Professor</label><br>
      <input type=checkbox name=isadministrator id=isadministrator value=1 checked><label for=isadministrator>Administrator</label>
    <input type=hidden name=uni value="<%=$uni%>">
    </form>
    <p><a href="fakelogin.php">Back</a></p>
    <%    
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
  return true;

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


<form name=fakelogin action="fakelogin.php" method=get>
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
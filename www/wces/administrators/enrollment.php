<?
require_once("wces/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");

login_protect(login_administrator);
page_top("Enrollment Viewer"); 

?>
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
  this.form.box.value = this.options[this.selectedIndex].text;
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
  var list = this.form.cunix;
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

  var list = this.form.cunix;
  if (code == 8)
  {
    var boxtext = this.value.length > 1 ? this.value.slice(0,this.value.length-1) : "";
    textchange(list,boxtext);
  };
};

// -->
</script>

<p>This form allows you to view the class enrollments of a person who has logged in previously.</p>

<form name=f action="info.php" method=get>
<input type=hidden name=surveys value=1>
<table>
<tr>
<td valign=top>Username:</td>
<td>
<div><input type=text size=10 name=box value="" style="width:90px"></div>
<div><?
  print('<select name=cunix size=7 style="width:90px">');
  $db = wces_connect();
  $cunixes = db_exec("SELECT cunix FROM users ORDER BY cunix", $db, __FILE__, __LINE__);
  while($cunix = mysql_fetch_array($cunixes))
    print('<option value="' . htmlspecialchars($cunix[0]) . '">' . htmlspecialchars($cunix[0]). '</option>');
  print('<option value="">--------------</option>');
  print('</select>');  
?></div>
</td>
</tr>
<tr><td>&nbsp;</td><td><input type=submit value=Submit></td></tr>
</table>
</form>

<script>
<!--
  var list = document.forms.f.cunix;
  list.onchange = listchange;
  list.options[list.options.length - 1] = null;
  var box = document.forms.f.box;
  box.onkeydown = textonkeydown;
  box.onkeypress = textonkeypress;
  box.onchange = new Function("return textchange(this.form.cunix,this.value);");
// -->
</script>

<? page_bottom(); ?>







<?
require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");

if (!isset($HTTP_POST_VARS["pass"]) || md5($HTTP_POST_VARS["pass"]) != "5d41402abc4b2a76b9719d911017c592")
  login_protect(login_administrator);

param($unilist);
param($debug);

$db = wces_connect();
$data = false;

if ($unilist)
{
  $result = db_exec("
    SELECT u.status, u.userid, u.isprofessor, u.isadmin, IFNULL(p.professorid,0) AS profid
    FROM users AS u
    LEFT JOIN professors AS p ON u.userid = p.userid
    WHERE u.cunix = '" . addslashes($unilist) . "'", $db, __FILE__, __LINE__);
  
  if (mysql_num_rows($result) == 1)
    $data = mysql_fetch_assoc($result);
}

if ($data)
{
  if ($data["isprofessor"] == "true")
    $status = login_professor; else $status = login_student;
  if ($data["profid"])
    $status |= login_knownprofessor;
  if ($data["isadmin"] == "true")
    $status |= login_administrator;
  if ($data["status"] == "student")
    $status |= login_student;
  login_setup
  (
    $db,
    $data["userid"],
    $unilist,
    $status,
    "Impersonator",
    $data["profid"]
  );
  redirect("{$wces_path}index.php$QSID");
  page_top("Fake Logon Success!");
  print("You are now logged in as '" . login_getuni() . ",' with " . login_whoami() . " privileges.<br>Use the links at the left to navigate the site as this user.");
}
else if ($debug)
{
  login_setup
  (
    $db,
    0,
    "abc123",
    login_any,
    "Impersonator",
    165
  );
  redirect($wces_path);
  page_top("Fake Logon Success!");
  print("Debug mode enabled. You can access any area of the site.");
}
else
{
  page_top("Fake Login Page");
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
  if (listtext != boxtext) list.selectedIndex = -1;
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
<? if ($unilist) print ("<p><b>The user '$unilist' does not exist.</b></p>"); ?>

This form allows you bypass ACIS authentication and log on as any user. If you
type in a username that is unknown, you will have the option of adding that
username to the database.

<form name=fakelogin action="fakelogin.php" method=get>
<?=$ISID?>
<table>
<tr>
<td valign=top>Username:</td>
<td>
<div><input type=text size=10 name=uni value="" style="width:90px"></div>
<div><?
  print('<select name=unilist size=7 style="width:90px">');
  $cunixes = db_getcolumn($db,"users",0,"cunix");
  sort($cunixes);
  array_push($cunixes,"--------------");
  foreach($cunixes as $cunix)
    print('<option value="' . htmlspecialchars($cunix) . '">' . htmlspecialchars($cunix). '</option>');
  print('</select>');  
?></div>
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

<p><font size="-1">Log on with <a href="<?=$server_url->toString()?>?debug=1<?=$ASID?>">full access</a> (debug mode).</font></p>

<?
};
page_bottom();
?>
<?

require_once("wbes/server.inc");
require_once("wbes/general.inc");
require_once("wbes/session.inc");
require_once("team/page.inc");


//-----------------------------------------------------------------------------
// LOGIN GLOBALS

define("login_user",           0x00000001);
define("login_administrator",  0x00000008);
define("login_any",            0x0000000F);

$LOGIN_SESSION_VARS = array("userid", "uni", "status", "name", "profid", "depts");
$LOGIN_SESSION = session_restore($LOGIN_SESSION_VARS, "login/");
$LOGIN_PROMPTING = false;

//-----------------------------------------------------------------------------
// LOGIN MANAGEMENT FUNCTIONS

function login_protect($requiredstatus)
{
  global $rs, $server_url, $team_path;
  if ($requiredstatus && !($requiredstatus & login_getstatus()))
    login_prompt($server_url->toString(true, true, true), $requiredstatus, false);
};

function login_setup($db, $userid, $uni, $status, $name)
{
  global $LOGIN_SESSION, $LOGIN_SESSION_VARS;

  $depts = false;
    
  session_initialize($LOGIN_SESSION_VARS, "login/");
  $LOGIN_SESSION["userid"] = (int)$userid;
  $LOGIN_SESSION["uni"] = $uni;
  $LOGIN_SESSION["status"] = (int)$status;
  $LOGIN_SESSION["name"] = $name;
  $LOGIN_SESSION["depts"] = $depts;
};

function login_logout()
{
  global $LOGIN_SESSION;
  $LOGIN_SESSION["userid"] = NULL;
  $LOGIN_SESSION["uni"]    = NULL;
  $LOGIN_SESSION["status"] = NULL;
  $LOGIN_SESSION["name"]   = NULL;
  session_kill();
};

//-----------------------------------------------------------------------------
// LOGIN INFORMATION FUNCTIONS

function login_getstatus()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["status"]) ? $LOGIN_SESSION["status"] : 0;
}

function login_getuni()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["uni"]) ? $LOGIN_SESSION["uni"] : 0;
};

function login_getdepts()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["depts"]) ? $LOGIN_SESSION["depts"] : false;
};

function login_getuserid()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["userid"]) ? $LOGIN_SESSION["userid"] : false;
};

function login_getname()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["name"]) ? $LOGIN_SESSION["name"] : false;
};

function login_whoami()
{
  global $LOGIN_SESSION;
  if (!isset($LOGIN_SESSION["status"])) return "";
  
  $result = "no";
  
  if ($LOGIN_SESSION["status"] & login_user)
    $result .= "user";

  if ($LOGIN_SESSION["status"] & login_administrator)
  {
    if ($result) $result .= ", ";
    $result .= "administrator";
  }

  $p = strrpos($result, ",");
  if ($p !== false) $result = substr_replace($result," and ",$p,2);
  return $result;
}

//-----------------------------------------------------------------------------
// PASSWORD VALIDATION ROUTINES

function login_validate($uni,$password)
{
  $uni = str_replace("@columbia.edu","",$uni);
  $dump = nl2br(acisget($uni,$password));

  if (!$dump)
  {
    sendwarning("Login warning: Could not contact AcIS server to authenticate '$uni'", __FILE__, __LINE__);
    return false;
  }
     
  if (strpos($dump,"no soup for you") !== false)
    return false; // BAD PASSWORD

  if (ereg(".*REMOTE_USER=([^<]+)<br",$dump,$result))
    $cunix = $result[1];
  else
  {
    $cunix = $uni;
    sendwarning("Login parser warning: username '$uni' could not be verified.\n\n----------\n\n$dump", __FILE__, __LINE__);
  }  

  $db = team_connect();
  $status = 0;

  $result = db_exec("SELECT user_id FROM users WHERE cunix = '" . addslashes($cunix) . "'",$db, __FILE__, __LINE__);
  if (mysql_num_rows($result) == 1)
  {
    $userid = mysql_result($result,0);
    db_exec("UPDATE users SET lastlogin = NOW() WHERE user_id = '$userid'",$db, __FILE__, __LINE__);
    $status = login_user;
  }

  //if (db_getvalue($db,"users",Array("userid" => $userid),"isadmin") == "true")
  //  $status |= login_administrator;
  
  login_setup($db,$userid,$cunix,$status,$name,$profid);
 
  return true;
};

function contains($haystack,$needles)
{
  foreach($needles as $needle)
    if (!(false === strpos($haystack,$needle))) return true;
  return false;  
};

function acisget($uni,$pw)
{
  global $server_win32, $server_tempdir;
  
  if ($server_win32)
  {
    $grabber = new COM("Oracle.AffilGrabber");
    $grabber->Validate((string)$uni,(string)$pw,"https://www1.columbia.edu/~rey4/info.html");
    return $grabber->rawoutput;
  }
  else
  {
    $fname = tempnam($server_tempdir, "booty");

    $ch = curl_init("https://www1.columbia.edu/~rey4/info.html");
    $fp = fopen ($fname, "w");
    curl_setopt ($ch, CURLOPT_FILE, $fp);
    curl_setopt ($ch, CURLOPT_USERPWD, "$uni:$pw");
    curl_setopt ($ch, CURLOPT_HEADER, 1);
    curl_exec ($ch);
    curl_close ($ch);
    fclose ($fp);

    $fp = fopen($fname,"r");
    $result = fread($fp, filesize($fname));
    fclose($fp);
    unlink($fname);
    return $result;
  }
};

//-----------------------------------------------------------------------------
// LOGIN DISPLAY

function login_prompt($url, $rs, $de = false, $uni = "")
{
  global $wces_path, $team_path, $server_url, $server_base, $server_sbase, $AISID, $AASID, $LOGIN_PROMPTING, $LOGIN_SESSION, $LOGIN_SESSION_VARS;
  $LOGIN_PROMPTING = true;
  session_initialize($LOGIN_SESSION_VARS, "login/");

  page_top("Login Page");  

  $luni = login_getuni();
  if($luni)
  {
    if ($url && $rs && !($rs & login_getstatus()))
      print("<p><b><font color=red>Access Denied</font></b></p>\n<p><b>You are currently logged in as $luni with " . login_whoami() . " privileges. You do not have permission to view <a href=\"$url\">$url</a>. If this is a mistake, please <a href=\"../about/feedback.php\">let us know</a>.</b></p>");
    else     
      print("<p>You are currently logged in as $luni. Click <a href=\"${team_path}login/logout.php\">here</a> to logout or use the form below to log in with another set of credentials.</p>");
  }
  else if ($uni)
    print("<p><b><font color=red>Invalid username or password.</font></b></p>\n");

  $securesite = "$server_sbase${team_path}login/login.php";
  $insecuresite = "$server_base${team_path}login/login.php";
  $urlargs = "?url=" . urlencode($url) . "&rs=" . urlencode($rs) . $AASID;

?>  
  <p><strong>Enter your CUNIX username and password to log in:</strong> <a href="javascript:showhelp('login')"><img src="<?=$wces_path?>media/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a></p>
  <form method=post action="<?= $de ? $insecuresite : $securesite ?>">
  <input type=hidden name=url value="<?= htmlspecialchars($url) ?>">
  <input type=hidden name=rs value="<?= htmlspecialchars($rs) ?>">
  <input type=hidden name=de value="<?= htmlspecialchars($de) ?>">
  <?= $AISID ?>
  <table>
  <tr><td>Username:</td><td><input name="uni" type="text" value="<?= $uni ?>" size="20"></td></tr>
  <tr><td>Password:</td><td><input name="pw" type="password" value="" size="20"></td></tr>
  <tr><td>&nbsp;</td><td><input type="submit" value="Log In" name=submit></td></tr>
  </table>
  </form>
<?

  if (!$server_url->secure && !$de)
  {
?><font size="-1"><b>Note:</b> The information entered in this form will
be transmitted using your browser's SSL encryption. If you would like to
connect with no encryption, click
<a href="<?= $insecuresite . $urlargs ?>&de=1">here</a>.
If you would like to view a secure version of this page so you can examine
our site's certificate before entering your password, click
<a href="<?= $securesite . $urlargs ?>">here</a>.</font>
<?
  }
  else if ($de)
  {
?><font size="-1"><b>Warning:</b> The information submitted through this form will
be transmitted without encryption. If you would like to use a secure version
of this form, click <a href="<?= $securesite . $urlargs?>">here</a>.
</font>
<?  
  }
  print("<p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>");
  page_bottom();  
  exit();
}

?>
<?

require_once("wbes/server.inc");
require_once("wbes/general.inc");
require_once("wbes/session.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wbes/postgres.inc");

//-----------------------------------------------------------------------------
// LOGIN GLOBALS

define("login_student",        0x00000001);
define("login_deptadmin",      0x00000002);
define("login_professor",      0x00000004);
define("login_administrator",  0x00000008);
define("login_any",            0x0000000F);
define("login_knownprofessor", 0x10000000);

$LOGIN_SESSION_VARS = array("userid", "uni", "status", "name", "depts", "fake", "fakeuni");
$LOGIN_SESSION = session_restore($LOGIN_SESSION_VARS, "login/");
$LOGIN_PROMPTING = false;

//-----------------------------------------------------------------------------
// LOGIN MANAGEMENT FUNCTIONS

function login_protect($requiredstatus)
{
  global $rs, $server_url, $wces_path;
  if ($requiredstatus && !($requiredstatus & login_getstatus()) || !$requiredstatus && !login_getuserid())
    login_prompt($server_url->toString(true, true, true), $requiredstatus, false);
};

function login_update($user_id, $fake = NULL, $fakeuni = NULL)
{
  global $wces;
  wces_connect();
  
  $result = pg_query("
    select u.uni, btrim(u.firstname || ' ' || u.lastname) AS name, u.flags, COALESCE(u.department_id,0) AS dept, CASE WHEN count(distinct class_id) > 0 THEN 1 ELSE 0 END AS known
    from users AS u
    left join enrollments AS e ON 
      u.user_id = e.user_id
      AND
      e.status = 3
    where u.user_id = $user_id
    group by u.user_id, u.flags, u.department_id, u.firstname, u.lastname, u.uni
  ", $wces, __FILE__, __LINE__);  

  if (pg_numrows($result) == 1)
    $row = pg_fetch_array($result,0,PGSQL_ASSOC);
  else
    $row = array("uni" => "", "name" => "__", "flags" => 0, "dept" => 0, "known" => false);
  
  $status = 0;
  if ($row['flags'] & 8)
    $status |= login_student;

  if ($row['flags'] & 4)
    $status |= login_professor;

  if ($row['flags'] & 2)
    $status |= login_deptadmin;
  
  if ($row['known'])
    $status |= login_knownprofessor;
    
  if ($row['flags'] & 1)
    $status |= login_administrator;
    
  $p['user_id'] = $user_id;
  
  login_setup($user_id,$row['uni'],$status,$row['name'],(int)$row['dept'],$fake,$fakeuni);
}

function login_setup($userid, $uni, $status, $name, $deptid, $fake, $fakeuni)
{
  global $LOGIN_SESSION, $LOGIN_SESSION_VARS;
  session_initialize($LOGIN_SESSION_VARS, "login/");
  $LOGIN_SESSION["userid"] = (int)$userid;
  $LOGIN_SESSION["uni"] = $uni;
  $LOGIN_SESSION["status"] = (int)$status;
  $LOGIN_SESSION["name"] = $name;
  $LOGIN_SESSION["depts"] = $deptid ? array($deptid) : false;
  $LOGIN_SESSION["fake"] = $fake;
  $LOGIN_SESSION["fakeuni"] = $fakeuni;
};

function login_logout()
{
  global $LOGIN_SESSION;
  $LOGIN_SESSION["userid"] = NULL;
  $LOGIN_SESSION["uni"]    = NULL;
  $LOGIN_SESSION["status"] = NULL;
  $LOGIN_SESSION["name"]   = NULL;
  $LOGIN_SESSION["depts"]  = NULL;
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

function login_getfake()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["fake"]) ? $LOGIN_SESSION["fake"] : false;
};

function login_getfakeuni()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["fakeuni"]) ? $LOGIN_SESSION["fakeuni"] : false;
};


function login_whoami()
{
  global $LOGIN_SESSION;
  if (!isset($LOGIN_SESSION["status"])) return "";
  
  $result = "";
  
  if ($LOGIN_SESSION["status"] & login_student)
    $result .= "student";

  if ($LOGIN_SESSION["status"] & login_professor)
  {
    if ($result) $result .= ", ";
    $result .= "professor";
  }  

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

$login_fail = false;

function login_validate($uni,$password)
{
  global $wces, $LOGIN_SESSION, $login_fail;
  
  $uni = str_replace("@columbia.edu","",$uni);
  $uni = str_replace("@barnard.edu","",$uni);
  $uni = trim($uni);
  $dump = acisget($uni,$password);
  
  if (!$dump)
  {
    sendwarning("Login warning: Could not contact AcIS server to authenticate '$uni'", __FILE__, __LINE__);
    $login_fail = "Authentication Error. Login warning: Could not contact AcIS server.";
    return false;
  }
     
  if (strpos($dump,"no soup for you") !== false)
  {
    $login_fail = "Invalid username or password.";    
    return false; // BAD PASSWORD
  }
  
  if(preg_match('/^REMOTE_USER=(.*)$/m', $dump, $matches))
    $cunix = $matches[1];
  else
  {
    $cunix = $uni;
    sendwarning("Login parser warning: username '$uni' could not be verified.\n\n----------\n\n$dump", __FILE__, __LINE__);
  }  
  
  $email = preg_match('/^USER_EMAIL=(.*)$/m', $dump, $matches) ? $matches[1] : "";
  $affils = preg_match('/^USER_AFFILIATIONS=(.*)$/m', $dump, $matches) ? $matches[1] : "";
  
  $ds=ldap_connect("ldap.columbia.edu");
  $r=ldap_bind($ds);
  $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$cunix");  
  $info = ldap_get_entries($ds, $sr);
  $ldap = array("givenname" => "", "sn" => "", "mail" => false);
  if ($info["count"] == 1)
  {
    $result = $info[0];
    foreach($ldap as $key => $value)
    {
      if ((int)$result[$key]["count"] > 0)
        $ldap[$key] = $result[$key][0];
    }
  }  

  $p = array('cunix' => $cunix, 'email' => $email ? $email : $ldap['mail'], 'last' => $ldap['sn'], 'first' => $ldap['givenname'], 'affils' => $affils);
  aarray_map('quot',$p);
  
  wces_connect();
  $result = pg_query("SELECT login_parse($p[cunix], $p[email], $p[last], $p[first], $p[affils])", $wces, __FILE__, __LINE__);
  $user_id = (int)pg_result($result,0,0);
  login_update($user_id);

  if ($LOGIN_SESSION["status"] == 0)
    sendwarning("Login parser warning for uni '$uni': User authenticates, but has no access to the site.\n\n----------\n\n$dump", __FILE__, __LINE__);
 
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
  global $login_fail, $wces_path, $server_url, $server_base, $server_sbase, $AISID, $AASID, $LOGIN_PROMPTING, $LOGIN_SESSION, $LOGIN_SESSION_VARS;
  $LOGIN_PROMPTING = true;
  session_initialize($LOGIN_SESSION_VARS, "login/");

  page_top("Login Page");  

  $luni = login_getuni();
  if($luni)
  {
    if ($url && $rs && !($rs & login_getstatus()))
      print("<p><b><font color=red>Access Denied</font></b></p>\n<p><small>You are currently logged in as $luni with " . (($x = login_whoami()) ? $x : "no") . " privileges. You do not have permission to view <a href=\"$url\">$url</a>. If this is a mistake, please <a href=\"../about/feedback.php\">let us know</a>.</small></p>");
    else     
      print("<p>You are currently logged in as $luni. Click <a href=\"${wces_path}login/logout.php\">here</a> to logout or use the form below to log in with another set of credentials.</p>");
  }
  else if ($login_fail)
    print("<p><b><font color=red>$login_fail</font></b></p>\n");

  $securesite = "$server_sbase${wces_path}login/login.php";
  $insecuresite = "$server_base${wces_path}login/login.php";
  $urlargs = "?url=" . urlencode($url) . "&rs=" . urlencode($rs) . $AASID;

?>  
  <p><strong>Enter your Columbia UNI and password to log in:</strong> <a href="javascript:showhelp('login')"><img src="<?=$wces_path?>media/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a></p>
  <form method=post action="<?= $de ? $insecuresite : $securesite ?>">
  <input type=hidden name=url value="<?= htmlspecialchars($url) ?>">
  <input type=hidden name=rs value="<?= htmlspecialchars($rs) ?>">
  <input type=hidden name=de value="<?= htmlspecialchars($de) ?>">
  <?= $AISID ?>
  <table>
  <tr><td>Username:</td><td><input name=uni type=text value="<?= $uni ?>" size=20></td></tr>
  <tr><td>Password:</td><td><input name=pw type=password value="" size=20></td></tr>
  <tr><td>&nbsp;</td><td><input type=submit value="Log In" name=submit> <input type=button value="Help" name=help onclick="showhelp('login')"></td></tr>
  </table>
  </form>
<?

  if (!$server_url->secure && !$de)
  {
?><font size="-1"><b>Note:</b> The information entered in this form will
be transmitted using your browser's SSL encryption. If you would like to
connect with no encryption, click
<a onclick="alert('Due to security concerns from AcIS, this function has been disabled. Sending your CUNIX username and password over a public network without encryption puts your information and Columbia\'s computer resources at risk. Please use the SSL features of this site.'); return false;" href="<?= $insecuresite . $urlargs ?>&de=1">here</a>.
If you would like to view a secure version of this page so you can examine
our site's certificate before entering your password, click
<a href="<?= $securesite . $urlargs ?>">here</a>. If your browser does not recognize our
certificate, you'll need to download the latest AcIS
<a href="http://www.columbia.edu/acis/access/secure/ca.html" target="_blank">root certificate</a> into your browser.</font>
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
<?

require_once("wbes/server.inc");
require_once("wbes/general.inc");
require_once("wbes/session.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

//-----------------------------------------------------------------------------
// LOGIN GLOBALS

define("login_student",        0x00000001);
define("login_knownprofessor", 0x00000002);
define("login_professor",      0x00000004);
define("login_administrator",  0x00000008);
define("login_any",            0x0000000F);

$LOGIN_SESSION_VARS = array("userid", "uni", "status", "name", "profid", "depts");
$LOGIN_SESSION = session_restore($LOGIN_SESSION_VARS, "login/");
$LOGIN_PROMPTING = false;

//-----------------------------------------------------------------------------
// LOGIN MANAGEMENT FUNCTIONS

function login_protect($requiredstatus)
{
  global $rs, $server_url, $wces_path;
  if ($requiredstatus && !($requiredstatus & login_getstatus()))
    login_prompt($server_url->toString(true, true, true), $requiredstatus, false);
};

function login_setup($db, $userid, $uni, $status, $name, $profid)
{
  global $LOGIN_SESSION, $LOGIN_SESSION_VARS;

  $depts = false;
  if($status & login_administrator)
  {
    $result = db_exec("SELECT departmentid FROM admins WHERE userid = '$userid'", $db, __FILE__, __LINE__);
    while($row = mysql_fetch_assoc($result))
      $depts[] = (int)$row["departmentid"];
  }
    
  session_initialize($LOGIN_SESSION_VARS, "login/");
  $LOGIN_SESSION["userid"] = (int)$userid;
  $LOGIN_SESSION["uni"] = $uni;
  $LOGIN_SESSION["status"] = (int)$status;
  $LOGIN_SESSION["name"] = $name;
  $LOGIN_SESSION["profid"] = (int)$profid;
  $LOGIN_SESSION["depts"] = $depts;
};

function login_logout()
{
  global $LOGIN_SESSION;
  $LOGIN_SESSION["userid"] = NULL;
  $LOGIN_SESSION["uni"]    = NULL;
  $LOGIN_SESSION["status"] = NULL;
  $LOGIN_SESSION["name"]   = NULL;
  $LOGIN_SESSION["profid"] = NULL;
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
// PROFESSOR LOGIN FUNCTIONS

function login_getprofid()
{
  global $LOGIN_SESSION;
  return isset($LOGIN_SESSION["profid"]) ? $LOGIN_SESSION["profid"] : "";
}

function login_saveprofid($profid)
{
  global $LOGIN_SESSION;
  if (!isset($LOGIN_SESSION["userid"])) return;
  $userid = $LOGIN_SESSION["userid"];
  $LOGIN_SESSION["profid"] = $profid;
  $db = wces_connect();
  db_updatevalues($db, "professors", Array("userid" => $userid),Array("userid" => false));
  db_updatevalues($db, "professors", Array("professorid" => $profid),Array("userid" => $userid));
  mysql_close($db);
};

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

  if (ereg(".*USER_EMAIL=([^<>]+)<br",$dump,$result))
    $email = $result[1];
  else
  {
    $email = "";
    sendwarning("Login parser warning: email address not found.\n\n----------\n\n$dump",__FILE__,__LINE__);
  }  
  
  $db = wces_connect();

  $result = db_exec("SELECT userid FROM users WHERE cunix = '" . addslashes($cunix) . "'",$db);
  if (mysql_num_rows($result) == 1)
  {
    $userid = mysql_result($result,0);
    $email = $email ? ", email = '" . addslashes($email) . "'" : "";
    db_exec("UPDATE users SET lastlogin = NOW() $email WHERE userid = '$userid'",$db);
  }
  else
  {
    db_exec("REPLACE INTO users(cunix,lastlogin) VALUES ('" . addslashes($cunix) . "',NOW())",$db);
    $userid = mysql_insert_id($db);
  }

  wces_GetCurrentQuestionPeriod($db, $null, $null, $year, $semester);
  $result = db_exec ("
  SELECT e.classid
  FROM enrollments AS e
  INNER JOIN classes AS cl USING (classid)
  WHERE cl.year >= '$year' AND cl.semester >= '$semester' AND e.userid = '$userid'", $db, __FILE__, __LINE__);

  $badenrollments = array();
  while($row = mysql_fetch_assoc($result))
    $badenrollments[$row["classid"]] = true;

  $pos = strpos($dump,"CUcourse_");
  $insertfailed = false;
  while(!($pos === false))
  {
    $classid = class_update($db, substr($dump,$pos+9,20), $null);
    if ($classid)
    {
      if (isset($badenrollments[$classid])) unset($badenrollments[$classid]);
      db_replacenoid($db,"enrollments",Array("userid" => $userid, "classid" => $classid),0);
    }
    else
      $insertfailed = true;
    $pos = strpos($dump,"CUcourse_",$pos+1);
  };
  
  if ($insertfailed)
    sendwarning("Login parser warning: class insertions failed.\n\n----------\n\n$dump", __FILE__, __LINE__);
  
  if (count($badenrollments) > 0)
    db_exec("DELETE FROM enrollments WHERE userid = '$userid' AND classid IN (" . implode(",",array_keys($badenrollments)) . ")", $db, __FILE__, __LINE__);

  // begin validation
  $status = 0;

  if (contains($dump,Array("CUstudent","BCstudent","CPMCstudent","TCstudent","UTSstudent")))
    $status |= login_student;
  
  if (db_getvalue($db,"users",Array("userid" => $userid),"isprofessor") == "true")
    $status |= login_professor;
  else if (contains($dump,Array("CUinstructor", "BCinstructor", "CUNIX_instructor", "CUadministrator", "CUNIX_Instructor")))
  {
     $status |= login_professor;
     db_updatevalues($db,"users",Array("userid" => $userid),Array("isprofessor" => "true"));
  }

  $ds=ldap_connect("ldap.columbia.edu");
  $r=ldap_bind($ds);
  $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$cunix");  
  $info = ldap_get_entries($ds, $sr);
  $ldap = array("cn" => false, "title" => false, "ou" => false, "mail" => false);
  if ($info["count"] == 1)
  {
    $result = $info[0];
    foreach($ldap as $key => $value)
    {
      if ((int)$result[$key]["count"] > 0)
        $ldap[$key] = $result[$key][0];
    }
  }  

  // TODO: put this information in ldapcache
  
  $name = $ldap["cn"];
  $profid = 0;  
  
  if ($status & login_professor)
  {
    $result = db_getrow($db,"professors",Array("userid" => $userid),array("professorid",$name));
    if ($result)
    {
      list($profid,$name) = $result;
      $status |= login_knownprofessor;
    }  
  }

  if (db_getvalue($db,"users",Array("userid" => $userid),"isadmin") == "true")
    $status |= login_administrator;
  
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
  global $wces_path, $server_url, $server_base, $server_sbase, $AISID, $AASID, $LOGIN_PROMPTING, $LOGIN_SESSION, $LOGIN_SESSION_VARS;
  $LOGIN_PROMPTING = true;
  session_initialize($LOGIN_SESSION_VARS, "login/");

  page_top("Login Page");  

  $luni = login_getuni();
  if($luni)
  {
    if ($url && $rs && !($rs & login_getstatus()))
      print("<p><b><font color=red>Access Denied</font></b></p>\n<p><b>You are currently logged in as $luni with " . login_whoami() . " privileges. You do not have permission to view <a href=\"$url\">$url</a>. If this is a mistake, please <a href=\"../about/feedback.php\">let us know</a>.</b></p>");
    else     
      print("<p>You are currently logged in as $luni. Click <a href=\"${wces_path}login/logout.php\">here</a> to logout or use the form below to log in with another set of credentials.</p>");
  }

  $securesite = "$server_sbase${wces_path}login/login.php";
  $insecuresite = "$server_base${wces_path}login/login.php";
  $urlargs = "?url=" . urlencode($url) . "&rs=" . urlencode($rs) . $AASID;

?>  
  <p><strong>Enter your CUNIX username and password to log in:</strong> <a href="javascript:showhelp('login')"><img src="<?=$wces_path?>media/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a></p>
  <form method=post action="<?= $securesite ?>">
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
  page_bottom();  
  exit();
}

?>
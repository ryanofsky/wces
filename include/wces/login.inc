<%

require_once("wces/server.inc");
require_once("wces/wces.inc");
require_once("wces/general.inc");

session_name("WCES_TEMPORARY_SESSION");

define("login_student","1");
define("login_professor", "3");
define("login_administrator", "4");

if (isset($HTTP_COOKIE_VARS[session_name()]))
{
  $login_sessionstarted = true;
  session_start();

  //workaround. copy obnoxious globals into HTTP_SESSION_VARS
  $a = array("login_userid","login_cunixid","login_isstudent","login_isprofessor","login_isadministrator","login_profid");
  foreach($a as $b) $HTTP_SESSION_VARS[b] = ${$b};
  unset($a,$b);
}
else
  $login_sessionstarted = false;

function login_protect($requiredstatus)
{
  if (!login_isvalid($requiredstatus))
    login_prompt(server_getrequest(),$requiredstatus,"");
};

//-----------------------------------------------------------------------------
// SESSION CREATION AND DESTRUCTION

function login_setup($db, $userid, $uni, $isstudent, $isprofessor, $isadministrator)
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  $login_sessionstarted = true;
  session_start();
  session_register("login_userid");
  session_register("login_cunixid");
  session_register("login_isstudent");
  session_register("login_isprofessor");
  session_register("login_isadministrator");
  session_register("login_profid");
  $HTTP_SESSION_VARS["login_userid"] = $userid;
  $HTTP_SESSION_VARS["login_cunixid"] = $uni;
  $HTTP_SESSION_VARS["login_isstudent"] = $isstudent ? true : false;
  $HTTP_SESSION_VARS["login_isprofessor"] = $isprofessor ? true : false;
  $HTTP_SESSION_VARS["login_isadministrator"] = $isadministrator ? true : false;
  $HTTP_SESSION_VARS["login_profid"] = $isprofessor ? db_getvalue($db,"Professors",Array("userid" => $userid),"professorid") : false;
  
  //WORKAROUND. Copy HTTP_SESSION_VARS into obnoxious globals
  foreach($HTTP_SESSION_VARS as $k => $v) {  global ${$k}; ${$k} = $HTTP_SESSION_VARS[$k]; }
};

function login_logout()
{
  session_start();
  @session_destroy();
  $p = session_get_cookie_params(); 
  setcookie(session_name(), "", 0, $p["path"], $p["domain"]); 
};

function login_prompt($url,$status,$uni)
{
  global $server_wcespath;
  $destination = $server_wcespath . "login/login.php?destination=" . urlencode($url) . "&requestedstatus=" .urlencode($status) . "&uni=" . urlencode($uni);
  if (!redirect($destination)) sloppyredirect($destination);
  exit();
};

//-----------------------------------------------------------------------------
// SESSION INFORMATION FUNCTIONS

function login_isvalid($requestedstatus)
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  if (!$login_sessionstarted) return false;
  if ($requestedstatus == login_student && $HTTP_SESSION_VARS["login_isstudent"]) return true;
  if ($requestedstatus == login_professor && $HTTP_SESSION_VARS["login_isprofessor"]) return true;
  if ($requestedstatus == login_administrator && $HTTP_SESSION_VARS["login_isadministrator"]) return true;
  return false;
};

function login_getuni()
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  if (!$login_sessionstarted) return false;
  return $HTTP_SESSION_VARS["login_cunixid"];
};

function login_getuserid()
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  if (!$login_sessionstarted) return false;
  return $HTTP_SESSION_VARS["login_userid"];
};

function login_isloggedin()
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  if (!$login_sessionstarted) return false;
  return ( $HTTP_SESSION_VARS["login_isstudent"] || $HTTP_SESSION_VARS["login_isprofessor"] || $HTTP_SESSION_VARS["login_isadministrator"] ) ? true : false;
};

function login_getstatus()
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  if (!$login_sessionstarted) return "";
  $first = true;
  $result = "";
  
  if ($HTTP_SESSION_VARS["login_isstudent"])
  {
    if ($first) $first = 0;
    $result .= "student";
  }  
  if ($HTTP_SESSION_VARS["login_isprofessor"])
  {
    if ($first) $first = 0; else $result .= ", ";
    $result .= "professor";
  }  
  if ($HTTP_SESSION_VARS["login_isadministrator"])
  {
    if ($first) $first = 0; else $result .= ", ";
    $result .= "administrator";
  }
  $p = strrpos($result,", ");
  if ($p > 0) $result = substr_replace($result," and ",$p,2);
  return $result;
}

//-----------------------------------------------------------------------------
// PASSWORD VALIDATION ROUTINES

function login_validate($uni,$password,$requiredstatus)
{
  $uni = str_replace("@columbia.edu","",$uni);
  $dump = nl2br(acisget($uni,$password));

  if (!(strpos($dump,"no soup for you") === false))
    return false; // BAD PASSWORD
  
  if (ereg(".*REMOTE_USER=([^<>]+)<br>",$dump,$result))
    $cunix = $result[1];
  else
    return false; // OK PASSWORD, BAD LOGIN?

  if (ereg(".*USER_EMAIL=([^<>]+)<br>",$dump,$result))
    $email = $result[1];
  else
    $email = "";
    
  $db = wces_connect();
  
  $result = db_exec("SELECT userid FROM Users WHERE cunix = '" . addslashes($cunix) . "'",$db);
  if (mysql_num_rows($result) == 1)
  {
    $userid = mysql_result($result,0);
    db_exec("UPDATE Users SET lastlogin = NOW() WHERE userid = $userid",$db);
  }
  else
  {
    db_exec("INSERT INTO Users(cunix,lastlogin) VALUES ('" . addslashes($cunix) . "',NOW())",$db);
    $userid = mysql_insert_id($db);
  }  
 
  $pos = strpos($dump,"CUcourse_");
  while(!($pos === false))
  {
    $class = substr($dump,$pos+9,20);
    $classid = class_update($db, $class);
    db_replacenoid($db,"Enrollments",Array("userid" => $userid, "classid" => $classid),0);
    $pos = strpos($dump,"CUcourse_",$pos+1);
  };

  // begin validation

  if (contains($dump,Array("CUstudent","BCstudent","CPMCstudent","TCstudent","UTSstudent")))
    $isstudent = 1;  
  
  if (db_getvalue($db,"Users",Array("userid" => $userid),"isprofessor") == "true")
    $isprofessor = 1;
  else if (contains($dump,Array("CUinstructor", "BCinstructor", "CUNIX_instructor", "CUadministrator", "CUNIX_Instructor")))
  {
     $isprofessor = 1;
     db_updatevalues($db,"Users",Array("userid" => $userid),Array("isprofessor" => "true"));
  }    

  if (db_getvalue($db,"Users",Array("userid" => $userid),"isadmin") == "true")
    $isadministrator = 1;

  login_setup($db,$userid,$cunix,$isstudent,$isprofessor,$isadministrator);
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
  global $server_win32,$server_tempdir;
  
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
    curl_exec ($ch);
    curl_close ($ch);
    fclose ($fp);

    $fp = fopen($fname,"r");
    $result = fread($fp, 100000);
    fclose($fp);
    unlink($fname);
    return $result;
  }
};

//-----------------------------------------------------------------------------
// PROFESSOR LOGIN FUNCTIONS
//
// TODO: INTEGRATE PROFESSOR LOOKUP WITH LOGON PROCESS, eliminate double redirects

function login_getprofid()
{
  global $server_wcespath, $login_profid;
  if (session_is_registered("login_profid") && $login_profid) return $login_profid;
  $destination = $server_wcespath . "login/profsearch.php?destination=" . urlencode(server_getrequest());
  if (!redirect($destination)) sloppyredirect($destination);
  exit();
}

function login_saveprofid($profid)
{
  global $login_userid, $login_profid;
  
  if (session_is_registered("login_userid"))
  {
    $db = wces_connect();
    db_updatevalues($db,"Professors",Array("userid" => $login_userid),Array("userid" => false));
    db_updatevalues($db,"Professors",Array("professorid" => $profid),Array("userid" => $login_userid));
    mysql_close($db);
    session_register("login_profid");
    $login_profid = $profid;
  };  
};


%>
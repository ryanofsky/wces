<%

require_once("server.inc");
require_once("wces.inc");
require_once("general.inc");

define("login_reject","0");
define("login_student","1");
define("login_professor", "3");
define("login_administrator", "4");

$login_entrance = $server_wcespath . "login/login.php";

session_start();

function login_setup($db,$userid, $uni,$isstudent,$isprofessor,$isadministrator)
{
  global $login_profid, $login_userid, $login_cunixid, $login_isstudent, $login_isprofessor, $login_isadministrator;
  session_register("login_userid");
  session_register("login_cunixid");
  session_register("login_isstudent");
  session_register("login_isprofessor");
  session_register("login_isadministrator");
  session_register("login_profid");
  $login_userid = $userid;
  $login_cunixid = $uni;
  $login_isstudent = $isstudent ? 1 : 0;
  $login_isprofessor = $isprofessor ? 1 : 0;
  $login_isadministrator = $isadministrator ? 1 : 0;
  $login_profid = $isprofessor ? db_getvalue($db,"Professors",Array("userid" => $userid),"professorid") : false;
};

function login_isvalid($requestedstatus)
{
  global $login_isstudent, $login_isprofessor, $login_isadministrator;
  
  if ($requestedstatus == login_student && session_is_registered("login_isstudent") && $login_isstudent)
    return 1;
  else if ($requestedstatus == login_professor && session_is_registered("login_isprofessor") && $login_isprofessor) 
    return 1;
  else if ($requestedstatus == login_administrator && session_is_registered("login_isadministrator") && $login_isadministrator)
    return 1;
  else
    return 0;
};

function login_getuni()
{
  global $login_cunixid;
  return $login_cunixid;
};

function login_getuserid()
{
  global $login_userid;
  return $login_userid;
};

function login_isloggedin()
{
  global $login_userid, $login_cunixid, $login_isstudent, $login_isprofessor, $login_isadministrator;
  if ( (session_is_registered("login_isstudent") && $login_isstudent) ||
       (session_is_registered("login_isprofessor") && $login_isprofessor) ||
       (session_is_registered("login_isadministrator") && $login_isadministrator) )
    return true;
  else
    return false;        
};

function login_getstatus()
{
  global $login_cunixid, $login_isstudent, $login_isprofessor, $login_isadministrator;
  
  $first = 1;
  $result = "";
  
  if ($login_isstudent)
  {
    if ($first) $first = 0;
    $result .= "student";
  }  
  if ($login_isprofessor)
  {
    if ($first) $first = 0; else $result .= ", ";
    $result .= "professor";
  }  
  if ($login_isadministrator)
  {
    if ($first) $first = 0; else $result .= ", ";
    $result .= "administrator";
  }
  $p = strrpos($result,", ");
  if ($p > 0) $result = substr_replace($result," and ",$p,2);
  
  return $result;
}

function login_prompt($url,$status,$msg,$uni)
{
  global $server_wcespath;
  $suffix = server_secure() ? "" : "_insecure";
  $destination = $server_wcespath . "login/login$suffix.php?destination=" . urlencode($url) . "&requestedstatus=" .urlencode($status) . "&message=" . urlencode($msg) . "&uni=" . urlencode($uni);
  if (!redirect($destination)) sloppyredirect($destination);
  exit();
};

function login_protect($requiredstatus)
{
  if (!login_isvalid($requiredstatus))
  {
    login_prompt(server_getrequest(),$requiredstatus,'<p><b>Enter your CUNIX Username and Password below to log in:</b></p>',"");
  }
};

function contains($haystack,$needles)
{
  foreach($needles as $needle)
    if (!(false === strpos($haystack,$needle))) return true;
  return false;  
};

function login_validate($uni,$password,$requiredstatus)
{
  $uni = str_replace("@columbia.edu","",$uni);
  $dump = nl2br(acisget($uni,$password));

  if (!(strpos($dump,"no soup for you") === false)) return false;
  
  if (ereg(".*REMOTE_USER=([^<>]+)<br>",$dump,$result))
    $cunix = $result[1];
  else
  {
    $db = wces_connect();
    db_addrow($db,"ErrorLog",Array("description" => "Abnormal Login. Username: $uni Dump: $dump"));
    return false;
  }  

  if (ereg(".*USER_EMAIL=([^<>]+)<br>",$dump,$result))
    $email = $result[1];
  else
    $email = "";
    
  $db = wces_connect();
  $userid = db_replace($db,"Users",Array("cunix" => $cunix),Array("email" => $email),"userid");
  
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
  {
    $isadministrator = 1;
  };

  login_setup($db,$userid,$cunix,$isstudent,$isprofessor,$isadministrator);
  return true;
};

function login_logout()
{
  session_start();
  @session_destroy();
  $p = session_get_cookie_params(); 
  setcookie(session_name(), "", 0, $p["path"], $p["domain"]); 
};

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

function acisget($uni,$pw)
{
  global $server_win32,$server_tempdir;
  
  if ($server_win32)
  {
    $grabber = new COM("Oracle.AffilGrabber");
    return $grabber->retrieve("www1.columbia.edu",443,"/~rey4/info.html",(string)$uni,(string)$pw);
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
%>
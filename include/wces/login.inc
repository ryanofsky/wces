<?

require_once("wces/server.inc");
require_once("wces/wces.inc");
require_once("wces/general.inc");

session_name("WCES_TEMPORARY_SESSION");

define("login_student","1");
define("login_professor", "3");
define("login_administrator", "4");

if (isset($SESSION_TRANSFER_ID) && $SESSION_TRANSFER_ID)
  session_id($SESSION_TRANSFER_ID);

if (isset($HTTP_COOKIE_VARS[session_name()]) || (isset($SESSION_TRANSFER_ID) && $SESSION_TRANSFER_ID))
{
  $login_sessionstarted = true;
  session_start();

  //workaround. copy obnoxious globals into HTTP_SESSION_VARS
  $a = array("login_userid","login_cunixid","login_isstudent","login_isprofessor","login_isadministrator","login_profid","login_depts");
  foreach($a as $b)
    $HTTP_SESSION_VARS[$b] = isset(${$b}) ? ${$b} : false;
  unset($a,$b);
}
else
  $login_sessionstarted = false;

function login_protect($requiredstatus)
{
  if (!login_isvalid($requiredstatus))
    login_prompt(server_getrequest(true),$requiredstatus,"");
};

//-----------------------------------------------------------------------------
// SESSION CREATION AND DESTRUCTION

function login_setup($db, $userid, $uni, $isstudent, $isprofessor, $isadministrator)
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  
  $result = db_exec("SELECT departmentid FROM admins WHERE userid = '$userid'", $db, __FILE__, __LINE__);
  $depts = array();
  while($row = mysql_fetch_assoc($result))
  {
    $depts[] = $row["departmentid"];
  }
  if (count($depts) == 0) $depts = false;
  
  $login_sessionstarted = true;
  session_start();
  session_register("login_userid");
  session_register("login_cunixid");
  session_register("login_isstudent");
  session_register("login_isprofessor");
  session_register("login_isadministrator");
  session_register("login_profid");
  session_register("login_depts");
  $HTTP_SESSION_VARS["login_userid"] = $userid;
  $HTTP_SESSION_VARS["login_cunixid"] = $uni;
  $HTTP_SESSION_VARS["login_isstudent"] = $isstudent ? true : false;
  $HTTP_SESSION_VARS["login_isprofessor"] = $isprofessor ? true : false;
  $HTTP_SESSION_VARS["login_isadministrator"] = $isadministrator ? true : false;
  $HTTP_SESSION_VARS["login_profid"] = $isprofessor ? db_getvalue($db,"professors",Array("userid" => $userid),"professorid") : false;
  $HTTP_SESSION_VARS["login_depts"] = $depts;
  //WORKAROUND. Copy HTTP_SESSION_VARS into obnoxious globals
  foreach($HTTP_SESSION_VARS as $k => $v) {  global ${$k}; ${$k} = $HTTP_SESSION_VARS[$k]; }
};

function login_logout()
{
  @session_start();
  @session_destroy();
  $p = session_get_cookie_params(); 
  @setcookie(session_name(), "", 0, $p["path"], $p["domain"]); 
};

function login_prompt($url,$status,$uni)
{
  global $server_sbase, $server_wcespath;
  $destination = $server_sbase . $server_wcespath . "login/login.php?destination=" . urlencode($url) . "&requestedstatus=" .urlencode($status) . "&uni=" . urlencode($uni) . "&SESSION_TRANSFER_ID=" . urlencode(session_id());
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

function login_getdepts()
{
  global $HTTP_SESSION_VARS, $login_sessionstarted;
  if (!$login_sessionstarted || $HTTP_SESSION_VARS["login_isadministrator"]) return false;
  return $HTTP_SESSION_VARS["login_depts"];
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
  $null = false;
  
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

  if (contains($dump,Array("CUstudent","BCstudent","CPMCstudent","TCstudent","UTSstudent")))
    $isstudent = 1;
  
  if (db_getvalue($db,"users",Array("userid" => $userid),"isprofessor") == "true")
    $isprofessor = 1;
  else if (contains($dump,Array("CUinstructor", "BCinstructor", "CUNIX_instructor", "CUadministrator", "CUNIX_Instructor")))
  {
     $isprofessor = 1;
     db_updatevalues($db,"users",Array("userid" => $userid),Array("isprofessor" => "true"));
  }
  else
    $isprofessor = 0;

  if (db_getvalue($db,"users",Array("userid" => $userid),"isadmin") == "true")
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
    curl_setopt ($ch, CURLOPT_HEADER, 1);
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

function login_getprofid($enforce = true)
{
  global $server_wcespath, $login_profid;
  if (session_is_registered("login_profid") && $login_profid) return $login_profid;
  if ($enforce)
  {
    $destination = $server_wcespath . "login/profsearch.php?destination=" . urlencode(server_getrequest());
    if (!redirect($destination)) sloppyredirect($destination);
    exit();
  }
  else
    return false;  
}

function login_saveprofid($profid)
{
  global $login_userid, $login_profid;
  
  if (session_is_registered("login_userid"))
  {
    $db = wces_connect();
    db_updatevalues($db, "professors", Array("userid" => $login_userid),Array("userid" => false));
    db_updatevalues($db, "professors", Array("professorid" => $profid),Array("userid" => $login_userid));
    mysql_close($db);
    session_register("login_profid");
    $login_profid = $profid;
  };  
};

?>

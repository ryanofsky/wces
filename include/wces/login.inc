<?

require_once("wces/server.inc");
require_once("wces/wces.inc");
require_once("wces/general.inc");

define("login_student","1");
define("login_professor", "3");
define("login_administrator", "4");

function login_protect($requiredstatus)
{
};

//-----------------------------------------------------------------------------
// SESSION CREATION AND DESTRUCTION

function login_setup($db, $userid, $uni, $isstudent, $isprofessor, $isadministrator)
{
};

function login_logout()
{
};

function login_prompt($url,$status,$uni)
{
};

//-----------------------------------------------------------------------------
// SESSION INFORMATION FUNCTIONS

function login_isvalid($requestedstatus)
{
  return true;
};

function login_getuni()
{
  return "abc123";
};

function login_getdepts()
{
  return false;
};

function login_getuserid()
{
  return 140;
};

function login_isloggedin()
{
  return true;
};

function login_getstatus()
{
  return "student, professor and administrator";
}

//-----------------------------------------------------------------------------
// PASSWORD VALIDATION ROUTINES

function login_validate($uni,$password,$requiredstatus)
{
  return true;
};

//-----------------------------------------------------------------------------
// PROFESSOR LOGIN FUNCTIONS
//
// TODO: INTEGRATE PROFESSOR LOOKUP WITH LOGON PROCESS, eliminate double redirects

function login_getprofid($enforce = true)
{
  return 72;
}

function login_saveprofid($profid)
{
};

?>

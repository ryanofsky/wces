<?

require_once("wbes/server.inc");
require_once("wbes/general.inc");
require_once("wbes/session.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("widgets/basic.inc");

require_once("$WBES_LOGIN_INCLUDE");

function & LoginInstance()
{
  if (!isset($GLOBALS['LoginInstance'])) 
    $GLOBALS['LoginInstance'] =& new $GLOBALS['WBES_LOGIN'];
  return $GLOBALS['LoginInstance'];
}

function LoginProtect($requiredStatus)
{
  global $server_url;
  $login =& LoginInstance();
  if (!$login->meetsStatus($requiredStatus))
    $login->prompt($server_url->toString(true, true, true), $requiredStatus);
};

function & LoginValue($param)
{
  $login =& LoginInstance();
  return $login->get($param);
}

//-----------------------------------------------------------------------------
// LOGIN GLOBALS

define("login_student",        0x00000001);
define("login_deptadmin",      0x00000002);
define("login_professor",      0x00000004);
define("login_administrator",  0x00000008);
define("login_any",            0x0000000F);
define("login_knownprofessor", 0x10000000);

session_restore();

class WcesLogin
{
  var $error;
  
  function get($name)
  {
    $n = $GLOBALS['WBES_LOGIN_PREFIX'] . $name;
    if (isset($_SESSION[$n])) return $_SESSION[$n];
    return null;
  }
  
  function meetsStatus($status)
  {
    if ($status)
      return (bool)($this->get('status') & $status);
    else // true when logged in
      return (bool)$this->get('user_id');
  }
  
  function & getWidget($name, &$parent)
  {
    return new LoginWidget($name, $parent);
  }
  
  function prompt($destUrl = '', $requestedStatus = 0)
  {
    global $WBES_LOGIN_URL, $WBES_LOGIN_SURL;
    $page =& WcesPageInstance();
    $f =& new Form('f', $GLOBALS['WBES_LOGIN_URL']);
    $w =& $this->getWidget('login', $f);
    $w->destUrl = $destUrl;
    $w->requestedStatus = $requestedStatus;
    $w->uni = $this->get('uni');
    $f->loadState();
    
    if ($w->done && $w->destUrl)
    {
      sloppyredirect($w->destUrl);
      exit();
    }
    
    $page->loggingIn = true;
    $page->printTop("Login Page");
    $url = $w->plaintext ? $WBES_LOGIN_URL : $WBES_LOGIN_SURL;
    print("<form name=f method=post action=\"$url\">\n");
    $f->display();
    $w->display();
    print("</form>\n");
    $page->printBottom();
    exit();
  }
  
  function whatami()
  {
    $c = array();
    $status = (int)$this->get('status');

    if ($status & login_student)       $c[] = "student";
    if ($status & login_professor)     $c[] = "professor";
    if ($status & login_administrator) $c[] = "administrator";

    $l = array_splice ($c, -1);

    if (count($c))
      return implode(', ', $c) . ' and ' . $l[0];
    else
      return $l[0];
  }
  
  function logout()
  {
    session_kill();
  }
};

class LoginWidget extends ParentWidget
{
  var $login;
  
  var $done = false;
  var $destUrl = '';
  var $uni = '';
  var $plaintext = false;
  var $requestedStatus = 0;
  
  function LoginWidget($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->urlShortName('rs');
    $this->urlShortName('url');
    $this->urlShortName('p');
  }
  
  function loadInitialState()
  {
    // technically, it is not neccessary to call session_initialize here since
    // no data is being stored in the session on this page load.
    // The reason it is called here is to allow detection of browser support 
    // for cookies on the next page load. That way, the SID will never
    // have to be passed as a URL parameter for browsers that support cookies.
    
    session_initialize();
  }
  
  function loadState()
  {
    ParentWidget::loadState();
    $mode = WIDGET_FORM | WIDGET_URL;
    $this->plaintext = (bool)$this->readValue('p', $mode);
    $this->requestedStatus = (int)$this->readValue('rs', $mode);
    $this->destUrl = (string)$this->readValue('url', $mode);
    $this->uni = $this->readValue('uni', $mode);
    $pw = $this->readValue('pw', $mode);
 
    if ($this->uni || $pw)
    {
      $login =& LoginInstance();
      if ($login->validate($this->uni, $pw))
      {
        // ($status meets at least one required flag) || (there aren't any required flags)
        $this->done = !$login->meetsStatus($this->requestedStatus);
      }
    }
  }
  
  function printVisible()
  {
    global $WBES_CONFIG_MEDIA_PATH, $WCES_CONFIG_PATH, $WBES_LOGIN_URL, $WBES_LOGIN_SURL, $AASID, $AISID, $server_url, $WBES_LOGIN_LOGOUT;
  
    $mode = WIDGET_FORM | WIDGET_URL;
    
    if ($this->destUrl)
      $this->printValue('url', $this->destUrl, $mode);
    
    if ($this->requestedStatus);
      $this->printValue('rs', $this->requestedStatus, $mode);
      
    if ($this->plaintext)
      $this->printValue('p', 1, $mode);
  
    $login =& LoginInstance();
    $luni = $login->get('uni');
    
    if ($login->error)
      print("<p><b><font color=red>$login->error</font></b></p>\n");
    else if($luni)
    {
      if (!$login->meetsStatus($this->requestedStatus))
        print("<p><b><font color=red>Access Denied</font></b></p>\n<p><small>You are currently logged in as $luni with " . (($x = $login->whatami()) ? $x : "no") . " privileges. You do not have permission to view <a href=\"$this->destUrl\">$this->destUrl</a>. If this is a mistake, please <a href=\"{$WCES_CONFIG_PATH}about/feedback.php\">let us know</a>.</small></p>");
      else     
        print("<p>You are currently logged in as $luni. Click <a href=\"${WBES_LOGIN_LOGOUT}\">here</a> to logout or use the form below to log in with another set of credentials.</p>");
    }
  
?>  
  <p><strong>Enter your Columbia UNI and password to log in:</strong> <a href="javascript:showhelp('login')"><img src="<?=$WBES_CONFIG_MEDIA_PATH?>help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a></p>
  <?=$AISID?>
  <table>
  <tr><td>Username:</td><td><input name=<?=$this->fullName('uni')?> type=text value="<?=$this->uni?>" size=20></td></tr>
  <tr><td>Password:</td><td><input name=<?=$this->fullName('pw')?> type=password size=20></td></tr>
  <tr><td>&nbsp;</td><td><input type=submit value="Log In" name=submit> <input type=button value="Help" name=help onclick="showhelp('login')"></td></tr>
  </table>
<?
    if (!$server_url->secure && !$this->plaintext)
    {
      $plainUrl = $this->getUrl(array('p' => 1));
      $certUrl = $GLOBALS['WBES_LOGIN_SURL'] . $this->form->getUrlVars();
      
      
?><font size="-1"><b>Note:</b> The information entered in this form will
be transmitted using your browser's SSL encryption. If you would like to
connect with no encryption, click
<a onclick="alert('Due to security concerns from AcIS, this function has been disabled. Sending your CUNIX username and password over a public network without encryption puts your information and Columbia\'s computer resources at risk. Please use the SSL features of this site.'); return false;" href="<?=$plainUrl?>">here</a>.
If you would like to view a secure version of this page so you can examine
our site's certificate before entering your password, click
<a href="<?=$certUrl?>">here</a>. If your browser does not recognize our
certificate, you'll need to download the latest AcIS
<a href="http://www.columbia.edu/acis/access/secure/ca.html" target="_blank">root certificate</a> into your browser.</font>
<?
    }
    else if ($this->plaintext)
    {
      $encryptedUrl = $this->getUrl(array('p' => false));
      
?><font size="-1"><b>Warning:</b> The information submitted through this form will
be transmitted without encryption. If you would like to use a secure version
of this form, click <a href="<?=$encryptedUrl?>">here</a>.
</font>
<?  
    }
    print("<p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>");
  }
}

/////////////
///////
/////////////////////

function login_protect($s) 
{
  LoginProtect($s);
}


?>
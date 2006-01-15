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
  {
    // technically, it is not neccessary to call session_initialize here since
    // no data is being stored in the session on this page load.
    // The reason it is called here is to allow detection of browser support 
    // for cookies on the next page load. That way, the SID will never
    // have to be passed as a URL parameter for browsers that support cookies.     
    session_initialize();
        
    $login->prompt(LoginUrl(), $requiredStatus);
  }
};

function LoginLink($requiredStatus = null, $destUrl = null)
{
  global $WBES_LOGIN_HANDLER, $ASID;
  if (!isset($destUrl)) $destUrl = LoginUrl();
  $r = $WBES_LOGIN_HANDLER . "?url=" . urlencode($destUrl);
  if (strlen($requiredStatus)) $r .= "&rs=" . urlencode($requiredStatus);
  return $r . $ASID;
}

function LoginValue($param)
{
  $login =& LoginInstance();
  return $login->get($param);
}

function LoginUrl()
{
  global $server_url;

  // current page url
  $u = $server_url->toString(false, true, true);

  $sn = session_name();

  // strip out session identifiers before the end of the parameter list
  $u = preg_replace("/([?&])$sn=[^&]*&/", "\\1", $u);

  // strip out session identifiers at the end of the parameter list
  $u = preg_replace("/[?&]$sn=[^&#]*(\$|#)/", "\\1", $u);
  
  return $u;
}

//-----------------------------------------------------------------------------
// LOGIN GLOBALS

define("LOGIN_STUDENT",         0x00000001);
define("LOGIN_DEPT_ADMIN",      0x00000002);
define("LOGIN_PROFESSOR",       0x00000004);
define("LOGIN_ADMIN",           0x00000008);
define("LOGIN_ANY",             0x0000000F);
define("LOGIN_KNOWN_PROFESSOR", 0x10000000);

session_initialize(true);

class WcesLogin
{
  var $error;
  var $bouncer = false;
  
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
  
  function prompt($destUrl, $requestedStatus, $bounce = false)
  {
    global $WBES_LOGIN_HANDLER, $WBES_LOGIN_BOUNCER, 
      $WBES_LOGIN_SECURE_HANDLER, $server_url;
    $page =& WcesPageInstance();
    $f =& new Form('f', $WBES_LOGIN_HANDLER);
    $w =& $this->getWidget('login', $f);
    $w->destUrl = $destUrl;
    $w->requestedStatus = $requestedStatus;

    $f->loadState();
    
    if ($bounce)
    {
      sloppyredirect($w->bounceUrl());
      exit();
    }
    
    if ($w->done && $w->destUrl)
    {
      sloppyredirect(sid($w->base . $w->destUrl));
      exit();
    }

    if (!$server_url->secure && !$w->plaintext)
    {
      if (!empty($WBES_LOGIN_BOUNCER))
        $f->pageName = $WBES_LOGIN_BOUNCER;
      else if (!empty($WBES_LOGIN_SECURE_HANDLER))
        $f->pageName = $WBES_LOGIN_SECURE_HANDLER;
    }    
    
    $page->loggingIn = true;
    $page->printTop("Login Page");
    print("<form name=$f->formName method=post action=\"$f->pageName\">\n");
    if ($f->pageName == $WBES_LOGIN_HANDLER)
      print($GLOBALS['ISID']);
    else
    {
      print($GLOBALS['AISID']);
      if (!isset($w->base))
        $w->base = $server_url->toString(true, false, false, false);
    }
    
    $w->display();
    print("</form>\n");
    $page->printBottom();
    exit();
  }
  
  function bounce()
  {
    $this->prompt(null, null, true);
  }
  
  function handle()
  {
    $this->prompt(null, null, false);
  }

  function logout()
  {
    session_kill();
  }

  function whatami()
  {
    $c = array();
    $status = (int)$this->get('status');

    if ($status & LOGIN_STUDENT) $c[] = "student";
    if ($status & LOGIN_PROFESSOR) $c[] = "professor";
    if ($status & LOGIN_ADMIN) $c[] = "administrator";

    $l = array_splice ($c, -1);

    if (count($c))
      return implode(', ', $c) . ' and ' . $l[0];
    else
      return $l[0];
  }
};

class LoginWidget extends ParentWidget
{
  var $login;
  
  var $done = false;
  var $destUrl;
  var $requestedStatus;
  var $base;
  var $uni = '';
  var $plaintext = false;
  
  function LoginWidget($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->shortName('requestedStatus', 'rs');
    $this->shortName('destUrl', 'url');
    $this->shortName('plaintext', 'p');
    $this->shortName('uni');
    $this->shortName('pw');
    $this->shortName('base');
    $this->shortName('error');
  }
  
  function loadState($new)
  {
    ParentWidget::loadState($new);
    if ($new) return;

    // let externally specified values override form values
    if (!isset($this->destUrl))
      $this->destUrl = (string)$this->readValue('destUrl');

    if (!isset($this->requestedStatus))
      $this->requestedStatus = (int)$this->readValue('requestedStatus');

    if (!isset($this->base))
      $this->base = $this->readValue('base');

    // let form values override externally specifed values
    $plaintext = $this->readValue('plaintext');    
    $uni = $this->readValue('uni');
    $pw = $this->readValue('pw');
    
    if (isset($plaintext))
      $this->plaintext = (bool)$plaintext;
      
    if (isset($uni))
      $this->uni = $uni;
    
    if (isset($pw))
    {
      $login =& LoginInstance();
      if ($login->validate($this->uni, $pw, $this->error))
        $this->done = $login->meetsStatus($this->requestedStatus);
    }
    else
      $this->error = $this->readValue('error');
  }

  function printVisible()
  {
    global $WBES_CONFIG_MEDIA_PATH, $WBES_CONFIG_PATH, $ISID, $server_url, $WBES_LOGIN_LOGOUT;
  
    $mode = WIDGET_FORM | WIDGET_URL;
    
    if ($this->destUrl)
      $this->printValue('destUrl', $this->destUrl, $mode);
    
    if ($this->requestedStatus);
      $this->printValue('requestedStatus', $this->requestedStatus, $mode);
      
    if ($this->plaintext)
      $this->printValue('plaintext', 1, $mode);

    if ($this->base)
      $this->printValue('base', $this->base, $mode);
  
    $login =& LoginInstance();
    $luni = $login->get('uni');
    
    if($luni)
    {
      if (!$login->meetsStatus($this->requestedStatus))
        print("<p><b><font color=red>Access Denied</font></b></p>\n<p><small>You are currently logged in as $luni with " . (($x = $login->whatami()) ? $x : "no") . " privileges. You do not have permission to view <a href=\"$this->destUrl\">$this->destUrl</a>. If this is a mistake, please <a href=\"$GLOBALS[WCES_CONFIG_PATH]about/feedback.php\">let us know</a>.</small></p>");
      else
        print("<p>You are currently logged in as $luni. Click <a href=\"${WBES_LOGIN_LOGOUT}\">here</a> to logout or use the form below to log in as someone else.</p>");
    }
    
    if ($this->error)
      print("<p><b><font color=red>$this->error</font></b></p>\n");
    
?>  
  <p><strong>Enter your Columbia UNI and password to log in:</strong> <a href="javascript:showhelp('login')"><img src="<?=$WBES_CONFIG_MEDIA_PATH?>help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a></p>
  <table>
  <tr><td>Username:</td><td><input name=<?=$this->name('uni')?> type=text value="<?=$this->uni?>" size=20></td></tr>
  <tr><td>Password:</td><td><input name=<?=$this->name('pw')?> type=password size=20></td></tr>
  <tr><td>&nbsp;</td><td><input type=submit value="Log In"> <input type=button value="Help" onclick="showhelp('login')"></td></tr>
  </table>
<?
    if (!$server_url->secure)
    {
      $plainUrl = $this->getUrl(array('plaintext' => 1, 'base' => false), 
        $GLOBALS['WBES_LOGIN_HANDLER']);
      
      if (!strcasecmp(substr($this->form->pageName, 0, 8), "https://"))
      {
        print("<p><font size=\"-1\"><b>Note:</b> The information entered in "
          . "this form will be transmitted using your browser's SSL "
          . "encryption.");
	  
	if (empty($GLOBALS['WBES_LOGIN_SSL_ONLY']))
	  print("\nIf you would like to connect with no encryption, click "
	    . "<a href=\"$plainUrl\">here</a>.");
        
        if (!empty($GLOBALS['WBES_LOGIN_SECURE_HANDLER']))
        {
          $certUrl = $this->getUrl(array(), 
            $GLOBALS['WBES_LOGIN_SECURE_HANDLER'], true);
          print("\nIf you would like to view a secure version of this page "
            . "so you can examine our site's certificate before entering "
            . "your password, click <a href=\"$certUrl\">here</a>.");
        }
        
        print("\nIf your browser does not recognize our certificate, you'll "
          . "need to download the latest AcIS <a href=\"" 
          . "http://www.columbia.edu/acis/access/secure/ca.html\" "
          . "target=_blank>root certificate</a> into your browser.</font>\n");
      }
      else
      {
        print("<font size=\"-1\"><b>Warning:</b> The information submitted "
          . "through this form will be transmitted without encryption."); 
        
        if ($this->plaintext)
        {
          $encryptedUrl = $this->getUrl(array('plaintext' => false));
          print("\nIf you would like to use a secure version of this form, "
            . "click <a href=\"$encryptedUrl\">here</a>.");
        }
        
        print("</font>\n");
      }
    }
    print("<p>&nbsp;</p>\n<p>&nbsp;</p>\n<p>&nbsp;</p>\n");
  }

  function bounceUrl()
  {
    if ($this->done && $this->destUrl)
      return sid($this->base . $this->destUrl);
    else
      return $this->getUrl(array('destUrl' => $this->destUrl, 
        'requestedStatus' => $this->requestedStatus, 
        'uni' => $this->uni, 'error' => $this->error),
        $this->base . $GLOBALS['WBES_LOGIN_HANDLER']);
  }
}

?>

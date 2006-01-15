<?

// Start up session and set helper *SID variables

function session_initialize($existing_only = false)
{
  global $AUSID, $AQSID, $AASID, $AISID, $USID, $QSID, $ASID, $ISID;
  // avoid calling session_start twice if this is called from multiple places
  if (isset($AUSID) && ($existing_only || $AUSID))
    return (bool)$AUSID;

  $sn = session_name();
  
  if (isset($_POST[$sn]))
    session_id($si = $_POST[$sn]);
  else if (isset($_GET[$sn]))
    session_id($si = $_GET[$sn]);
  else if (isset($_COOKIE[$sn]))
    session_id($si = $_COOKIE[$sn]);
  else if ($existing_only)
  {
    $AUSID = $AQSID = $AASID = $AISID = $USID = $QSID = $ASID = $ISID = '';
    return false;
  }
  
  session_start();

  if (!isset($si)) $si = session_id();
  $AUSID = "$sn=" . urlencode($si);
  $AQSID = "?$AUSID";
  $AASID = "&$AUSID";
  $AISID = "<input type=hidden name=$sn value=\"".htmlspecialchars($si)."\">";

  if (isset($_COOKIE[$sn]))
    $USID = $QSID = $ASID = $ISID = '';
  else
    list($USID, $QSID, $ASID, $ISID) = array($AUSID, $AQSID, $AASID, $AISID);

  return true;
}

function session_kill()
{
  if (session_initialize(true)) session_destroy();
  
  global $AUSID, $AQSID, $AASID, $AISID, $USID, $QSID, $ASID, $ISID;
  $AUSID = $AQSID = $AASID = $AISID = $USID = $QSID = $ASID = $ISID = '';
  
  $p = session_get_cookie_params();
  setcookie(session_name(), '', 0, $p['path'], $p['domain']);
  $_SESSION = null;
}

function sid($url)
{
  global $QSID, $ASID;
  if ($QSID)
    return $url . (strrpos($url, '?') === false ? $QSID : $ASID);
  else
    return $url;
}

?>

<?

// Start up session and set helper *SID variables

function session_initialize()
{
  global $USID, $QSID, $ASID, $ISID, $AUSID, $AQSID, $AASID, $AISID;
  session_start();

  $si = session_id();
  $sn = session_name();  
  $AUSID = "$sn=" . urlencode($si);
  $AQSID = "?$AUSID";
  $AASID = "&$AUSID";
  $AISID = "<input type=hidden name=$sn value=\"".htmlspecialchars($si)."\">";

  if (isset($_COOKIE[$sn]))
    $USID = $QSID = $ASID = $ISID = '';
  else
    list($USID, $QSID, $ASID, $ISID) = array($AUSID, $AQSID, $AASID, $AISID);
}

// Call session_initialize only if there is a pre-existing session

function session_restore()
{
  global $USID, $QSID, $ASID, $ISID, $AUSID, $AQSID, $AASID, $AISID;
  
  // avoid doing duplicate work if session_restore is called
  // from different places.
  if (isset($USID)) return;
  
  $sn = session_name();
  
  if (isset($_SESSION) || isset($_COOKIE[$sn]) || isset($_POST[$sn]) || isset($_GET[$sn]))
    session_initialize();
  else
    $USID = $QSID = $ASID = $ISID = $AUSID = $AQSID = $AASID = $AISID = '';
}

function session_kill()
{
  global $USID, $QSID, $ASID, $ISID, $AUSID, $AQSID, $AASID, $AISID;
  $USID = $QSID = $ASID = $ISID = $AUSID = $AQSID = $AASID = $AISID = '';
  if (!isset($_SESSION)) session_start();
  session_destroy();
  $p = session_get_cookie_params();
  setcookie(session_name(), '', 0, $p['path'], $p['domain']);
}

?>
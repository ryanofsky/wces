<?

function session_initialize($vars, $prefix = "")
{
  global $HTTP_COOKIE_VARS, $HTTP_POST_VARS, $HTTP_GET_VARS,
         $USID, $QSID, $ASID, $ISID, $AUSID, $AQSID, $AASID, $AISID;

  foreach($vars as $var) // To ensure security, overwrite any existing values
    if (!session_is_registered("$prefix$var")) 
      $GLOBALS["$prefix$var"] = NULL;

  if (!defined("SID")) session_start();

  $r = array();
  foreach($vars as $var)
  {
    session_register("$prefix$var");
    $r[$var] = &$GLOBALS["$prefix$var"];
  }

  $si = session_id();
  $sn = session_name();  
  $AUSID = "$sn=" . urlencode($si);
  $AQSID = "?$AUSID";
  $AASID = "&$AUSID";
  $AISID = "<input type=hidden name=$sn value=\"".htmlspecialchars($si)."\">";
  if (isset($HTTP_COOKIE_VARS[$sn]))
    $USID = $QSID = $ASID = $ISID = "";
  else
    list($USID, $QSID, $ASID, $ISID) = array($AUSID, $AQSID, $AASID, $AISID);

  return $r;  
}

function session_restore($vars, $prefix = "")
{
  global $HTTP_COOKIE_VARS, $HTTP_POST_VARS, $HTTP_GET_VARS, 
         $USID, $QSID, $ASID, $ISID, $AUSID, $AQSID, $AASID, $AISID;
         
  $sn = session_name();
  if (isset($HTTP_COOKIE_VARS[$sn]) || isset($HTTP_POST_VARS[$sn]) || isset($HTTP_GET_VARS[$sn]))
    return session_initialize($vars,$prefix);
  else
  {
    $USID = $QSID = $ASID = $ISID = $AUSID = $AQSID = $AASID = $AISID = "";
    return NULL;
  }  
}

function session_kill()
{
  global $USID, $QSID, $ASID, $ISID, $AUSID, $AQSID, $AASID, $AISID;
  $USID = $QSID = $ASID = $ISID = $AUSID = $AQSID = $AASID = $AISID = "";
  if (!defined("SID")) session_start();
  session_destroy();
  $p = session_get_cookie_params();
  setcookie(session_name(), "", 0, $p["path"], $p["domain"]);
}

?>
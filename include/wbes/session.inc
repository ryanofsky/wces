<?

function session_initialize($vars, $prefix = "")
{
  global $HTTP_COOKIE_VARS, $HTTP_POST_VARS, $HTTP_GET_VARS,
         $ASID, $QSID, $ISID, $AASID, $AQSID, $AISID;

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
  $AASID = "&$sn=" . urlencode($si);
  $AQSID = "?$sn=" . urlencode($si);
  $AISID = "<input type=hidden name=$sn value=\"".htmlspecialchars($si)."\">";
  if (isset($HTTP_COOKIE_VARS[$sn]))
    list($ASID, $QSID, $ISID) = array("", "", "");
  else
    list($ASID, $QSID, $ISID) = array($AASID, $AQSID, $AISID);

  return $r;  
}

function session_restore($vars, $prefix = "")
{
  global $HTTP_COOKIE_VARS, $HTTP_POST_VARS, $HTTP_GET_VARS, 
         $ASID,$QSID,$ISID,$AASID,$AQSID,$AISID;
         
  $sn = session_name();
  if (isset($HTTP_COOKIE_VARS[$sn]) || isset($HTTP_POST_VARS[$sn]) || isset($HTTP_GET_VARS[$sn]))
    return session_initialize($vars,$prefix);
  else
  {
    list($ASID,$QSID,$ISID,$AASID,$AQSID,$AISID) = array("","","","","","");
    return NULL;
  }  
}

function session_kill()
{
  if (!defined("SID")) session_start();
  session_destroy();
  $p = session_get_cookie_params();
  setcookie(session_name(), "", 0, $p["path"], $p["domain"]);
}

?>
<%
  require_once("wces/login.inc");
  session_start(); 
  
  if (login_validate($uni,$password,$requestedstatus) && login_isvalid($requestedstatus))
  {
    if (!$destination) $destination = "login.php";
    $last = substr($destination,-1);
    if (!($last == "?" || last == "&"))
      $destination .= strpos($destination,"?") === false ? "?" : "&";
    $destination .= "SESSION_TRANSFER_ID=" . urlencode(session_id());
    if (!redirect($destination)) sloppyredirect($destination);
    exit();
  }  
  else
  {
    login_prompt($destination,$requestedstatus,$uni);
  }
%>
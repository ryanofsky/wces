<%
  require_once("wces/login.inc");
  session_start(); 
  
  if (login_validate($uni,$password,$requestedstatus) && login_isvalid($requestedstatus))
  {
    if (!$destination) $destination = "login.php";
    if (!redirect($destination)) sloppyredirect($destination);
    exit();
  }  
  else
  {
    login_prompt($destination,$requestedstatus,$uni);
  }
%>
<%
  require_once("login.inc");
 
  function bounce($url)
  {
    redirect($url);
    if (!redirect($url)) sloppyredirect($url);
    exit();
  }

  if (login_validate($uni,$password,$requestedstatus))
  {
    if (login_isvalid($requestedstatus))
    {
      if (!$destination) $destination = "login.php";
      bounce($destination);
    }
    else
      login_prompt($destination,$requestedstatus,"",$uni);
  }
  else
  {
    login_prompt($destination,$requestedstatus,'<font color=red>Invalid User Name or Password. Please Try Again</font>',$uni);
  }
%>
<%
  require_once("server.inc");
  require_once("login.inc");
  login_protect(login_professor);
 
  if (!$destination) $destination = $server_wcespath . "professors/index.php";
 
  function bounce($url)
  {
    redirect($url);
    if (!redirect($url)) sloppyredirect($url);
    exit();
  }

  if ($profid) { login_saveprofid($profid); }
  bounce($destination);
%>
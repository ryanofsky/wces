<?

  require_once("wbes/server.inc");
  require_once("wces/login.inc");
  require_once("wces/page.inc");

  param($uni,"");                            // username
  param($pw,"");                             // password
  param($url,"");                            // requested url
  $rs = (int) rparam($rs,0);                 // requested status (status needed to access requested page)
  $de = (boolean) rparam($de,false);         // disable encryption

  $badpw = false; $message = "";
  if($uni || $pw) $badpw = !login_validate($uni, $pw);
 
  if ($badpw)
    $message = "<p><b><font color=red>Invalid username or password</font></b></p>";
  else
  {
    $status = login_getstatus();
    if ($rs & $status)
    {
      if ($url) sloppyredirect(addurlparam($url,$PSID));
    }
    else if (($status & login_professor) && ($rs & login_knownprofessor))
      sloppyredirect($server_sbase . $wces_path . "login/profsearch.php?url=" . urlencode($url));
  }

  login_prompt($url, $rs, $de, $uni);

?>
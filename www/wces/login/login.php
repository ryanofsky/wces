<?
ini_set("include_path", ini_get("include_path") . ";:.:/afs/thayer/web/eval/include");

  require_once("wbes/server.inc");
  require_once("wces/login.inc");
  require_once("wces/page.inc");

  param($uni,"");                            // username
  param($pw,"");                             // password
  param($url,"");                            // requested url
  $rs = (int) rparam($rs,0);                 // requested status (status needed to access requested page)
  $de = (boolean) rparam($de,false);         // disable encryption

  if (($uni || $pw) && login_validate($uni, $pw))
  {
    $uni = $pw = "";
    $status = login_getstatus();
    if ($rs & $status || !$rs) // ($status meets at least one required flag) || (there aren't any required flags)
    {
      if ($url) sloppyredirect(addurlparam($url,$USID));
    }
    else if (($status & login_professor) && ($rs & login_knownprofessor))
      sloppyredirect($server_sbase . $wces_path . "login/profsearch.php?url=" . urlencode($url));
  }

  login_prompt($url, $rs, $de, $uni);

?>
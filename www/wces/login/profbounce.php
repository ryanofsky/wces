<?
  require_once("wces/server.inc");
  require_once("wces/login.inc");
  login_protect(login_professor);
  param($url);
  if (!$url) $url = $wces_path . "index.php";
  if ($profid) { login_saveprofid($profid); }
  redirect($url)
?>
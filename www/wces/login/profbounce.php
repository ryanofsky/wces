<?
  require_once("wbes/server.inc");
  require_once("wbes/postgres.inc");
  require_once("wces/login.inc");
  login_protect(login_professor);
  param('url');
  if (!$url) $url = $wces_path . "index.php";
  if (isset($user_id))
  {
    $curuser = login_getuserid();
    $user_id = (int)$user_id;
    if ($curuser != $user_id)
    {
      wces_connect();
      $r = pg_go("SELECT cunix_associate($curuser,$user_id)", $wces, __FILE__, __LINE__);
      $user_id = pg_result($r, 0, 0);
      if ($user_id) login_update($user_id); else die("Failed to update cunix association.");
    } 
  }
  redirect($url)
?>
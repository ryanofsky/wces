<?
  require_once("team/page.inc");
  require_once("wces/database.inc");

  $server_isproduction = false;
  $db_debug = true;

  $team = team_connect();

  $s32r = db_exec("SELECT user_id, cunix FROM users", $team, __FILE__, __LINE__);
  while($row = mysql_fetch_assoc($s32r))
  {
    $ds=ldap_connect("ldap.columbia.edu");
    $r=ldap_bind($ds);
    $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$row[cunix]");  
    $info = ldap_get_entries($ds, $sr);
    $ldap = array("sn" => false, "givenname" => false, "mail" => false);
    if ($info["count"] == 1)
    {
      $result = $info[0];
      foreach($ldap as $key => $value)
      {
        if ((int)$result[$key]["count"] > 0)
          $ldap[$key] = $result[$key][0];
      }
      print("Updating $row[user_id] $row[cunix] $ldap[givenname]<br>");
      db_exec("
        UPDATE users SET firstname = '$ldap[givenname]',
        lastname = '$ldap[sn]', email = '$ldap[mail]'
        WHERE user_id = $row[user_id]
      ", $team, __FILE__, __LINE__);
    }
    else
      print("no info on $row[cunix]<br>");
  }

?>
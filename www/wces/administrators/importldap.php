<?

require_once("wbes/server.inc");

require_once("wbes/taskwindow.inc");
require_once("wces/page.inc");

page_top("LDAP Import");

$db = wces_connect();
$users = db_exec("
  SELECT u.userid, u.cunix
  FROM users AS u
  LEFT JOIN ldapcache AS ld USING (userid)
  WHERE u.email IS NULL", $db, __FILE__, __LINE__);

$ds=ldap_connect("ldap.columbia.edu");
$r=ldap_bind($ds);

$row = 0;
taskwindow_start("Task Window");
while($user = mysql_fetch_assoc($users))
{
  $cunix = $userid = "";
  extract($user);
  $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$cunix");  
  $info = ldap_get_entries($ds, $sr);
  if (isset($info[0]))
  {
    $cn    = addslashes($info[0]["cn"][0]);
    $title = addslashes($info[0]["title"][0]);
    $ou    = addslashes($info[0]["ou"][0]);
    $email = addslashes($info[0]["mail"][0]);
    db_exec("REPLACE INTO ldapcache (userid, cn, title, ou) values ('$userid', '$cn', '$title', '$ou')", $db, __FILE__, __LINE__);
    if ($email)
      db_exec("UPDATE users SET email = '$email' WHERE userid = '$userid'", $db, __FILE__, __LINE__);
    else
      $email = "<i>no email address found</i>"; 
    taskwindow_cprint("<b>$cunix</b> - $email - $cn<br>\n");
  }
  else
  {
    taskwindow_cprint("<b>$cunix</b> - no records found<br>\n");
    //printarray($info,"info");
  }   
  if ((++$row) % 5 == 0) taskwindow_flush();
}
taskwindow_end();
ldap_close($ds);
?>
<?

require_once("wces/server.inc");

require_once("wces/taskwindow.inc");
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
    $cn = $info[0]["cn"][0];
    $title = $info[0]["title"][0];
    $ou = $info[0]["ou"][0];
    $email = $info[0]["mail"][0];
    //db_addrow($db,"ldapcache", array("userid" => $userid, "cn" => $cn, "title" => $title, "ou" => $ou));
    if ($email)
    {
      db_updatevalues($db,"users",array("userid"=>$userid),array("email" => $email));
      taskwindow_cprint("<b>$cunix</b><br>\n - $email<br>\n");
      // - $cn<br>\n - $title<br>\n - $ou<br>\n
    }
  }
  else
  {
    taskwindow_cprint("<b>$cunix</b><br>\n");
    printarray($info,"info");
  }   
  if ((++$row) % 5 == 0) taskwindow_flush();
}
taskwindow_end();
ldap_close($ds);

if ($uni)
{
    print("<h4>Results</h4>\n");
    print("<p><strong>" . $info["count"] . " results found</strong></p>");
    foreach($info as $number => $result)
    if (strcmp($number,"count") != 0)
    {
      print("<h5>" . $result["cn"][0] . "</h5>\n");
      print("<ul>\n");
      foreach($result as $itemname => $itemarray)
      if(is_array($itemarray))
        foreach($itemarray as $key => $value)
        if (strcmp($key,"count") != 0)
          print("  <li><code><b>$itemname</b> = $value</code></li>\n");
      print("</ul>\n");
    }
  }
}
?>
<?

require_once("wbes/server.inc");
require_once("wces/page.inc");

page_top("LDAP Import", true);

wces_connect();

$users = pg_go("
  SELECT u.user_id, u.uni, u.firstname, u.lastname, u.email
  FROM users AS u
  WHERE u.email IS NULL AND u.uni IS NOT NULL
  ORDER BY u.uni
", $wces, __FILE__, __LINE__);

$ds=ldap_connect("ldap.columbia.edu");
$r=ldap_bind($ds);

$n = pg_numrows($users);
for($i = 0; $i < $n; ++$i)
{
  $firstname = $lastname = "";
  extract(pg_fetch_row($users, $i, PGSQL_ASSOC));
  
  bprint("[$i/$n] $uni user_id =$user_id ");

  $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$uni");  
  $info = ldap_get_entries($ds, $sr);
  if ($info['count'] == 1)
  {
    $first = $info[0]["givenname"][0];
    $last = $info[0]["sn"][0];
    $title = $info[0]["title"][0];
    $ou = $info[0]["ou"][0];
    $email = isset($info[0]["mail"][0]) ? $info[0]["mail"][0] : "";

    if ($email)
    {
      bprint(", email = $email");
      pg_go("UPDATE users SET email = '" . addslashes($email) . "' WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
    }
    
    if (!$firstname && !$lastname)
    {
      bprint(", lastname = '$last', firstname = '$first'");
      pg_go("UPDATE users SET firstname = '" . addslashes($first) . "',
        lastname = '" . addslashes($last) . "'
        WHERE user_id = $user_id
      ", $wces, __FILE__, __LINE__);
    }
  }
  else if ($info['count'] != 0)
  {
    printarray($info,"info");
  }   
  bprint("<br>");
}
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

page_bottom(true);

?>
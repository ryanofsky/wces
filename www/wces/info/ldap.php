<%

if (function_exists('ldap_connect'))
{
  require_once("wces/taskwindow.inc");
  require_once("wces/page.inc");

  page_top("LDAP UNI Search");

  if (!isset($uni)) $uni = "";

%>

<form name=f method=get action="ldap.php">
  <p><strong>Lookup a Columbia UNI:</strong>
  <input name=uni type=text size=20 value="<%=htmlspecialchars($uni)%>">
  <input name=go type=submit value="Go">
  </p>
</form>

<% 
if ($uni)
{
    print("<h4>Results</h4>\n");
    $ds=ldap_connect("ldap.columbia.edu");  // must be a valid LDAP server!
    $r=ldap_bind($ds);
    $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$uni");  
    $info = ldap_get_entries($ds, $sr);
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
    ldap_close($ds);
  }
}
else
  readfile('http://160.39.128.25:63' . server_getrequest());
%>
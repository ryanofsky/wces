<%
  require_once("login.inc");
  require_once("page.inc");
  session_start();  
  if (!$destination) $destination = "login.php";
  if (!$requestedstatus) $requestedstatus = login_student;
  if (!$message) $message = "";
  page_top("Login Page");
  if (login_isloggedin())
  {
    print("You are currently logged in as '" . login_getuni() . "' as a " . login_getstatus() . ". Click <a href=logout.php>here</a> to log out, or fill out the form below to log in with a different ID.");
    if (!login_isvalid($requestedstatus))
      print('<br>&nbsp;<br>You have logged in with a valid username and password, but this username has not been given the neccessary permissions to access ' . $destination . ' You can attempt to log in with a different username, or contact an administrator if you believe this to be in error.');
  };    
%>
<form name="login" method="post" action="<%=$server_wcespath%>login/bounce.php"><%=$message%>
<table>
<tr><td>Username:</td><td><input name="uni" type="text" value="<%=htmlspecialchars($uni)%>" size="20"></td></tr>
<tr><td>Password:</td><td><input name="password" type="password" value="" size="20"></td></tr>
<tr><td colspan=2><center><input type="submit" value="Log In" name=submit></center></td></tr>
</table>
<input type="hidden" name=destination value="<%=htmlspecialchars($destination)%>">
<input type="hidden" name=requestedstatus value="<%=htmlspecialchars($requestedstatus)%>">
</form>

<h3>Security Alert</h3>

<p>The information sent in the form above is not encrypted, and could potentially be intercepted
by someone monitoring http traffic on your network. <a href="http://www.columbia.edu/acis/security/">AcIS</a>recommends that you use encryption technology whenever possible, and to this end we've set up twobackup servers that support encryption. Expect the site to be fully encrypted during the upcomingspring evaluation periods.</p><%$params="destination=" . urlencode($destination) . "&requestedstatus=" .urlencode($requestedstatus) . "&message=" . urlencode($message) . "&uni=" . urlencode($uni); %>

<ul>
  <li><a href="https://160.39.209.146/wces/login/login.php?<%=$params%>">Secure Server 1</a></li>
  <li><a href="https://oracle.yahweh.detour.net:999/wces/login/login.php?<%=$params%>">Secure Server 2</a></li>
</ul>

<%
  page_bottom();
%>


<?
  require_once("wces/login.inc");
  require_once("wces/page.inc");
  page_top("Login Page");

  if (!isset($requestedstatus)) $requestedstatus = login_student;
  if (!isset($uni)) $uni = "";
  if (!isset($destination)) $destination = "";
  

  if (login_isloggedin())
  {
    if (!login_isvalid($requestedstatus))
    {
      print("<p><b><font color=red>Access Denied</font></b></p>");
      print("<p>You have logged in with a valid username and password, but this username has not been given the neccessary permissions to access <a href=\"$destination\">$destination</a>. You can attempt to log in with a different username, or contact an administrator if you believe this to be in error.");
    }  
    print("<p><i>You are currently logged in as '" . login_getuni() . "' as a " . login_getstatus() . ". Click <a href=logout.php>here</a> to log out, or fill in the form below to log in with a different ID.</i></p>");
  }   
  else if ($uni)
    print("<p><b><font color=red>Invalid User Name or Password. Please Try Again</font></b></p>");
  else
    print("<p><b>Enter your CUNIX Username and Password below to log in:</b></p>");
?>
<form name="login" method="post" action="<?=$server_wcespath?>login/bounce.php">
<table>
<tr><td>Username:</td><td><input name="uni" type="text" value="<?=htmlspecialchars($uni)?>" size="20"></td></tr>
<tr><td>Password:</td><td><input name="password" type="password" value="" size="20"></td></tr>
<tr><td colspan=2><center><input type="submit" value="Log In" name=submit></center></td></tr>
</table>
<input type="hidden" name=destination value="<?=htmlspecialchars($destination)?>">
<input type="hidden" name=requestedstatus value="<?=htmlspecialchars($requestedstatus)?>">
</form>

<? if (!server_secure()) { ?>

<h3>Security Alert</h3>

<p>The information sent in the form above is not encrypted, and could potentially be intercepted
by someone monitoring http traffic on your network. <a href="http://www.columbia.edu/acis/security/">AcIS</a>recommends that you use encryption whenever possible. To start an encrypted session, click the link below:</p><?$params="destination=" . urlencode($destination) . "&requestedstatus=" .urlencode($requestedstatus) . "&uni=" . urlencode($uni); ?>

<ul>
  <li><a href="<?=$server_sbase . $server_wcespath . "login/login.php?destination=" . urlencode($destination) . "&requestedstatus=" .urlencode($requestedstatus) . "&uni=" . urlencode($uni) . "&SESSION_TRANSFER_ID=" . urlencode(session_id())?>">Secure Server</a></li>
</ul>  

<? }
  page_bottom();
?>

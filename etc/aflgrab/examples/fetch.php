<HTML>
<HEAD>
<META NAME="GENERATOR" Content="Microsoft Visual Studio 6.0">
</HEAD>
<BODY>

<p>Enter a CUNIX username and password below to retrieve AcIS affiliations</p>
<form name=default method=post action=fetch.php>
<TABLE>
  <TR>
    <TD>UNI</TD>
    <TD><INPUT type=text name=uni value="<?php echo $uni; ?>"></TD>
  </TR>
  <TR>
    <TD>Password</TD>
    <TD><INPUT type=password name=password value="<?php echo $password; ?>"></TD>
  </TR>
  <tr><td colspan=2 align=middle><INPUT type="submit" value="Get My Affiliations"></td></tr> 
</TABLE>
</FORM>

<hr>

<?php
  if (isset($uni) && isset($password))
  {
    $grabber = new COM("Oracle.AffilGrabber");
    if (!$grabber->Validate($uni,$password,"https://www1.columbia.edu/~rey4/info.html"))
      print("<p><strong><font color=red>Invalid User Name or Password</font></strong></p>");
    else
      print("<p><strong><font color=blue>Valid Logon</font></strong></p>");
    print('<pre>' . htmlspecialchars($grabber->rawoutput) . '</pre><hr>');
  }
?>



</BODY>
</HTML>

<? 
  if (!isset($uni)) $uni = "";
  if (!isset($pw)) $pw = "";
?>

<HTML>
<HEAD>

<META NAME="GENERATOR" Content="Microsoft Visual Studio 6.0">
</HEAD>
<BODY>

<P>
<form name=default method=post action=fetch.php>
<TABLE>
  <TR>
    <TD>UNI</TD>
    <TD>
    
    <INPUT type=text name=uni value="<?=$uni?>">
    
    </TD></TR>
  <TR>
    <TD>Password</TD>
    <TD>
    
    <INPUT type=password name=pw value="<?=$pw?>">
  </TD></TR>
   <tr>
   <td colspan=2 align=middle><INPUT type="submit" value="Get My Affiliations" id=submit1 name=submit1></td>
   </tr> 
</TABLE>
</P>
<hr>

<?

flush();

if($uni != "" || $pw != "")
{
  $tempdir = "D:\server\shares\php_upload";
  $fname = tempnam($tempdir, "booty");

  $ch = curl_init("https://$uni:$pw@www1.columbia.edu/~rey4/info.html");
  $fp = fopen ($fname, "w");
  curl_setopt ($ch, CURLOPT_FILE, $fp);
  curl_setopt ($ch, CURLOPT_HEADER, 0);
  curl_exec ($ch);
  curl_close ($ch);
  fclose ($fp);

  print("<pre>");
  $fp = fopen($fname,"r");
  print(htmlspecialchars(fread($fp, 100000)));
  fclose($fp);
  print("</pre><hr>");
  unlink($fname);
}

?>

</body>
</html>
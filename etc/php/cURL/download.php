<html>
<title>Student Login</title>
<body>


<?php 
  if (!isset($url)) $url = "http://www.mail2web.com";
?>

<form method=get>
<input name="url" type=text" value="<?php echo $url; ?>" size=30>
<input type="submit" value="Retrieve URL">
</form>


<?php

//$tempdir = "D:\server\shares\php_upload";
$tempdir = "/home/httpd/cgi-bin/123/russ";

$fname = tempnam($tempdir, "booty");
$fnamee = tempnam($tempdir, "booty");

print("Downloading <b>$url</b> to <b>$fname</b></br><hr>");

flush();

$ch = curl_init ($url);
$fp = fopen ($fname, "w");
$fpe = fopen ($fnamee, "w");

curl_setopt ($ch, CURLOPT_FILE, $fp);
curl_setopt ($ch, CURLOPT_HEADER, 1);
curl_setopt ($ch, CURLOPT_VERBOSE, 1);
curl_setopt ($ch, CURLOPT_FOLLOWLOCATION, 0);
curl_setopt ($ch, CURLOPT_STDERR, $fpe);
curl_exec ($ch);
@curl_close ($ch);
fclose ($fp);
fclose($fpe);

$fp = fopen($fname,"r");
print("<pre>");
print(htmlspecialchars(fread($fp, 1000000)));
print("</pre><hr>");
fclose($fp);
unlink($fname);

$fpe = fopen($fnamee,"r");
print("<h1>Errors</h1>");
print("<pre>" . htmlspecialchars(fread($fpe, 1000000)) . "</pre>");
fclose($fpe);
unlink($fnamee);

?>

</body>
</html>
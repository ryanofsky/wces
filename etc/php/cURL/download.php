<html>
<title>Student Login</title>
<body>


<?php 
  if (!$url) $url = "http://www.mail2web.com";
?>

<form method=get>
<input name="url" type=text" value="<?php echo $url; ?>" size=30>
<input type="submit" value="Retrieve URL">
</form>


<?php

//$tempdir = "D:\server\shares\php_upload";
$tempdir = "/home/httpd/cgi-bin/123/russ";

$fname = tempnam($tempdir, "booty");

print("Downloading <b>$url</b> to <b>$fname</b></br><hr>");

flush();

$ch = curl_init ($url);
$fp = fopen ($fname, "w");

curl_setopt ($ch, CURLOPT_FILE, $fp);
curl_setopt ($ch, CURLOPT_HEADER, 1);

curl_exec ($ch);
@curl_close ($ch);
fclose ($fp);

print("<pre>");

$fp = fopen($fname,"r");

print(htmlspecialchars(fread($fp, 1000000)));

fclose($fp);

print("</pre><hr>");

unlink($fname);

?>



</body>
</html>
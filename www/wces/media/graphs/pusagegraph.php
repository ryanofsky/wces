<%
Header("Content-type: image/gif");

if (function_exists('imagecreate'))
{

$RGB["wcesyellow"] = Array(0xFF,0xFB,0x18);
$RGB["wcesblue"] = Array(0x00,0x30,0xE7);
$a = array(array(0,0,0),array(0+$neverloggedin,0+$custom,0+$nocustom));

phpplot(array(
    "cubic" => true,
    "colorset" => array("lred","wcesyellow","wcesblue"),
    
phpdata($a);

phpshow($fname,"gif");
readfile($fname);
unlink($fname);
}
else
  readfile('http://oracle.yahweh.dyndns.org' . server_getrequest());
%>
<%
error_reporting (0);

include('wces/server.inc');
Header("Content-type: image/png");

if (function_exists('imagecreate'))
{
include('VHGraph1_0/class.graph1');

$RGB["wcesyellow"] = Array(0xFF,0xFB,0x18);
$RGB["wcesblue"] = Array(0x00,0x30,0xE7);
$a = array(array(0,0,0),array(0+$neverloggedin,0+$custom,0+$nocustom));

phpplot(array(
    "cubic" => true,
    "transparent" => true,
    "colorset" => array("lred","wcesyellow","wcesblue"),
    "title" => false,
    "size"=> array(200,200) ));
    
phpdata($a);

phpdraw("piegraph", array(
  "scale" => 1.5,
  "showlabel" => false,
  "piemode" => "explode",
  "drawsets" => array(1)
));


$fname = tempnam($server_tempdir, "graph");
phpshow($fname,"png");
readfile($fname);
unlink($fname);
}
else
  readfile('http://160.39.128.25:63' . server_getrequest());
%>
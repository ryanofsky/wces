<%
error_reporting (0);

include('wces/server.inc');
Header("Content-type: image/gif");

if (function_exists('imagecreate'))
{
  include('VHGraph1_0/class.graph1');

$RGB["wcesyellow"] = Array(0xFF,0xFB,0x18);

$a = array(array(0,0),array(0+$blank,0+$filled));

phpplot(array(
    "cubic" => true,
    "transparent" => true,
    "colorset" => array("lred","wcesyellow"),
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
phpshow($fname,"gif");
readfile($fname);
unlink($fname);
}
else
  readfile('http://160.39.128.25:63' . server_getrequest());
%>
<%
Header("Content-type: image/gif");

if (function_exists('imagecreate'))
{
  include('VHGraph1_0/class.graph1');

$RGB["wcesyellow"] = Array(0xFF,0xFB,0x18);

$a = array(array(0,0),array(0+$blank,0+$filled));

phpplot(array(
    "cubic" => true,
    "colorset" => array("lred","wcesyellow"),
    
phpdata($a);

phpshow($fname,"gif");
readfile($fname);
unlink($fname);
}
else
  readfile('http://oracle.yahweh.dyndns.org' . server_getrequest());
%>
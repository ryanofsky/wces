<%
Header("Content-type: image/gif");

if (function_exists('imagecreate'))
{
include('class.graph.inc');
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
{
  //readfile("http://160.39.209.146/wces/administrators/susagegraph.php?blank=$blank&filled=$filled");
  readfile("http://oracle.yahweh.detour.net/wces/administrators/susagegraph.php?blank=$blank&filled=$filled");
};
%>
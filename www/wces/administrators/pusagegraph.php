<%
Header("Content-type: image/gif");

if (function_exists('imagecreate'))
{
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
{
  //readfile("http://160.39.209.146/wces/administrators/pusagegraph.php?neverloggedin=$neverloggedin&custom=$custom&nocustom=$nocustom");
  readfile("http://oracle.yahweh.detour.net/wces/administrators/pusagegraph.php?neverloggedin=$neverloggedin&custom=$custom&nocustom=$nocustom");
};
%>
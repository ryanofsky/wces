<%
$a = array(array(0,0,0),array(0+$neverloggedin,0+$custom,0+$nocustom));

phpplot(array(
    "cubic" => true,
    "colorset" => array("lred","wcesyellow","wcesblue"),
    
phpdata($a);

phpshow($fname,"gif");
readfile($fname);
unlink($fname);
%>
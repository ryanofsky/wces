<%
$a = array(array(0,0),array(0,0),array(0,0));

phpplot(array(
    "legend" => true,
    "cubic" => false,
    "box" => false,
    "grid" => false,
    "colorset" => array("wcesyellow","lred"),
    "lepos" => array(35,50)
     ));
    
phpdata($a);

phpshow($fname,"gif");
readfile($fname);
unlink($fname);
%>
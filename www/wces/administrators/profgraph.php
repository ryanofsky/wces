<%include('server.inc');include('class.graph.inc');
$a = array(array(0,0,0),array(0+$neverloggedin,0+$custom,0+$nocustom));

phpplot(array(
    "cubic" => true,    "transparent" => true,
    "colorset" => array("lred","wcesyellow","wcesblue"),    "title" => false,    "size"=> array(200,200) ));
    
phpdata($a);
phpdraw("piegraph", array(  "scale" => 1.5,  "showlabel" => false,  "piemode" => "explode",  "drawsets" => array(1)));Header("Content-type: image/gif");$fname = tempnam($server_tempdir, "graph");
phpshow($fname,"gif");
readfile($fname);
unlink($fname);
%>
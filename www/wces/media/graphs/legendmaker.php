<%include('wces/server.inc');include('VHGraph1_0/class.graph1');
$a = array(array(0,0),array(0,0),array(0,0));

$RGB["wcesyellow"] = Array(0xFF,0xFB,0x18);
$RGB["wcesblue"] = Array(0x00,0x30,0xE7);

phpplot(array(
    "legend" => true,
    "cubic" => false,
    "box" => false,
    "grid" => false,    "transparent" => true,
    "colorset" => array("wcesyellow","lred"),    "title" => false,    "size"=> array(200,200),
    "lepos" => array(35,50)
     ));
    
phpdata($a);
phpdraw("bargraph",Array("drawsets" => array(1,2), "legend" => Array("Completed Surveys", "Uncompleted Surveys")));Header("Content-type: image/gif");$fname = tempnam($server_tempdir, "graph");
phpshow($fname,"gif");
readfile($fname);
unlink($fname);
%>
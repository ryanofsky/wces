<?
$a = array(array(0,0),array(0,0),array(0,0));

$RGB["wcesyellow"] = Array(0xFF,0xFB,0x18);
$RGB["wcesblue"] = Array(0x00,0x30,0xE7);

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
?>
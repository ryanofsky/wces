<?php


error_reporting(0);
Header("Content-type: image/png");

if (function_exists('imagecreate'))
{
   include('VHGraph1_0/class.graph1');

// pass $list_of_questionperiods, $list_of_avgs

$array_of_questionperiods = explode(",",$list_of_questionperiods);
$array_of_avgs = explode(",",$list_of_avgs);

$a = array(
     $array_of_questionperiods,
     $array_of_avgs);

phpplot(array(
    "box_showbox"=> true,
    "grid"=> true,
    "cubic"=> false,
    "colorset" => array("lblue"),
    "title"=> false,
    "yaxis_labeltext"=> "Average",
    "xaxis_labeltext"=> "",
    "ymarkset"  => array(0,1,2,3,4,5),
    "legend_shift"=> array(-300,10),
    "size"=> array(400,250) ));

phpdata($a);

phpdraw("linepoints",array(
    "drawsets" => array(1),
    "showpoint"=> true,
    "showvalue"=> true ));


$fname = tempnam("", "graph");
phpshow($fname,"png");
readfile($fname);
unlink($fname);
}




?>
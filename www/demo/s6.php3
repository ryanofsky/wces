#!/usr/local/bin/php4
<?php

include('../class.graph1');

$a = array(
     array("Jan.","Feb.","March","April","May","June"),
     array(1.3,1.8,2.3,1.8,1.9,2.4),
     array(1.0,2.1,2.0,1.3,2.3,2.0) );

phpplot(array(
    "box_showbox"=> true,
    "grid"=> true,
    "cubic"=> true,
    "title_text"=> "Mix Graph",
    "yaxis_labeltext"=> "Revenue (in million $)",
    "xaxis_labeltext"=> "Months",
    "legend_shift"=> array(-300,10),
    "size"=> array(400,250) ));

phpdata($a);

phpdraw("bargraph",array(
    "drawsets" => array(1),
    "legend"=> array("First Half Revenue"),
    "barspacing" => 5,
    "showvalue"=> false ));

phpdraw("linepoints",array(
    "drawsets" => array(2),
    "legend" => array("Last Year Result"),
        "legendtype"    => 10,
        "linewidth"     => 2,
        "showpoint"     => false
    ));

phpshow('out.png');

?>

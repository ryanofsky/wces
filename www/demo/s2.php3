#!/usr/local/bin/php4
<?php

include('../class.graph1');

$a = array(
     array("Jan.","Feb.","March","April","May","June"),
     array(1.3,1.8,2.3,1.8,1.9,2.4),
     array(1.0,2.1,2.0,1.3,2.3,2.0) );

phpplot(array(
        "box_showbox"=> true,
        "grid"       => true,
        "cubic"      => true,
        "title_text" => "Real Bar Graph",
        "legend"     => false,
        "xaxis_labeltext" => "Revenue (in million $)",
        "yaxis_labeltext" => "Months",
        "size"       => array(400,250) ));

phpdata($a);

phpdraw("bargraph2",array(
        "drawsets"      => array(1,2),
        "barmode"       => "stack",
        "barspacing"    => 9
       ));

phpshow('out.png');

?>

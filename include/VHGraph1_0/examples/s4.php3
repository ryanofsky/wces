#!/usr/local/bin/php4
<?php

include('../class.graph1');

$a = array(
     array("Jan.","Feb.","March","April","May","June"),
     array(1.3,1.8,2.3,1.8,1.9,2.4),
     array(1.0,2.1,2.0,1.3,2.3,2.0) );

phpplot(array(
    "cubic"=> true,
    "title_text"=> "Pie Graph",
    "size"=> array(400,250) ));

phpdata($a);

phpdraw("piegraph",array( "drawsets" => array(1)));

phpshow('out.png');


?>

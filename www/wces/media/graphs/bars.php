<?php

error_reporting(0);

// Example for use of JpGraph, 
// ljp, 01/03/01 20:32
include ("jpgraph/jpgraph.php");
include ("jpgraph/jpgraph_bar.php");

$datax = @unserialize($datax);
$datay = @unserialize($datay);

$graphWidth = 300;
$graphHeight = 200;
$graphMargin = 30;

$bottom = $height - $graphHeight - 2 * $graphMargin;
$left = $width - $graphWidth - 2 * $graphMargin;

// Setup the graph. 
$graph = new Graph($width,$height,"auto");	
$graph->img->SetMargin($graphMargin + $left, $graphMargin, $graphMargin, $graphMargin + $bottom);
$graph->SetScale("textlin");
$graph->SetMarginColor("silver");
$graph->SetShadow();

// Set up the title for the graph
//$graph->title->Set("Example bar gradient fill");
$graph->title->SetFont(FF_VERDANA,FS_NORMAL,18);
$graph->title->SetColor("darkred");

// Setup font for axis
$graph->xaxis->SetFont(FF_VERDANA,FS_NORMAL,12);
$graph->yaxis->SetFont(FF_VERDANA,FS_NORMAL,11);

// Show 0 label on Y-axis (default is not to show)
$graph->yscale->ticks->SupressZeroLabel(false);

// Setup X-axis labels
$graph->xaxis->SetTickLabels($datax);
$graph->xaxis->SetLabelAngle(50);

// Create the bar pot
$bplot = new BarPlot($datay);
$bplot->SetWidth(0.4);

// Setup color for gradient fill style 
$bplot->SetFillGradient("navy","lightsteelblue",GRAD_MIDVER);

// Set color for the frame of each bar
$bplot->SetColor("navy");
$graph->Add($bplot);

// Finally send the graph to the browser
$graph->Stroke();
?>

<?php

error_reporting(0);

require_once("jpgraph/jpgraph.php");
require_once("jpgraph/jpgraph_bar.php");
require_once("wces/report_help.inc");

$datax = @unserialize($datax);
$datay = @unserialize($datay);

$params = compact("width", "height");
GraphSize($params, $datax);
extract($params);

// Setup the graph. 
$graph = new Graph($width,$height,"auto");	
$graph->img->SetMargin($graphMargin + $left, $graphMargin, $graphMargin, $graphMargin + $bottom);
$graph->SetScale("textlin");
$graph->SetMarginColor("silver");
$graph->SetShadow();
$graph->SetColor(array(254,254,254));
$graph->img->SetTransparent("white");

// Set up the title for the graph
//$graph->title->Set("Example bar gradient fill");
$graph->title->SetFont(FF_VERDANA,FS_NORMAL,18);
$graph->title->SetColor("darkred");

// Setup font for axis
$graph->xaxis->SetFont($family,$style,$size);
$graph->yaxis->SetFont(FF_VERDANA,FS_NORMAL,11);

// Show 0 label on Y-axis (default is not to show)
$graph->yscale->ticks->SupressZeroLabel(false);

// Setup X-axis labels
$graph->xaxis->SetTickLabels($datax);
$graph->xaxis->SetLabelAngle($angle);

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

<?php

error_reporting(0);

require_once("jpgraph/jpgraph.php");
require_once("jpgraph/jpgraph_bar.php");
require_once("wces/report_help.inc");
require_once("wbes/png.inc");

$filename = png_start(__FILE__, 60*60*24);
$width = $_GET['width'];
$height = $_GET['height'];
$datax = @unserialize($_GET['datax']);
$datay = @unserialize($_GET['datay']);
$title = @(string)$_GET['title'];

$params = compact("width", "height");
LineGraphSize($params, $datax);
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
$graph->title->Set($title);
$graph->title->SetFont(FF_VERDANA,FS_NORMAL,12);
$graph->title->SetColor("black");

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
$graph->Stroke($filename);

png_end($filename);

?>
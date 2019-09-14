<?php 
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------

session_start();

include("phpgraphlib.php");

$interval = 0;
$format = "degrees";

if ( isset($_GET['XInterval']) )
	$interval = $_GET['XInterval'];
if ( isset($_GET['Format']) )
	$format = $_GET['Format'];


$graph = new PHPGraphLib(400,320);

// 4 data sets maximum
if ( isset($_SESSION['graph_data'][3]['data']) )
{
	$graph->addData( $_SESSION['graph_data'][0]['data'], $_SESSION['graph_data'][1]['data'], $_SESSION['graph_data'][2]['data'], $_SESSION['graph_data'][3]['data'] );
	$graph->setLegendTitle( $_SESSION['graph_data'][0]['name'], $_SESSION['graph_data'][1]['name'], $_SESSION['graph_data'][2]['name'], $_SESSION['graph_data'][3]['name'] );
}
else if ( isset($_SESSION['graph_data'][2]['data']) )
{
	$graph->addData( $_SESSION['graph_data'][0]['data'], $_SESSION['graph_data'][1]['data'], $_SESSION['graph_data'][2]['data'] );
	$graph->setLegendTitle( $_SESSION['graph_data'][0]['name'], $_SESSION['graph_data'][1]['name'], $_SESSION['graph_data'][2]['name'] );
}
else if ( isset($_SESSION['graph_data'][1]['data']) )
{
	$graph->addData( $_SESSION['graph_data'][0]['data'], $_SESSION['graph_data'][1]['data'] );
	$graph->setLegendTitle( $_SESSION['graph_data'][0]['name'], $_SESSION['graph_data'][1]['name'] );
}
else
{
	$graph->addData( $_SESSION['graph_data'][0]['data'] );
	$graph->setLegendTitle( $_SESSION['graph_data'][0]['name'] );
}

$graph->setLegend(true);
$graph->setLine(true);
$graph->setBars(false);
$graph->setDataPoints(true);
$graph->setDataPointColor('olive');
$graph->setDataPointSize(2);
if ( $interval != 0 )
{
	$graph->setXValuesInterval( intval($interval) );
}
$graph->setDataFormat( $format );
//$graph->setTitle( $name );
$graph->setLineColor("navy","teal","aqua");
$graph->setTextColor("blue");
$graph->createGraph();



?>
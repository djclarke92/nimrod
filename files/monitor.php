<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------

include_once( "common.php" );

if ( !isset($_SESSION['us_AuthLevel']) )
{	// access not via main page - access denied
    func_unauthorisedaccess();
    return;
}

function func_find_monitor_device( $de_no, $ch )
{
    $rc = false;
    
    foreach( $_SESSION['MonitorDevices'] as $gg )
    {
        if ( $gg['di_DeviceNo'] == $de_no && $gg['di_IOChannel'] == $ch )
        {
            $rc = true;
            break;
        }
    }
    
    return $rc;
}



if ( !isset($_SESSION['MonitorDevices']) )
    $_SESSION['MonitorDevices'] = array();
if ( !isset($_SESSION['MonitorPeriod']) )
    $_SESSION['MonitorPeriod'] = 1.0;
if ( !isset($_SESSION['MonitorDate']) )
    $_SESSION['MonitorDate'] = "";
if ( !isset($_SESSION['MonitorTime']) )
    $_SESSION['MonitorTime'] = "";
                

$error_msg = "";
$info_msg = "";
$monitor_period = 1.0;
$monitor_date = "";
$monitor_time = "";
$monitor_page = 1;
if ( isset($_GET['PageMode']) && $_GET['PageMode'] == "Monitor2" )
{
    $monitor_page = 2;
}
if ( isset($_GET['MonitorPage']) && $_GET['MonitorPage'] == "2" )
{
    $monitor_page = 2;
}


// $_POST vars read in index.php
$monitor_period = $_SESSION['MonitorPeriod'];
$monitor_date = $_SESSION['MonitorDate'];
$monitor_time = $_SESSION['MonitorTime'];

if ( $monitor_date != "" && $monitor_time == "" )
{
    $monitor_time = date("H:i");
}
if ( $monitor_date == "" && $monitor_time != "" )
{
    $monitor_date = sprintf( "%02d/%02d/%d", date("d/m/Y"));
}

$datetime = time();
if ( $monitor_date != "" && $monitor_time != "" )
{
    $hour = 0;
    $min = 0;
    $day = 0;
    $month = 0;
    $year = 0;
    $expl = explode( ":", $monitor_time );
    if ( isset($expl[0]) && isset($expl[1]) )
    {
        $hour = $expl[0];
        $min = $expl[1];
    }
    $expl = explode( "/", $monitor_date );
    if ( isset($expl[0]) && isset($expl[1]) && isset($expl[2]) )
    {
        $day = $expl[0];
        $month = $expl[1];
        $year = $expl[2];
    }
    
    $dd = mktime( $hour, $min, 0, $month, $day, $year );
    if ( $dd !== false )
    {
        $datetime = $dd;
    }
}


$temperatures = $db->GetLatestTemperatures( $monitor_period, $datetime ); // previous 2 hours data
$voltages = $db->GetLatestVoltages( $monitor_period, $datetime ); // previous 2 hours data
$levels = $db->GetLatestLevels( $monitor_period, $datetime );
$currents = $db->GetLatestCurrents( $monitor_period, $datetime );
$powers = $db->GetLatestPowers( $monitor_period, $datetime );
$frequencies = $db->GetLatestFrequencies( $monitor_period, $datetime );
$torques = $db->GetLatestTorques( $monitor_period, $datetime );
$rpmspeeds = $db->GetLatestRpmSpeeds( $monitor_period, $datetime );


$f1_devices = $db->GetMonitorDevices( sprintf( "%dF1", $monitor_page ) );
if ( count($f1_devices) != 0 )
{
    $f1_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $f1_devices );
}
else
{
    $l1_devices = $db->GetMonitorDevices( sprintf( "%dL1", $monitor_page ) );
    $r1_devices = $db->GetMonitorDevices( sprintf( "%dR1", $monitor_page ) );

    $l1_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $l1_devices );
    $r1_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $r1_devices );
}
$f2_devices = $db->GetMonitorDevices( sprintf( "%dF2", $monitor_page ) );
if ( count($f2_devices) != 0 )
{
    $f2_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $f2_devices );
}
else
{
    $l2_devices = $db->GetMonitorDevices( sprintf( "%dL2", $monitor_page ) );
    $r2_devices = $db->GetMonitorDevices( sprintf( "%dR2", $monitor_page ) );
    
    $l2_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $l2_devices );
    $r2_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $r2_devices );
}
$f3_devices = $db->GetMonitorDevices( sprintf( "%dF3", $monitor_page ) );
if ( count($f3_devices) != 0 )
{
    $f3_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $f3_devices );
}
else
{
    $l3_devices = $db->GetMonitorDevices( sprintf( "%dL3", $monitor_page ) );
    $r3_devices = $db->GetMonitorDevices( sprintf( "%dR3", $monitor_page ) );
    
    $l3_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $l3_devices );
    $r3_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $r3_devices );
}
$f4_devices = $db->GetMonitorDevices( sprintf( "%dF4", $monitor_page ) );
if ( count($f4_devices) != 0 )
{
    $f4_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $f4_devices );
}
else
{
    $l4_devices = $db->GetMonitorDevices( sprintf( "%dL4", $monitor_page ) );
    $r4_devices = $db->GetMonitorDevices( sprintf( "%dR4", $monitor_page ) );
    
    $l4_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $l4_devices );
    $r4_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $torques, $rpmspeeds, $r4_devices );
}


$alert_width = "8";     // percent
$graph_bgcolor = "#AABBCC";
$alert_okcolor = "#58D68D";
$alert_ngcolor = "#FE2E2E";


//printf( "<tr><td colspan='3'>" );

printf( "<div id='tablediv' style='position: absolute; left: 10px; right: 15px; top: 60px; bottom: 10px;'>" );

printf( "<table width='100%%' height='100%%'>" );

printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f1_devices) != 0 )
{
    func_draw_graph_div( false, "graphdiv1f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f1_data );
}
else
{
    func_draw_graph_div( false, "graphdiv1l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l1_data );
    func_draw_graph_div( false, "graphdiv1r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r1_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );

printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f2_devices) != 0 )
{
    func_draw_graph_div( false, "graphdiv2f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f2_data );
}
else
{
    func_draw_graph_div( false, "graphdiv2l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l2_data );
    func_draw_graph_div( false, "graphdiv2r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r2_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );

printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f3_devices) != 0 )
{
    func_draw_graph_div( false, "graphdiv3f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f3_data );
}
else
{
    func_draw_graph_div( false, "graphdiv3l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l3_data );
    func_draw_graph_div( false, "graphdiv3r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r3_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );


printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f4_devices) != 0 )
{
    func_draw_graph_div( false, "graphdiv4f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f4_data );
}
else
{
    func_draw_graph_div( false, "graphdiv4l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l4_data );
    func_draw_graph_div( false, "graphdiv4r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r4_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );

printf( "</table>" );


if ( count($f1_devices) != 0 )
{
    func_create_graph( $f1_data, "graphdiv1f" );
}
else 
{
    if ( count($l1_devices) != 0 )
    {
        func_create_graph( $l1_data, "graphdiv1l" );
    }
    if ( count($r1_devices) != 0 )
    {
        func_create_graph( $r1_data, "graphdiv1r" );
    }
}
if ( count($f2_devices) != 0 )
{
    func_create_graph( $f2_data, "graphdiv2f" );
}
else 
{
    if ( count($l2_devices) != 0 )
    {
        func_create_graph( $l2_data, "graphdiv2l" );
    }
    if ( count($r2_devices) != 0 )
    {
        func_create_graph( $r2_data, "graphdiv2r" );
    }
}
if ( count($f3_devices) != 0 )
{
    func_create_graph( $f3_data, "graphdiv3f" );
}
else 
{
    if ( count($l3_devices) != 0 )
    {
        func_create_graph( $l3_data, "graphdiv3l" );
    }
    if ( count($r3_devices) != 0 )
    {
        func_create_graph( $r3_data, "graphdiv3r" );
    }
}
if ( count($f4_devices) != 0 )
{
    func_create_graph( $f4_data, "graphdiv4f" );
}
else 
{
    if ( count($l4_devices) != 0 )
    {
        func_create_graph( $l4_data, "graphdiv4l" );
    }
    if ( count($r4_devices) != 0 )
    {
        func_create_graph( $r4_data, "graphdiv4r" );
    }
}

printf( "</div>" );

//printf( "</td>" );
//printf( "</tr>" );


//printf( "<tr>" );
//printf( "<td colspan='3'>" );
	


//printf( "</td>" );
//printf( "</tr>" );




?>

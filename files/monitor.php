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

function func_get_graph_data( $temperatures, $devices )
{
    $data = array();
    foreach ( $devices as $gg )
    {
        $atype = "V";
        $gname = "?";
        $gvoltage = false;
        
        $found = false;
        foreach ( $temperatures as $tt )
        {
            if ( $tt['di_DeviceNo'] == $gg['di_DeviceNo'] && $tt['di_IOChannel'] == $gg['di_IOChannel'] )
            {
                $found = true;
                $myarray = $tt;
                $gname = $tt['di_IOName'];
                break;
            }
        }
        if ( $found == false )
        {
            $gvoltage = true;
            foreach ( $voltages as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['di_DeviceNo'] && $tt['di_IOChannel'] == $gg['di_IOChannel'] )
                {
                    $found = true;
                    $myarray = $tt;
                    $gname = $tt['di_IOName'];
                    $atype = $tt['di_AnalogType'];
                    break;
                }
            }
        }
    
        if ( $found )
        {
            $alert = false;
            if ( $myarray['di_MonitorLo'] != 0 && $myarray['di_MonitorHi'] != 0 )
            {   // check if the last value is in the alert state
                $val = $myarray['data'][count($myarray['data'])-1]['ev_Value'];
                if ( func_calc_temperature($val) < $myarray['di_MonitorLo'] || func_calc_temperature($val) > $myarray['di_MonitorHi'] )
                {
                    $alert = true;
                }
            }
            
            $data[] = array( 'data'=>$myarray['data'], 'name'=>$gname, 'voltage'=>$gvoltage, 'atype'=>$atype, 'alert'=>$alert, 'di_MonitorLo'=>$myarray['di_MonitorLo'], 'di_MonitorHi'=>$myarray['di_MonitorHi'],
                'di_ValueRangeLo'=>$myarray['di_ValueRangeLo'], 'di_ValueRangeHi'=>$myarray['di_ValueRangeHi'], 'SeqNo'=>0 );
            
        }
    }
    
    $cc = 0;
    foreach ( $data as &$dd )
    {
        $dd['SeqNo'] = $cc;
        
        $cc = $cc + 1;
    }
    
    return $data;
}

function func_create_graph( $gdata, $divname )
{
    printf( "<script type='text/javascript'>" );
    printf( "g = new Dygraph(" );
    
    // containing div
    printf( "document.getElementById('%s'),", $divname );
    
    $num_lines = count($gdata);
    
    $combined = array();
    foreach ( $gdata as $graph )
    {
        foreach ( $graph['data'] as $data )
        {
            $combined[] = array( 'ev_Timestamp'=>$data['ev_Timestamp'], 'ev_Value'=>$data['ev_Value'], 'SeqNo'=>$graph['SeqNo'] );   
        }
    }
    array_multisort( $combined );
    
    $combined2 = array();
    $cc = 0;
    while ( $cc < count($combined) )
    {
        $vv = array();
        for ( $i = 0; $i < $num_lines; $i++ )
        {
            $vv[$i] = "null";
        }
        $vv[$combined[$cc]['SeqNo']] = $combined[$cc]['ev_Value'];
        
        $ts = $combined[$cc]['ev_Timestamp'];
        $n = 1;
        while ( $cc+1 < count($combined) && $ts == $combined[$cc+$n]['ev_Timestamp'] )
        {
            $cc = $cc + 1;
            $vv[$combined[$cc]['SeqNo']] = $combined[$cc]['ev_Value'];
            
            $n = $n + 1;
        }
        
        $combined2[] = array( 'ev_Timestamp'=>$ts, 'values'=>$vv );
           
        $cc = $cc + 1;
    }
    
    printf( "[" );
    $count = 0;
    foreach ( $combined2 as $data )
    {
        printf( "[new Date('%s')", $data['ev_Timestamp'] );
    
        foreach ( $data['values'] as $vv )
        {
            if ( $vv == "null" )
                printf( ",null" );
            else
                printf( ",%.1f", func_calc_temperature($vv) );
        }
        
        printf( "]" );
        
        $count += 1;
        if ( $count < count($combined2) )
        {
            printf( "," );
        }
    }
    
    printf("],");
    
    printf( "{ labels: ['Date', '%s' ", $gdata[0]['name'] );
    $count = 1;
    while ( $count < $num_lines )
    {
        printf( ", '%s' ", $gdata[$count]['name'] );
        $count += 1;
    }
    printf( "], " );

    $valRangeLo = "";
    $valRangeHi = "";
    foreach ( $gdata as $gg )
    {
        if ( $gg['di_ValueRangeLo'] != "" )
        {
            if ( $valRangeLo == "" && $gg['di_ValueRangeLo'] == "null" )
            {
                $valRangeLo = "null";
            }
            else if ( floatval($valRangeLo) > floatval($gg['di_ValueRangeLo']) )
            {
                $valRangeLo = floatval($gg['di_ValueRangeLo']);
            }
        }
        if ( $gg['di_ValueRangeHi'] != "" )
        {
            if ( $valRangeHi == "" && $gg['di_ValueRangeHi'] == "null" )
            {
                $valRangeHi = "null";
            }
            else if ( floatval($valRangeHi) < floatval($gg['di_ValueRangeHi']) )
            {
                $valRangeHi = floatval($gg['di_ValueRangeHi']);
            }
        }
    }
    
    //printf( "  title: 'Temperature (C)', " );
    printf( "  legend: 'always', " );
    printf( "  connectSeparatedPoints: true, " );
    //printf( "  showRangeSelector: true, " );
    //printf( "  rangeSelectorPlotFillColor: 'MediumSlateBlue', " );
    //printf( "  rangeSelectorPlotFillGradientColor: 'rgba(123, 104, 238, 0)', " );
    //printf( "  colorValue: 0.9, " );
    //printf( "  fillAlpha: 0.4, " );
    if ( $valRangeLo != "" && $valRangeHi != "" )
    {
        if ( $valRangeLo == "null" && $valRangeHi == "null" )
            printf( "  valueRange: [%s,%s],", $valRangeLo, $valRangeHi );
        else if ( $valRangeLo == "null" )
            printf( "  valueRange: [%s,%.1f],", $valRangeLo, $valRangeHi );
        else
            printf( "  valueRange: [%.1f,%.1f],", $valRangeLo, $valRangeHi );
    }
    printf( "  drawPoints: true, " );
    printf( "  underlayCallback: function(canvas, area, g) {" );
    
    printf( "    canvas.fillStyle = 'rgba(255, 179, 179, 0.5)';" );    
    printf( "    function highlight_period(x_start, x_end) {" );
    printf( "      var canvas_left_x = g.toDomXCoord(x_start);" );
    printf( "      var canvas_right_x = g.toDomXCoord(x_end);" );
    printf( "      var canvas_width = canvas_right_x - canvas_left_x;" );
    printf( "      canvas.fillRect(canvas_left_x, area.y, canvas_width, area.h);" );
    printf( "    }" );
    
    printf( "    var min_data_x;" );
    printf( "    var max_data_x;" );
    printf( "    var start_x_highlight;" );
    printf( "    var end_x_highlight;" );
    printf( "    var w;" );
    printf( "    var didx = 1;" );
    
    $count = 0;
    while ( $count < $num_lines )
    {
    
    printf( "    w = 0;" );
    printf( "    min_data_x = g.getValue(w,0);" );
    printf( "    while ( min_data_x == null ) {" );
    printf( "       w += 1;" );
    printf( "       min_data_x = g.getValue(w,0);" );
    printf( "    }" );
    
    printf( "    w = g.numRows()-1;" );
    printf( "    max_data_x = g.getValue(w,0);" );
    printf( "    while ( max_data_x == null ) {" );
    printf( "       w -= 1;" );
    printf( "       max_data_x = g.getValue(w,0);" );
    printf( "    }" );
    
    printf( "    w = 0;" );
    printf( "    start_x_highlight = 0;" );
    printf( "    end_x_highlight = 0;" );
    printf( "    if ( %f != 0.0 && %f != 0.0 ) {", $gdata[$count]['di_MonitorLo'], $gdata[$count]['di_MonitorHi'] );
    printf( "      while (w < g.numRows()) {" );

    printf( "        if ( g.getValue(w,didx) == null || g.getValue(w,0) == null ) {" );
    printf( "        } else if ( g.getValue(w,didx) < %f || g.getValue(w,didx) > %f ) {", $gdata[$count]['di_MonitorLo'], $gdata[$count]['di_MonitorHi'] );
    printf( "          /* value is out of range */" );
    printf( "          if ( start_x_highlight == 0 ) {" );
    printf( "            if ( w > 0 ) {" );
    printf( "              start_x_highlight = g.getValue(w-1,0);" );
    printf( "              if ( start_x_highlight == null )" );
    printf( "                start_x_highlight = g.getValue(w,0);" );
    printf( "            } else" );
    printf( "              start_x_highlight = g.getValue(w,0);" );
    printf( "            if ( start_x_highlight < min_data_x )" );
    printf( "              start_x_highlight = min_data_x;" );
    printf( "            end_x_highlight = start_x_highlight;" );
    printf( "          }" );
    printf( "        } else {" );
    printf( "          /* value is within range again */" );
    printf( "          end_x_highlight = g.getValue(w-1,0);" );
//    printf( "          if ( end_x_highlight == null || end_x_highlight > max_data_x )" );
//    printf( "            end_x_highlight = g.getValue(w,0);" );
    printf( "          if ( start_x_highlight > 0 ) {" );
    printf( "            /* highlight the previous out of range block */" );
    printf( "            highlight_period(start_x_highlight,end_x_highlight);" );
    printf( "            start_x_highlight = 0;" );
    printf( "            end_x_highlight = 0;" );
    printf( "          }" );
    printf( "        }" );
        
    printf( "        if ( w+1 >= g.numRows() && start_x_highlight > 0 ) {" );
    printf( "          end_x_highlight = g.getValue(g.numRows()-1,0);" );
//    printf( "          if ( end_x_highlight > max_data_x )" );
//    printf( "            end_x_highlight = max_data_x;" );
    printf( "          highlight_period(start_x_highlight,end_x_highlight);" );
    printf( "        }" );
    
    printf( "        w += 1;" );
    printf( "      }" );
    printf( "    }" );
    
    printf( "    didx = didx + 1;" );
    
        $count += 1;
    }
    
    printf( "  }" );    // end callback function
    
    printf( "}" );
    
    printf( ");" );
    printf( "</script>" );
}

function func_draw_graph_div( $div_name, $graph_per_line, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $div_data )
{
    $graph_width = (100 / $graph_per_line) - ($alert_width / $graph_per_line);
    $al_width = $alert_width / $graph_per_line;
    $a_bgcolor = $graph_bgcolor;
    $a_name = "";
    $a_nameok = "";
    printf( "<td width='%d%%'><div id='%s' style='height:100%%; width:100%%; background-color:%s; position:relative;'></div></td>", $graph_width, $div_name, $graph_bgcolor );
    if ( count($div_data) != 0 )
    {   // set alert color
        $a_bgcolor = $alert_okcolor;
        foreach ( $div_data as $dat )
        {
            if ( $a_nameok != "" )
                $a_nameok .= "<br>";
            $a_nameok .= $dat['name'];
                
            if ( $dat['alert'] )
            {
                $a_bgcolor = $alert_ngcolor;
                if ( $a_name != "" )
                    $a_name .= "<br>";
                    $a_name .= $dat['name'];
            }
        }
    }
    printf( "<td width='%d%%' style='background-color: %s; text-align: center;'><h2>%s</h2></td>", $al_width, $a_bgcolor, ($a_name != "" ? $a_name : $a_nameok) );
}



if ( !isset($_SESSION['MonitorDevices']) )
    $_SESSION['MonitorDevices'] = array();
if ( !isset($_SESSION['MonitorPeriod']) )
    $_SESSION['MonitorPeriod'] = 2.0;
if ( !isset($_SESSION['MonitorDate']) )
    $_SESSION['MonitorDate'] = "";
if ( !isset($_SESSION['MonitorTime']) )
    $_SESSION['MonitorTime'] = "";
                

$error_msg = "";
$info_msg = "";
$monitor_period = 2.0;
$monitor_date = "";
$monitor_time = "";
$monitor_page = 1;
if ( $_SESSION['page_mode'] == "Monitor2" )
{
    $monitor_page = 2;
}


// $_POST vars read in index.php
$monitor_period = $_SESSION['MonitorPeriod'];
$monitor_date = $_SESSION['MonitorDate'];
$monitor_time = $_SESSION['MonitorTime'];

    
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

$f1_devices = $db->GetMonitorTemperatures( sprintf( "%dF1", $monitor_page ) );
if ( count($f1_devices) != 0 )
{
    $f1_data = func_get_graph_data( $temperatures, $f1_devices );
}
else
{
    $l1_devices = $db->GetMonitorTemperatures( sprintf( "%dL1", $monitor_page ) );
    $r1_devices = $db->GetMonitorTemperatures( sprintf( "%dR1", $monitor_page ) );

    $l1_data = func_get_graph_data( $temperatures, $l1_devices );
    $r1_data = func_get_graph_data( $temperatures, $r1_devices );
}
$f2_devices = $db->GetMonitorTemperatures( sprintf( "%dF2", $monitor_page ) );
if ( count($f2_devices) != 0 )
{
    $f2_data = func_get_graph_data( $temperatures, $f2_devices );
}
else
{
    $l2_devices = $db->GetMonitorTemperatures( sprintf( "%dL2", $monitor_page ) );
    $r2_devices = $db->GetMonitorTemperatures( sprintf( "%dR2", $monitor_page ) );
    
    $l2_data = func_get_graph_data( $temperatures, $l2_devices );
    $r2_data = func_get_graph_data( $temperatures, $r2_devices );
}
$f3_devices = $db->GetMonitorTemperatures( sprintf( "%dF3", $monitor_page ) );
if ( count($f3_devices) != 0 )
{
    $f3_data = func_get_graph_data( $temperatures, $f3_devices );
}
else
{
    $l3_devices = $db->GetMonitorTemperatures( sprintf( "%dL3", $monitor_page ) );
    $r3_devices = $db->GetMonitorTemperatures( sprintf( "%dR3", $monitor_page ) );
    
    $l3_data = func_get_graph_data( $temperatures, $l3_devices );
    $r3_data = func_get_graph_data( $temperatures, $r3_devices );
}
$f4_devices = $db->GetMonitorTemperatures( sprintf( "%dF4", $monitor_page ) );
if ( count($f4_devices) != 0 )
{
    $f4_data = func_get_graph_data( $temperatures, $f4_devices );
}
else
{
    $l4_devices = $db->GetMonitorTemperatures( sprintf( "%dL4", $monitor_page ) );
    $r4_devices = $db->GetMonitorTemperatures( sprintf( "%dR4", $monitor_page ) );
    
    $l4_data = func_get_graph_data( $temperatures, $l4_devices );
    $r4_data = func_get_graph_data( $temperatures, $r4_devices );
}


$alert_width = "8";     // percent
$graph_bgcolor = "#AABBCC";
$alert_okcolor = "#58D68D";
$alert_ngcolor = "#FE2E2E";

$a1f_bgcolor = $graph_bgcolor;
$a2f_bgcolor = $graph_bgcolor;
$a2l_bgcolor = $graph_bgcolor;
$a2r_bgcolor = $graph_bgcolor;
$a3f_bgcolor = $graph_bgcolor;
$a3l_bgcolor = $graph_bgcolor;
$a3r_bgcolor = $graph_bgcolor;
$a4f_bgcolor = $graph_bgcolor;
$a4l_bgcolor = $graph_bgcolor;
$a4r_bgcolor = $graph_bgcolor;

$a1f_name = "";
$a1f_nameok = "";
$a2f_name = "";
$a2f_nameok = "";
$a2l_name = "";
$a2l_nameok = "";
$a2r_name = "";
$a2r_nameok = "";
$a3f_name = "";
$a3f_nameok = "";
$a3l_name = "";
$a3l_nameok = "";
$a3r_name = "";
$a3r_nameok = "";
$a4f_name = "";
$a4f_nameok = "";
$a4l_name = "";
$a4l_nameok = "";
$a4r_name = "";
$a4r_nameok = "";


printf( "<tr><td colspan='3'>" );

printf( "<div id='tablediv' style='position: absolute; left: 10px; right: 15px; top: 60px; bottom: 15px;'>" );

printf( "<table width='100%%' height='100%%'>" );

printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f1_devices) != 0 )
{
    func_draw_graph_div( "graphdiv1f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f1_data );
}
else
{
    func_draw_graph_div( "graphdiv1l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l1_data );
    func_draw_graph_div( "graphdiv1r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r1_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );

printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f2_devices) != 0 )
{
    func_draw_graph_div( "graphdiv2f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f2_data );
}
else
{
    func_draw_graph_div( "graphdiv2l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l2_data );
    func_draw_graph_div( "graphdiv2r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r2_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );

printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f3_devices) != 0 )
{
    func_draw_graph_div( "graphdiv3f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f3_data );
}
else
{
    func_draw_graph_div( "graphdiv3l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l3_data );
    func_draw_graph_div( "graphdiv3r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r3_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );


printf( "<tr height='25%%' valign='top'><td width='100%%'>" );

printf( "<table width='100%%' height='100%%' border='0'><tr>" );
if ( count($f4_devices) != 0 )
{
    func_draw_graph_div( "graphdiv4f", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $f4_data );
}
else
{
    func_draw_graph_div( "graphdiv4l", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $l4_data );
    func_draw_graph_div( "graphdiv4r", 2, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $r4_data );
}
printf( "</tr></table>" );

printf( "</td></tr>" );

printf( "</table>" );


if ( count($f1_devices) != 0 )
{
    func_create_graph( $f1_data, "graphdiv1f" );
}
else if ( count($l1_devices) != 0 )
{
    func_create_graph( $l1_data, "graphdiv1l" );
    if ( count($r1_devices) != 0 )
    {
        func_create_graph( $r1_data, "graphdiv1r" );
    }
}
if ( count($f2_devices) != 0 )
{
    func_create_graph( $f2_data, "graphdiv2f" );
}
else if ( count($l2_devices) != 0 )
{
    func_create_graph( $l2_data, "graphdiv2l" );
    if ( count($r2_devices) != 0 )
    {
        func_create_graph( $r2_data, "graphdiv2r" );
    }
}
if ( count($f3_devices) != 0 )
{
    func_create_graph( $f3_data, "graphdiv3f" );
}
else if ( count($l3_devices) != 0 )
{
    func_create_graph( $l3_data, "graphdiv3l" );
    if ( count($r3_devices) != 0 )
    {
        func_create_graph( $r3_data, "graphdiv3r" );
    }
}
if ( count($f4_devices) != 0 )
{
    func_create_graph( $f4_data, "graphdiv4f" );
}
else if ( count($l4_devices) != 0 )
{
    func_create_graph( $l4_data, "graphdiv4l" );
    if ( count($r4_devices) != 0 )
    {
        func_create_graph( $r4_data, "graphdiv4r" );
    }
}

printf( "</div>" );

printf( "</td>" );
printf( "</tr>" );


printf( "<tr>" );
printf( "<td colspan='3'>" );
	


printf( "</td>" );
printf( "</tr>" );




?>

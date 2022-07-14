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

function func_find_deviceinfo_name( $di_list, $device_no, $iochannel, $rule_type )
{
    $name = sprintf( "(%d,%d)", $device_no, $iochannel+1 );
    if ( $device_no == 0 && $iochannel == -1 )
    {   // immediate timer
        $name = sprintf( "Timer" );
    }
    else
    {
        foreach ( $di_list as $di )
        {
            if ( $di['di_DeviceNo'] == $device_no && $di['di_IOChannel'] == $iochannel && $rule_type == "I" && $di['di_IOType'] == E_IO_OUTPUT )
            {
                $name = sprintf( "%s", $di['di_IOName'] );
                break;
            }
            else if ( $di['di_DeviceNo'] == $device_no && $di['di_IOChannel'] == $iochannel && $rule_type == "E" && $di['di_IOType'] != E_IO_OUTPUT )
            {
                $name = sprintf( "%s", $di['di_IOName'] );
                break;
            }
        }
    }
    
    return $name;
}

$print_op = false;

if ( !isset($_SESSION['plc_Operation']) )
    $_SESSION['plc_Operation'] = "";
if ( !isset($_SESSION['plc_StateName']) )
    $_SESSION['plc_StateName'] = "";
        
    
if ( isset($_POST['ps_Operation']) )
    $_SESSION['plc_Operation'] = $_POST['ps_Operation'];

if ( isset($_GET['PrintOperation']) )
{
    $print_op = true;
}
else if ( isset($_GET['PlcEventButtonTime']) && isset($_GET['DeviceNo']) && isset($_GET['IOChannel']) )
{
    $period = intval($_GET['PlcEventButtonTime']);
    if ( substr($_GET['PlcEventButtonTime'],strlen($_GET['PlcEventButtonTime'])-1,1) == 'm' )
    {   // minutes
        $period *= 60;
    }
    
    $device_no = $_GET['DeviceNo'];
    $io_channel = $_GET['IOChannel'];
    
    $state_list = $db->ReadPlcStatesTable( 0, $_SESSION['plc_Operation'] );
    
    $state_name = "";
    foreach ( $state_list as $state )
    {
        if ( $state['pl_StateIsActive'] == 'Y' )
        {
            $state_name = $state['pl_StateName'];
            break;
        }
    }
    
    foreach ( $state_list as $state )
    {   // find the timer event for this state
        if ( $state['pl_StateName'] == $state_name && $state['pl_RuleType'] == "E" && $state['pl_TimerValues'] != "" && $state['pl_DeviceNo'] == $device_no && $state['pl_IOChannel'] == $io_channel )
        {   // found it
            $db->PlcStateChangeTimer( $_SESSION['plc_Operation'], $state['pl_NextStateName'], $period );
            
            break;
        }
    }
}
else if ( isset($_GET['PlcEventButton']) )
{
    $state_no = $_GET['PlcEventButton'];
    
    $state = $db->ReadPlcStatesTable( $state_no, "" );
    if ( $state[0]['pl_Operation'] == $_SESSION['plc_Operation'] )
    {
        $db->NotifyPlcStatesScreenButton( $state_no );

        $last_state_name = $state[0]['pl_StateName'];        
    }
}

    
    

$op_list = $db->SelectPlcOperations();

$state_list = $db->ReadPlcStatesTable( 0, $_SESSION['plc_Operation'] );

$di_list = $db->ReadDeviceInfoTable();


?>
<script type = "text/javascript">
function ClickRefreshButton()
{
	var refreshButton = document.getElementById("PlcStateRefresh");
	refreshButton.click();
}
function onChangeTimeSelection( tt, de_no, ch_no )
{
	var pl = "pl_TimePeriod" + tt;
	var ps = "ps_EventButtonTime" + tt;
	var sel = document.getElementById(pl);
	var btn = document.getElementById(ps);
	var pos = btn.textContent.search(":");
	if ( pos >= 0 )
		btn.textContent = btn.textContent.substr(0,pos+1) + " " + sel.value;
	else
		btn.textContent += ": " + sel.value;

	var url = "?PageMode=PlcState&PlcEventButtonTime=" + sel.value + "&DeviceNo=" + de_no + "&IOChannel=" + ch_no;
	window.location.href = url;
}
</script>
<?php 

printf( "<font size='4'>" );

printf( "<table border='1' width='50%%'>" );

printf( "<tr><td>&nbsp;</td></tr>" );

printf( "<tr>" );
printf( "<td>Select the<br>Operation</td>" );
printf( "<td>" );
printf( "<select size='1' name='ps_Operation' id='ps_Operation' onchange='ClickRefreshButton();' style='width:100%%;'>" );
printf( "<option></option>" );
foreach ( $op_list as $op )
{
    printf( "<option %s>%s</option>", ($op['pl_Operation'] == $_SESSION['plc_Operation'] ? "selected" : ""), $op['pl_Operation'] );
}
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr><td>&nbsp;</td></tr>" );

if ( $_SESSION['plc_Operation'] != "" )
{
    printf( "<tr valign='top'>" );
    printf( "<td>" );
    
    $state_name = "";
    foreach ( $state_list as $state )
    {
        if ( $state['pl_StateIsActive'] == 'Y' )
        {
            $state_name = $state['pl_StateName'];
            break;
        }
    }
    
    if ( $state_name != "" )
        printf( "<b>Active State:</b></br>&nbsp;<br><span style='color:#ff0000;'><b>%s</b></span>", $state_name );
    else
        printf( "Error: there is no active state" );
    
    printf( "</td>" );
    
    if ( $state_name != "" )
    {   
        // print the Init actions
        printf( "<td>" );
        $header = false;
        $count = 1;
        foreach ( $state_list as $state )
        {
            if ( $state['pl_StateName'] == $state_name && $state['pl_RuleType'] == "I" )
            {
                if ( !$header )
                {
                    printf( "<b>Initial Actions:</b>&nbsp;<br><br>" );
                    $header = true;
                }
                $name = func_find_deviceinfo_name( $di_list, $state['pl_DeviceNo'], $state['pl_IOChannel'], $state['pl_RuleType'] );
                printf( "%d: %s = %.1f<br>", $count, $name, $state['pl_Value'] );
                $count += 1;
            }
        }
        printf( "</td>" );
        
        // print the event buttons
        printf( "<td>" );
        printf( "<p>" );
        $header = false;
        $count = 1;
        $tt = 0;
        foreach ( $state_list as $state )
        {
            if ( $state['pl_StateName'] == $state_name && $state['pl_RuleType'] == "E" )
            {
                if ( !$header )
                {
                    printf( "<b>Events:</b><br>" );
                    $header = true;
                    printf( "<table border='0' width='100%%'>" );
                }
                printf( "<tr>" );
                
                $name = func_find_deviceinfo_name( $di_list, $state['pl_DeviceNo'], $state['pl_IOChannel'], $state['pl_RuleType'] );
                $test = "";
                $btnName = "ps_EventButton";
                if ( $state['pl_Test'] != '' )
                {
                    $test = sprintf( "<small>%s %.1f</small><br>", $state['pl_Test'], $state['pl_Value'] );
                }
                else if ( $state['pl_DelayTime'] != 0 )
                {
                    $test = sprintf( "<small>Delay %d sec</small><br>", $state['pl_DelayTime'] );
                }
                else if ( $state['pl_TimerValues'] != '' )
                {
                    $delay = $db->PlcStateReadDelayTime( $_SESSION['plc_Operation'], $state['pl_NextStateName'] );
                    
                    $opt = "<option></option>";
                    $expl = explode( ",", $state['pl_TimerValues'] );
                    foreach ( $expl as $ex )
                    {
                        $opt .= sprintf( "<option>%s</option>", $ex );
                    }
                    $test = sprintf( "<small><select size='1' name='pl_TimePeriod%d' id='pl_TimePeriod%d' onChange='onChangeTimeSelection(%d,%d,%d);'>%s</select> Time Selection</small><br>", 
                        $tt, $tt, $tt, $state['pl_DeviceNo'], $state['pl_IOChannel'], $opt );
                    $btnName = sprintf( "ps_EventButtonTime%d", $tt );
                    
                    if ( $delay > 100 )
                        $name = sprintf( "%s: %.1f min", $name, $delay/60 );
                    else
                        $name = sprintf( "%s: %d sec", $name, $delay );
                    
                    $tt += 1;
                }
                printf( "<td>" );
                printf( "<p><a href='?PageMode=PlcState&PlcEventButton=%d'><button type='button' style='display: block; width: 100%%;' name='%s' id='%s'>%s</button></a></p>", 
                    $state['pl_StateNo'], $btnName, $btnName, $name );
                printf( "</td>" );
                
                printf( "<td>" );
                printf( "<p>%sGoto -> %s</p>", $test, $state['pl_NextStateName'] );
                printf( "</td>" );
                
                printf( "</tr>" );
            }
        }
        if ( $header )
        {
            printf( "</table>" );
        }
        printf( "</p>" );
        printf( "</td>" );
    }
    
    printf( "</tr>" );
}

printf( "</table>" );

printf( "</font>" );


?>


<p>
<div id='ws_message'></div>
<select name='ws_list' id='ws_list' size='6' style='width: 100%;'>
</select>
</p>
<?php
if ( $_SERVER['HTTPS'] != "" )
{
    printf( "<small>If wss is not working, browse to 'https://%s:8000' and accept the self signed certificate.</small>", $_SERVER['SERVER_ADDR'] );
}
if ( $_SESSION['plc_Operation'] != "" )
{
    printf( "<br><a href='?PageMode=PlcState&PrintOperation=1'><button type='button' class='btn btn-info'>Print</button></a><br>" );
}

if ( $print_op )
{
    $state_order = array();
    foreach ( $state_list as $state )
    {
        if ( $state['pl_RuleType'] == "" )
        {
            $state_order[] = array( 'pl_PrintOrder'=>$state['pl_PrintOrder'], 'pl_StateName'=>$state['pl_StateName'] );
        }
    }
    sort( $state_order );
    
    $tt = getdate();
    printf( "<h3>Operation: %s&nbsp;&nbsp;&nbsp;<small>%02d/%02d/%d %02d:%02d</small></h3>", $_SESSION['plc_Operation'], $tt['mday'], $tt['mon'], $tt['year'], $tt['hours'], $tt['minutes'] );
    
    printf( "<table border='1' width='100%%'>" );
    
    printf( "<thead>" );
    printf( "<tr>" );
    printf( "<th>State Name</th>" );
    printf( "<th>Rule Type</th>" );
    printf( "<th>Device/Name</th>" );
    printf( "<th>Next State</th>" );
    printf( "<th>Value</th>" );
    printf( "<th>Delay</th>" );
    printf( "<th>Timer Values</th>" );
    printf( "<th>Test</th>" );
    printf( "</tr>" );
    printf( "</thead>" );
    
    $printed = false;
    $last_state = "";
    foreach ( $state_order as $order )
    {
        foreach ( $state_list as $state )
        {
            if ( $order['pl_StateName'] != $state['pl_StateName'] )
                continue;
            
            if ( $last_state != "" && $last_state != $state['pl_StateName'] )
            {
                printf( "<tr><td>&nbsp;</td></tr>" );  
                $printed = false;
            }
            $last_state = $state['pl_StateName'];
      
            $name = func_find_deviceinfo_name( $di_list, $state['pl_DeviceNo'], $state['pl_IOChannel'], $state['pl_RuleType'] );
            printf( "<tr>" );
            if ( $state['pl_RuleType'] == "" )
            {
                //printf( "<td><b>%s</b></td>", $state['pl_StateName'] );
            }
            else 
            {
                if ( $state['pl_RuleType'] == "E" )
                {   // event
                    if ( !$printed )
                    {
                        $printed = true;
                        printf( "<td><b>%s</b></td>", $state['pl_StateName'] );
                    }
                    else
                    {
                        printf( "<td></td>" );
                    }
                    printf( "<td>E</td>" );
                    printf( "<td>%s</td>", $name );
                    printf( "<td><b>%s</b></td>", $state['pl_NextStateName'] );
                }    
                else
                {   // initial
                    if ( !$printed )
                    {
                        $printed = true;
                        printf( "<td><b>%s</b></td>", $state['pl_StateName'] );
                    }
                    else
                    {
                        printf( "<td></td>" );
                    }
                    printf( "<td>I</td>" );
                    printf( "<td>%s</td>", $name );
                    printf( "<td></td>" );
                    
                }
                if ( $state['pl_RuleType'] == "E" && $state['pl_Test'] == "" )
                    printf( "<th></th>" );
                else
                    printf( "<th>%.1f</th>", $state['pl_Value'] );
                printf( "<th>%s</th>", ($state['pl_DelayTime'] == 0 ? "" : $state['pl_DelayTime']) );
                printf( "<th>%s</th>", $state['pl_TimerValues'] );
                printf( "<th>%s</th>", $state['pl_Test'] );
            }
            
            printf( "</tr>" );
        }
    }
    
    printf( "</table>" );
}
?>


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
        $name = sprintf( "Immediate" );
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


if ( !isset($_SESSION['plc_Operation']) )
    $_SESSION['plc_Operation'] = "";
if ( !isset($_SESSION['plc_StateName']) )
    $_SESSION['plc_StateName'] = "";
        
    
if ( isset($_POST['ps_Operation']) )
    $_SESSION['plc_Operation'] = $_POST['ps_Operation'];

if ( isset($_GET['PlcEventButtonTime']) )
{
    $period = intval($_GET['PlcEventButtonTime']);
    if ( substr($_GET['PlcEventButtonTime'],strlen($_GET['PlcEventButtonTime'])-1,1) == 'm' )
    {   // minutes
        $period *= 60;
    }
    
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
        if ( $state['pl_StateName'] == $state_name && $state['pl_RuleType'] == "E" && $state['pl_TimerValues'] != "" )
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
function onChangeTimeSelection()
{
	var sel = document.getElementById("pl_TimePeriod");
	var btn = document.getElementById("ps_EventButtonTime");
	var pos = btn.textContent.search(":");
	if ( pos >= 0 )
		btn.textContent = btn.textContent.substr(0,pos+1) + " " + sel.value;
	else
		btn.textContent += ": " + sel.value;

	var url = "?PageMode=PlcState&PlcEventButtonTime=" + sel.value;
	window.location.href = url;
}
</script>
<?php 

printf( "<font size='4'>" );

printf( "<table border='1'>" );

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
                    $test = sprintf( "<small><select size='1' name='pl_TimePeriod' id='pl_TimePeriod' onChange='onChangeTimeSelection();'>%s</select> Time Selection</small><br>", $opt );
                    $btnName = "ps_EventButtonTime";
                    
                    if ( $delay > 100 )
                        $name = sprintf( "%s %.1f min", $name, $delay/60 );
                    else
                        $name = sprintf( "%s %d sec", $name, $delay );
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


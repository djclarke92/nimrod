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
    $name = sprintf( "(%d,%d)", $device_no, $iochannel );
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
    
    return $name;
}


if ( !isset($_SESSION['plc_Operation']) )
    $_SESSION['plc_Operation'] = "";
if ( !isset($_SESSION['plc_StateName']) )
    $_SESSION['plc_StateName'] = "";
        
    
if ( isset($_POST['ps_Operation']) )
    $_SESSION['plc_Operation'] = $_POST['ps_Operation'];

if ( isset($_GET['PlcEventButton']) )
{
    $state_no = $_GET['PlcEventButton'];
    
    $state = $db->ReadPlcStatesTable( $state_no, "" );
    if ( $state[0]['pl_Operation'] == $_SESSION['plc_Operation'] )
    {
        $db->NotifyPlcStatesScreenButton( $state_no );

        $last_state_name = $state[0]['pl_StateName'];
        
        $count = 0;
        while ( $count < 20 )
        {  // wait here until the active state has changed
            $count += 1;
            time_nanosleep( 0, 300000000 );
            $state_name = $db->PlcGetActiveStateName( $state[0]['pl_Operation'] );
            
            if ( $state_name != $last_state_name )
            {
                break;
            }
        }
        
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
    printf( "<tr>" );
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
        printf( "Active State<br><b>%s</b>", $state_name );
    else
        printf( "Error: there is no active state" );
    
    printf( "</td>" );
    
    if ( $state_name != "" )
    {   
        // print the Init actions
        printf( "<td>" );
        $count = 1;
        foreach ( $state_list as $state )
        {
            if ( $state['pl_StateName'] == $state_name && $state['pl_RuleType'] == "I" )
            {
                $name = func_find_deviceinfo_name( $di_list, $state['pl_DeviceNo'], $state['pl_IOChannel'], $state['pl_RuleType'] );
                printf( "Action %d: %s = %d<br>", $count, $name, $state['pl_Value'] );
                $count += 1;
            }
        }
        printf( "</td>" );
        
        // print the event buttons
        printf( "<td>" );
        $count = 1;
        foreach ( $state_list as $state )
        {
            if ( $state['pl_StateName'] == $state_name && $state['pl_RuleType'] == "E" )
            {
                $name = func_find_deviceinfo_name( $di_list, $state['pl_DeviceNo'], $state['pl_IOChannel'], $state['pl_RuleType'] );
                printf( "<p><a href='?PageMode=PlcState&PlcEventButton=%d'><input type='button' name='ps_EventButton' id='ps_EventButton' value='%s'></a> Goto -> %s</p><br>", 
                    $state['pl_StateNo'], $name, $state['pl_NextStateName'] );
            }
        }
        printf( "</td>" );
    }
    
    printf( "</tr>" );
}

printf( "</table>" );

printf( "</font>" );


?>


<p>
<div id='ws_message'></div>
<select name='ws_list' id='ws_list' size='4' style='width: 100%;'>
</select>
</p>


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
$delay_key = "";
$delay_value = "";
$delay_msg = "";
$new_operation = "";
$existing_op = "";
$newop_msg = "";
$delete_password = "";
$op_changed = false;

if ( !isset($_SESSION['plc_Operation']) )
    $_SESSION['plc_Operation'] = "";
if ( !isset($_SESSION['plc_StateName']) )
    $_SESSION['plc_StateName'] = "";
        
    
if ( isset($_POST['ps_SelectOperation']) )
{
    if ( $_SESSION['plc_Operation'] != $_POST['ps_SelectOperation'] )
        $op_changed = true;
}
if ( isset($_POST['PlcDelayKeys']) )
    $delay_key = $_POST['PlcDelayKeys'];
if ( isset($_POST['PlcDelayValue']) )
    $delay_value = $_POST['PlcDelayValue'];
if ( isset($_POST['PlcNewOperation']) )
    $new_operation = $_POST['PlcNewOperation'];
if ( isset($_POST['ExistingOperation']) )
    $existing_op = $_POST['ExistingOperation'];
if ( isset($_POST['PlcDeletePassword']) )
    $delete_password = $_POST['PlcDeletePassword'];
        
if ( isset($_GET['PrintOperation']) )
{
    $print_op = true;
}
else if ( isset($_POST['PlcDeleteOperation']) )
{
    if ( $_SESSION['plc_Operation'] != "" )
    {
        $newop_msg = sprintf( "You must unselect all Operations before deleting an Operation" );
    }
    else if ( $existing_op != "" && $delete_password == "deleteme" )
    {
        $state_list = $db->ReadPlcStatesTable( 0, $existing_op );
        
        foreach ( $state_list as $state )
        {
            if ( !$db->DeletePlcStateRecord( false, $state['pl_StateNo'], "", "" ) )
            {
                $newop_msg = sprintf( "Error: failed to delete record %d", $state['pl_StateNo'] );
            }
        }
        if ( $newop_msg == "" )
        {
            $newop_msg = sprintf( "Operation '%s' has been deleted", $existing_op );
        }
    
        $db->NotifyPlcStatesTableChangeAll();
    }
    else
    {
        $newop_msg = sprintf( "You must enter the Operation Name and Delete Password" );
    }
}
else if ( isset($_POST['PlcCopyOperation']) )
{
    if ( $new_operation != "" && $existing_op != "" )
    {
        $op_list = $db->SelectPlcOperations();
        $state_list = $db->ReadPlcStatesTable( 0, $existing_op );
        
        foreach ( $op_list as $op )
        {
            if ( $op['pl_Operation'] == $new_operation )
            {   // error
                $newop_msg = sprintf( "Error: The new Operation name '%s' already exists", $new_operation );
                break;
            }
        }
        
        if ( $_SESSION['plc_Operation'] != "" )
        {
            $newop_msg = sprintf( "You must unselect all Operations before copying an Operation" );
        }
        else if ( $newop_msg == "" )
        {
            foreach ( $state_list as $state )
            {
                $state_no = 0;
                $state_timestamp = "";
                        
                if ( !$db->SavePlcState( false, $state_no, $new_operation, $state['pl_StateName'], 'N', $state_timestamp, $state['pl_RuleType'], $state['pl_DeviceNo'], 
                    $state['pl_IOChannel'], $state['pl_Value'], $state['pl_Test'], $state['pl_NextStateName'], $state['pl_Order'], $state['pl_DelayTime'], $state['pl_TimerValues'], 
                    $state['pl_PrintOrder'], $state['pl_DelayKey'], $state['pl_InitialState'] ) )
                {
                    $newop_msg = sprintf( "Error: Failed to insert record '%s', '%s'", $new_operation, $state['pl_StateName'] );
                    break;
                }
            }
            if ( $newop_msg == "" )
            {
                $newop_msg = sprintf( "Created the new Operation '%s'", $new_operation );
            }
            
            $db->NotifyPlcStatesTableChangeAll();
        }
    }
    else
    {
        $newop_msg = sprintf( "You must select the existing Operatioin and enter the new Operation Name" );
    }
}
else if ( isset($_POST['PlcRenameOperation']) )
{
    if ( $new_operation != "" && $existing_op != "" )
    {
        $op_list = $db->SelectPlcOperations();
        $state_list = $db->ReadPlcStatesTable( 0, $existing_op );
        
        foreach ( $op_list as $op )
        {
            if ( $op['pl_Operation'] == $new_operation )
            {   // error
                $newop_msg = sprintf( "Error: The new Operation name '%s' already exists", $new_operation );
                break;
            }
        }
        
        if ( $_SESSION['plc_Operation'] != "" )
        {
            $newop_msg = sprintf( "You must unselect all Operations before renaming an Operation" );
        }
        else if ( $newop_msg == "" )
        {
            if ( !$db->RenameOperation( $existing_op, $new_operation ) )
            {
                $newop_msg = sprintf( "Error renaming the '%s' to '%s'", $existing_op, $new_operation );
            }
            else
            {
                $newop_msg = sprintf( "Renamed operation '%s' to '%s'", $existing_op, $new_operation );
            }
            
            $db->NotifyPlcStatesTableChangeAll();
        }
    }
    else
    {
        $newop_msg = sprintf( "You must select the existing Operatioin and enter the new Operation Name" );
    }
}
else if ( isset($_POST['SetPlcDelay']) )
{
    if ( $delay_key != "" && $delay_value > 0 && $_SESSION['plc_Operation'] != "" )
    {
        $vmin = 0;
        $vmax = 0;
        
        $delay_keys = $db->ReadPlcDelayKeys( $_SESSION['plc_Operation'] );
        foreach ( $delay_keys as $key )
        {
            if ( $key['pl_DelayKey'] == $delay_key )
            {
                $expl = explode( ",", $key['pl_TimerValues'] );
                foreach ( $expl as $ex )
                {
                    $val = floatval($ex);
                    if ( strstr($ex,"m") )
                        $val *= 60;
                    
                    if ( $vmin == 0 )
                        $vmin = $val;
                    else if ( $val > $vmax )
                        $vmax = $val;
                }
                break;
            }
        }
        
        if ( $vmin <= 0 || $vmax <= 0 )
        {
            $delay_msg = sprintf( "Error: Delay Key '%s' value range has not been set", $delay_key );
        }
        else if ( $delay_value < $vmin || $delay_value > $vmax )
        {
            $delay_msg = sprintf( "Error: Invalid value - Delay Key '%s' range is %.1f to %.1f seconds", $delay_key, $vmin, $vmax );
        }
        else if ( $db->SetPlcDelayKeys( $_SESSION['plc_Operation'], $delay_key, $delay_value ) )
        {
            $delay_msg = sprintf( "Updated Delay Key '%s' to %.1f", $delay_key, $delay_value );
        }
        else 
        {
            $delay_msg = sprintf( "Failed to update Delay Key '%s'", $delay_key );
        }
    }
    else
    {
        $delay_msg = sprintf( "You must select the Delay Key and enter a time period in seconds" );
    }
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
else if ( isset($_POST['ps_SelectOperation']) )
{
    
    if ( $op_changed )
    {
        $db->PlcClearActiveState( $_SESSION['plc_Operation'] );
    }

    $_SESSION['plc_Operation'] = $_POST['ps_SelectOperation'];
}
else if ( isset( $_GET['PlcStateFinished']) )
{   // only action this if we are in a stopped state
    $state_list = $db->ReadPlcStatesTable( 0, $_SESSION['plc_Operation'] );
    
    $init_state = false;
    foreach ( $state_list as $state )
    {
        if ( $state['pl_StateIsActive'] == 'Y' )
        {
            if ( $state['pl_InitialState'] == 'Y' )
                $init_state = true;
            break;
        }
    }
    
    if ( $init_state )
    {
        $_SESSION['plc_Operation'] = "";
        $db->PlcClearActiveState( $_SESSION['plc_Operation'] );
    }
}
    

$db->PlcSetActiveState( $_SESSION['plc_Operation'] );

$op_list = $db->SelectPlcOperations();

$state_list = $db->ReadPlcStatesTable( 0, $_SESSION['plc_Operation'] );

$di_list = $db->ReadDeviceInfoTable();

$delay_keys = $db->ReadPlcDelayKeys( $_SESSION['plc_Operation'] );

$plc_events = $db->ReadPlcEvents( 1 );  // last 1 minute


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

printf( "<table border='1' width='66%%'>" );

printf( "<tr><td>&nbsp;</td></tr>" );

printf( "<tr>" );
printf( "<td colspan='2'>Select the Operation: " );
//printf( "<td>" );

$disabled = "";
if ( $_SESSION['plc_Operation'] != "" )
{
    foreach ( $state_list as $state )
    {
        if ( $state['pl_StateIsActive'] == 'Y' )
        {
            if ( $state['pl_InitialState'] != "Y" )
                $disabled = "disabled";
            break;       
        }
    }
}
        
printf( "<select size='1' name='ps_SelectOperation' id='ps_SelectOperation' onchange='ClickRefreshButton();' style='width:100%%;' %s>", $disabled );
printf( "<option></option>" );
foreach ( $op_list as $op )
{
    printf( "<option %s>%s</option>", ($op['pl_Operation'] == $_SESSION['plc_Operation'] ? "selected" : ""), $op['pl_Operation'] );
}
printf( "</select><br>&nbsp;" );
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
            $_SESSION['plc_StateName'] = $state_name;
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
                $disabled = "";
                $btnName = "ps_EventButton";
                if ( $state['pl_Test'] != '' )
                {
                    $disabled = "disabled";
                    $test = sprintf( "<small>%s %.1f</small><br>", $state['pl_Test'], $state['pl_Value'] );
                }
                else if ( $state['pl_DelayTime'] != 0 )
                {
                    $test = sprintf( "<small>Delay %.1f sec</small><br>", $state['pl_DelayTime'] );
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
                        $name = sprintf( "%s: %.1f sec", $name, $delay );
                    
                    $tt += 1;
                }
                
                $colour = "";
                if ( stristr( $name, "All Stop" ) != false )
                    $colour = "background-color:#FC4E5B;"; // red
                else if ( stristr( $name, "Stop" ) != false )
                    $colour = "background-color:#FCAD4E;"; // orange
                
                printf( "<td>" );
                printf( "<p><a href='?PageMode=PlcState&PlcEventButton=%d'><button type='button' style='display: block; width: 100%%;%s' name='%s' id='%s' %s><p>%s</p></button></a></p>", 
                    $state['pl_StateNo'], $colour, $btnName, $btnName, $disabled, $name );
                printf( "</td>" );
                
                printf( "<td>" );
                if ( $state['pl_NextStateName'] != "" )
                    printf( "<p>%sGoto -> '%s'</p>", $test, $state['pl_NextStateName'] );
                else
                    printf( "<p>%sPre Condition</p>", $test );

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

printf( "<table border='1' width='66%%'>" );

if ( $_SESSION['plc_Operation'] != "" )
{
    printf( "<tr>" );
    printf( "<td>" );
    printf( "Delay Keys: <select name='PlcDelayKeys' id='PlcDelayKeys' size='1'>" );
    printf( "<option> </option>" );
    foreach ( $delay_keys as $key )
    {
        printf( "<option>%s</option>", $key['pl_DelayKey'] );
    }
    printf( "</select>" );
    printf( "&nbsp;" );
    printf( "<input type='text' name='PlcDelayValue' id='PlcDelayValue' size='3'> X.Y secs" );
    printf( "<br>" );
    printf( "<input type='submit' name='SetPlcDelay' id='SetPlcDelay' value='Update Timing'>" );
    printf( "<br><div class='text-primary'>%s</div>", ($delay_msg != "" ? $delay_msg : "&nbsp;") );
    printf( "</td>" );
}
else
{
    printf( "<tr>" );
    printf( "<td></td>" );
}

if ( $_SESSION['plc_Operation'] == "" )
{
    printf( "<td align='right'>" );
    printf( "<select name='ExistingOperation' id='ExistingOperation' size='1'>" );
    printf( "<option></option>" );
    foreach ( $op_list as $op )
    {
        printf( "<option>%s</option>", $op['pl_Operation'] );
    }
    printf( "</select>" );
    printf( "&nbsp;" );
    printf( "New Name: <input type='text' name='PlcNewOperation' id='PlcNewOperation' size='15'>" );
    printf( "&nbsp;" );
    printf( "<input type='submit' name='PlcCopyOperation' id='PlcCopyOperation' value='Copy Operation'>" );
    printf( "&nbsp;" );
    printf( "<input type='submit' name='PlcRenameOperation' id='PlcRenameOperation' value='Rename Operation'>" );
    printf( "<br>" );
    printf( "Delete Password: <input type='text' name='PlcDeletePassword' id='PlcDeletePassword' size='8' autocomplete='off'>" );
    printf( "&nbsp;" );
    printf( "<input type='submit' name='PlcDeleteOperation' id='PlcDeleteOperation' value='Delete Operation'>" );
    printf( "<br><div class='text-primary'>%s</div>", ($newop_msg != "" ? $newop_msg : "&nbsp;") );
    printf( "</td>" );
    
    printf( "</tr>" );
    
}
else
{
    printf( "</tr>" );
    printf( "<tr>" );
    printf( "<td colspan='2'>" );
    
    printf( "Events<br>" );
    printf( "<select name='pl_PlcEvents' id='pl_PlcEvents' size='6' class='form-select'>" ); 
    foreach ( $plc_events as $event )
    {
        $colour = "";
        if ( strstr( $event['ev_Description'], "exception" ) != false )
            $colour = sprintf( "class='text-danger'" );
        else if ( strstr( $event['ev_Description'], "Changed active" ) != false )
            $colour = sprintf( "class='text-info'" );
                
        printf( "<option %s>%s&nbsp;&nbsp;&nbsp;%s</option>", $colour, $event['ev_Timestamp'], $event['ev_Description'] );
    }
    printf( "</select>" );
    
    printf( "</td>" );
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
//if ( $_SERVER['HTTPS'] != "" )
{
    printf( "<small>If wss is not working, browse to <a href='https://%s:8000' target='_blank'>'https://%s:8000'</a> and accept the self signed certificate.</small>", $_SERVER['SERVER_ADDR'], $_SERVER['SERVER_ADDR'] );
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
    
    $act = false;
    $init = false;
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
                $init = false;
                $act = false;
            }
            $last_state = $state['pl_StateName'];
            
            foreach( $state_list as $sl )
            {
                if ( $sl['pl_StateName'] != $last_state )
                {
                    continue;
                }
                else if ( $sl['pl_RuleType'] == "" )
                {
                    if ( $sl['pl_InitialState'] == "Y" )
                        $init = true;
                    if ( $sl['pl_StateIsActive'] == "Y" )
                        $act = true;
                    break;
                }
            }
      
            $name = func_find_deviceinfo_name( $di_list, $state['pl_DeviceNo'], $state['pl_IOChannel'], $state['pl_RuleType'] );
            printf( "<tr>" );
            if ( $state['pl_RuleType'] == "" )
            {    // printed last
                //printf( "<td><b>%s</b></td>", $state['pl_StateName'] );
            }
            else 
            {
                $extra = "";
                if ( $init && $act )
                    $extra = "&nbsp;(Active/Init)";
                else if ( $act )
                    $extra = "&nbsp;(Active)";
                else if ( $init )
                    $extra = "&nbsp;(Init)";
                
                if ( $state['pl_RuleType'] == "E" )
                {   // event
                    if ( !$printed )
                    {
                        $printed = true;
                        printf( "<td><b>%s%s</b></td>", $state['pl_StateName'], $extra );
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
                        printf( "<td><b>%s%s</b></td>", $state['pl_StateName'], $extra  );
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
                    printf( "<td></td>" );
                else
                    printf( "<td>%.1f</td>", $state['pl_Value'] );
                
                printf( "<td>%s</td>", ($state['pl_DelayTime'] == 0 ? "" : sprintf("%.1f",$state['pl_DelayTime'])) );
                printf( "<td>%s</td>", $state['pl_TimerValues'] );
                printf( "<td>%s</td>", $state['pl_Test'] );
            }
            
            printf( "</tr>" );
        }
    }
    
    printf( "</table>" );
}
?>


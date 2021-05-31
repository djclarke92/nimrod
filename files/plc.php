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

function func_clear_pl_array( &$pl_array )
{
	$pl_array['pl_StateNo'] = 0;
	$pl_array['pl_Operation'] = "";
	$pl_array['pl_StateName'] = "";
	$pl_array['pl_StateIsActive'] = "";
	$pl_array['pl_StateTimestamp'] = "";
	$pl_array['pl_RuleType'] = "";
	$pl_array['pl_DeviceNo'] = 0;
	$pl_array['pl_IOChannel'] = 0;
	$pl_array['pl_Value'] = 0;
	$pl_array['pl_Test'] = "";
	$pl_array['pl_NextStateName'] = "";
	$pl_array['pl_Order'] = 0;
	$pl_array['pl_DelayTime'] = 0;
	$pl_array['error_msg'] = "";
	$pl_array['info_msg'] = "";
	$pl_array['state_filter'] = "";
	$pl_array['op_filter'] = "";
}

function func_check_pl_array( &$pl_array, $marker, $next_marker )
{
	$pl_array['error_msg'] = "";
	$pl_array['info_msg'] = "";
	
	if ( $pl_array['pl_Operation'] == "" )
	{
		$pl_array['error_msg'] = "You must enter the Operation Name.";
		return false;
	}
	else if ( $pl_array['pl_StateName'] == "" )
	{
	    $pl_array['error_msg'] = "You must enter the State Name.";
	    return false;
	}
	else if ( $marker == false && $pl_array['pl_DeviceNo'] != 0 )
	{
	    $pl_array['error_msg'] = "The Operation and State Name marker record must be created first.";
	    return false;
	}
	else if ( $marker !== false && $marker == $pl_array['pl_StateNo'] && 
	    ($pl_array['pl_DeviceNo'] != 0 || $pl_array['pl_IOChannel'] != "" || $pl_array['pl_Test'] != "" || $pl_array['pl_Value'] != 0 || $pl_array['pl_NextStateName'] != ""))
	{  // this is the marker record
	    $pl_array['error_msg'] = "The Operation / State Name marker cannot contain any rule details.";
	    return false;
	}
	else if ( ($pl_array['pl_DeviceNo'] != 0 && $pl_array['pl_IOChannel'] === "") || ($pl_array['pl_DeviceNo'] == 0 && $pl_array['pl_IOChannel'] != "" && $pl_array['pl_RuleType'] != "E") )
	{
	    $pl_array['error_msg'] = sprintf( "The input/output device must be selected (%d,%d).", $pl_array['pl_DeviceNo'], $pl_array['pl_IOChannel'] );
	    return false;
	}
	else if ( $pl_array['pl_DeviceNo'] != 0 && $pl_array['pl_RuleType'] == "" )
	{
	    $pl_array['error_msg'] = "The Rule Type must be selected.";
	    return false;
	}
	else if ( $pl_array['pl_RuleType'] == "" && ($pl_array['pl_DeviceNo'] != 0 || $pl_array['pl_IOChannel'] != "" || $pl_array['pl_Test'] != "" || $pl_array['pl_NextStateName'] != "") )
	{
	    $pl_array['error_msg'] = "The Rule Type must be selected before entering the input/output device, Operator or 'Next State Name'.";
	    return false;
	}
	else if ( $pl_array['pl_RuleType'] == "I" && ($pl_array['pl_Test'] != "" || $pl_array['pl_NextStateName'] != "") )
	{
	    $pl_array['error_msg'] = "The Operator and Next State Name can only be entered for Init rules.";
	    return false;
	}
	else if ( $pl_array['pl_RuleType'] != "" && $pl_array['pl_DeviceNo'] == 0 && $pl_array['pl_IOChannel'] != -1 )
	{
	    $pl_array['error_msg'] = "The input/output device must be selected for Init or Event rules.";
	    return false;
	}
	else if ( $pl_array['pl_RuleType'] == "E" && $pl_array['pl_NextStateName'] == "" )
	{
	    $pl_array['error_msg'] = "The Next State Name must be entered for Event rules.";
	    return false;
	}
	else if ( $next_marker == false && $pl_array['pl_NextStateName'] != "" )
	{
	    $pl_array['error_msg'] = "The Next State Name marker record must be created before you can link to it.";
	    return false;
	}
	else if ( $pl_array['pl_StateName'] == $pl_array['pl_NextStateName'] )
	{
	    $pl_array['error_msg'] = "The 'Next State Name' must be different to the current 'State Name'.";
	    return false;
	}
	
	return true;
}

function func_check_device_hostname( $di_list, $device_no )
{
    $rc = false;
    
    $hostname = gethostname();
    foreach ( $di_list as $di )
    {
        if ( $di['di_DeviceNo'] == $device_no )
        {
            if ( $di['de_Hostname'] == $hostname )
            {
                $rc = true;
            }
            break;
        }
    }
    
    return $rc;
}


$pl_array = array();
func_clear_pl_array( $pl_array );
$new_state = false;

if ( isset($_GET['StateNo']) )
    $pl_array['pl_StateNo'] = $_GET['StateNo'];
if ( isset($_POST['pl_StateNo']) )
    $pl_array['pl_StateNo'] = $_POST['pl_StateNo'];
if ( isset( $_POST['pl_Operation']) )
    $pl_array['pl_Operation'] = $_POST['pl_Operation'];
if ( isset( $_POST['pl_StateName']) )
    $pl_array['pl_StateName'] = $_POST['pl_StateName'];
if ( isset( $_POST['pl_StateIsActive']) )
    $pl_array['pl_StateIsActive'] = "Y";
if ( isset( $_POST['pl_StateTimestamp']) )
    $pl_array['pl_StateTimestamp'] = $_POST['pl_StateTimestamp'];
if ( isset( $_POST['pl_RuleType']) )
    $pl_array['pl_RuleType'] = substr($_POST['pl_RuleType'],0,1);
if ( isset( $_POST['pl_DeviceChannel']) )
{   // xx,xx: name
    $expl = explode( ":", $_POST['pl_DeviceChannel'] );
    $expl2 = explode( ",", $expl[0] );
    if ( isset( $expl2[0]) && isset($expl2[1]) )
    {
        $pl_array['pl_DeviceNo'] = intval($expl2[0]);
        $pl_array['pl_IOChannel'] = intval($expl2[1])-1;
    }
}
if ( isset( $_POST['pl_DeviceNo']) )
    $pl_array['pl_DeviceNo'] = $_POST['pl_DeviceNo'];
if ( isset( $_POST['pl_IOChannel']) )
    $pl_array['pl_IOChannel'] = $_POST['pl_IOChannel'];
if ( isset( $_POST['pl_Value']) )
    $pl_array['pl_Value'] = $_POST['pl_Value'];
if ( isset( $_POST['pl_Test']) )
    $pl_array['pl_Test'] = $_POST['pl_Test'];
if ( isset( $_POST['pl_NextStateName']) )
    $pl_array['pl_NextStateName'] = $_POST['pl_NextStateName'];
if ( isset( $_POST['pl_Order']) )
    $pl_array['pl_Order'] = $_POST['pl_Order'];
if ( isset( $_POST['pl_DelayTime']) )
    $pl_array['pl_DelayTime'] = $_POST['pl_DelayTime'];
if (isset($_GET['StateFilter']))
    $pl_array['state_filter'] = $_GET['StateFilter'];
if (isset($_POST['StateFilter']))
    $pl_array['state_filter'] = $_POST['StateFilter'];
if (isset($_GET['OpFilter']))
    $pl_array['op_filter'] = $_GET['OpFilter'];
if (isset($_POST['OpFilter']))
    $pl_array['op_filter'] = $_POST['OpFilter'];
        
    
$di_list = $db->ReadDeviceInfoTable();
    
    
if ( isset($_POST['ClearState']) )
{
    $state_filter = $pl_array['state_filter'];
    $op_filter = $pl_array['op_filter'];

    func_clear_pl_array( $pl_array );
    
    $pl_array['state_filter'] = $state_filter;
    $pl_array['op_filter'] = $op_filter;
}
else if ( isset($_GET['StateNo']) )
{
    $pl_array['pl_StateNo'] = $_GET['StateNo'];
    if ( $pl_array['pl_StateNo'] == 0 )
    {
        $new_state = true;
    }
    else
    {
        $info = $db->ReadPlcStatesTable( $pl_array['pl_StateNo'], "" );

        if ( isset($info[0]) )
        {
            $pl_array['pl_Operation'] = $info[0]['pl_Operation'];
            $pl_array['pl_StateName'] = $info[0]['pl_StateName'];
            $pl_array['pl_StateIsActive'] = $info[0]['pl_StateIsActive'];
            $pl_array['pl_StateTimestamp'] = $info[0]['pl_StateTimestamp'];
            $pl_array['pl_RuleType'] = $info[0]['pl_RuleType'];
            $pl_array['pl_DeviceNo'] = $info[0]['pl_DeviceNo'];
            $pl_array['pl_IOChannel'] = $info[0]['pl_IOChannel'];
            $pl_array['pl_Value'] = $info[0]['pl_Value'];
            $pl_array['pl_Test'] = $info[0]['pl_Test'];
            $pl_array['pl_NextStateName'] = $info[0]['pl_NextStateName'];
            $pl_array['pl_Order'] = $info[0]['pl_Order'];
            $pl_array['pl_DelayTime'] = $info[0]['pl_DelayTime'];
            
            if ( strstr( $pl_array['pl_StateTimestamp'], "0000-00-00" ) != false )
                $pl_array['pl_StateTimestamp'] = "";
        }
        else
        {
            $pl_array['error_msg'] = sprintf( "Failed to read plcstates record for StateNo=%d", $pl_array['pl_StateNo'] );
        }
    }
}
else if ( isset($_POST['UpdateState']) || isset($_POST['NewState']) )
{
    if ( isset($_POST['NewState']) )
    {
        $new_state = true;
    }
    
    $marker = false;
    if ( $pl_array['pl_Operation'] != "" && $pl_array['pl_StateName'] != "" )
        $marker = $db->OperationStateNameExists( $pl_array['pl_Operation'], $pl_array['pl_StateName'] );
    
    $next_marker = false;
    if ( $pl_array['pl_Operation'] != "" && $pl_array['pl_NextStateName'] != "" )
        $next_marker = $db->OperationStateNameExists( $pl_array['pl_Operation'], $pl_array['pl_NextStateName'] );
            
    // check the device is on this host
    if ( $pl_array['pl_DeviceNo'] != 0 && func_check_device_hostname($di_list,$pl_array['pl_DeviceNo']) == false )
    {
        $pl_array['error_msg'] = sprintf( "All devices in the PLC state machine must be on this host '%s'", gethostname() );
    }
    else if ( func_check_pl_array( $pl_array, $marker, $next_marker ) )
    {
        if ( $db->SavePlcState( $pl_array['pl_StateNo'], $pl_array['pl_Operation'], $pl_array['pl_StateName'], 
            $pl_array['pl_StateIsActive'], $pl_array['pl_StateTimestamp'], $pl_array['pl_RuleType'], $pl_array['pl_DeviceNo'], $pl_array['pl_IOChannel'], $pl_array['pl_Value'],
            $pl_array['pl_Test'], $pl_array['pl_NextStateName'], $pl_array['pl_Order'], $pl_array['pl_DelayTime'] ) )
        {	// success
            //func_clear_pl_array( $pl_array );
            
            $pl_array['info_msg'] = "State details saved successfully.";
            $new_state = false;
        }
        else
        {
            $pl_array['error_msg'] = sprintf( "Failed to update State record %s", $pl_array['pl_StateName'] );
        }
    }
}
else if ( isset($_POST['DeleteState']) )
{
    $state_no = $pl_array['pl_StateNo'];
    
    $info = $db->ReadPlcStatesTable( $state_no, "" );
    if ( isset($info[0]) )
    {
        if ( $info[0]['pl_RuleType'] != "" )
        {   // delete this rule detail record
            if ( $db->DeletePlcStateRecord( $state_no ) )
            {
                $pl_array['info_msg'] = sprintf( "Successfully delete the rule detail plcstate record" );
            }
            else
            {
                $pl_array['error_msg'] = sprintf( "Failed to deleted the rule detail plcstate record %d", $state_no );
            }
            
            $pl_array['pl_StateNo'] = 0;
        }
        else
        {   // marker record
            $count = $db->CountOperationStateName( $info[0]['pl_Operation'], $info[0]['pl_StateName'] );
            if ( $count == 1 )
            {
                if ( $db->DeletePlcStateRecord( $state_no ) )
                {
                    $pl_array['info_msg'] = sprintf( "Successfully deleted the marker plcstate record" );
                }
                else
                {
                    $pl_array['error_msg'] = sprintf( "Failed to delete the marker plcstate record %d", $state_no );
                }
            
                $pl_array['pl_StateNo'] = 0;
            }
            else
            {
                $pl_array['error_msg'] = sprintf( "You cannot delete a marker record while rule detail records exist." );
            }
        }
    }
    else
    {
        $pl_array['error_msg'] = sprintf( "Delete failed to read plcstates record for StateNo=%d", $pl_array['pl_StateNo'] );
    }
}



$state_list = $db->ReadPlcStatesTable(0,"");

if ( $pl_array['op_filter'] == "" )
{
    if ( isset($state_list[0]) )
    {
        $pl_array['op_filter'] = $state_list[0]['pl_Operation'];
}
}


?>
<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-4">
			<h3>PLC States</h3>
        </div>
		<div class="col-sm-1">
			<a href='#ruleslist' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
        </div>
        <div class="col-sm-2">
			<div class='input-group'>
			<select class='form-control custom-select' size='1' name='OpFilter' id='OpFilter'>
			<option></option>
			<?php
			$op_name = "";
			foreach ($state_list as $state) 
			{
			    if ( $state['pl_Operation'] != $op_name )
			    {
    			    printf("<option %s>%s</option>", ($pl_array['op_filter'] == $state['pl_Operation'] ? "selected" : ""), $state['pl_Operation'] );
    			    $op_name = $state['pl_Operation'];
			    }
			}
			?>
			</select> 
			</div>
		</div>
        <div class="col-sm-2">
			<div class='input-group'>
			<select class='form-control custom-select' size='1' name='StateFilter' id='StateFilter'>
			<option></option>
			<?php
			$state_name = "";
			foreach ($state_list as $state) 
			{
			    if ( $state['pl_StateName'] != $state_name )
			    {
    			    printf("<option %s>%s</option>", ($pl_array['state_filter'] == $state['pl_StateName'] ? "selected" : ""), $state['pl_StateName'] );
    			    $state_name = $state['pl_StateName'];
			    }
			}
			?>
			</select> 
			&nbsp;
			<button type='submit' class='btn btn-outline-dark mt-2' name='Refresh' id='Refresh'>Filter</button>
			</div>
		</div>
        
    </div>

	<div id="ruleslist" class="collapse <?php ($new_state || $pl_array['pl_StateNo'] != 0 ? printf("") : printf("show"))?>">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-10">
			<?php
			if ( $pl_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $pl_array['error_msg'] );
		    else if ( $pl_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $pl_array['info_msg'] );
            ?>

            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN && count($state_list) > 6 )
            {
                printf( "<a href='?StateNo=0&StateFilter=%s&OpFilter=%s'>Add New State</a></br>", $pl_array['state_filter'], $pl_array['op_filter'] );
            }
            ?>

    		<table class='table table-striped'>
    		<thead class="thead-light">
              <tr>
              <th>Operation</th>
              <th>State Name</th>
              <th>Rule Type</th>
              <th>Device / Event</th>
              <th>Next State</th>
              <th>Value</th>
              <th>Active</th>
            </thead>

            <?php
            foreach( $state_list as $state )
            {
                if ( $pl_array['state_filter'] != "" && $state['pl_StateName'] != $pl_array['state_filter']) {
                    continue;
                }
                else if ( $pl_array['op_filter'] != "" && $state['pl_Operation'] != $pl_array['op_filter']) {
                    continue;
                }
                
                printf( "<tr>" );
                
                printf( "<td><a href='?StateNo=%d&StateFilter=%s&OpFilter=%s'>%s</a></td>", $state['pl_StateNo'], $pl_array['state_filter'], $pl_array['op_filter'], $state['pl_Operation'] );
                printf( "<td><a href='?StateNo=%d&StateFilter=%s&OpFilter=%s'>%s</a></td>", $state['pl_StateNo'], $pl_array['state_filter'], $pl_array['op_filter'], $state['pl_StateName'] );
                printf( "<td><a href='?StateNo=%d&StateFilter=%s&OpFilter=%s'>%s</a></td>", $state['pl_StateNo'], $pl_array['state_filter'], $pl_array['op_filter'], $state['pl_RuleType'] );
                if ( $state['pl_DeviceNo'] == 0 && $state['pl_IOChannel'] == 0 )
                {
                    printf( "<td></td>" );
                }
                else if ( $state['pl_DeviceNo'] == 0 && $state['pl_IOChannel'] == -1 )
                {
                    printf( "<td>Immediate</td>" );
                }
                else
                {
                    $desc = sprintf( "%d,%d", $state['pl_DeviceNo'], $state['pl_IOChannel']+1 );
                    foreach ( $di_list as $di )
                    {
                        if ( $di['di_DeviceNo'] == $state['pl_DeviceNo'] && $di['di_IOChannel'] == $state['pl_IOChannel'] && $state['pl_RuleType'] == "I" && $di['di_IOType'] == E_IO_OUTPUT )
                        {
                            $desc = sprintf( "%s", $di['di_IOName'] );
                            break;
                        }
                        else if ( $di['di_DeviceNo'] == $state['pl_DeviceNo'] && $di['di_IOChannel'] == $state['pl_IOChannel'] && $state['pl_RuleType'] == "E" && $di['di_IOType'] != E_IO_OUTPUT )
                        {
                            $desc = sprintf( "%s", $di['di_IOName'] );
                            break;
                        }
                    }
                   
                    printf( "<td>%s</td>", $desc );
                }
                
                if ( $state['pl_NextStateName'] == "" )
                    printf( "<td></td>" );
                else
                    printf( "<td>%s</td>", $state['pl_NextStateName'] );
                
                printf( "<td>%d</td>", $state['pl_Value'] );
                
                printf( "<td>%s</td>", ($state['pl_StateIsActive'] == "Y" ? "Yes" : "") );
                
                printf( "<tr>" );
            }
            ?>
            
            </table>
            
            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
            {
                printf( "<p><a href='?StateNo=0&StateFilter=%s&OpFilter=%s'>Add New State</a></p>", $pl_array['state_filter'], $pl_array['op_filter'] );
            }
            ?>
            
		</div>
	</div>	<!-- end of row -->
	</div>
	
    <?php
	if ( $pl_array['pl_StateNo'] != 0 || $new_state )
	{
    ?>

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-8">
			<h3>State Detail</h3>

			<?php
			if ( $pl_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $pl_array['error_msg'] );
		    else if ( $pl_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $pl_array['info_msg'] );
			
		    $disabled = "";
		    $disabled2 = "";
		    if ( $pl_array['pl_RuleType'] != "" )
		        $disabled = "disabled";
		    else if ( $pl_array['pl_StateNo'] != 0 )
		        $disabled2 = "disabled";
		        
			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='pl_Operation'>Operation: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		$op = $pl_array['pl_Operation'];
    		if ( $op == "" && $pl_array['op_filter'] != "" )
    		    $op = $pl_array['op_filter'];
    		printf( "<input type='text' class='form-control' name='pl_Operation' id='pl_Operation' size='30' value='%s'> ", $op );
    		printf( "</div>" );
    		printf( "</div>" );
	
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_StateName'>State Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		$state_name = $pl_array['pl_StateName'];
    		if ( $state_name == "" && $pl_array['state_filter'] != "" )
    		    $state_name = $pl_array['state_filter'];
    		printf( "<input type='text' class='form-control' name='pl_StateName' id='pl_StateName' size='30' value='%s'> ", $state_name );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col form-check'>" );
    		printf( "<label class='form-check-label'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='pl_StateIsActive' id='pl_StateIsActive' %s %s %s>", ($pl_array['pl_StateIsActive'] == "Y" ? "checked" : ""), $disabled,
    		    ($pl_array['pl_StateIsActive'] == "Y" ? "disabled" : "") );
    		printf( "State Is Active</label>" );
    		printf ( "&nbsp;&nbsp;&nbsp;Timestamp: %s",  $pl_array['pl_StateTimestamp'] );
    		printf( "</div>" );
    		printf( "</div>" );

    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_RuleType'>Rule Type: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<select size='1' class='form-control custom-select' name='pl_RuleType' id='pl_RuleType' %s>", $disabled2 );
    		printf( "<option></option>" );
    		printf( "<option %s>I. Init</option>", ($pl_array['pl_RuleType'] == "I" ? "selected" : "") );
    		printf( "<option %s>E. Event</option>", ($pl_array['pl_RuleType'] == "E" ? "selected" : "") );
    		printf( "</select>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_DeviceChannel'>Device: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<select size='1' class='form-control custom-select' name='pl_DeviceChannel' id='pl_DeviceChannel' %s>", $disabled2 );
    		printf( "<option></option>" );
    		printf( "<option %s>0,0: Immediate</option>", ($pl_array['pl_DeviceNo'] == 0 && $pl_array['pl_RuleType'] == 'E' ? "selected" : "") );
    		foreach ( $di_list as $di )
    		{
    		    $sel = "";
    		    if ( $pl_array['pl_RuleType'] == "I" && $di['di_IOType'] == E_IO_OUTPUT && $pl_array['pl_DeviceNo'] == $di['di_DeviceNo'] && $pl_array['pl_IOChannel'] == $di['di_IOChannel'] )
    		        $sel = "selected";
		        else if ( $pl_array['pl_RuleType'] == "E" && $di['di_IOType'] != E_IO_OUTPUT && $pl_array['pl_DeviceNo'] == $di['di_DeviceNo'] && $pl_array['pl_IOChannel'] == $di['di_IOChannel'] )
		            $sel = "selected";
    		            
    		    printf( "<option %s>%d,%d: %s</option>", $sel, $di['di_DeviceNo'], $di['di_IOChannel']+1, $di['di_IOName'] );
    		}
    		printf( "</select>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_Value'>Value: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='pl_Value' id='pl_Value' size='6' value='%s' %s> ", $pl_array['pl_Value'], $disabled2 );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_Order'>Execution Order: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='pl_Order' id='pl_Order' size='2' value='%s' %s> ", $pl_array['pl_Order'], $disabled2 );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_DelayTime'>DelayTime: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='pl_DelayTime' id='pl_DelayTime' size='2' value='%s' %s> <i>(seconds)</i>", $pl_array['pl_DelayTime'], $disabled2 );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_Test'>Operator: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<select class='form-control custom-select' name='pl_Test' id='pl_Test' size='1' %s>", $disabled2 );
    		printf( "<option></option>" );
    		foreach ( $_SESSION['link_tests'] as $dd )
    		{
    		    $sel = "";
    		    if ( $dd == $pl_array['pl_Test'] )
    		        $sel = "selected";
   		        printf( "<option %s>%s</option>", $sel, $dd );
    		}
    		printf( "</select>" );
    		if ( strstr( $pl_array['pl_Test'], "/" ) !== false )
    		{
    		    printf( " (Invert State)" );
    		}
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='pl_NextStateName'>Next State Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<select class='form-control custom-select' name='pl_NextStateName' id='pl_NextStateName' size='1' %s>", $disabled2 );
    		printf( "<option></option>" );
    		$state_name = "";
    		foreach ( $state_list as $state )
    		{
    		    if ( $state_name != $state['pl_StateName'] )
    		    {
    		        $state_name = $state['pl_StateName'];
        		    $sel = "";
        		    if ( $state_name == $pl_array['pl_NextStateName'] )
        		        $sel = "selected";
        		    printf( "<option %s>%s</option>", $sel, $state_name );
    		    }
    		}
    		printf( "</select>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row mb-2 mt-2'>" );
    		printf( "<p>" );
    		printf( "<input type='hidden' name='pl_StateNo' value='%d'>", $pl_array['pl_StateNo'] );
    		printf( "<input type='hidden' name='pl_StateTimestamp' value='%s'>", $pl_array['pl_StateTimestamp'] );
    		printf( "<input type='hidden' name='StateFilter' value='%s'>", $pl_array['state_filter'] );
    		printf( "<input type='hidden' name='OpFilter' value='%s'>", $pl_array['op_filter'] );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='UpdateState' id='UpdateState' value='Update' %s>Update</button>", ($pl_array['pl_StateNo'] == 0 ? "disabled" : "") );
    		printf( "&nbsp;&nbsp;&nbsp;" );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='NewState' id='NewState' value='New' %s>New</button>", ($pl_array['pl_StateNo'] != 0 || func_disabled_non_admin() ? "disabled" : "") );
    		printf( "&nbsp;&nbsp;&nbsp;" );
    		$onclick = sprintf( "return confirm(\"Are you sure you want to delete this state record ?\")" );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='DeleteState' id='DeleteState' value='Delete' onclick='%s' %s>Delete</button>", $onclick,
    		    ($pl_array['pl_StateNo'] == "" || func_disabled_non_admin() != "" ? "disabled" : "") );
    		printf( "&nbsp;&nbsp;&nbsp;" );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='ClearState' id='ClearState' value='Clear'>Clear</button>" );
    		printf( "</p>" );
    		printf( "</div>" );
    		
    		
    		?>

		</div>
	</div>	<!-- end or row -->
    		
    <?php
	}
    ?>
    		

<?php 


?>            

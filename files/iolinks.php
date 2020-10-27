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


function func_clear_il_array( &$il_array )
{
	$il_array['il_LinkNo'] = 0;
	$il_array['il_InDeviceNo'] = "";
	$il_array['il_InChannel'] = "";
	$il_array['il_OutDeviceNo'] = "";
	$il_array['il_OutChannel'] = "";
	$il_array['il_EventType'] = "";
	$il_array['il_OnPeriod'] = "";
	$il_array['il_LinkDeviceNo'] = "";
	$il_array['il_LinkChannel'] = "";
	$il_array['il_LinkTest'] = "";
	$il_array['il_LinkValue'] = "";
	$il_array['error_msg'] = "";
	$il_array['info_msg'] = "";
}

function func_check_il_array( &$il_array )
{
	$il_array['error_msg'] = "";
	$il_array['info_msg'] = "";
	
	if ( $il_array['il_InDeviceNo'] == "" )
	{
		$il_array['error_msg'] = "You must select an Input Device.";
		return false;
	}
	else if ( $il_array['il_OutDeviceNo'] == "" )
	{
		$il_array['error_msg'] = "You must select an Output Device.";
		return false;
	}
	else if ( $il_array['il_InChannel'] < 0 || $il_array['il_InChannel'] > 15 )
	{
		$il_array['error_msg'] = "You must select the In Channel (1-16).";
		return false;
	}
	else if ( $il_array['il_OutChannel'] < 0 || $il_array['il_OutChannel'] > 15 )
	{
		$il_array['error_msg'] = "You must select the Out Channel (1-16).";
		return false;
	}
	else if ( $il_array['il_EventType'] === "" )
	{
		$il_array['error_msg'] = "The must select the Event Type.";
		return false;
	}
	else if ( $il_array['il_OnPeriod'] < 0 )
	{
		$il_array['error_msg'] = "The On Period must be >= zero.";
		return false;
	}
	
	return true;
}

function func_get_ioname( $dd_list, $de_no, $ch, $in )
{
	$name = "?";
	
	foreach ( $dd_list as $dd )
	{
		if ( $in && $dd['de_DeviceNo'] == $de_no && $dd['di_IOChannel'] == $ch )
		{
			$name = $dd['di_IOName'];
			break;
		}
		else if ( !$in && $dd['de_DeviceNo'] == $de_no && $dd['di_IOChannel'] == $ch )
		{
			$name = $dd['di_IOName'];
			break;
		}
	}
	
	return $name;
}

function func_get_deviceno( $dd_list, $addr )
{
	$de_no = 0;
	
	foreach( $dd_list as $dd )
	{
		if ( $dd['de_Address'] == $addr )
		{
			$de_no = $dd['de_DeviceNo'];
			break;
		}	
	}	
	
	return $de_no;
}



$il_array = array();
func_clear_il_array( $il_array );

$new_iolink = false;

$ddi_list = $db->GetIOAddresses( true, false );
$ddo_list = $db->GetIOAddresses( false, true );



if ( isset( $_GET['LinkNo']) )
	$il_array['il_LinkNo'] = $_GET['LinkNo'];
if ( isset( $_POST['il_LinkNo']) )
	$il_array['il_LinkNo'] = $_POST['il_LinkNo'];
if ( isset( $_POST['il_InDeviceNo']) )
{
	$il_array['il_InDeviceNo'] = func_get_deviceno( $ddi_list, intval($_POST['il_InDeviceNo']));
	$il_array['il_InChannel'] = intval(substr($_POST['il_InDeviceNo'],3)) - 1;
}
if ( isset( $_POST['il_OutDeviceNo']) )
{
	$il_array['il_OutDeviceNo'] = func_get_deviceno( $ddo_list, intval($_POST['il_OutDeviceNo']));
	$il_array['il_OutChannel'] = intval(substr($_POST['il_OutDeviceNo'],3)) - 1;
}
if ( isset( $_POST['il_EventType']) )
	$il_array['il_EventType'] = func_get_eventtype($_POST['il_EventType']);
if ( isset( $_POST['il_OnPeriod']) )
	$il_array['il_OnPeriod'] = func_get_duration( $_POST['il_OnPeriod'] );
if ( isset( $_POST['il_LinkDeviceNo']) && strlen($_POST['il_LinkDeviceNo']) > 0 )
{
	$il_array['il_LinkDeviceNo'] = func_get_deviceno( $ddi_list, intval($_POST['il_LinkDeviceNo']));
	$il_array['il_LinkChannel'] = intval(substr($_POST['il_LinkDeviceNo'],3)) - 1;
}
if ( isset( $_POST['il_LinkTest']) )
	$il_array['il_LinkTest'] = $_POST['il_LinkTest'];
if ( isset( $_POST['il_LinkValue']) )
	$il_array['il_LinkValue'] = $_POST['il_LinkValue'];



if ( isset($_GET['LinkNo']) )
{
    if ( $_GET['LinkNo'] == 0 )
    {
        $new_iolink = true;
    }
	else if ( ($line=$db->GetFields( 'iolinks', 'il_LinkNo', $il_array['il_LinkNo'], "il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,
			il_LinkDeviceNo,il_LinkChannel,il_LinkTest,il_LinkValue" )) !== false )
	{	// success
		$il_array['il_InDeviceNo'] = $line[0];
		$il_array['il_InChannel'] = $line[1];
		$il_array['il_OutDeviceNo'] = $line[2];
		$il_array['il_OutChannel'] = $line[3];
		$il_array['il_EventType'] = $line[4];
		$il_array['il_OnPeriod'] = $line[5];
		$il_array['il_LinkDeviceNo'] = $line[6];
		$il_array['il_LinkChannel'] = $line[7];
		$il_array['il_LinkTest'] = $line[8];
		$il_array['il_LinkValue'] = $line[9];
		
		if ( $il_array['il_OnPeriod'] == 0 )
			$il_array['il_OnPeriod'] = "";
		if ( $il_array['il_LinkDeviceNo'] == 0 )
			$il_array['il_LinkValue'] = "";
	}
	else
	{
		$il_array['error_msg'] = sprintf( "Failed to read iolinks table for LinkNo=%d", $il_array['il_LinkNo'] );
	}
}
else if ( isset($_POST['DeleteIOLink']) )
{
	$il_array['il_LinkNo'] = $_POST['il_LinkNo']; 
	if ( $db->DeleteIOLinks( $il_array['il_LinkNo'] ) )
	{
		$il_array['info_msg'] = sprintf( "IOLink deleted" );
		$new_iolink = false;
		func_clear_il_array( $il_array );
	}
	else 
	{
		$il_array['error_msg'] = sprintf( "Failed to delete iolink with LinkNo=%d", $il_array['il_LinkNo'] );
	}
}
else if ( isset($_POST['NewIOLink']) || isset($_POST['UpdateIOLink']) )
{
	if ( isset($_POST['NewIOLink']) )
	{
	    $new_iolink = true;
		$il_array['il_LinkNo'] = 0;
	}
	
	if ( func_check_il_array( $il_array ) )
	{
		if ( $db->UpdateIOLinksTable( $il_array['il_LinkNo'], $il_array['il_InDeviceNo'], $il_array['il_InChannel'], $il_array['il_OutDeviceNo'],
			$il_array['il_OutChannel'], $il_array['il_EventType'], $il_array['il_OnPeriod'], $il_array['il_LinkDeviceNo'], $il_array['il_LinkChannel'],
			$il_array['il_LinkTest'], $il_array['il_LinkValue'] ) )
		{	// success
		    $new_iolink = false;
			func_clear_il_array( $il_array );
			
			$il_array['info_msg'] = "New iolink saved successfully.";
		}
		else
		{
			$il_array['error_msg'] = sprintf( "Failed to update IOLink record %d", $il_array['il_LinkNo'] );
		}
	}
}
else if ( isset($_POST['ClearIOLink']) )
{
	func_clear_il_array( $il_array );
}


$il_list = $db->ReadIOLinksTable();


?>

<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-5 mb-2">
			<h3>IO Links</h3>
		</div>
		<div class="col-sm-1">
			<a href='#iolinkslist' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
        </div>
			
	</div>

	<div id="iolinkslist" class="collapse <?php ($new_iolink || $il_array['il_LinkNo'] != 0 ? printf("") : printf("show"))?>">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-6">
			<?php
			if ( $il_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $il_array['error_msg'] );
		    else if ( $il_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $il_array['info_msg'] );
		    ?>
		    
    		<table class='table table-striped'>
    		<thead class="thead-light">
              <tr>
              <th>Output Name</th>
              <th>On Period</th>
              <th>Event Type</th>
              <th>Input Name</th>
              </tr>
            </thead>
            <tbody>
            <?php 
            foreach ( $il_list as $info )
            {
            	printf( "<tr>" );
            	
            	printf( "<td><a href='?LinkNo=%d'>%s</a></td>", $info['il_LinkNo'], func_get_ioname( $ddo_list, $info['il_OutDeviceNo'], $info['il_OutChannel'], false ) );
            	printf( "<td><a href='?LinkNo=%d'>%s</a></td>", $info['il_LinkNo'], func_get_on_period( $info['il_OnPeriod'] ) );
            	printf( "<td><a href='?LinkNo=%d'>%s</a></td>", $info['il_LinkNo'], func_get_eventtype_desc( $info['il_EventType'] ) );
            	printf( "<td><a href='?LinkNo=%d'>%s</a></td>", $info['il_LinkNo'], func_get_ioname( $ddi_list, $info['il_InDeviceNo'], $info['il_InChannel'], true ) );
            
            	printf( "</tr>" );
            }
            ?>
            </tbody>
            </table>

            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
                printf( "<p><a href='?LinkNo=0'>Add New IOLink</a></p>" );
            ?>
            
        </div>
    </div>
    </div>

    <?php
    if ( $il_array['il_LinkNo'] != "" || $new_iolink )
    {
    ?>

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-6">
			<h3>IO Links Detail</h3>

			<?php
			if ( $il_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $il_array['error_msg'] );
		    else if ( $il_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $il_array['info_msg'] );
			
		    printf( "<div class='form-row'>" );
		    printf( "<div class='col'>" );
		    printf( "<label for='il_InDeviceNo'>In: </label>" );
		    printf( "</div>" );
		    printf( "<div class='col'>" );
		    printf( "<select size='1' class='form-control custom-select' name='il_InDeviceNo' id='il_InDeviceNo'>" );
		    printf( "<option></option>" );
		    foreach ( $ddi_list as $dd )
		    {
		        $sel = "";
		        if ( $dd['de_DeviceNo'] == $il_array['il_InDeviceNo'] && $dd['di_IOChannel'] == $il_array['il_InChannel'] )
		            $sel = "selected";
	            printf( "<option %s>%02d.%02d %s</option>", $sel, $dd['de_Address'], $dd['di_IOChannel']+1, $dd['di_IOName'] );
		    }
	        printf( "</select>" );
	        printf( "</div>" );
		    printf( "</div>" );

		    printf( "<div class='form-row'>" );
		    printf( "<div class='col'>" );
		    printf( "<label for='il_EventType'>Event: </label>" );
		    printf( "</div>" );
		    printf( "<div class='col'>" );
		    printf( "<select class='form-control custom-select' name='il_EventType' id='il_EventType' size='1'>" );
		    printf( "<option>" );
		    printf( "<option %s>%s</option>", ($il_array['il_EventType'] == E_ET_CLICK && $il_array['il_EventType'] !== "" ? "selected" : ""), E_ETD_CLICK );
		    printf( "<option %s>%s</option>", ($il_array['il_EventType'] == E_ET_DBLCLICK ? "selected" : ""), E_ETD_DBLCLICK );
		    printf( "<option %s>%s</option>", ($il_array['il_EventType'] == E_ET_LONGPRESS ? "selected" : ""), E_ETD_LONGPRESS );
		    printf( "</select>" );
		    printf( "</div>" );
		    printf( "</div>" );
		    
		    printf( "<div class='form-row'>" );
		    printf( "<div class='col'>" );
		    printf( "<label for='il_OutDeviceNo'>Out: </label>" );
		    printf( "</div>" );
		    printf( "<div class='col'>" );
		    printf( "<select class='form-control custom-select' name='il_OutDeviceNo' id='il_OutDeviceNo' size='1'>" );
		    printf( "<option>" );
		    foreach ( $ddo_list as $dd )
		    {
		        $sel = "";
		        if ( $dd['de_DeviceNo'] == $il_array['il_OutDeviceNo'] && $dd['di_IOChannel'] == $il_array['il_OutChannel'] )
		            $sel = "selected";
		        printf( "<option %s>%02d.%02d %s</option>", $sel, $dd['de_Address'], $dd['di_IOChannel']+1, $dd['di_IOName'] );
		    }
		    printf( "</select>" );
		    printf( "</div>" );
		    printf( "</div>" );
		    
			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='il_OnPeriod'>On Period: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' size='6' name='il_OnPeriod' id='il_OnPeriod' value='%s'> (xx s, xx.x m, xx.x h)", func_get_on_period($il_array['il_OnPeriod']) );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='il_LinkDeviceNo'>Condition Link: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<select class='form-control custom-select' name='il_LinkDeviceNo' id='ilLinkDeviceNo size='1'>" );
    		printf( "<option></option>" );
    		foreach ( $ddi_list as $dd )
    		{
    		    $sel = "";
    		    if ( $dd['de_DeviceNo'] == $il_array['il_LinkDeviceNo'] && $dd['di_IOChannel'] == $il_array['il_LinkChannel'] )
    		        $sel = "selected";
    		    printf( "<option %s>%02d.%02d %s</option>", $sel, $dd['de_Address'], $dd['di_IOChannel']+1, $dd['di_IOName'] );
    		}
    		printf( "</select>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='il_LinkTest'>Operator: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<select class='form-control custom-select' name='il_LinkTest' id='il_LinkTest' size='1'>" );
    		printf( "<option></option>" );
    		foreach ( $_SESSION['link_tests'] as $dd )
    		{
    		    $sel = "";
    		    if ( $dd == $il_array['il_LinkTest'] )
    		        $sel = "selected";
    		    printf( "<option %s>%s</option>", $sel, $dd );
    		}
    		printf( "</select>" );
    		if ( strstr( $il_array['il_LinkTest'], "/" ) !== false )
    		{
    		    printf( " (Invert State)" );
    		}
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='il_LinkValue'>Value: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' size='4' name='il_LinkValue' id='il_LinkValue' value='%s'>", $il_array['il_LinkValue'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row mb-2 mt-2'>" );
    		printf( "<p>" );
    		printf( "<input type='hidden' name='il_LinkNo' value='%d'>", $il_array['il_LinkNo'] );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='UpdateIOLink' id='UpdateIOLink' %s>Update</button>", ($il_array['il_LinkNo'] == 0 || func_disabled_non_admin() != "" ? "disabled" : "") );
    		printf( "&nbsp;&nbsp;" );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='NewIOLink' id='NewIOLink' %s>New</button>", func_disabled_non_admin() );
    		printf( "&nbsp;&nbsp;" );
    		$onclick = sprintf( "return confirm(\"Are you sure you want to delete iolink with address %s ?\")", $il_array['il_InDeviceNo'] );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='DeleteIOLink' id='DeleteIOLink' onclick='%s' %s>Delete</button>", $onclick, ($il_array['il_LinkNo'] == 0 || func_disabled_non_admin() != "" ? "disabled" : "") );
    		printf( "&nbsp;&nbsp;" );
    		printf( "<button type='submit' class='btn btn-outline-dark' name='ClearIOLink' id='ClearIOLink'>Clear</button>" );
    		printf( "</p>" );
    		printf( "</div>" );
    		
            ?>

		</div>
	</div>

    <?php
    }
    ?>

</div>

<?php 
?>


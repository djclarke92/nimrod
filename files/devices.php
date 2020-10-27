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

function func_clear_de_array( &$de_array )
{
	$de_array['de_DeviceNo'] = 0;
	$de_array['de_ComPort'] = "";
	$de_array['de_Address'] = "";
	$de_array['de_NumInputs'] = "";
	$de_array['de_NumOutputs'] = "";
	$de_array['de_Type'] = "";
	$de_array['de_Name'] = "";
	$de_array['de_Hostname'] = "";
	$de_array['error_msg'] = "";
	$de_array['info_msg'] = "";
}

function func_check_de_array( &$de_array )
{
	$de_array['error_msg'] = "";
	$de_array['info_msg'] = "";
	
	if ( $de_array['de_ComPort'] == "" )
	{
		$de_array['error_msg'] = "You must enter the Com Port (e.g. ttyUSB0 or MCU).";
		return false;
	}
	else if ( $de_array['de_Address'] == "" || ($de_array['de_Address'] < 2 && $de_array['de_Address'] != 0) || $de_array['de_Address'] > 0xfe ||
	    ($de_array['de_Address'] == 0 && $de_array['de_ComPort'] != 'Timer' && strncmp( $de_array['de_ComPort'], 'MCU', 3 ) != 0) )
	{
		$de_array['error_msg'] = "You must enter an Address in the range 0x02 - 0xfe.";
		return false;
	}
	else if ( $de_array['de_NumInputs'] < 0 || $de_array['de_NumInputs'] > 16 )
	{
		$de_array['error_msg'] = "You must enter the number of inputs in the range 0 to 16.";
		return false;
	}
	else if ( $de_array['de_NumOutputs'] < 0 || $de_array['de_NumOutputs'] > 16 )
	{
		$de_array['error_msg'] = "You must enter the number of outputs in the range 0 to 16.";
		return false;
	}
	else if ( $de_array['de_Type'] == "" )
	{
		$de_array['error_msg'] = "You must select the Device Type.";
		return false;
	}
	else if ( $de_array['de_Name'] == "" )
	{
		$de_array['error_msg'] = "You must enter a name for this device.";
		return false;
	}
	else if ( $de_array['de_Hostname'] == "" || strstr($de_array['de_Hostname'],".") !== false )
	{
		$de_array['error_msg'] = "You must enter the hostname for this device.";
		return false;
	}
	
	return true;
}


$de_array = array();
func_clear_de_array( $de_array );

$new_device = false;



if ( isset( $_GET['DeviceNo']) )
	$de_array['de_DeviceNo'] = $_GET['DeviceNo'];
if ( isset( $_POST['de_DeviceNo']) )
	$de_array['de_DeviceNo'] = $_POST['de_DeviceNo'];
if ( isset( $_POST['de_ComPort']) )
	$de_array['de_ComPort'] = $_POST['de_ComPort'];
if ( isset( $_POST['de_Address']) )
	$de_array['de_Address'] = $_POST['de_Address'];
if ( isset( $_POST['de_NumInputs']) )
	$de_array['de_NumInputs'] = $_POST['de_NumInputs'];
if ( isset( $_POST['de_NumOutputs']) )
	$de_array['de_NumOutputs'] = $_POST['de_NumOutputs'];
if ( isset( $_POST['de_Type']) )
	$de_array['de_Type'] = func_get_device_type($_POST['de_Type']);
if ( isset( $_POST['de_Name']) )
	$de_array['de_Name'] = $_POST['de_Name'];
if ( isset( $_POST['de_Hostname']) )
	$de_array['de_Hostname'] = $_POST['de_Hostname'];



if ( isset($_GET['DeviceNo']) )
{
    if ( $_GET['DeviceNo'] == 0 )
    {
        $new_device = true;
    }
	else if ( ($line=$db->GetFields( 'devices', 'de_DeviceNo', $de_array['de_DeviceNo'], "de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Hostname" )) !== false )
	{	// success
		$de_array['de_ComPort'] = $line[0];
		$de_array['de_Address'] = $line[1];
		$de_array['de_NumInputs'] = $line[2];
		$de_array['de_NumOutputs'] = $line[3];
		$de_array['de_Type'] = $line[4];
		$de_array['de_Name'] = $line[5];
		$de_array['de_Hostname'] = $line[6];
	}
	else
	{
		$de_array['error_msg'] = sprintf( "Failed to read devices table for DeviceNo=%d", $de_array['de_DeviceNo'] );
	}
}
else if ( isset($_POST['DeleteDevice']) )
{
    $msg = "";
	$de_array['de_DeviceNo'] = $_POST['de_DeviceNo']; 
	if ( $db->DeleteDevice( $de_array['de_DeviceNo'], $msg ) )
	{
		$de_array['info_msg'] = sprintf( "Device deleted" );
		$de_array['de_DeviceNo'] = 0;
	}
	else 
	{
		$de_array['error_msg'] = sprintf( "Failed to delete device with DeviceNo=%d (%s)", $de_array['de_DeviceNo'], $msg );
	}
}
else if ( isset($_POST['NewDevice']) || isset($_POST['UpdateDevice']) )
{
	if ( isset($_POST['NewDevice']) )
	{
	    $new_device = true;
		$de_array['de_DeviceNo'] = 0;
	}
	
	if ( strncmp( $de_array['de_ComPort'], "MCU", 3 ) == 0 && ($info = $db->ReadDeviceWithName( $de_array['de_Name'] )) !== false && $info['de_DeviceNo'] != $de_array['de_DeviceNo'] )
	{
	    $de_array['error_msg'] = sprintf( "A device with name %s @ %s already exists in the database.", $de_array['de_Name'], $de_array['de_Hostname'] );
	}
	else if ( strncmp( $de_array['de_ComPort'], "MCU", 3 ) != 0 && ($info = $db->ReadDeviceWithAddress( $de_array['de_Address'] )) !== false && $info['de_DeviceNo'] != $de_array['de_DeviceNo'] )
	{
		$de_array['error_msg'] = sprintf( "A device with address %d @ %s already exists in the database.", $de_array['de_Address'], $de_array['de_Hostname'] );
	}
	else if ( func_check_de_array( $de_array ) )
	{
		if ( $db->UpdateDevicesTable( $de_array['de_DeviceNo'], $de_array['de_ComPort'], $de_array['de_Address'],
			$de_array['de_NumInputs'], $de_array['de_NumOutputs'], $de_array['de_Type'], $de_array['de_Name'], $de_array['de_Hostname'] ) )
		{	// success
		    $new_device = false;
			func_clear_de_array( $de_array );
			
			$de_array['info_msg'] = "New device saved successfully.";
		}
		else
		{
			$de_array['error_msg'] = sprintf( "Failed to update Device record %d", $de_array['di_DeviceNo'] );
		}
	}
}
else if ( isset($_POST['ClearDevice']) )
{
	func_clear_de_array( $de_array );
}

$devices_list = $db->ReadDevicesTable();

?>
<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-4">
			<h3>Device List</h3>
        </div>
		<div class="col-sm-1">
			<a href='#devicelist' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
        </div>
    </div>

	<div id="devicelist" class="collapse <?php ($new_device || $de_array['de_DeviceNo'] != 0 ? printf("") : printf("show"))?>">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-5">
			<?php
			if ( $de_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $de_array['error_msg'] );
		    else if ( $de_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $de_array['info_msg'] );
            ?>

    		<table class='table table-striped'>
    		<thead class="thead-light">
              <tr>
              <th>Type</th>
              <th>Name</th>
              <th>Hostname</th>
            </thead>
 
			<?php            
            foreach ( $devices_list as $info )
            {
	           printf( "<tr>" );
	           
	           $l2 = sprintf( "<div class='small'>@%d (In %d / Out %d)</div>", $info['de_Address'], $info['de_NumInputs'], $info['de_NumOutputs'] );
	           printf( "<td align='left'><a href='?DeviceNo=%d'>%s<br>%s</a></td>", $info['de_DeviceNo'], func_get_device_type_desc($info['de_Type']), $l2 );
	           printf( "<td align='left'><a href='?DeviceNo=%d'>%s</a></td>", $info['de_DeviceNo'], $info['de_Name'] );
	           $l2 = sprintf( "<div class='small'>%s</div>", $info['de_ComPort'] );
	           printf( "<td align='left'><a href='?DeviceNo=%d'>%s<br>%s</a></td>", $info['de_DeviceNo'], $info['de_Hostname'], $l2 );

	           printf( "</tr>" );
            }            
            ?>
            
			</table>
			
            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
                printf( "<p><a href='?DeviceNo=0'>Add New Device</a></p>" );
            ?>
		</div>
	</div>	<!-- end of row -->
	</div>
	
    <?php
	if ( $de_array['de_Address'] != "" || $new_device )
	{
    ?>

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-5">
			<h3>Device Detail</h3>

			<?php
			if ( $de_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $de_array['error_msg'] );
		    else if ( $de_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $de_array['info_msg'] );
			
			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='de_ComPort'>COM Port: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='de_ComPort' id='de_ComPort' size='12' value='%s'> ", $de_array['de_ComPort'] );
    		printf( "</div>" );
    		printf( "</div>" );

    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='de_Name'>Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='de_Name' id='de_Name' size='15' value='%s'> ", $de_array['de_Name'] );
  			printf( "</div>" );
  			printf( "</div>" );
  			
  			printf( "<div class='form-row'>" );
  			printf( "<div class='col'>" );
  			printf( "<label for='de_Hostname'>Hostname: </label>" );
  			printf( "</div>" );
  			printf( "<div class='col'>" );
  			printf( "<input type='text' class='form-control' name='de_Hostname' id='de_Hostname' size='15' value='%s'> ", $de_array['de_Hostname'] );
  			printf( "</div>" );
  			printf( "</div>" );
  			
  			printf( "<div class='form-row'>" );
  			printf( "<div class='col'>" );
  			printf( "<label for='de_ComPort'>Address: </label>" );
  			printf( "</div>" );
  			printf( "<div class='col'>" );
  			printf( "<input type='text' class='form-control' name='de_Address' id='de_Address' size='4' value='%s'> ", $de_array['de_Address'] );
  			printf( "</div>" );
  			printf( "</div>" );
  			
  			printf( "<div class='form-row'>" );
  			printf( "<div class='col'>" );
  			printf( "<label for='de_NumInputs'>Num Inputs: </label>" );
  			printf( "</div>" );
  			printf( "<div class='col'>" );
  			printf( "<input type='text' class='form-control' name='de_NumInputs' id='de_NumInputs' size='3' value='%s'> ", $de_array['de_NumInputs'] );
  			printf( "</div>" );
  			printf( "</div>" );
  			
  			printf( "<div class='form-row'>" );
  			printf( "<div class='col'>" );
  			printf( "<label for='de_NumOutputs'>Num Outputs: </label>" );
  			printf( "</div>" );
  			printf( "<div class='col'>" );
  			printf( "<input type='text' class='form-control' name='de_NumOutputs' id='de_NumOutputs' size='3' value='%s'> ", $de_array['de_NumOutputs'] );
  			printf( "</div>" );
  			printf( "</div>" );
  			
  			printf( "<div class='form-row'>" );
  			printf( "<div class='col'>" );
  			printf( "<label for='de_Type'>Device Type: </label>" );
  			printf( "</div>" );
  			printf( "<div class='col'>" );
  			printf( "<select size='1' class='form-control custom-select' name='de_Type' id='de_Type'>" );
  			printf( "<option></option>" );
  			printf( "<option %s>%s</option>", ($de_array['de_Type'] == E_DT_DIGITAL_IO ? "selected" : ""), E_DTD_DIGITAL_IO );
  			printf( "<option %s>%s</option>", ($de_array['de_Type'] == E_DT_TEMPERATURE_DS ? "selected" : ""), E_DTD_TEMPERATURE_DS );
  			printf( "<option %s>%s</option>", ($de_array['de_Type'] == E_DT_TIMER ? "selected" : ""), E_DTD_TIMER );
  			printf( "<option %s>%s</option>", ($de_array['de_Type'] == E_DT_VOLTAGE ? "selected" : ""), E_DTD_VOLTAGE );
  			printf( "<option %s>%s</option>", ($de_array['de_Type'] == E_DT_TEMPERATURE_K1 ? "selected" : ""), E_DTD_TEMPERATURE_K1 );
  			printf( "</select>" );
  			printf( "</div>" );
  			printf( "</div>" );
  			
  			
  			printf( "<div class='form-row'>" );
  			printf( "<p class='small'><i>COM port is 'MCU' plus chip id for NodeMCU devices</i></p>" );
  			printf( "</div>" );
  			
  			printf( "<div class='form-row mb-2 mt-2'>" );
  			printf( "<p>" );
  			printf( "<input type='hidden' class='form-control' name='de_DeviceNo' value='%d'> ", $de_array['de_DeviceNo'] );
  			printf( "<button type='submit' class='btn btn-outline-dark' name='UpdateDevice' id='UpdateDevice' %s>Update</button> ", ($de_array['de_DeviceNo'] == 0 || func_disabled_non_user() != "" ? "disabled" : "") );
  			printf( "&nbsp;&nbsp;" );
  			printf( "<button type='submit' class='btn btn-outline-dark' name='NewDevice' id='NewDevice' %s>New</button> ", func_disabled_non_user() );
  			printf( "&nbsp;&nbsp;" );
  			$onclick = sprintf( "return confirm(\"Are you sure you want to delete device with address %s ?\")", $de_array['de_Address'] );
  			printf( "<button type='submit' class='btn btn-outline-dark' name='DeleteDevice' id='DeleteDevice' onclick='%s' %s>Delete</button> ", $onclick, ($de_array['de_DeviceNo'] == 0 || func_disabled_non_user() != "" ? "disabled" : "") );
  			printf( "&nbsp;&nbsp;" );
  			printf( "<button type='submit' class='btn btn-outline-dark' name='ClearDevice' id='ClearDevice'>Clear</button>" );
  			printf( "</p>" );
  			printf( "</div>" );
  			
            ?>

		</div>
	</div>	<!-- end or row -->

    <?php
	}
    ?>

</div>

<?php 

?>

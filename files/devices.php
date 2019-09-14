<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------


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
	if ( ($line=$db->GetFields( 'devices', 'de_DeviceNo', $de_array['de_DeviceNo'], "de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Hostname" )) !== false )
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
else if ( isset($_GET['DeleteDeviceNo']) )
{
	$de_array['de_DeviceNo'] = $_GET['DeleteDeviceNo']; 
	if ( $db->DeleteDevice( $de_array['de_DeviceNo'] ) )
	{
		$de_array['info_msg'] = sprintf( "Device deleted" );
	}
	else 
	{
		$de_array['error_msg'] = sprintf( "Failed to delete device with DeviceNo=%d", $de_array['de_DeviceNo'] );
	}
}
else if ( isset($_POST['NewDevice']) || isset($_POST['UpdateDevice']) )
{
	if ( isset($_POST['NewDevice']) )
	{
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


printf( "<tr>" );
printf( "<td>" );
printf( "<b>Devices</b>" );
printf( "</td>" );
printf( "<td colspan='3' align='right'>" );
if ( $de_array['error_msg'] != "" )
	printf( "<div class='style-error'>%s</div>", $de_array['error_msg'] );
else if ( $de_array['info_msg'] != "" )
	printf( "<div class='style-info'>%s</div>", $de_array['info_msg'] );
printf( "</td>" );
printf( "</tr>" );


printf( "<tr valign='top'>" );
printf( "<td colspan='2'>" );
	
$devices_list = $db->ReadDevicesTable();
	
$table_info = array();
$table_info[] = array( 120, "Com Port" );
$table_info[] = array( 60, "Address" );
$table_info[] = array( 50, "Inputs" );
$table_info[] = array( 60, "Outputs" );
$table_info[] = array( 80, "Type" );
$table_info[] = array( 80, "Name" );
$table_info[] = array( 80, "Hostname" );
$table_info[] = array( 44, "&nbsp;" );
func_html_header_table( $table_info );

printf( "<table cellspacing='0' cellpadding='2' class='style-table' border='0' width='%d' height='310'>",
func_html_get_table_width( $table_info ) );
func_html_set_col_widths( $table_info );
printf( "<tbody class='style-tbody'>" );
$count = 0;
foreach ( $devices_list as $info )
{
	$style = "alternateRow";
	if ( $info['de_DeviceNo'] == $de_array['de_DeviceNo'] )
		$style = "selectedRow";
	else if ( ($count % 2) == 0 )
		$style = "normalRow";
	printf( "<tr class='%s'>", $style );
	printf( "<td><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], $info['de_ComPort'] );
	printf( "<td align='center'><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], $info['de_Address'] );
	printf( "<td align='center'><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], $info['de_NumInputs'] );
	printf( "<td align='center'><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], $info['de_NumOutputs'] );
	printf( "<td align='center'><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], func_get_device_type_desc($info['de_Type']) );
	printf( "<td align='center'><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], $info['de_Name'] );
	printf( "<td align='center'><a href='index.php?DeviceNo=%d' class='style-tablelink'>%s</a></td>",
		$info['de_DeviceNo'], $info['de_Hostname'] );
	
	$onclick = sprintf( "return confirm(\"Are you sure you want to delete device with address %s ?\")", $info['de_Address'] );
	printf( "<td><a href='index.php?DeleteDeviceNo=%d' onclick='%s;'>delete</a></td>", $info['de_DeviceNo'], $onclick );
	printf( "</tr>" );
	$count += 1;
}

func_html_extend_table( $table_info, $count, 16 );
printf( "</tbody>" );
printf( "</table>" );

printf( "<p><i>COM port is 'MCU' plus chip id for NodeMCU devices</i></p>" );

printf( "<br><input type='submit' name='Refresh' value='Refresh'>" );
printf( "</td>" );
	
printf( "<td colspan='2'>" );
	
printf( "<table>" );
	
printf( "<tr>" );
printf( "<td><b>Com Port</b></td>" );
printf( "<td><input type='text' size='12' name='de_ComPort' value='%s'></td>", $de_array['de_ComPort'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Name</b></td>" );
printf( "<td><input type='text' size='12' name='de_Name' value='%s'></td>", $de_array['de_Name'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Hostname</b></td>" );
printf( "<td><input type='text' size='15' name='de_Hostname' value='%s'></td>", $de_array['de_Hostname'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Address</b></td>" );
printf( "<td><input type='text' size='4' name='de_Address' value='%s'></td>", $de_array['de_Address'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Num Inputs</b></td>" );
printf( "<td><input type='text' size='3' name='de_NumInputs' value='%s'></td>", $de_array['de_NumInputs'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Num Outputs</b></td>" );
printf( "<td><input type='text' size='3' name='de_NumOutputs' value='%s'></td>", $de_array['de_NumOutputs'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Device Type</b></td>" );
printf( "<td>" );
printf( "<select size='1' name='de_Type'>" );
printf( "<option>" );
printf( "<option %s>%s", ($de_array['de_Type'] == E_DT_DIGITAL_IO ? "selected" : ""), E_DTD_DIGITAL_IO );
printf( "<option %s>%s", ($de_array['de_Type'] == E_DT_TEMPERATURE ? "selected" : ""), E_DTD_TEMPERATURE );
printf( "<option %s>%s", ($de_array['de_Type'] == E_DT_TIMER ? "selected" : ""), E_DTD_TIMER );
printf( "<option %s>%s", ($de_array['de_Type'] == E_DT_VOLTAGE ? "selected" : ""), E_DTD_VOLTAGE );
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td colspan='2'>" );
printf( "<input type='hidden' name='de_DeviceNo' value='%d'>", $de_array['de_DeviceNo'] );
printf( "<input type='submit' name='UpdateDevice' value='Update'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='NewDevice' value='New'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='ClearDevice' value='Clear'>" );
printf( "</td>" );
printf( "</tr>" );

printf( "</table>" );

printf( "</td>" );
printf( "</tr>" );




?>
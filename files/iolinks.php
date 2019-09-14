<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------


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
		$il_array['error_msg'] = "You must select an In Address in the range 0x02 - 0xfe.";
		return false;
	}
	else if ( $il_array['il_OutDeviceNo'] == "" )
	{
		$il_array['error_msg'] = "You must select an Out Address in the range 0x02 - 0xfe.";
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
	if ( ($line=$db->GetFields( 'iolinks', 'il_LinkNo', $il_array['il_LinkNo'], "il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,
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
else if ( isset($_GET['DeleteLinkNo']) )
{
	$il_array['il_LinkNo'] = $_GET['DeleteLinkNo']; 
	if ( $db->DeleteIOLinks( $il_array['il_LinkNo'] ) )
	{
		$il_array['info_msg'] = sprintf( "IOLink deleted" );
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
		$il_array['il_LinkNo'] = 0;
	}
	
	if ( func_check_il_array( $il_array ) )
	{
		if ( $db->UpdateIOLinksTable( $il_array['il_LinkNo'], $il_array['il_InDeviceNo'], $il_array['il_InChannel'], $il_array['il_OutDeviceNo'],
			$il_array['il_OutChannel'], $il_array['il_EventType'], $il_array['il_OnPeriod'], $il_array['il_LinkDeviceNo'], $il_array['il_LinkChannel'],
			$il_array['il_LinkTest'], $il_array['il_LinkValue'] ) )
		{	// success
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


printf( "<tr>" );
printf( "<td>" );
printf( "<b>IO Links</b>" );
printf( "</td>" );
printf( "<td colspan='3' align='right'>" );
if ( $il_array['error_msg'] != "" )
	printf( "<div class='style-error'>%s</div>", $il_array['error_msg'] );
else if ( $il_array['info_msg'] != "" )
	printf( "<div class='style-info'>%s</div>", $il_array['info_msg'] );
printf( "</td>" );
printf( "</tr>" );


printf( "<tr valign='top'>" );
printf( "<td colspan='2'>" );
	
$il_list = $db->ReadIOLinksTable();


$table_info = array();
$table_info[] = array( 170, "Output Name" );
$table_info[] = array( 60, "On Period" );
$table_info[] = array( 80, "Event Type" );
$table_info[] = array( 170, "Input Name" );
$table_info[] = array( 44, "&nbsp;" );
func_html_header_table( $table_info );

printf( "<table cellspacing='0' cellpadding='2' class='style-table' border='0' width='%d' height='320'>",
func_html_get_table_width( $table_info ) );
func_html_set_col_widths( $table_info );
printf( "<tbody class='style-tbody'>" );
$count = 0;
foreach ( $il_list as $info )
{
	$style = "alternateRow";
	if ( $info['il_LinkNo'] == $il_array['il_LinkNo'] )
		$style = "selectedRow";
	else if ( ($count % 2) == 0 )
		$style = "normalRow";
	printf( "<tr class='%s'>", $style );
	printf( "<td><a href='index.php?LinkNo=%d' class='style-tablelink'>%s</a></td>",
		$info['il_LinkNo'], func_get_ioname( $ddo_list, $info['il_OutDeviceNo'], $info['il_OutChannel'], false ) );
	printf( "<td><a href='index.php?LinkNo=%d' class='style-tablelink'>%s</a></td>",
		$info['il_LinkNo'], func_get_on_period( $info['il_OnPeriod'] ) );
	printf( "<td><a href='index.php?LinkNo=%d' class='style-tablelink'>%s</a></td>",
		$info['il_LinkNo'], func_get_eventtype_desc( $info['il_EventType'] ) );
	printf( "<td><a href='index.php?LinkNo=%d' class='style-tablelink'>%s</a></td>",
		$info['il_LinkNo'], func_get_ioname( $ddi_list, $info['il_InDeviceNo'], $info['il_InChannel'], true ) );

	$onclick = sprintf( "return confirm(\"Are you sure you want to delete iolink with address %s ?\")", $info['il_InDeviceNo'] );
	printf( "<td><a href='index.php?DeleteLinkNo=%d' onclick='%s;'>delete</a></td>", $info['il_LinkNo'], $onclick );
	printf( "</tr>" );
	$count += 1;
}

func_html_extend_table( $table_info, $count, 30 );
printf( "</tbody>" );
printf( "</table>" );

printf( "<br><input type='submit' name='Refresh' value='Refresh'>" );
printf( "</td>" );
	
printf( "<td colspan='2'>" );
	
printf( "<table>" );
	
printf( "<tr>" );
printf( "<td><b>In</b></td>" );
printf( "<td>" );
printf( "<select name='il_InDeviceNo' size='1'>" );
printf( "<option>" );
foreach ( $ddi_list as $dd )
{
	$sel = "";
	if ( $dd['de_DeviceNo'] == $il_array['il_InDeviceNo'] && $dd['di_IOChannel'] == $il_array['il_InChannel'] )
		$sel = "selected";
	printf( "<option %s>%02d.%02d %s", $sel, $dd['de_Address'], $dd['di_IOChannel']+1, $dd['di_IOName'] );
}
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Event Type</b></td>" );
printf( "<td>" );
printf( "<select name='il_EventType' size='1'>" );
printf( "<option>" );
printf( "<option %s>%s", ($il_array['il_EventType'] == E_ET_CLICK && $il_array['il_EventType'] !== "" ? "selected" : ""), E_ETD_CLICK );
printf( "<option %s>%s", ($il_array['il_EventType'] == E_ET_DBLCLICK ? "selected" : ""), E_ETD_DBLCLICK );
printf( "<option %s>%s", ($il_array['il_EventType'] == E_ET_LONGPRESS ? "selected" : ""), E_ETD_LONGPRESS );
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Out</b></td>" );
printf( "<td>" );
printf( "<select name='il_OutDeviceNo' size='1'>" );
printf( "<option>" );
foreach ( $ddo_list as $dd )
{
	$sel = "";
	if ( $dd['de_DeviceNo'] == $il_array['il_OutDeviceNo'] && $dd['di_IOChannel'] == $il_array['il_OutChannel'] )
		$sel = "selected";
	printf( "<option %s>%02d.%02d %s", $sel, $dd['de_Address'], $dd['di_IOChannel']+1, $dd['di_IOName'] );
}
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>On Period</b></td>" );
printf( "<td><input type='text' size='6' name='il_OnPeriod' id='il_OnPeriod' value='%s'> (xx s, xx.x m, xx.x h)</td>", func_get_on_period($il_array['il_OnPeriod']) );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>ConditionLink</b></td>" );
printf( "<td>" );
printf( "<select name='il_LinkDeviceNo' size='1'>" );
printf( "<option>" );
foreach ( $ddi_list as $dd )
{
	$sel = "";
	if ( $dd['de_DeviceNo'] == $il_array['il_LinkDeviceNo'] && $dd['di_IOChannel'] == $il_array['il_LinkChannel'] )
		$sel = "selected";
	printf( "<option %s>%02d.%02d %s", $sel, $dd['de_Address'], $dd['di_IOChannel']+1, $dd['di_IOName'] );
}
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Operator</b></td>" );
printf( "<td>" );
printf( "<select name='il_LinkTest' size='1'>" );
printf( "<option>" );
foreach ( $_SESSION['link_tests'] as $dd )
{
	$sel = "";
	if ( $dd == $il_array['il_LinkTest'] )
		$sel = "selected";
	printf( "<option %s>%s", $sel, $dd );
}
printf( "</select>" );
if ( strstr( $il_array['il_LinkTest'], "/" ) !== false )
{
    printf( " (Invert State)" );
}
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Value</b></td>" );
printf( "<td><input type='text' size='6' name='il_LinkValue' id='il_LinkValue' value='%s'></td>", $il_array['il_LinkValue'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td colspan='2'>" );
printf( "<input type='hidden' name='il_LinkNo' value='%d'>", $il_array['il_LinkNo'] );
printf( "<input type='submit' name='UpdateIOLink' value='Update'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='NewIOLink' value='New'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='ClearIOLink' value='Clear'>" );
printf( "</td>" );
printf( "</tr>" );

printf( "</table>" );

printf( "</td>" );
printf( "</tr>" );




?>

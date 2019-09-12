<?php


function func_clear_di_array( &$di_array )
{
	$di_array['di_DeviceInfoNo'] = 0;
	$di_array['di_DeviceNo'] = 0;
	$di_array['de_Address'] = "";
	$di_array['de_Hostname'] = "";
	$di_array['di_IOChannel'] = "";
	$di_array['di_IOName'] = "";
	$di_array['di_IOType'] = "";
	$di_array['di_OnPeriod'] = "";
	$di_array['di_StartTime'] = "";
	$di_array['di_Hysteresis'] = "";
	$di_array['di_Temperature'] = "";
	$di_array['di_Weekdays'] = "";
	$di_array['di_AnalogType'] = "";
	$di_array['di_CalcFactor'] = "";
	$di_array['di_Voltage'] = "";
	$di_array['di_Offset'] = "";
	$di_array['error_msg'] = "";
	$di_array['info_msg'] = "";
	$di_array['addr_filter'] = -1;
}

function func_check_di_array( &$di_array )
{
	$di_array['error_msg'] = "";
	$di_array['info_msg'] = "";
	
	if ( $di_array['de_Address'] === "" )
	{
		$di_array['error_msg'] = "You must select the Device Address.";
		return false;
	}
	else if ( $di_array['di_IOChannel'] < 0 || $di_array['di_IOChannel'] > 15 )
	{
		$di_array['error_msg'] = "You must enter the IO Channel.";
		return false;
	}
	else if ( $di_array['di_IOName'] == "" )
	{
		$di_array['error_msg'] = "You must enter the Name.";
		return false;
	}
	else if ( $di_array['di_IOType'] == "" )
	{
		$di_array['error_msg'] = "You must select the IO Type.";
		return false;
	}
	else if ( $di_array['di_OnPeriod'] == "" && ($di_array['di_IOType'] == E_IO_ON_TIMER || $di_array['di_IOType'] == E_IO_ON_OFF_TIMER) )
	{
		$di_array['error_msg'] = "You must enter the On Period in seconds.";
		return false;
	}
	else if ( $di_array['de_Address'] == 0 && $di_array['di_StartTime'] == "" )
	{
		$di_array['error_msg'] = "You must enter the start time for timer events.";
		return false;
	}
	else if ( func_convert_time($di_array['di_StartTime']) < 0 || func_convert_time($di_array['di_StartTime']) > 1439 )
	{
		$di_array['error_msg'] = "The start time must be from 00:00 to 11:59 pm.";
		return false;
	}
	else if ( ($di_array['di_IOType'] == E_IO_TEMP_HIGH || $di_array['di_IOType'] == E_IO_TEMP_LOW) && $di_array['di_Temperature'] === "" )
	{
		$di_array['error_msg'] = "You must enter the temperature trigger value.";
		return false;
	}
	else if ( ($di_array['di_IOType'] == E_IO_VOLT_HIGH || $di_array['di_IOType'] == E_IO_VOLT_LOW) && $di_array['di_Voltage'] === "" )
	{
		$di_array['error_msg'] = "You must enter the voltage trigger value.";
		return false;
	}
	else if ( ($di_array['di_IOType'] == E_IO_VOLT_HIGH || $di_array['di_IOType'] == E_IO_VOLT_LOW || 
			$di_array['di_IOType'] == E_IO_VOLT_MONITOR || $di_array['di_IOType'] == E_IO_VOLT_DAYNIGHT) 
			&& doubleval($di_array['di_CalcFactor']) == 0.0 )
	{
		$di_array['error_msg'] = "You must enter a Calculation Factor > 0.";
		return false;
	}
	else if ( $di_array['di_IOType'] == E_IO_ON_TIMER && $di_array['di_Weekdays'] == "NNNNNNN" )
	{
		$di_array['error_msg'] = "Timer inputs must trigger on at least one day of the week.";
		return false;
	}
	else if ( $di_array['di_AnalogType'] == "A" && $di_array['di_Offset'] == 0.0 )
	{
		$di_array['error_msg'] = "Analog sensors require an Offset voltage";
		return false;
	}
	
	return true;
}

function func_get_dir( $type, &$sdir )
{
	$dir = 0;
	$sdir = "?";
	
	switch ( $type )
	{
		default:	// input
			$dir = 0;
			$sdir = "In";
			break;
			
		case E_IO_OUTPUT:
			$dir = 1;
			$sdir = "Out";
			break;
		case E_IO_TEMP_HIGH:
			$dir = 0;
			$sdir = "Temp.H";
			break;		
		case E_IO_TEMP_LOW:
			$dir = 0;
			$sdir = "Temp.L";
			break;
		case E_IO_TEMP_MONITOR:
			$dir = 0;
			$sdir = "T.Mon";
			break;
		case E_IO_VOLT_HIGH:
			$dir = 0;
			$sdir = "Volt.H";
			break;		
		case E_IO_VOLT_LOW:
			$dir = 0;
			$sdir = "Volt.L";
			break;
		case E_IO_VOLT_MONITOR:
			$dir = 0;
			$sdir = "V.Mon";
			break;
	}	
	
	return $dir;
}



$di_array = array();
func_clear_di_array( $di_array );

$dw_sun = "N";
$dw_mon = "N";
$dw_tue = "N";
$dw_wed = "N";
$dw_thu = "N";
$dw_fri = "N";
$dw_sat = "N";


if ( isset( $_GET['DeviceInfoNo']) )
	$di_array['di_DeviceInfoNo'] = $_GET['DeviceInfoNo'];
if ( isset( $_GET['AddrFilter']) )
	$di_array['addr_filter'] = intval($_GET['AddrFilter']);
if ( isset( $_POST['di_DeviceInfoNo']) )
	$di_array['di_DeviceInfoNo'] = $_POST['di_DeviceInfoNo'];
if ( isset( $_POST['di_DeviceNo']) )
	$di_array['di_DeviceNo'] = $_POST['di_DeviceNo'];
if ( isset( $_POST['de_Address']) )
	$di_array['de_Address'] = intval($_POST['de_Address']);
if ( isset( $_POST['de_Hostname']) )
	$di_array['de_Hostname'] = $_POST['de_Hostnane'];
if ( isset( $_POST['di_IOChannel']) )
	$di_array['di_IOChannel'] = $_POST['di_IOChannel']-1;
if ( isset( $_POST['di_IOName']) )
	$di_array['di_IOName'] = $_POST['di_IOName'];
if ( isset( $_POST['di_IOType']) )
	$di_array['di_IOType'] = func_get_io_type($_POST['di_IOType']);
if ( isset( $_POST['di_OnPeriod']) )
	$di_array['di_OnPeriod'] = func_get_duration( $_POST['di_OnPeriod'] );
if ( isset( $_POST['di_StartTime']) )
	$di_array['di_StartTime'] = $_POST['di_StartTime'];
if ( isset( $_POST['di_Hysteresis']) )
	$di_array['di_Hysteresis'] = $_POST['di_Hysteresis'];
if ( isset( $_POST['di_Temperature']) )
	$di_array['di_Temperature'] = $_POST['di_Temperature'];

	
if ( isset($_POST['di_Weekdays_sun']) )
	$dw_sun = "Y";
if ( isset($_POST['di_Weekdays_mon']) )
	$dw_mon = "Y";
if ( isset($_POST['di_Weekdays_tue']) )
	$dw_tue = "Y";
if ( isset($_POST['di_Weekdays_wed']) )
	$dw_wed = "Y";
if ( isset($_POST['di_Weekdays_thu']) )
	$dw_thu = "Y";
if ( isset($_POST['di_Weekdays_fri']) )
	$dw_fri = "Y";
if ( isset($_POST['di_Weekdays_sat']) )
	$dw_sat = "Y";
$di_array['di_Weekdays'] = $dw_sun . $dw_mon . $dw_tue . $dw_wed . $dw_thu . $dw_fri . $dw_sat;

if ( isset( $_POST['di_AnalogType']) )
	$di_array['di_AnalogType'] = substr($_POST['di_AnalogType'],0,1);
if ( isset( $_POST['di_CalcFactor']) )
	$di_array['di_CalcFactor'] = $_POST['di_CalcFactor'];
if ( isset( $_POST['di_Voltage']) )
	$di_array['di_Voltage'] = $_POST['di_Voltage'];
if ( isset( $_POST['di_Offset']) )
	$di_array['di_Offset'] = $_POST['di_Offset'];
if ( isset( $_POST['addr_filter']) )
{
    if ( strlen($_POST['addr_filter']) == 0 )
        $di_array['addr_filter'] = -1;
    else
    	$di_array['addr_filter'] = intval($_POST['addr_filter']);
}


if ( isset($_GET['DeviceInfoNo']) )
{
	if ( ($line=$db->GetFields( 'deviceinfo', 'di_DeviceInfoNo', $di_array['di_DeviceInfoNo'], "di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,di_Temperature,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset" )) !== false )
	{	// success
		$di_array['di_DeviceNo'] = $line[0];
		$di_array['di_IOChannel'] = $line[1];
		$di_array['di_IOName'] = $line[2];
		$di_array['di_IOType'] = $line[3];
		$di_array['di_OnPeriod'] = $line[4];
		$di_array['di_StartTime'] = func_convert_time( $line[5] );
		$di_array['di_Hysteresis'] = $line[6];
		$di_array['di_Temperature'] = $line[7];
		$di_array['di_Weekdays'] = $line[8];
		$di_array['di_AnalogType'] = $line[9];
		$di_array['di_CalcFactor'] = $line[10];
		$di_array['di_Voltage'] = $line[11];
		$di_array['di_Offset'] = $line[12];
		
		$di_array['de_Address'] = 0;
		$di_array['de_Hostname'] = '?';
		
		if ( ($line=$db->GetFields( 'devices', 'de_DeviceNo', $di_array['di_DeviceNo'], "de_Address,de_Hostname" )) !== false )
		{
			$di_array['de_Address'] = $line[0];
			$di_array['de_Hostname'] = $line[1];
		}
		else
		{
			$di_array['error_msg'] = sprintf( "Failed to read devices table for DeviceNo=%d", $di_array['di_DeviceNo'] );
		}
		
		if ( $di_array['de_Address'] > 0 )
		{	// not a timer
			$di_array['di_StartTime'] = "";
		}
	}
	else
	{
		$di_array['error_msg'] = sprintf( "Failed to read deviceinfo table for DeviceInfoNo=%d", $di_array['di_DeviceInfoNo'] );
	}
}
else if ( isset($_GET['DeleteDeviceInfoNo']) )
{
	$info = $db->ReadDeviceInfo( $_GET['DeleteDeviceInfoNo'] );
	if ( $db->DeleteDeviceInfo( $info['di_DeviceInfoNo'], $info['di_DeviceNo'], $info['di_IOChannel'] ) )
	{
		$di_array['info_msg'] = sprintf( "Device deleted" );
	}
	else 
	{
		$di_array['error_msg'] = sprintf( "Failed to delete deviceinfo with DeviceInfoNo=%d (delete related IO Links first)", $info['di_DeviceInfoNo'] );
	}
}
else if ( isset($_POST['NewDeviceInfo']) || isset($_POST['UpdateDeviceInfo']) )
{
	$new = false;
	$device = false;
	if ( isset($_POST['NewDeviceInfo']) )
	{
		$new = true;
		$di_array['di_DeviceInfoNo'] = 0;
		$di_array['di_DeviceNo'] = 0;
		
		// get DeviceNo from address
		$info = $db->ReadDeviceWithAddress( $di_array['de_Address'] );
		if ( $info !== false )
		{
			$di_array['di_DeviceNo'] = $info['de_DeviceNo'];
		}
	}
	else
	{
		$device = $db->ReadDeviceWithAddress( $di_array['de_Address'] );
	}
	
	$dup = false;
	$info = $db->ReadDeviceInfoDC( $di_array['di_DeviceNo'], $di_array['di_IOChannel'] );
	if ( $info !== false && $info['di_DeviceInfoNo'] != $di_array['di_DeviceInfoNo'] && 
		func_get_dir($info['di_IOType'],$dir) == func_get_dir($di_array['di_IOType'],$dir) )
	{
		$dup = true;
	}
	
	$record = $db->ReadDeviceInfo( $di_array['di_DeviceInfoNo'] );
	
	
	if ( $dup )
	{
		$di_array['error_msg'] = sprintf( "Device info already exists for address %d channel %d", $di_array['de_Address'], $di_array['di_IOChannel']+1 );
	}
	else if ( $device !== false && $di_array['di_IOChannel'] >= $device['de_NumInputs'] && func_get_dir($info['di_IOType'],$dir) == 0 )
	{
		$di_array['error_msg'] = sprintf( "Device %s on address %d only has %d input channels", $device['de_Name'], $device['de_Address'], $device['de_NumInputs'] );
	}
	else if ( $device !== false && $di_array['di_IOChannel'] >= $device['de_NumOutputs'] && func_get_dir($info['di_IOType'],$dir) == 1 )
	{
		$di_array['error_msg'] = sprintf( "Device %s on address %d only has %d output channels", $device['de_Name'], $device['de_Address'], $device['de_NumInputs'] );
	}
	else if ( func_check_di_array( $di_array ) )
	{
		$stime = func_convert_time( $di_array['di_StartTime'] );
		if ( $db->UpdateDeviceInfoTable( $di_array['di_DeviceInfoNo'], $di_array['di_DeviceNo'], 
				$di_array['di_IOChannel'], $di_array['di_IOName'], $di_array['di_IOType'], $di_array['di_OnPeriod'], $stime, $di_array['di_Hysteresis'],
				$di_array['di_Temperature'], $di_array['di_Weekdays'], $di_array['di_AnalogType'], $di_array['di_CalcFactor'], $di_array['di_Voltage'],
				$di_array['di_Offset'] ) )
		{	// success
			// update iolinks
			if ( $record !== false )
			{
				if ( $record['di_IOChannel'] != $di_array['di_IOChannel'] )
				{	// channel has changed, remove the iolinks
					$db->RefactorIOLinksIn( $record['di_DeviceNo'], $record['di_IOChannel'] );
				}
			}

			$addr_filter = $di_array['addr_filter'];

			func_clear_di_array( $di_array );
			
			$di_array['info_msg'] = "New deviceinfo saved successfully.";
			$di_array['addr_filter'] = $addr_filter;
		}
		else
		{
			$di_array['error_msg'] = sprintf( "Failed to update Device Info record %d", $di_array['di_DeviceInfoNo'] );
		}
	}
}
else if ( isset($_POST['ClickDeviceInfo']) )
{
	if ( $di_array['di_DeviceNo'] != "" && $di_array['di_IOChannel'] != "" )
	{
		func_create_click_file( $di_array['di_DeviceNo'], $di_array['di_IOChannel'] );
	}
	else
	{
		$di_array['error_msg'] = "You must select an IO Device first.";
	}
}
else if ( isset($_POST['ClearDeviceInfo']) )
{
	func_clear_di_array( $di_array );
}

?>
<script language='javascript'>
function pageOnLoad()
{
	ioAddressChange();
}

function ioAddressChange()
{
	console.log( "ioAddressChange()" );
	var addr = document.getElementById('de_Address');
	var sel = document.getElementById('di_IOType');
	if ( sel != null && addr != null )
	{
		console.log( "ioAddressChange(): 1" );
		sel.disabled = false;
	
		if ( addr.value.indexOf('Timer') >= 0 )
		{
			console.log( "ioAddressChange(): Timer" );
			for ( var i = 0; i < sel.options.length; i++ ) 
			{
        		if ( sel.options[i].innerHTML.indexOf('Timer') >= 0 ) 
        		{
           			sel.selectedIndex = i;
           			sel.disabled = true;
           			break;
        		}
    		}
		}
	}
	
	ioTypeChange();
}

function ioTypeChange() 
{
	console.log( "ioTypeChange()" );
	var sel = document.getElementById('di_IOType');
	var addr = document.getElementById('de_Address');
	if ( sel != null && addr != null )
	{
		console.log( "ioTypeChange(): 1" );
		document.getElementById('di_rowTemperature').style.visibility = 'hidden';
		document.getElementById('di_rowHysteresis').style.visibility = 'hidden';
		document.getElementById('di_rowStartTime').style.visibility = 'hidden';
		document.getElementById('di_rowOnPeriod').style.visibility = 'hidden';
		document.getElementById('di_rowWeekdays').style.visibility = 'hidden';
		document.getElementById('di_rowAnalogType').style.visibility = 'hidden';
		document.getElementById('di_rowVoltage').style.visibility = 'hidden';
		document.getElementById('di_rowCalcFactor').style.visibility = 'hidden';
		document.getElementById('di_rowOffset').style.visibility = 'hidden';
		
		if ( sel.value.indexOf("Timer") >= 0 && addr.value.indexOf("Timer") >= 0 )
		{
			console.log( "ioTypeChange(): Timer" );
			document.getElementById('di_rowStartTime').style.visibility = 'visible';
			document.getElementById('di_rowOnPeriod').style.visibility = 'visible';
			document.getElementById('di_rowWeekdays').style.visibility = 'visible';
		}
		else if ( sel.value.indexOf("Timer") >= 0 )
		{
			console.log( "ioTypeChange(): Timer 2" );
			document.getElementById('di_rowOnPeriod').style.visibility = 'visible';
			document.getElementById('di_rowWeekdays').style.visibility = 'visible';
		}
		else if ( sel.value.indexOf("Temperature") >= 0 )
		{
			console.log( "ioTypeChange(): temperature" );
			document.getElementById('di_rowTemperature').style.visibility = 'visible';
			document.getElementById('di_rowHysteresis').style.visibility = 'visible';
		}
		else if ( sel.value.indexOf("Voltage") >= 0 )
		{
			console.log( "ioTypeChange(): voltage" );
			document.getElementById('di_rowAnalogType').style.visibility = 'visible';
			document.getElementById('di_rowVoltage').style.visibility = 'visible';
			document.getElementById('di_rowCalcFactor').style.visibility = 'visible';
			document.getElementById('di_rowOffset').style.visibility = 'visible';
}
	}
}
</script>
<?php 

$di_list = $db->ReadDeviceInfoTable();
$de_list = $db->ReadDevicesTable();
$current_value = $db->GetCurrentValue( $di_array['di_DeviceNo'], $di_array['di_IOChannel'] );

printf( "<tr>" );
printf( "<td>" );
printf( "<b>Device Info</b>" );
printf( "</td>" );

printf( "<td>" );
printf( "Address Filter: " );
printf( "<select size='1' name='addr_filter' id='addr_filter'>" );
printf( "<option> " );
foreach ( $de_list as $device )
{
	printf( "<option %s>%02d. %s", ($di_array['addr_filter'] == $device['de_Address'] ? "selected" : ""), $device['de_Address'], $device['de_Name'] );
}
printf( "</select>" );
printf( "</td>" );

printf( "<td colspan='3' align='right'>" );
if ( $di_array['error_msg'] != "" )
	printf( "<div class='style-error'>%s</div>", $di_array['error_msg'] );
else if ( $di_array['info_msg'] != "" )
	printf( "<div class='style-info'>%s</div>", $di_array['info_msg'] );
printf( "</td>" );
printf( "</tr>" );


printf( "<tr valign='top'>" );
printf( "<td colspan='2'>" );
	

$table_info = array();
$table_info[] = array( 58, "Address" );
$table_info[] = array( 58, "Channel" );
$table_info[] = array( 160, "Name" );
$table_info[] = array( 60, "Type" );
$table_info[] = array( 68, "On Period" );
$table_info[] = array( 44, "&nbsp;" );
func_html_header_table( $table_info );

printf( "<table cellspacing='0' cellpadding='2' class='style-table' border='0' width='%d' height='320'>",
		func_html_get_table_width( $table_info ) );
func_html_set_col_widths( $table_info );
printf( "<tbody class='style-tbody'>" );
$count = 0;
foreach ( $di_list as $info )
{
	if ( $di_array['addr_filter'] >= 0 && $info['de_Address'] != $di_array['addr_filter'] )
	{
		continue;
	}
	$style = "alternateRow";
	if ( $info['di_DeviceInfoNo'] == $di_array['di_DeviceInfoNo'] )
		$style = "selectedRow";
	else if ( ($count % 2) == 0 )
		$style = "normalRow";
	printf( "<tr class='%s'>", $style );
	printf( "<td align='center'><a href='index.php?DeviceInfoNo=%d&AddrFilter=%d' class='style-tablelink'>%s</a></td>",
		$info['di_DeviceInfoNo'], $di_array['addr_filter'], $info['de_Address'] );
	printf( "<td align='center'><a href='index.php?DeviceInfoNo=%d&AddrFilter=%d' class='style-tablelink'>%s</a></td>",
		$info['di_DeviceInfoNo'], $di_array['addr_filter'], $info['di_IOChannel']+1 );
	printf( "<td><a href='index.php?DeviceInfoNo=%d&AddrFilter=%d' class='style-tablelink'>%s</a></td>",
		$info['di_DeviceInfoNo'], $di_array['addr_filter'], $info['di_IOName'] );
	func_get_dir( $info['di_IOType'], $dir ); 
	printf( "<td align='center'><a href='index.php?DeviceInfoNo=%d&AddrFilter=%d' class='style-tablelink'>%s</a></td>",
		$info['di_DeviceInfoNo'], $di_array['addr_filter'], $dir );
	printf( "<td><a href='index.php?DeviceInfoNo=%d&AddrFilter=%d' class='style-tablelink'>%s</a></td>",
		$info['di_DeviceInfoNo'], $di_array['addr_filter'], func_get_on_period($info['di_OnPeriod']) );
	
	$onclick = sprintf( "return confirm(\"Are you sure you want to delete deviceinfo with channel %s ?\")", $info['di_IOChannel'] );
	printf( "<td><a href='index.php?DeleteDeviceInfoNo=%d&AddrFilter=%d' onclick='%s;'>delete</a></td>", 
		$info['di_DeviceInfoNo'], $di_array['addr_filter'], $onclick );
	printf( "</tr>" );
	$count += 1;
}
func_html_extend_table( $table_info, $count, 24 );
printf( "</tbody>" );
printf( "</table>" );

printf( "<br><input type='submit' name='Refresh' value='Refresh'>" );
printf( "</td>" );
	
printf( "<td colspan='2'>" );
	
printf( "<table>" );
	
printf( "<tr>" );
printf( "<td><b>Address</b></td>" );
printf( "<td>" );
printf( "<select size='1' name='de_Address' id='de_Address' onChange='ioAddressChange();'>" );
printf( "<option> " );
foreach ( $de_list as $device )
{
	if ( $device['de_Address'] == 0 )
		printf( "<option %s>Timer", ($di_array['di_DeviceNo'] == $device['de_DeviceNo'] ? "selected" : "") );
	else
		printf( "<option %s>%02d: %s on %s", ($di_array['di_DeviceNo'] == $device['de_DeviceNo'] ? "selected" : ""), $device['de_Address'], $device['de_Name'],
				$device['de_Hostname'] );
}
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>IO Channel</b></td>" );
printf( "<td><input type='text' size='4' name='di_IOChannel' value='%s'> (1-16)</td>", ($di_array['di_IOChannel'] == "" ? "" : $di_array['di_IOChannel']+1) );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Name</b></td>" );
printf( "<td><input type='text' size='20' name='di_IOName' value='%s'></td>", $di_array['di_IOName'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>IO Type</b></td>" );
printf( "<td>" );
printf( "<select size='1' name='di_IOType' id='di_IOType' onChange='ioTypeChange();'>" );
printf( "<option> " );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_ON_OFF ? "selected" : ""), E_IOD_ON_OFF );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_ON_TIMER ? "selected" : ""), E_IOD_ON_TIMER );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_TOGGLE ? "selected" : ""), E_IOD_TOGGLE );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_ON_OFF_TIMER ? "selected" : ""), E_IOD_ON_OFF_TIMER );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_OUTPUT ? "selected" : ""), E_IOD_OUTPUT );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_TEMP_HIGH ? "selected" : ""), E_IOD_TEMP_HIGH );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_TEMP_LOW ? "selected" : ""), E_IOD_TEMP_LOW );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_TEMP_MONITOR ? "selected" : ""), E_IOD_TEMP_MONITOR );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_HIGH ? "selected" : ""), E_IOD_VOLT_HIGH );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_LOW ? "selected" : ""), E_IOD_VOLT_LOW );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_MONITOR ? "selected" : ""), E_IOD_VOLT_MONITOR );
printf( "<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_DAYNIGHT ? "selected" : ""), E_IOD_VOLT_DAYNIGHT );
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr id='di_rowStartTime'>" );
printf( "<td><b>Start Time</b></td>" );
printf( "<td><input type='text' size='7' name='di_StartTime' id='di_StartTime' value='%s'> (hh:mm am/pm)</td>", $di_array['di_StartTime'] );
printf( "</tr>" );

printf( "<tr id='di_rowOnPeriod'>" );
printf( "<td><b>On Period</b></td>" );
printf( "<td><input type='text' size='5' name='di_OnPeriod' id='di_OnPeriod' value='%s'> (xx s, xx.x m, xx.x h)</td>", func_get_on_period($di_array['di_OnPeriod']) );
printf( "</tr>" );

printf( "<tr id='di_rowWeekdays'>" );
printf( "<td colspan='2'><b>Days</b> " );
printf( "<label><input type='checkbox' name='di_Weekdays_sun' id='di_Weekdays_sun' %s>Su&nbsp;</label>", (substr($di_array['di_Weekdays'],0,1) == "Y" ? "checked" : "") );
printf( "<label><input type='checkbox' name='di_Weekdays_mon' id='di_Weekdays_mon' %s>Mo&nbsp;</label>", (substr($di_array['di_Weekdays'],1,1) == "Y" ? "checked" : "") );
printf( "<label><input type='checkbox' name='di_Weekdays_tue' id='di_Weekdays_tue' %s>Tu&nbsp;</label>", (substr($di_array['di_Weekdays'],2,1) == "Y" ? "checked" : "") );
printf( "<label><input type='checkbox' name='di_Weekdays_wed' id='di_Weekdays_wed' %s>We&nbsp;</label>", (substr($di_array['di_Weekdays'],3,1) == "Y" ? "checked" : "") );
printf( "<label><input type='checkbox' name='di_Weekdays_thu' id='di_Weekdays_thu' %s>Th&nbsp;</label>", (substr($di_array['di_Weekdays'],4,1) == "Y" ? "checked" : "") );
printf( "<label><input type='checkbox' name='di_Weekdays_fri' id='di_Weekdays_fri' %s>Fr&nbsp;</label>", (substr($di_array['di_Weekdays'],5,1) == "Y" ? "checked" : "") );
printf( "<label><input type='checkbox' name='di_Weekdays_sat' id='di_Weekdays_sat' %s>Sa&nbsp;</label>", (substr($di_array['di_Weekdays'],6,1) == "Y" ? "checked" : "") );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr id='di_rowTemperature'>" );
printf( "<td><b>Temperature</b></td>" );
printf( "<td><input type='text' size='5' name='di_Temperature' id='di_Temperature' value='%s'> (deg C)</td>", $di_array['di_Temperature'] );
printf( "</tr>" );

printf( "<tr id='di_rowHysteresis'>" );
printf( "<td><b>Hysteresis</b></td>" );
printf( "<td><input type='text' size='5' name='di_Hysteresis' id='di_Hysteresis' value='%s'> (deg C)</td>", $di_array['di_Hysteresis'] );
printf( "</tr>" );

printf( "<tr id='di_rowAnalogType'>" );
printf( "<td><b>Analog Type</b></td>" );
printf( "<td>" );
printf( "<select size='1' name='di_AnalogType' id='di_AnalogType'>" );
printf( "<option>" );
printf( "<option %s>%s", ($di_array['di_AnalogType'] == "V" ? "selected" : ""), "V. Voltage" );
printf( "<option %s>%s", ($di_array['di_AnalogType'] == "A" ? "selected" : ""), "A. Current" );
printf( "</select>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr id='di_rowVoltage'>" );
printf( "<td><b>Voltage</b></td>" );
printf( "<td><input type='text' size='6' name='di_Voltage' id='di_Voltage' value='%s'>", $di_array['di_Voltage'] );
switch ( $di_array['di_IOType'] )
{
default:
	break;
case E_IO_VOLT_HIGH:
case E_IO_VOLT_LOW:
case E_IO_VOLT_MONITOR:
	printf( " Latest Value" );
	break;
}
printf( "</td>" );
printf( "</tr>" );

printf( "<tr id='di_rowCalcFactor'>" );
printf( "<td><b>Calc. Factor</b></td>" );
printf( "<td><input type='text' size='6' name='di_CalcFactor' id='di_CalcFactor' value='%s'>", $di_array['di_CalcFactor'] );
switch ( $di_array['di_IOType'] )
{
default:
	break;
case E_IO_VOLT_HIGH:
case E_IO_VOLT_LOW:
case E_IO_VOLT_MONITOR:
	if ( $di_array['di_AnalogType'] == "V" )
		printf( " %.2fV %s", func_calc_voltage($current_value['ev_Value'], $di_array['di_AnalogType']), $current_value['ev_Timestamp'] );
	else
		printf( " %.2fA %s", func_calc_voltage($current_value['ev_Value'], $di_array['di_AnalogType']), $current_value['ev_Timestamp'] );
	break;
}
printf( "</td>" );
printf( "</tr>" );

printf( "<tr id='di_rowOffset'>" );
printf( "<td><b>Offset Voltage</b></td>" );
printf( "<td><input type='text' size='6' name='di_Offset' id='di_Offset' value='%s'></td>", $di_array['di_Offset'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td colspan='2'>" );
printf( "<input type='hidden' name='di_DeviceInfoNo' value='%d'>", $di_array['di_DeviceInfoNo'] );
printf( "<input type='hidden' name='di_DeviceNo' value='%d'>", $di_array['di_DeviceNo'] );

printf( "<input type='submit' name='UpdateDeviceInfo' value='Update' %s>", ($di_array['di_DeviceInfoNo'] == "" ? "disabled" : "") );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='NewDeviceInfo' value='New'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='ClearDeviceInfo' value='Clear'>" );

$disabled = "disabled";
func_get_dir( $di_array['di_IOType'], $sdir );
if ( $sdir == "In" && $di_array['di_DeviceInfoNo'] != "" )
	$disabled = "";
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='ClickDeviceInfo' value='Click' %s>", $disabled );
printf( "</td>" );
printf( "</tr>" );

printf( "</table>" );

printf( "</td>" );
printf( "</tr>" );




?>
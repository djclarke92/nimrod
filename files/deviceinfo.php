<?php
// --------------------------------------------------------------------------------------
//
// Nimrod Website
// Copyright (c) 2015 Dave Clarke
//
// --------------------------------------------------------------------------------------

include_once( "common.php" );

if ( !isset($_SESSION['us_AuthLevel']) )
{	// access not via main page - access denied
    func_unauthorisedaccess();
    return;
}

function func_clear_di_array(&$di_array)
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
    $di_array['di_MonitorPos1'] = "";
    $di_array['di_MonitorPos2'] = "";
    $di_array['di_MonitorPos3'] = "";
    $di_array['di_MonitorPos4'] = "";
    $di_array['di_MonitorPos5'] = "";
    $di_array['di_MonitorHi'] = "";
    $di_array['di_MonitorLo'] = "";
    $di_array['di_ValueRangeHi'] = "";
    $di_array['di_ValueRangeLo'] = "";
    $di_array['error_msg'] = "";
    $di_array['info_msg'] = "";
    $di_array['addr_filter'] = - 1;
}

// char 1 must be the monitor number 1 or 2
function func_MonPosChar1Valid( $val )
{
    if ( substr($val,0,1) == "1" || substr($val,0,1) == "2" )
    {
        return true;
    }
    
    return false;
}

// char 2 must be F, L or R
function func_MonPosChar2Valid( $val )
{
    if ( substr($val,1,1) == "F" || substr($val,1,1) == "L" || substr($val,1,1) == "R" )
    {
        return true;
    }
    
    return false;
}

// char 3 must be 1-4
function func_MonPosChar3Valid( $val )
{
    $ii = intval(substr($val,2,1));
    if ( $ii >= 1 && $ii <= 4 )
    {
        return true;
    }
    
    return false;
}

function func_check_di_array(&$di_array)
{
    $di_array['error_msg'] = "";
    $di_array['info_msg'] = "";
    
    if ($di_array['de_Address'] === "") {
        $di_array['error_msg'] = "You must select the Device Address.";
        return false;
    } else if ($di_array['di_IOChannel'] < 0 || $di_array['di_IOChannel'] > MAX_IO_PORTS-1 ) {
        $di_array['error_msg'] = sprintf( "You must enter the IO Channel in the range 1 to %d.", MAX_IO_PORTS );
        return false;
    } else if ($di_array['di_IOName'] == "") {
        $di_array['error_msg'] = "You must enter the Name.";
        return false;
    } else if ($di_array['di_IOType'] == "") {
        $di_array['error_msg'] = "You must select the IO Type.";
        return false;
    } else if ($di_array['di_OnPeriod'] == "" && ($di_array['di_IOType'] == E_IO_ON_TIMER || $di_array['di_IOType'] == E_IO_ON_OFF_TIMER)) {
        $di_array['error_msg'] = "You must enter the On Period in seconds.";
        return false;
    } else if ($di_array['de_Address'] == 0 && $di_array['di_StartTime'] == "") {
        $di_array['error_msg'] = "You must enter the start time for timer events.";
        return false;
    } else if (func_convert_time($di_array['di_StartTime']) < 0 || func_convert_time($di_array['di_StartTime']) > 1439) {
        $di_array['error_msg'] = "The start time must be from 00:00 to 11:59 pm.";
        return false;
    } else if (($di_array['di_IOType'] == E_IO_TEMP_HIGH || $di_array['di_IOType'] == E_IO_TEMP_LOW) && $di_array['di_Temperature'] === "") {
        $di_array['error_msg'] = "You must enter the temperature trigger value.";
        return false;
    } else if (($di_array['di_IOType'] == E_IO_VOLT_HIGH || $di_array['di_IOType'] == E_IO_VOLT_LOW) && $di_array['di_Voltage'] === "") {
        $di_array['error_msg'] = "You must enter the voltage trigger value.";
        return false;
    } else if (($di_array['di_IOType'] == E_IO_VOLT_HIGH || $di_array['di_IOType'] == E_IO_VOLT_LOW || $di_array['di_IOType'] == E_IO_VOLT_MONITOR || $di_array['di_IOType'] == E_IO_VOLT_DAYNIGHT) && doubleval($di_array['di_CalcFactor']) == 0.0) {
        $di_array['error_msg'] = "You must enter a Calculation Factor > 0.";
        return false;
    } else if ($di_array['di_IOType'] == E_IO_ON_TIMER && $di_array['di_Weekdays'] == "NNNNNNN") {
        $di_array['error_msg'] = "Timer inputs must trigger on at least one day of the week.";
        return false;
    } else if ($di_array['di_AnalogType'] == "A" && $di_array['di_Offset'] == 0.0) {
        $di_array['error_msg'] = "Analog sensors require an Offset voltage";
        return false;
    } else if ($di_array['di_MonitorPos1'] != "" && (strlen($di_array['di_MonitorPos1']) != 3 || !func_MonPosChar1Valid($di_array['di_MonitorPos1']) || !func_MonPosChar2Valid($di_array['di_MonitorPos1']) || !func_MonPosChar3Valid($di_array['di_MonitorPos1']))) {
        $di_array['error_msg'] = "The Monitor Position #1 must be in the format [12][F|L|R][1..4]";
        return false;
    } else if ($di_array['di_MonitorPos2'] != "" && (strlen($di_array['di_MonitorPos2']) != 3 || !func_MonPosChar1Valid($di_array['di_MonitorPos2']) || !func_MonPosChar2Valid($di_array['di_MonitorPos2']) || !func_MonPosChar3Valid($di_array['di_MonitorPos2']))) {
        $di_array['error_msg'] = "The Monitor Position #2 must be in the format [12][F|L|R][1..4]";
        return false;
    } else if ($di_array['di_MonitorPos3'] != "" && (strlen($di_array['di_MonitorPos3']) != 3 || !func_MonPosChar1Valid($di_array['di_MonitorPos3']) || !func_MonPosChar2Valid($di_array['di_MonitorPos3']) || !func_MonPosChar3Valid($di_array['di_MonitorPos3']))) {
        $di_array['error_msg'] = "The Monitor Position #3 must be in the format [12][F|L|R][1..4]";
        return false;
    } else if ($di_array['di_MonitorPos4'] != "" && (strlen($di_array['di_MonitorPos4']) != 3 || !func_MonPosChar1Valid($di_array['di_MonitorPos4']) || !func_MonPosChar2Valid($di_array['di_MonitorPos4']) || !func_MonPosChar3Valid($di_array['di_MonitorPos4']))) {
        $di_array['error_msg'] = "The Monitor Position #4 must be in the format [12][F|L|R][1..4]";
        return false;
    } else if ($di_array['di_MonitorPos5'] != "" && (strlen($di_array['di_MonitorPos5']) != 3 || !func_MonPosChar1Valid($di_array['di_MonitorPos5']) || !func_MonPosChar2Valid($di_array['di_MonitorPos5']) || !func_MonPosChar3Valid($di_array['di_MonitorPos5']))) {
        $di_array['error_msg'] = "The Monitor Position #5 must be in the format [12][F|L|R][1..4]";
        return false;
    }
    else if ( $di_array['di_IOType'] == E_IO_TEMP_MONITOR && ((strlen($di_array['di_MonitorHi']) != 0 && strlen($di_array['di_MonitorLo']) == 0) || 
        (strlen($di_array['di_MonitorHi']) == 0 && strlen($di_array['di_MonitorLo']) != 0)) )
    {
        $di_array['error_msg'] = "Both monitor values must be entered for alerting";
        return false;
    }
    else if ( $di_array['di_IOType'] == E_IO_TEMP_MONITOR && strlen($di_array['di_MonitorHi']) != 0 && strlen($di_array['di_MonitorLo']) != 0 &&
        floatval($di_array['di_MonitorLo']) > floatval($di_array['di_MonitorHi']) )
    {
        $di_array['error_msg'] = "The 'Monitor Lo' value must be lower than the 'Monitor Hi' value";
        return false;
    }
    
    return true;
}

function func_get_dir($type, &$sdir)
{
    $dir = 0;
    $sdir = "?";
    
    switch ($type) {
        default: // input
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
func_clear_di_array($di_array);

$new_deviceinfo = false;
$dw_sun = "N";
$dw_mon = "N";
$dw_tue = "N";
$dw_wed = "N";
$dw_thu = "N";
$dw_fri = "N";
$dw_sat = "N";

if (isset($_GET['DeviceInfoNo']))
    $di_array['di_DeviceInfoNo'] = $_GET['DeviceInfoNo'];
if (isset($_GET['AddrFilter']))
    $di_array['addr_filter'] = intval($_GET['AddrFilter']);
if (isset($_POST['di_DeviceInfoNo']))
    $di_array['di_DeviceInfoNo'] = $_POST['di_DeviceInfoNo'];
if (isset($_POST['di_DeviceNo']))
    $di_array['di_DeviceNo'] = $_POST['di_DeviceNo'];
if (isset($_POST['de_Address']))
    $di_array['de_Address'] = intval($_POST['de_Address']);
if (isset($_POST['de_Hostname']))
    $di_array['de_Hostname'] = $_POST['de_Hostnane'];
if (isset($_POST['di_IOChannel']))
    $di_array['di_IOChannel'] = $_POST['di_IOChannel'] - 1;
if (isset($_POST['di_IOName']))
    $di_array['di_IOName'] = $_POST['di_IOName'];
if (isset($_POST['di_IOType']))
    $di_array['di_IOType'] = func_get_io_type($_POST['di_IOType']);
if (isset($_POST['di_OnPeriod']))
    $di_array['di_OnPeriod'] = func_get_duration($_POST['di_OnPeriod']);
if (isset($_POST['di_StartTime']))
    $di_array['di_StartTime'] = $_POST['di_StartTime'];
if (isset($_POST['di_Hysteresis']))
    $di_array['di_Hysteresis'] = $_POST['di_Hysteresis'];
if (isset($_POST['di_Temperature']))
    $di_array['di_Temperature'] = $_POST['di_Temperature'];
        
if (isset($_POST['di_Weekdays_sun']))
    $dw_sun = "Y";
if (isset($_POST['di_Weekdays_mon']))
    $dw_mon = "Y";
if (isset($_POST['di_Weekdays_tue']))
    $dw_tue = "Y";
if (isset($_POST['di_Weekdays_wed']))
    $dw_wed = "Y";
if (isset($_POST['di_Weekdays_thu']))
    $dw_thu = "Y";
if (isset($_POST['di_Weekdays_fri']))
    $dw_fri = "Y";
if (isset($_POST['di_Weekdays_sat']))
    $dw_sat = "Y";
$di_array['di_Weekdays'] = $dw_sun . $dw_mon . $dw_tue . $dw_wed . $dw_thu . $dw_fri . $dw_sat;

if (isset($_POST['di_AnalogType']))
    $di_array['di_AnalogType'] = substr($_POST['di_AnalogType'], 0, 1);
if (isset($_POST['di_CalcFactor']))
    $di_array['di_CalcFactor'] = $_POST['di_CalcFactor'];
if (isset($_POST['di_Voltage']))
    $di_array['di_Voltage'] = $_POST['di_Voltage'];
if (isset($_POST['di_Offset']))
    $di_array['di_Offset'] = $_POST['di_Offset'];
if (isset($_POST['di_MonitorPos1']))
    $di_array['di_MonitorPos1'] = strtoupper($_POST['di_MonitorPos1']);
if (isset($_POST['di_MonitorPos2']))
    $di_array['di_MonitorPos2'] = strtoupper($_POST['di_MonitorPos2']);
if (isset($_POST['di_MonitorPos3']))
    $di_array['di_MonitorPos3'] = strtoupper($_POST['di_MonitorPos3']);
if (isset($_POST['di_MonitorPos4']))
    $di_array['di_MonitorPos4'] = strtoupper($_POST['di_MonitorPos4']);
if (isset($_POST['di_MonitorPos5']))
    $di_array['di_MonitorPos5'] = strtoupper($_POST['di_MonitorPos5']);
if (isset($_POST['di_MonitorLo']))
    $di_array['di_MonitorLo'] = $_POST['di_MonitorLo'];
if (isset($_POST['di_MonitorHi']))
    $di_array['di_MonitorHi'] = $_POST['di_MonitorHi'];
if (isset($_POST['di_ValueRangeLo']))
    $di_array['di_ValueRangeLo'] = $_POST['di_ValueRangeLo'];
if (isset($_POST['di_ValueRangeHi']))
    $di_array['di_ValueRangeHi'] = $_POST['di_ValueRangeHi'];
            
if (isset($_POST['addr_filter'])) {
    if (strlen($_POST['addr_filter']) == 0)
        $di_array['addr_filter'] = - 1;
    else
        $di_array['addr_filter'] = intval($_POST['addr_filter']);
}

if (isset($_GET['DeviceInfoNo'])) 
{
    if ( $_GET['DeviceInfoNo'] == 0 )
    {   // show blank detail page
        $new_deviceinfo = true;
    }
    else if (($line = $db->GetFields('deviceinfo', 'di_DeviceInfoNo', $di_array['di_DeviceInfoNo'], "di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,di_Temperature,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset,di_MonitorPos,di_MonitorHi,di_MonitorLo,di_ValueRangeHi,di_ValueRangeLo")) !== false) { // success
        $di_array['di_DeviceNo'] = $line[0];
        $di_array['di_IOChannel'] = $line[1];
        $di_array['di_IOName'] = $line[2];
        $di_array['di_IOType'] = $line[3];
        $di_array['di_OnPeriod'] = $line[4];
        $di_array['di_StartTime'] = func_convert_time($line[5]);
        $di_array['di_Hysteresis'] = $line[6];
        $di_array['di_Temperature'] = $line[7];
        $di_array['di_Weekdays'] = $line[8];
        $di_array['di_AnalogType'] = $line[9];
        $di_array['di_CalcFactor'] = $line[10];
        $di_array['di_Voltage'] = $line[11];
        $di_array['di_Offset'] = $line[12];
        $di_array['di_MonitorPos1'] = substr($line[13], 0, 3);
        $di_array['di_MonitorPos2'] = substr($line[13], 3, 3);
        $di_array['di_MonitorPos3'] = substr($line[13], 6, 3);
        $di_array['di_MonitorPos4'] = substr($line[13], 9, 3);
        $di_array['di_MonitorPos5'] = substr($line[13], 12, 3);
        $di_array['di_MonitorHi'] = $line[14];
        $di_array['di_MonitorLo'] = $line[15];
        $di_array['di_ValueRangeHi'] = $line[16];
        $di_array['di_ValueRangeLo'] = $line[17];
        
        $di_array['de_Address'] = 0;
        $di_array['de_Hostname'] = '?';
        
        if (($line = $db->GetFields('devices', 'de_DeviceNo', $di_array['di_DeviceNo'], "de_Address,de_Hostname")) !== false) 
        {
            $di_array['de_Address'] = $line[0];
            $di_array['de_Hostname'] = $line[1];
        } 
        else 
        {
            $di_array['error_msg'] = sprintf("Failed to read devices table for DeviceNo=%d", $di_array['di_DeviceNo']);
        }
        
        if ($di_array['de_Address'] > 0) { // not a timer
            $di_array['di_StartTime'] = "";
        }
    } 
    else 
    {
        $di_array['error_msg'] = sprintf("Failed to read deviceinfo table for DeviceInfoNo=%d", $di_array['di_DeviceInfoNo']);
    }
} 
else if (isset($_POST['DeleteDeviceInfo'])) 
{
    $info = $db->ReadDeviceInfo($_POST['di_DeviceInfoNo']);
    if ($db->DeleteDeviceInfo($info['di_DeviceInfoNo'], $info['di_DeviceNo'], $info['di_IOChannel'])) 
    {
        $di_array['info_msg'] = sprintf("Device deleted");
        func_clear_di_array($di_array);
    } 
    else 
    {
        $di_array['error_msg'] = sprintf("Failed to delete deviceinfo with DeviceInfoNo=%d (delete related IO Links first)", $info['di_DeviceInfoNo']);
    }
} 
else if (isset($_POST['NewDeviceInfo']) || isset($_POST['UpdateDeviceInfo'])) 
{
    $device = false;
    if (isset($_POST['NewDeviceInfo'])) 
    {
        $new_deviceinfo = true;
        $di_array['di_DeviceInfoNo'] = 0;
        $di_array['di_DeviceNo'] = 0;
        
        // get DeviceNo from address
        $info = $db->ReadDeviceWithAddress($di_array['de_Address']);
        if ($info !== false) 
        {
            $di_array['di_DeviceNo'] = $info['de_DeviceNo'];
        }
    } 
    else 
    {
        $device = $db->ReadDeviceWithAddress($di_array['de_Address']);
    }
    
    $dup = false;
    $info = $db->ReadDeviceInfoDC($di_array['di_DeviceNo'], $di_array['di_IOChannel']);
    if ($info !== false && $info['di_DeviceInfoNo'] != $di_array['di_DeviceInfoNo'] && func_get_dir($info['di_IOType'], $dir) == func_get_dir($di_array['di_IOType'], $dir)) 
    {
        $dup = true;
    }
    
    $record = $db->ReadDeviceInfo($di_array['di_DeviceInfoNo']);
    
    if ($dup) 
    {
        $di_array['error_msg'] = sprintf("Device info already exists for address %d channel %d", $di_array['de_Address'], $di_array['di_IOChannel'] + 1);
    } 
    else if ($device !== false && $di_array['di_IOChannel'] >= $device['de_NumInputs'] && func_get_dir($info['di_IOType'], $dir) == 0) 
    {
        $di_array['error_msg'] = sprintf("Device %s on address %d only has %d input channels", $device['de_Name'], $device['de_Address'], $device['de_NumInputs']);
    } 
    else if ($device !== false && $di_array['di_IOChannel'] >= $device['de_NumOutputs'] && func_get_dir($info['di_IOType'], $dir) == 1) 
    {
        $di_array['error_msg'] = sprintf("Device %s on address %d only has %d output channels", $device['de_Name'], $device['de_Address'], $device['de_NumInputs']);
    } 
    else if (func_check_di_array($di_array)) 
    {
        $stime = func_convert_time($di_array['di_StartTime']);
        $monitor_pos = sprintf("%s%s%s%s%s", $di_array['di_MonitorPos1'], $di_array['di_MonitorPos2'], $di_array['di_MonitorPos3'], $di_array['di_MonitorPos4'], $di_array['di_MonitorPos5']);
        if ($db->UpdateDeviceInfoTable($di_array['di_DeviceInfoNo'], $di_array['di_DeviceNo'], $di_array['di_IOChannel'], $di_array['di_IOName'], $di_array['di_IOType'], $di_array['di_OnPeriod'], $stime, $di_array['di_Hysteresis'], $di_array['di_Temperature'], $di_array['di_Weekdays'], $di_array['di_AnalogType'], $di_array['di_CalcFactor'], $di_array['di_Voltage'], $di_array['di_Offset'], $monitor_pos, 
            $di_array['di_MonitorHi'], $di_array['di_MonitorLo'], $di_array['di_ValueRangeHi'], $di_array['di_ValueRangeLo'] )) 
        { // success
          // update iolinks
            if ($record !== false) 
            {
                if ($record['di_IOChannel'] != $di_array['di_IOChannel']) 
                { // channel has changed, remove the iolinks
                    $db->RefactorIOLinksIn($record['di_DeviceNo'], $record['di_IOChannel']);
                }
            }
            
            $addr_filter = $di_array['addr_filter'];
            
            $new_deviceinfo = false;
            func_clear_di_array($di_array);
            
            $di_array['info_msg'] = "New deviceinfo saved successfully.";
            $di_array['addr_filter'] = $addr_filter;
        } 
        else 
        {
            $di_array['error_msg'] = sprintf("Failed to update Device Info record %d", $di_array['di_DeviceInfoNo']);
        }
    }
} 
else if (isset($_POST['ClickDeviceInfo'])) 
{
    if ($di_array['di_DeviceNo'] != "" && $di_array['di_IOChannel'] != "") 
    {
        if ( func_create_click_file($db, $di_array['di_DeviceNo'], $di_array['di_IOChannel']) == false )
        {
            $di_array['info_msg'] = "Failed to create click file";
        }
        else
        {
            $di_array['info_msg'] = "Click file created successfully";
        }
    } 
    else 
    {
        $di_array['error_msg'] = "You must select an IO Device first.";
    }
} 
else if (isset($_POST['ClearDeviceInfo'])) 
{
    func_clear_di_array($di_array);
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
		document.getElementById('di_rowMonitorPos').style.visibility = 'hidden';
		document.getElementById('di_rowMonitorHiLo').style.visibility = 'hidden';
		document.getElementById('di_rowValueRange').style.visibility = 'hidden';
		
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
			document.getElementById('di_rowMonitorPos').style.visibility = 'visible';
			document.getElementById('di_rowMonitorHiLo').style.visibility = 'visible';
			document.getElementById('di_rowValueRange').style.visibility = 'visible';
		}
		else if ( sel.value.indexOf("Voltage") >= 0 )
		{
			console.log( "ioTypeChange(): voltage" );
			document.getElementById('di_rowAnalogType').style.visibility = 'visible';
			document.getElementById('di_rowVoltage').style.visibility = 'visible';
			document.getElementById('di_rowCalcFactor').style.visibility = 'visible';
			document.getElementById('di_rowOffset').style.visibility = 'visible';
			document.getElementById('di_rowMonitorPos').style.visibility = 'visible';
			document.getElementById('di_rowMonitorHiLo').style.visibility = 'visible';
			document.getElementById('di_rowValueRange').style.visibility = 'visible';
		}
	}
}
</script>
<?php

$di_list = $db->ReadDeviceInfoTable();
$de_list = $db->ReadDevicesTable();
$current_value = $db->GetCurrentValue($di_array['di_DeviceNo'], $di_array['di_IOChannel']);

?>
<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-2">
			<h3>Device Info</h3>
		</div>
			
		<div class="col-sm-1">
			<a href='#deviceinfolist' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
        </div>

		<div class="col-sm-2">
			<div class='input-group'>
			<select class='form-control custom-select' size='1' name='addr_filter' id='addr_filter'>
			<option></option>
			<?php
			foreach ($de_list as $device) 
			{
			    printf("<option %s>%02d. %s</option>", ($di_array['addr_filter'] == $device['de_Address'] ? "selected" : ""), $device['de_Address'], $device['de_Name']);
			}
			?>
			</select> 
			&nbsp;
			<button type='submit' class='btn btn-outline-dark mt-2' name='Refresh' id='Refresh'>Filter</button>
			</div>
		</div>
	</div>
			
	<div id="deviceinfolist" class="collapse <?php ($new_deviceinfo || $di_array['di_DeviceInfoNo'] != 0 ? printf("") : printf("show"))?>">
			
    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-5 mb-2">

			<?php 
			if ( $di_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $di_array['error_msg'] );
		    else if ( $di_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $di_array['info_msg'] );
            ?>
            
    		<table class='table table-striped'>
    		<thead class="thead-light">
              <tr>
              <th>Address</th>
              <th>Channel</th>
              <th>Name</th>
              <th>Dir</th>
              <th>On Period</th>
              </tr>
            </thead>

			<?php
			foreach ( $di_list as $info )
			{
			    if ($di_array['addr_filter'] >= 0 && $info['de_Address'] != $di_array['addr_filter']) {
			        continue;
			    }
			    
			    printf( "<tr>" );
			    
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $info['de_Address']);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $info['di_IOChannel'] + 1);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $info['di_IOName']);
			    func_get_dir($info['di_IOType'], $dir);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $dir);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], func_get_on_period($info['di_OnPeriod']));
			    
			    printf( "</tr>" );
			}
            ?>
            </table>

            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
                printf( "<p><a href='?DeviceInfoNo=0'>Add New DeviceInfo</a></p>" );
            ?>

		</div>
	</div>	<!-- end of row -->
	</div>
	<?php
    if ( $di_array['de_Address'] != "" || $new_deviceinfo )
    {
    ?>

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-8">

			<h3>Device Info Detail</h3>

			<?php
			if ( $di_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $di_array['error_msg'] );
		    else if ( $di_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $di_array['info_msg'] );
			
	        printf( "<div class='form-row'>" );
	        printf( "<div class='col'>" );
	        printf( "<label for='de_Address'>Address: </label>" );
	        printf( "</div>" );
	        printf( "<div class='col'>" );
	        printf( "<select size='1' class='form-control custom-select' name='de_Address' id='de_Address'>" );
	        printf( "<option></option>" );
	        foreach ($de_list as $device) {
	            if ($device['de_Address'] == 0)
	                printf("<option %s>Timer", ($di_array['di_DeviceNo'] == $device['de_DeviceNo'] ? "selected" : ""));
	            else
	                printf("<option %s>%02d: %s on %s", ($di_array['di_DeviceNo'] == $device['de_DeviceNo'] ? "selected" : ""), $device['de_Address'], $device['de_Name'], $device['de_Hostname']);
	        }
	        printf( "</select>" );
	        printf( "</div>" );
	        printf( "</div>" );
		        
			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='di_IOChannel'>IO Channel: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_IOChannel' id='di_IOChannel' size='3' value='%s'> ", ($di_array['di_IOChannel'] === "" ? "" : $di_array['di_IOChannel'] + 1) );
    		printf( "</div>" );
    		printf( "</div>" );

    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_IOName'>Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_IOName' id='di_IOName' size='20' value='%s'> ", $di_array['di_IOName'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_IOType'>IO Type: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf("<select class='custom-select' size='1' name='di_IOType' id='di_IOType' onChange='ioTypeChange();'>");
    		printf("<option> ");
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_ON_OFF ? "selected" : ""), E_IOD_ON_OFF);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_ON_TIMER ? "selected" : ""), E_IOD_ON_TIMER);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_TOGGLE ? "selected" : ""), E_IOD_TOGGLE);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_ON_OFF_TIMER ? "selected" : ""), E_IOD_ON_OFF_TIMER);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_OUTPUT ? "selected" : ""), E_IOD_OUTPUT);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_TEMP_HIGH ? "selected" : ""), E_IOD_TEMP_HIGH);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_TEMP_LOW ? "selected" : ""), E_IOD_TEMP_LOW);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_TEMP_MONITOR ? "selected" : ""), E_IOD_TEMP_MONITOR);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_HIGH ? "selected" : ""), E_IOD_VOLT_HIGH);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_LOW ? "selected" : ""), E_IOD_VOLT_LOW);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_MONITOR ? "selected" : ""), E_IOD_VOLT_MONITOR);
    		printf("<option %s>%s", ($di_array['di_IOType'] == E_IO_VOLT_DAYNIGHT ? "selected" : ""), E_IOD_VOLT_DAYNIGHT);
    		printf("</select>");
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_StartTime'>Start Time: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_StartTime' id='di_StartTime' size='7' value='%s'> ", $di_array['di_StartTime'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_OnPeriod'>Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_OnPeriod' id='di_OnPeriod' size='5' value='%s'> ", $di_array['di_OnPeriod'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_Weekdays'>Days: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col form-check-inline'>" );
    		printf("<label><input type='checkbox' name='di_Weekdays_sun' id='di_Weekdays_sun' %s>Su&nbsp;</label>", (substr($di_array['di_Weekdays'], 0, 1) == "Y" ? "checked" : ""));
    		printf("<label><input type='checkbox' name='di_Weekdays_mon' id='di_Weekdays_mon' %s>Mo&nbsp;</label>", (substr($di_array['di_Weekdays'], 1, 1) == "Y" ? "checked" : ""));
    		printf("<label><input type='checkbox' name='di_Weekdays_tue' id='di_Weekdays_tue' %s>Tu&nbsp;</label>", (substr($di_array['di_Weekdays'], 2, 1) == "Y" ? "checked" : ""));
    		printf("<label><input type='checkbox' name='di_Weekdays_wed' id='di_Weekdays_wed' %s>We&nbsp;</label>", (substr($di_array['di_Weekdays'], 3, 1) == "Y" ? "checked" : ""));
    		printf("<label><input type='checkbox' name='di_Weekdays_thu' id='di_Weekdays_thu' %s>Th&nbsp;</label>", (substr($di_array['di_Weekdays'], 4, 1) == "Y" ? "checked" : ""));
    		printf("<label><input type='checkbox' name='di_Weekdays_fri' id='di_Weekdays_fri' %s>Fr&nbsp;</label>", (substr($di_array['di_Weekdays'], 5, 1) == "Y" ? "checked" : ""));
    		printf("<label><input type='checkbox' name='di_Weekdays_sat' id='di_Weekdays_sat' %s>Sa&nbsp;</label>", (substr($di_array['di_Weekdays'], 6, 1) == "Y" ? "checked" : ""));
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_Temperature'>Temperature: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_Temperature' id='di_Temperature' size='5' value='%s'> (deg C) ", $di_array['di_Temperature'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_Hysteresis'>Hysteresis: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_Hysteresis' id='di_Hysteresis' size='5' value='%s'> (deg C) ", $di_array['di_Hysteresis'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_AnalogType'>Analog Type: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf("<select class='custom-select' size='1' name='di_AnalogType' id='di_AnalogType'>");
    		printf("<option></option>");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "V" ? "selected" : ""), "V. Voltage");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "A" ? "selected" : ""), "A. Current");
    		printf("</select>");
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_Voltage'>Voltage: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_Voltage' id='di_Voltage' size='5' value='%s'> ", $di_array['di_Voltage'] );
    		switch ($di_array['di_IOType']) {
    		    default:
    		        break;
    		    case E_IO_VOLT_HIGH:
    		    case E_IO_VOLT_LOW:
    		    case E_IO_VOLT_MONITOR:
    		        printf(" Latest Value");
    		        break;
    		}
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_CalcFactor'>Calc Factor: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_CalcFactor' id='di_CalcFactor' size='5' value='%s'> ", $di_array['di_CalcFactor'] );
    		switch ($di_array['di_IOType']) {
    		    default:
    		        break;
    		    case E_IO_VOLT_HIGH:
    		    case E_IO_VOLT_LOW:
    		    case E_IO_VOLT_MONITOR:
    		        if ($di_array['di_AnalogType'] == "V")
    		            printf(" %.2fV %s", func_calc_voltage($current_value['ev_Value'], $di_array['di_AnalogType']), $current_value['ev_Timestamp']);
    		        else
    		            printf(" %.2fA %s", func_calc_voltage($current_value['ev_Value'], $di_array['di_AnalogType']), $current_value['ev_Timestamp']);
    		        break;
    		}
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_Offset'>Offset Voltage: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_Offset' id='di_Offset' size='5' value='%s'> ", $di_array['di_Offset'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_MonitorPos'>Monitor Position: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		$tip = sprintf( "Enter 3 characters:<br>1st: monitor number 1 or 2<br>2nd: F, L or R for full width, left or right<br>3rd: graph row number, 1 to 4" );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos1' id='di_MonitorPos1' size='3' value='%s' data-toggle='tooltip' data-html='true' title='%s'> ", $di_array['di_MonitorPos1'], $tip );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos2' id='di_MonitorPos2' size='3' value='%s'> ", $di_array['di_MonitorPos2'] );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos3' id='di_MonitorPos3' size='3' value='%s'> ", $di_array['di_MonitorPos3'] );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos4' id='di_MonitorPos4' size='3' value='%s'> ", $di_array['di_MonitorPos4'] );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos5' id='di_MonitorPos5' size='3' value='%s'> ", $di_array['di_MonitorPos5'] );
    		printf( "<br>[Monitor <b>1</b>,<b>2</b>][<b>F</b>ull,<b>L</b>eft,<b>R</b>ight][Graph No. <b>1</b>-<b>4</b>]" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_MonitorLoHi'>Monitor Alert Range: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_MonitorLo' id='di_MonitorLo' size='3' value='%s'> ", $di_array['di_MonitorLo'] );
    		printf( " to " );
    		printf( "<input type='text' class='form-control' name='di_MonitorHi' id='di_MonitorHi' size='3' value='%s'> ", $di_array['di_MonitorHi'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row'>" );
    		printf( "<div class='col'>" );
    		printf( "<label for='di_ValueRangeLoHi'>Y Axis Range: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='di_ValueRangeLo' id='di_ValueRangeLo' size='3' value='%s'> ", $di_array['di_ValueRangeLo'] );
    		printf( " to " );
    		printf( "<input type='text' class='form-control' name='di_ValueRangeHi' id='di_ValueRangeHi' size='3' value='%s'> ", $di_array['di_ValueRangeHi'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='form-row mb-2 mt-2'>" );
    		printf( "<p>" );
    		printf("<input type='hidden' class='form-control' name='di_DeviceInfoNo' value='%d'>", $di_array['di_DeviceInfoNo']);
    		printf("<input type='hidden' class='form-control' name='di_DeviceNo' value='%d'>", $di_array['di_DeviceNo']);
    		
    		printf("<button type='submit' class='btn btn-outline-dark' name='UpdateDeviceInfo' id='UpdateDeviceInfo' %s>Update</button>", ($di_array['di_DeviceInfoNo'] == 0 || func_disabled_non_user() != "" ? "disabled" : ""));
    		printf("&nbsp;&nbsp;");
    		printf("<button type='submit' class='btn btn-outline-dark' name='NewDeviceInfo' id='NewDeviceInfo' %s>New</button>", func_disabled_non_user() );
    		printf("&nbsp;&nbsp;");
    		$onclick = sprintf("return confirm(\"Are you sure you want to delete deviceinfo with channel %s ?\")", $di_array['di_IOChannel']+1 );
    		printf("<button type='submit' class='btn btn-outline-dark' name='DeleteDeviceInfo' id='DeleteDeviceInfo' onclick='%s' %s>Delete</button>", $onclick, ($di_array['di_DeviceInfoNo'] == 0 || func_disabled_non_user() != "" ? "disabled" : "") );
    		printf("&nbsp;&nbsp;");
    		printf("<button type='submit' class='btn btn-outline-dark' name='ClearDeviceInfo' id='ClearDeviceInfo'>Clear</button>");
    		$disabled = "disabled";
    		func_get_dir($di_array['di_IOType'], $sdir);
    		if ($sdir == "In" && $di_array['di_DeviceInfoNo'] != "")
    		    $disabled = "";
    		printf("&nbsp;&nbsp;");
    		printf("&nbsp;&nbsp;");
    		printf("&nbsp;&nbsp;");
    		printf("&nbsp;&nbsp;");
    		printf("<button type='submit' class='btn btn-outline-dark' name='ClickDeviceInfo' id='ClickDeviceInfo' %s>Click</button>", $disabled);
    		printf( "</p>" );
    		printf( "</div>" );
    		
    		
    		
    		?>

		</div>
	</div>	<!-- end of row -->
	
	<?php
	}
	?>	
</div>





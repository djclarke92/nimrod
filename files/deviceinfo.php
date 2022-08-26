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
    $di_array['di_Weekdays'] = "";
    $di_array['di_AnalogType'] = "";
    $di_array['di_CalcFactor'] = "";
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
    $di_array['addr_filter'] = -1;
    $di_array['dir_filter'] = -1;
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
    } else if (($di_array['di_IOType'] == E_IO_TEMP_HIGHLOW || $di_array['di_IOType'] == E_IO_TEMP_HIGH || $di_array['di_IOType'] == E_IO_TEMP_LOW) && $di_array['di_Hysteresis'] === "") {
        $di_array['error_msg'] = "You must enter the temperature hysteresis value.";
        return false;
    } else if (($di_array['di_IOType'] == E_IO_VOLT_HIGHLOW || $di_array['di_IOType'] == E_IO_VOLT_HIGH || $di_array['di_IOType'] == E_IO_VOLT_LOW) && $di_array['di_Hysteresis'] === "") {
        $di_array['error_msg'] = "You must enter the voltage hysteresis value.";
        return false;
    } else if (($di_array['di_IOType'] == E_IO_VOLT_HIGHLOW || $di_array['di_IOType'] == E_IO_VOLT_HIGH || $di_array['di_IOType'] == E_IO_VOLT_LOW || 
        $di_array['di_IOType'] == E_IO_VOLT_MONITOR || $di_array['di_IOType'] == E_IO_VOLT_DAYNIGHT) && 
        $di_array['di_CalcFactor'] === "" ) {
        $di_array['error_msg'] = "You must enter a Calculation Factor >= 0.";
        return false;
    } else if ($di_array['di_IOType'] == E_IO_ON_TIMER && $di_array['di_Weekdays'] == "NNNNNNN") {
        $di_array['error_msg'] = "Timer inputs must trigger on at least one day of the week.";
        return false;
    } else if ($di_array['di_AnalogType'] == "A" && $di_array['di_Offset'] == "") {
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
    else if ( ($di_array['di_IOType'] == E_IO_TEMP_MONITOR || $di_array['di_IOType'] == E_IO_VOLT_MONITOR) && ((strlen($di_array['di_MonitorHi']) != 0 && strlen($di_array['di_MonitorLo']) == 0) || 
        (strlen($di_array['di_MonitorHi']) == 0 && strlen($di_array['di_MonitorLo']) != 0)) )
    {
        $di_array['error_msg'] = "Both Monitor High and Low values must be entered for alerting";
        return false;
    }
    else if ( ($di_array['di_IOType'] == E_IO_TEMP_HIGHLOW || $di_array['di_IOType'] == E_IO_VOLT_HIGHLOW) && ((strlen($di_array['di_MonitorHi']) != 0 && strlen($di_array['di_MonitorLo']) == 0) ||
        (strlen($di_array['di_MonitorHi']) == 0 && strlen($di_array['di_MonitorLo']) != 0)) )
    {
        $di_array['error_msg'] = "Both Monitor High and Low values must be entered for actions to trigger.";
        return false;
    }
    else if ( ($di_array['di_IOType'] == E_IO_TEMP_HIGH || $di_array['di_IOType'] == E_IO_VOLT_HIGH) && strlen($di_array['di_MonitorHi']) == 0 )
    {
        $di_array['error_msg'] = "The 'Monitor High' value must be entered for actions to trigger.";
        return false;
    }
    else if ( ($di_array['di_IOType'] == E_IO_TEMP_LOW || $di_array['di_IOType'] == E_IO_VOLT_LOW) && strlen($di_array['di_MonitorLo']) == 0 )
    {
        $di_array['error_msg'] = "The 'Monitor Low' value must be entered for actions to trigger.";
        return false;
    }
    else if ( strlen($di_array['di_MonitorHi']) != 0 && strlen($di_array['di_MonitorLo']) != 0 &&
        floatval($di_array['di_MonitorLo']) > floatval($di_array['di_MonitorHi']) )
    {
        $di_array['error_msg'] = "The 'Monitor Low' value must be lower than the 'Monitor High' value";
        return false;
    }
    else if ( ($di_array['di_IOType'] == E_IO_ROTENC_LOW || $di_array['di_IOType'] == E_IO_ROTENC_LOW || $di_array['di_IOType'] == E_IO_ROTENC_HIGHLOW || $di_array['di_IOType'] == E_IO_ROTENC_MONITOR) && 
        strlen($di_array['di_CalcFactor']) == 0 )
    {
        $di_array['error_msg'] = "The 'Calc Factor' value must be entered for Rotary Encoders.";
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
        case E_IO_TEMP_HIGHLOW:
            $sdir = "Temp.H/L";
            break;
        case E_IO_TEMP_HIGH:
            $sdir = "Temp.H";
            break;
        case E_IO_TEMP_LOW:
            $sdir = "Temp.L";
            break;
        case E_IO_TEMP_MONITOR:
            $sdir = "T.Mon";
            break;
        case E_IO_VOLT_HIGHLOW:
            $sdir = "Volt.H/L";
            break;
        case E_IO_VOLT_HIGH:
            $sdir = "Volt.H";
            break;
        case E_IO_VOLT_LOW:
            $sdir = "Volt.L";
            break;
        case E_IO_VOLT_MONITOR:
            $sdir = "Volt.Mon";
            break;
        case E_IO_LEVEL_MONITOR:
            $sdir = "Lvl.Mon";
            break;
        case E_IO_LEVEL_HIGH:
            $sdir = "Lvl.H";
            break;
        case E_IO_LEVEL_LOW:
            $sdir = "Lvl.L";
            break;
        case E_IO_LEVEL_HIGHLOW:
            $sdir = "Lvl.H/L";
            break;
        case E_IO_ROTENC_MONITOR:
            $sdir = "RE.Mon";
            break;
        case E_IO_ROTENC_HIGH:
            $sdir = "RE.H";
            break;
        case E_IO_ROTENC_LOW:
            $sdir = "RE.L";
            break;
        case E_IO_ROTENC_HIGHLOW:
            $sdir = "RE.H/L";
            break;
        case E_IO_CURRENT_MONITOR:
            $sdir = "Amps.Mon";
            break;
        case E_IO_CURRENT_HIGH:
            $sdir = "Amps.H";
            break;
        case E_IO_CURRENT_LOW:
            $sdir = "Amps.L";
            break;
        case E_IO_CURRENT_HIGHLOW:
            $sdir = "Amps.H/L";
            break;
        case E_IO_FREQ_MONITOR:
            $sdir = "Freq.Mon";
            break;
        case E_IO_FREQ_HIGH:
            $sdir = "Freq.H";
            break;
        case E_IO_FREQ_LOW:
            $sdir = "Freq.L";
            break;
        case E_IO_FREQ_HIGHLOW:
            $sdir = "Freq.H/L";
            break;
        case E_IO_PWRFACT_MONITOR:
            $sdir = "PwrFact.H/L";
            break;
        case E_IO_POWER_MONITOR:
            $sdir = "Pwr.Mon";
            break;
        case E_IO_POWER_HIGH:
            $sdir = "Pwr.H";
            break;
        case E_IO_POWER_LOW:
            $sdir = "Pwr.L";
            break;
        case E_IO_POWER_HIGHLOW:
            $sdir = "Pwr.H/L";
            break;
        case E_IO_ON_OFF_INV:
            $sdir = "On/Off Inverted";
            break;
        case E_IO_TORQUE_MONITOR:
            $sdir = "Torque.Mon";
            break;
        case E_IO_TORQUE_HIGH:
            $sdir = "Torque.H";
            break;
        case E_IO_TORQUE_LOW:
            $sdir = "Torque.L";
            break;
        case E_IO_TORQUE_HIGHLOW:
            $sdir = "Torque.H/L";
            break;
        case E_IO_RPMSPEED_MONITOR:
            $sdir = "Rpm.Mon";
            break;
        case E_IO_RPMSPEED_HIGH:
            $sdir = "Rpm.H";
            break;
        case E_IO_RPMSPEED_LOW:
            $sdir = "Rpm.L";
            break;
        case E_IO_RPMSPEED_HIGHLOW:
            $sdir = "Rpm.H/L";
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
if (isset($_GET['DirFilter']))
    $di_array['dir_filter'] = intval($_GET['DirFilter']);
if (isset($_POST['di_DeviceInfoNo']))
    $di_array['di_DeviceInfoNo'] = $_POST['di_DeviceInfoNo'];
if (isset($_POST['di_DeviceNo']))
    $di_array['di_DeviceNo'] = $_POST['di_DeviceNo'];
if (isset($_POST['de_Address']))
    $di_array['de_Address'] = intval($_POST['de_Address']);
if (isset($_POST['de_Hostname']))
    $di_array['de_Hostname'] = $_POST['de_Hostnane'];
if (isset($_POST['di_IOChannel']))
    $di_array['di_IOChannel'] = intval($_POST['di_IOChannel']) - 1;
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
            
if (isset($_POST['addr_filter'])) 
{
    if (strlen($_POST['addr_filter']) == 0)
        $di_array['addr_filter'] = -1;
    else
        $di_array['addr_filter'] = intval($_POST['addr_filter']);
}
if (isset($_POST['dir_filter']))
{
    if (strlen($_POST['dir_filter']) == 0)
        $di_array['dir_filter'] = "";
    else
        $di_array['dir_filter'] = $_POST['dir_filter'];
}

if (isset($_GET['DeviceInfoNo'])) 
{
    if ( $_GET['DeviceInfoNo'] == 0 )
    {   // show blank detail page
        $new_deviceinfo = true;
    }
    else if (($line = $db->GetFields('deviceinfo', 'di_DeviceInfoNo', $di_array['di_DeviceInfoNo'], "di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,di_Weekdays,di_AnalogType,di_CalcFactor,di_Offset,di_MonitorPos,di_MonitorHi,di_MonitorLo,di_ValueRangeHi,di_ValueRangeLo")) !== false) { // success
        $di_array['di_DeviceNo'] = $line[0];
        $di_array['di_IOChannel'] = $line[1];
        $di_array['di_IOName'] = $line[2];
        $di_array['di_IOType'] = $line[3];
        $di_array['di_OnPeriod'] = $line[4];
        $di_array['di_StartTime'] = func_convert_time($line[5]);
        $di_array['di_Hysteresis'] = $line[6];
        $di_array['di_Weekdays'] = $line[7];
        $di_array['di_AnalogType'] = $line[8];
        $di_array['di_CalcFactor'] = $line[9];
        $di_array['di_Offset'] = $line[10];
        $di_array['di_MonitorPos1'] = substr($line[11], 0, 3);
        $di_array['di_MonitorPos2'] = substr($line[11], 3, 3);
        $di_array['di_MonitorPos3'] = substr($line[11], 6, 3);
        $di_array['di_MonitorPos4'] = substr($line[11], 9, 3);
        $di_array['di_MonitorPos5'] = substr($line[11], 12, 3);
        $di_array['di_MonitorHi'] = $line[12];
        $di_array['di_MonitorLo'] = $line[13];
        $di_array['di_ValueRangeHi'] = $line[14];
        $di_array['di_ValueRangeLo'] = $line[15];
        
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
    if ($db->DeleteDeviceInfo($info['di_DeviceInfoNo'], $info['di_DeviceNo'], $info['di_IOChannel'], $info['di_IOType'])) 
    {
        $addr_filter = $di_array['addr_filter'];
        $dir_filter = $di_array['dir_filter'];
        
        func_clear_di_array($di_array);
        
        $di_array['info_msg'] = sprintf("Device deleted");
        $di_array['addr_filter'] = $addr_filter;
        $di_array['dir_filter'] = $dir_filter;
    } 
    else 
    {
        $di_array['error_msg'] = sprintf("Failed to delete deviceinfo with DeviceInfoNo=%d (delete related IO Links and/or PLCStates first)", $info['di_DeviceInfoNo']);
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
        $device = $db->ReadDeviceWithAddress($di_array['de_Address']);
        if ($device !== false) 
        {
            $di_array['di_DeviceNo'] = $device['de_DeviceNo'];
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
        if ($db->UpdateDeviceInfoTable($di_array['di_DeviceInfoNo'], $di_array['di_DeviceNo'], $di_array['di_IOChannel'], $di_array['di_IOName'], $di_array['di_IOType'], $di_array['di_OnPeriod'], $stime, $di_array['di_Hysteresis'], $di_array['di_Weekdays'], $di_array['di_AnalogType'], $di_array['di_CalcFactor'], $di_array['di_Offset'], $monitor_pos, 
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
            $dir_filter = $di_array['dir_filter'];
            
            func_clear_di_array($di_array);
        
            if ( $new_deviceinfo )
                $di_array['info_msg'] = "New deviceinfo saved successfully.";
            else
                $di_array['info_msg'] = "Deviceinfo updated successfully.";
            $di_array['addr_filter'] = $addr_filter;
            $di_array['dir_filter'] = $dir_filter;
            $new_deviceinfo = false;
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
    $addr_filter = $di_array['addr_filter'];
    $dir_filter = $di_array['dir_filter'];
    
    func_clear_di_array($di_array);
    
    $di_array['addr_filter'] = $addr_filter;
    $di_array['dir_filter'] = $dir_filter;
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
		else if ( sel.value.indexOf("Level") >= 0 )
		{
			console.log( "ioTypeChange(): level" );
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

		<div class="col-sm-3">
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
			</div>
		</div>

		<div class="col-sm-2">
			<div class='input-group'>
			<select class='form-control custom-select' size='1' name='dir_filter' id='dir_filter'>
			<option></option>
			<?php
			printf( "<option %s>IN</option>", ($di_array['dir_filter'] == "IN" ? "selected" : "") );
			printf( "<option %s>OUT</option>", ($di_array['dir_filter'] == "OUT" ? "selected" : "") );
			?>
			</select> 
			&nbsp;
			<button type='submit' class='btn btn-outline-dark' name='Refresh' id='Refresh'>Filter</button>
			</div>
		</div>
	</div>
			
	<div id="deviceinfolist" class="collapse <?php ($new_deviceinfo || $di_array['di_DeviceInfoNo'] != 0 ? printf("") : printf("show"))?>">
			
    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-8 mb-2">

			<?php 
			if ( $di_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $di_array['error_msg'] );
		    else if ( $di_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $di_array['info_msg'] );
            ?>

            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN && count($di_list) > 8 )
            {
                printf( "<a href='?DeviceInfoNo=0&AddrFilter=%d&DirFIlter=%s'>Add New DeviceInfo</a></br>", $di_array['addr_filter'], $di_array['dir_filter'] );
            }
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
			    $dir = func_get_dir($info['di_IOType'], $sdir);
			    if ($di_array['addr_filter'] >= 0 && $info['de_Address'] != $di_array['addr_filter']) 
			    {
			        continue;
			    }
			    else if ( ($di_array['dir_filter'] == "IN" && $dir == 1) || ($di_array['dir_filter'] == "OUT" && $dir == 0) )
			    {
			        continue;
			    }
			    
			    printf( "<tr>" );
			    
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d&DirFilter=%s'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $di_array['dir_filter'], $info['de_Address']);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d&DirFilter=%s'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $di_array['dir_filter'], $info['di_IOChannel'] + 1);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d&DirFilter=%s'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $di_array['dir_filter'], $info['di_IOName']);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d&DirFilter=%s'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $di_array['dir_filter'], $sdir);
			    printf("<td align='left'><a href='?DeviceInfoNo=%d&AddrFilter=%d&DirFilter=%s'>%s</a></td>", $info['di_DeviceInfoNo'], $di_array['addr_filter'], $di_array['dir_filter'], func_get_on_period($info['di_OnPeriod']));
			    
			    printf( "</tr>" );
			}
            ?>
            </table>

            <?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
            {
                printf( "<p><a href='?DeviceInfoNo=0&AddrFilter=%d&DirFilter=%s'>Add New DeviceInfo</a></p>", $di_array['addr_filter'], $di_array['dir_filter'] );
            }
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
			
	        printf( "<div class='row'>" );
	        printf( "<div class='col-sm-3'>" );
	        printf( "<label for='de_Address'>Address: </label>" );
	        printf( "</div>" );
	        printf( "<div class='col-sm-4'>" );
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
		        
			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_IOChannel'>IO Channel: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		$tip = sprintf( "IO Channels 1-16.<br>VIPF device 6 channels are V/I/P/E/F/PF.<br>VSD device input channels are V/I/P/F/T, plus 1 output F." );
    		printf( "<input type='text' class='form-control' name='di_IOChannel' id='di_IOChannel' size='3' value='%s' data-bs-toggle='tooltip' data-bs-html=true title='%s'>", ($di_array['di_IOChannel'] === "" ? "" : $di_array['di_IOChannel'] + 1), $tip );
    		printf( "</div>" );
    		printf( "</div>" );

    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_IOName'>Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-4'>" );
    		printf( "<input type='text' class='form-control' name='di_IOName' id='di_IOName' size='20' value='%s'> ", $di_array['di_IOName'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_IOType'>IO Type: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
    		printf("<select class='form-control custom-select' size='1' name='di_IOType' id='di_IOType' onChange='ioTypeChange();'>");
    		printf("<option></option>");
    		$e_io = 0;
    		foreach ( $_SESSION['E_IOD'] as $e_iod )
    		{
    		    printf("<option %s>%s</option>", ($di_array['di_IOType'] == $e_io ? "selected" : ""), $e_iod );
    		    $e_io += 1;
    		}
    		printf("</select>");
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_StartTime'>Start Time: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='text' class='form-control' name='di_StartTime' id='di_StartTime' size='7' value='%s'> ", $di_array['di_StartTime'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_OnPeriod'>On Period: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='text' class='form-control' name='di_OnPeriod' id='di_OnPeriod' size='5' value='%s'> ", $di_array['di_OnPeriod'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
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
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_Hysteresis'>Hysteresis: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		$tip = sprintf( "For temperature in degrees C." );
    		printf( "<input type='text' class='form-control' name='di_Hysteresis' id='di_Hysteresis' size='5' value='%s' data-bs-toggle='tooltip' data-bs-html=true title='%s'>", $di_array['di_Hysteresis'], $tip );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_AnalogType'>Analog Type: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
    		printf("<select class='form-control custom-select' size='1' name='di_AnalogType' id='di_AnalogType'>");
    		printf("<option></option>");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "V" ? "selected" : ""), "V. Voltage");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "A" ? "selected" : ""), "A. Current");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "W" ? "selected" : ""), "W. Power");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "F" ? "selected" : ""), "F. Frequency");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "Q" ? "selected" : ""), "Q. Torque");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "R" ? "selected" : ""), "R. RPM Speed");
    		printf("<option %s>%s</option>", ($di_array['di_AnalogType'] == "T" ? "selected" : ""), "T. Temperature");
    		printf("</select>");
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		switch ($di_array['di_IOType']) 
    		{
   		    default:
   		        break;
   		    case E_IO_VOLT_HIGHLOW:
   		    case E_IO_VOLT_HIGH:
   		    case E_IO_VOLT_LOW:
   		    case E_IO_VOLT_MONITOR:
   		        if ($di_array['di_AnalogType'] == "V")
   		            printf("Latest Value: %.2fV %s", func_calc_voltage($current_value['ev_Value'], $di_array['di_AnalogType']), $current_value['ev_Timestamp']);
   		        else
   		            printf("Latest Value: %.2fA %s", func_calc_voltage($current_value['ev_Value'], $di_array['di_AnalogType']), $current_value['ev_Timestamp']);
   		        break;
    		}
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_CalcFactor'>Calc Factor: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		$tip = sprintf( "Analog Voltage devices only measure 0-10V directly, so if you are measuring 100V via a 10:1 divider the calc factor would be 10.0<br>" );
    		$tip .= sprintf( "For Level devices the Calc Factor is the max water level in mm.<br>" );
    		$tip .= sprintf( "For Rotary Encoders the Calc Factor is the distance in mm for one rotation." );
    		printf( "<input type='text' class='form-control' name='di_CalcFactor' id='di_CalcFactor' size='5' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", 
    		    $di_array['di_CalcFactor'], $tip );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_Offset'>Offset Voltage: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		$tip = sprintf( "For Analog Current the Offest is the 0 Amp voltage, e.g. 2.49V.<br>For Level devices the Offset is the sensor height above the max level." );
    		printf( "<input type='text' class='form-control' name='di_Offset' id='di_Offset' size='5' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", $di_array['di_Offset'], $tip );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_MonitorPos'>Monitor Position: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		$tip = sprintf( "Enter 3 characters:<br>1st: monitor number 1 or 2<br>2nd: F, L or R for full width, left or right<br>3rd: graph row number, 1 to 4" );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos1' id='di_MonitorPos1' size='3' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", 
    		    $di_array['di_MonitorPos1'], $tip );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos2' id='di_MonitorPos2' size='3' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", 
    		    $di_array['di_MonitorPos2'], $tip );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos3' id='di_MonitorPos3' size='3' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", 
    		    $di_array['di_MonitorPos3']. $tip );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos4' id='di_MonitorPos4' size='3' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", 
    		    $di_array['di_MonitorPos4'], $tip );
    		printf( "<input type='text' class='form-control' name='di_MonitorPos5' id='di_MonitorPos5' size='3' value='%s' data-bs-toggle='tooltip' data-bs-html='true' title='%s'> ", 
    		    $di_array['di_MonitorPos5'], $tip );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_MonitorLoHi'>Monitor Alert Range: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='text' class='form-control' name='di_MonitorLo' id='di_MonitorLo' size='3' value='%s'> ", $di_array['di_MonitorLo'] );
    		printf( "</div>" );
    		printf( "<div class='col-sm-1'>" );
    		printf( " to " );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='text' class='form-control' name='di_MonitorHi' id='di_MonitorHi' size='3' value='%s'> ", $di_array['di_MonitorHi'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='di_ValueRangeLoHi'>Graph Y Axis Range: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='text' class='form-control' name='di_ValueRangeLo' id='di_ValueRangeLo' size='3' value='%s'> ", $di_array['di_ValueRangeLo'] );
    		printf( "</div>" );
    		printf( "<div class='col-sm-1'>" );
    		printf( " to " );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='text' class='form-control' name='di_ValueRangeHi' id='di_ValueRangeHi' size='3' value='%s'> ", $di_array['di_ValueRangeHi'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row mb-2 mt-2'>" );
    		printf( "<p>" );
    		printf("<input type='hidden' class='form-control' name='di_DeviceInfoNo' value='%d'>", $di_array['di_DeviceInfoNo']);
    		printf("<input type='hidden' class='form-control' name='di_DeviceNo' value='%d'>", $di_array['di_DeviceNo']);
    		printf("<input type='hidden' class='form-control' name='addr_filter' value='%d'>", $di_array['addr_filter']);
    		printf("<input type='hidden' class='form-control' name='dir_filter' value='%d'>", $di_array['dir_filter']);
    		
    		printf("<button type='submit' class='btn btn-outline-dark' name='UpdateDeviceInfo' id='UpdateDeviceInfo' %s>Update</button>", ($di_array['di_DeviceInfoNo'] == 0 || func_disabled_non_user() != "" ? "disabled" : ""));
    		printf("&nbsp;&nbsp;");
    		printf("<button type='submit' class='btn btn-outline-dark' name='NewDeviceInfo' id='NewDeviceInfo' %s>New</button>", func_disabled_non_user() );
    		printf("&nbsp;&nbsp;");
    		$onclick = sprintf("return confirm(\"Are you sure you want to delete deviceinfo with channel %d ?\")", intval($di_array['di_IOChannel'])+1 );
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





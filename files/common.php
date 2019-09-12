<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Flat Cat IT Ltd
//	Author: Dave Clarke
//
// 	Nimrod - Neanderthal butler in the Doctor Who tale Ghost Light
//--------------------------------------------------------------------------------------


if ( !include_once( "site_config.php" ) )
{
	die("Configuration file 'files/site_config.php' not found !");
}

define( "WEBSITE_VERSION", "0.0.1" );

define( "SECURITY_LEVEL_NONE", 0 );
define( "SECURITY_LEVEL_GUEST", 1 );
define( "SECURITY_LEVEL_ADMIN", 9 );

define( "MAX_CAMERAS", 9 );
define( "CAMERA_USER", "camuser" );
define( "CAMERA_PWD", "passw0rd.39" );


define( "E_DT_UNUSED", 0 );
define( "E_DT_DIGITAL_IO", 1 );		// digital input and/or output
define( "E_DT_TEMPERATURE", 2 );	// temperature 
define( "E_DT_TIMER", 3 );			// timer 
define( "E_DT_VOLTAGE", 4 );		// voltage 
define( "E_DTD_UNUSED", "Unused" );
define( "E_DTD_DIGITAL_IO", "Digital IO" );
define( "E_DTD_TEMPERATURE", "Temperature" );	
define( "E_DTD_TIMER", "Timer" );
define( "E_DTD_VOLTAGE", "Voltage" );


define( "E_IO_UNUSED", 0 );
define( "E_IO_ON_OFF", 1 );			// 1: 	manual on off switch
define( "E_IO_ON_TIMER", 2 );		// 2: 	manual on, auto off by timer
define( "E_IO_TOGGLE", 3 );			// 3:	press on then press off
define( "E_IO_ON_OFF_TIMER", 4 );	// 4: 	manual on, auto off by timer or button
define( "E_IO_OUTPUT", 5 );			// 5:	output
define( "E_IO_TEMP_HIGH", 6 );		// 6:	temperature too high
define( "E_IO_TEMP_LOW", 7 );		// 7:	temperature too low
define( "E_IO_VOLT_HIGH", 8 );		// 8:	voltage too high
define( "E_IO_VOLT_LOW", 9 );		// 9:	voltage too low
define( "E_IO_TEMP_MONITOR", 10 );		// 10:	temperature monitor only
define( "E_IO_VOLT_MONITOR", 11 );		// 11:	voltage monitor only
define( "E_IO_VOLT_DAYNIGHT", 12 );		// 12:	voltage monitor day/night
define( "E_IOD_UNUSED", "Unused" );
define( "E_IOD_ON_OFF", "Manual On Off Switch" );
define( "E_IOD_ON_TIMER", "Manual On, Off by Timer" );
define( "E_IOD_TOGGLE", "Toggle On / Off" );
define( "E_IOD_ON_OFF_TIMER", "Manual On, Off by Timer/Button" );
define( "E_IOD_OUTPUT", "Output" );
define( "E_IOD_TEMP_HIGH", "Temperature Too High" );
define( "E_IOD_TEMP_LOW", "Temperature Too Low" );
define( "E_IOD_VOLT_HIGH", "Voltage Too High" );
define( "E_IOD_VOLT_LOW", "Voltage Too Low" );
define( "E_IOD_TEMP_MONITOR", "Temperature Monitor Only" );
define( "E_IOD_VOLT_MONITOR", "Voltage Monitor Only" );
define( "E_IOD_VOLT_DAYNIGHT", "Voltage Monitor Day/Night" );

define( "E_ET_CLICK", 0 );			// 0: single click
define( "E_ETD_CLICK", "Click" );
define( "E_ET_DBLCLICK", 1 );		// 1: double click
define( "E_ETD_DBLCLICK", "Dbl CLick" );
define( "E_ET_LONGPRESS", 2 );		// 2: long press
define( "E_ETD_LONGPRESS", "Long Press" );
define( "E_ET_TIMER", 3 );			// 3: timer
define( "E_ETD_TIMER", "Timer" );		
define( "E_ET_TEMPERATURE", 4 );	// 4: temperature
define( "E_ETD_TEMPERATURE", "Temperature" );		
define( "E_ET_DEVICE_NG", 5 );		// 5: device ng
define( "E_ETD_DEVICE_NG", "Device NG" );
define( "E_ET_DEVICE_OK", 6 );		// 6: device ok
define( "E_ETD_DEVICE_OK", "Device OK" );
define( "E_ET_VOLTAGE", 7 );		// 7: voltage
define( "E_ETD_VOLTAGE", "Voltage" );
define( "E_ET_STARTUP", 8 );		// 8: startup
define( "E_ETD_STARTUP", "Startup" );

define( "E_DS_ALIVE", 0 );			// 0:	alive
define( "E_DSD_ALIVE", "Alive" );	
define( "E_DS_DEAD", 1 );			// 1:	dead
define( "E_DSD_DEAD", "Dead" );
define( "E_DS_BURIED", 2 );			// 2:	buried
define( "E_DSD_BURIED", "Buried" );

define( "E_DN_NIGHT", 0 );			// 0:	night
define( "E_DND_NIGHT", "Night" );
define( "E_DN_DAWNDUSK", 1 );		// 1:	dawn/dusk
define( "E_DND_DAWNDUSK", "Night" );
define( "E_DN_OVERCAST", 2 );		// 2:	overcast	
define( "E_DND_OVERCASE", "Overcast" );
define( "E_DN_DAY", 3 );			// 3:	day
define( "E_DND_DAY", "Day" );



function func_session_init()
{
	// site admin settings
	if ( !isset($_SESSION['auth_level']) )
		$_SESSION['auth_level'] = "";
	if ( !isset($_SESSION['remote_addr']) )
		$_SESSION['remote_addr'] = "";
	if ( !isset($_SESSION['page_mode']) )
		$_SESSION['page_mode'] = "";

	if ( !isset($_SESSION['user_email']) )
		$_SESSION['user_email'] = "";
	if ( !isset($_SESSION['user_no']) )
		$_SESSION['user_no'] = "";
	if ( !isset($_SESSION['link_tests']) )
	{
		$_SESSION['link_tests'] = array();
		$_SESSION['link_tests'][] = "EQ";
		$_SESSION['link_tests'][] = "NE";
		$_SESSION['link_tests'][] = "LT";
		$_SESSION['link_tests'][] = "LE";
		$_SESSION['link_tests'][] = "GT";
		$_SESSION['link_tests'][] = "GE";
		$_SESSION['link_tests'][] = "EQ/";
		$_SESSION['link_tests'][] = "NE/";
	}
	if ( !isset($_SESSION['camera_list']) )
	{
		$_SESSION['camera_list'] = array();
        $_SESSION['camera_list'][] = array( 'addr'=>120, 'name'=>"Test", 'ptz'=>true, 'directory'=>"test/FI9821EP_00626E562835" );
        $_SESSION['camera_list'][] = array( 'addr'=>121, 'name'=>"Garage", 'ptz'=>false, 'directory'=>"garage/FI9853EP_00626E570130" );
        $_SESSION['camera_list'][] = array( 'addr'=>122, 'name'=>"Front Door", 'ptz'=>false, 'directory'=>"frontdoor/FI9853EP_00626E588A36" );
        $_SESSION['camera_list'][] = array( 'addr'=>123, 'name'=>"Car Port", 'ptz'=>false, 'directory'=>"carport/FI9853EP_00626E588A46" );
        $_SESSION['camera_list'][] = array( 'addr'=>124, 'name'=>"Back Yard", 'ptz'=>false, 'directory'=>"backyard/FI9853EP_00626E6174E5" );
        $_SESSION['camera_list'][] = array( 'addr'=>125, 'name'=>"Back Shed", 'ptz'=>false, 'directory'=>"backshed/FI9853EP_00626E617511" );
        $_SESSION['camera_list'][] = array( 'addr'=>126, 'name'=>"Front Yard", 'ptz'=>false, 'directory'=>"frontyard/FI9853EP_00626E588A1C" );
	}
	if ( !isset($_SESSION['show_camera_list']) )
	{
		$_SESSION['show_camera_list'] = array();
		for ( $i = 0; $i < MAX_CAMERAS; $i++ )
		{
			$_SESSION['show_camera_list'][] = 0;
		}	
	}
	if ( !isset($_SESSION['ShowCameraFile']) )
	{
		$_SESSION['ShowCameraFile'] = "";
	}
				
	date_default_timezone_set( 'Pacific/Auckland' );
}

function func_user_logout()
{
	// clear all the session variables
	$_SESSION = array();

	// delete the session cookie
	if ( isset($_COOKIE[session_name()]) )
	{
		setcookie( session_name(), '', time()-42000, '/' );
	}

	session_destroy();

	session_start();

	func_session_init();

	$_SESSION['auth_level'] = SECURITY_LEVEL_NONE;

	$_SESSION['user_name'] = "";
	$_SESSION['user_email'] = "";
	$_SESSION['user_no'] = "";
}


function ReportDBError($message, $db)
{
	printf( "<div class='error'>%s: %s</div>", $message, mysqli_error($db) );
}

// convert dd/mm/yyyy into dd/mm/yy
function func_MakeShortDate( $dd )
{
	return substr( $dd, 0, 6 ) . substr( $dd, 8, 2 );
}

function func_is_external_connection()
{
	if ( strncmp( $_SERVER['REMOTE_ADDR'], "192.168.1", 9 ) == 0 ) //|| strncmp( $_SERVER['REMOTE_ADDR'], "192.168.0", 9 ) == 0 )
	{
		return false;
	}
	
	return true;
}

// convert a db date ccyy-mm-dd into dd/mm/ccyy format
function func_convert_date_format( $date_str )
{
	$dd = "";

	$date_str = rtrim(ltrim($date_str));

	if ( substr( $date_str, 2, 1 ) == "/" )
	{	// convert to db format
		$expl = explode( "/", $date_str );
		$dd = sprintf( "%d-%02d-%02d", $expl[2], $expl[1], $expl[0] );
	}
	else if ( substr( $date_str, 4, 1 ) == "-" )
	{	// convert to display format
		$expl = explode( "-", $date_str );
		$dd = sprintf( "%02d/%02d/%d", $expl[2], $expl[1], $expl[0] );

		if ( $expl[2] == 0 )
		{	// date is 0000-00-00
			$dd = "";
		}
	}

	return $dd;
}

// date is in dd/mm/ccyy format
function func_is_date_valid( $d1 )
{
	$expl = explode( "/", $d1 );
	$d = $expl[0];
	$m = $expl[1];
	$y = $expl[2];
	if ( strlen($d1) != 10 )
	{
		$d = 0;
		$m = 0;
		$y = 0;
	}

	if ( checkdate( $m, $d, $y ) )
		return true;
	else
		return false;
}



class MySQLDB
{
	//database handle
	var $db_link;

	//returns true on success, false on error
	function Open($dbHost, $dbUserName, $dbPassword, $dbName)
	{
		$count = 3;
		
		while ( ($this->db_link = mysqli_connect($dbHost, $dbUserName, $dbPassword)) === false && $count > 0 )
		{	// retry, nimrod may be restarting
			$count -= 1;
			sleep( 2 );
		}
		
		if ( $this->db_link === false )
		{
			ReportDBError("Failed to connect to database on $dbHost with username $dbUserName", $this->db_link);
			
			return false;
		}

		if(mysqli_select_db($this->db_link, $dbName) === false)
		{
			ReportDBError("Failed to use $dbName database", $this->db_link);
			return false;
		}

		return true;
	}

	//returns true on success, false on error
	function Close()
	{
		if(mysqli_close($this->db_link) === false)
		{
			ReportDBError("Failed to close database", $this->db_link);
			return false;
		}

		return true;
	}

	//returns result on success, false on error.  Should call FreeQuery on the result.
	function RunQuery($sql)
	{
		$result = mysqli_query($this->db_link, $sql);
		if($result === false)
		{
			ReportDBError("Failed to run query '$sql'", $this->db_link);
			return false;
		}
		return $result;
	}

	//frees memory associated with a query
	function FreeQuery($result)
	{
		if(mysqli_free_result($result) === false)
			ReportDBError("Failed to free result memory", $this->db_link);
	}

	function GetDBError()
	{
		return mysqli_error();
	}

	function GetFields( $table, $name, $no, $fields )
	{
		$query = sprintf( "select %s from %s where %s='%s'", $fields, $table, $name, $no );
		$result = $this->RunQuery($query);
		if ( $line = mysqli_fetch_row($result) )
		{	// found
			$this->FreeQuery($result);
			return $line;
		}

		$this->FreeQuery($result);

		return false;
	}

	function GetDatabaseSize()
	{
		$mb = -1;
		
		$query = sprintf( "select round(sum(data_length + index_length) / 1024 / 1024,1) from information_schema.tables where table_schema='nimrod'" );
		$result = $this->RunQuery($query);
		if ( $line = mysqli_fetch_row($result) )
		{	// found
			$mb = $line[0];
			
			$this->FreeQuery($result);
		}
		
		return $mb;
	}
	

	//********************************************************
	//
	// device table
	//
	//********************************************************
	function GetDeviceCount()
	{
		$query = sprintf( "select count(*) from devices" );
		$result = $this->RunQuery($query);
		if ( $line = mysqli_fetch_row($result) )
		{
			$this->FreeQuery($result);

			return $line[0];
		}

		return false;
	}


	function UpdateDevicesTable( $de_no, $com_port, $addr, $num_inputs, $num_outputs, $type, $name, $hostname )
	{
		if ( $de_no == 0 )
		{	// insert
			$query = sprintf( "insert into devices (de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Hostname)
					values('%s',%d,%d,%d,%d,'%s','%s')",
					addslashes($com_port), $addr, $num_inputs, $num_outputs, $type, addslashes($name), addslashes($hostname) );
		}
		else
		{
			$query = sprintf( "update devices set de_ComPort='%s',de_Address=%d,de_NumInputs=%d,
					de_NumOutputs=%d,de_Type=%d,de_Name='%s',de_Hostname='%s' where de_DeviceNo=%d",
					addslashes($com_port), $addr, $num_inputs, $num_outputs, $type, addslashes($name), addslashes($hostname), $de_no );
		}
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) >= 0 )
		{	// success
			$this->TouchTriggerFile();
			return true;
		}

		return false;
	}

	function ReadDevicesTable()
	{
		$info = array();
		$query = sprintf( "select de_DeviceNo,de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Status,de_Hostname 
				from devices order by de_Address" ); 
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'de_DeviceNo'=>$line[0], 'de_ComPort'=>stripslashes($line[1]), 'de_Address'=>$line[2],
							'de_NumInputs'=>$line[3], 'de_NumOutputs'=>$line[4], 'de_Type'=>$line[5],
							'de_Name'=>stripslashes($line[6]), 'de_Status'=>$line[7], 'de_Hostname'=>$line[8] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function DeleteDevice( $de_no )
	{
		$ok = true;
		
		// TODO: check if we can delete the device
		if ( ($info = $this->ReadDeviceInfo( $de_no )) !== false )
			$ok = false;
		else if ( ($info = $this->ReadIOLinks( $de_no )) !== false )
			$ok = false;
		else if ( ($info = $this->ReadEventCount( $de_no )) !== false )
			$ok = false;
		
		if ( $ok )
		{
			$query = sprintf( "delete from devices where de_DeviceNo=%d limit 1", $de_no );
			$result = $this->RunQuery( $query );
			if ( mysqli_affected_rows($this->db_link) == 1 )
			{	// success
				return true;
			}
		}

		return false;
	}
	
	function ReadDeviceWithAddress( $addr )
	{
		$info = false;
		$query = sprintf( "select de_DeviceNo,de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Status,de_Hostname from devices 
				where de_Address=%d", $addr ); 
		
		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = array( 'de_DeviceNo'=>$line[0], 'de_ComPort'=>stripslashes($line[1]), 'de_Address'=>$line[2],
							'de_NumInputs'=>$line[3], 'de_NumOutputs'=>$line[4], 'de_Type'=>$line[5],
							'de_Name'=>$line[6], 'de_Status'=>$line[7], 'de_Hostname'=>$line[8] );
		}

		$this->FreeQuery($result);

		return $info;
	}
	
	function ReadDeviceWithName( $name )
	{
	    $info = false;
	    $query = sprintf( "select de_DeviceNo,de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Status,de_Hostname from devices
				where de_Name='%s'", $name );
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $info = array( 'de_DeviceNo'=>$line[0], 'de_ComPort'=>stripslashes($line[1]), 'de_Address'=>$line[2],
	            'de_NumInputs'=>$line[3], 'de_NumOutputs'=>$line[4], 'de_Type'=>$line[5],
	            'de_Name'=>$line[6], 'de_Status'=>$line[7], 'de_Hostname'=>$line[8] );
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	function UpdateDeviceStatus( $de_no, $status )
	{
		$query = sprintf( "update devices set de_Status=%d where de_DeviceNo=%d", $status, $de_no );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) >= 0 )
		{	// success
			return true;
		}

		return false;
	}
	
	//*******************************************
	//
	//	deviceinfo table
	//
	//*******************************************
	function GetIOAddresses( $in, $out )
	{
		$info = array();
		
		$ss = "";
		if ( $in )
			$ss = sprintf( "and di_IOType in (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", E_IO_ON_OFF, E_IO_ON_TIMER, E_IO_TOGGLE, E_IO_ON_OFF_TIMER, 
					E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_TEMP_MONITOR, E_IO_VOLT_MONITOR );
		else if ( $out )
			$ss = sprintf( "and di_IOType in (%d)", E_IO_OUTPUT );
		if ( $in && $out )
			$ss = sprintf( "and di_IOType in (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", E_IO_ON_OFF, E_IO_ON_TIMER, E_IO_TOGGLE, E_IO_ON_OFF_TIMER, E_IO_OUTPUT, 
					E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_TEMP_MONITOR, E_IO_VOLT_MONITOR );
		
		$query = sprintf( "select de_DeviceNo,de_Address,de_Hostname,di_IOChannel,di_IOName,di_IOType from
				deviceinfo,devices where di_DeviceNo=de_DeviceNo %s order by de_Hostname,de_Address,di_IOChannel",
				$ss );
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'de_DeviceNo'=>$line[0], 'de_Address'=>$line[1], 'de_Hostname'=>$line[2], 'di_IOChannel'=>$line[3], 
						'di_IOName'=>stripslashes($line[4]), 'di_IOType'=>$line[5] );
		}
		
		$this->FreeQuery($result);
		
		return $info;
	}
	
	function ReadDeviceInfoTable()
	{
		$info = array();
		$query = sprintf( "select di_DeviceInfoNo,di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,
				di_Hysteresis,di_Temperature,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset,de_Address,de_Hostname from 
				deviceinfo,devices where di_DeviceNo=de_DeviceNo order by di_DeviceNo,di_IOChannel,di_IOType" ); 
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'di_DeviceInfoNo'=>$line[0], 'di_DeviceNo'=>$line[1],
							'di_IOChannel'=>$line[2], 'di_IOName'=>$line[3], 'di_IOType'=>$line[4],
							'di_OnPeriod'=>$line[5], 'di_StartTime'=>$line[6], 'di_Hysteresis'=>$line[7], 
							'di_Temperature'=>$line[8], 'di_Weekdays'=>$line[9], 'di_AnalogType'=>$line[10], 
							'di_CalcFactor'=>$line[11], 'di_Voltage'=>$line[12], 'di_Offset'=>$line[13], 
							'de_Address'=>$line[14], 'de_Hostname'=>$line[15] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function ReadDeviceInfo( $di_no )
	{
		$info = false;
		$query = sprintf( "select di_DeviceInfoNo,di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,
				di_Temperature,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset from	deviceinfo where di_DeviceInfoNo=%d", $di_no );

		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = array( 'di_DeviceInfoNo'=>$line[0], 'di_DeviceNo'=>$line[1], 
							'di_IOChannel'=>$line[2], 'di_IOName'=>$line[3], 'di_IOType'=>$line[4], 
							'di_OnPeriod'=>$line[5], 'di_StartTime'=>$line[6], 'di_Hysteresis'=>$line[7],
							'di_Temperature'=>$line[8], 'di_Weekdays'=>$line[9], 'di_AnalogType'=>$line[10],
							'di_CalcFactor'=>$line[11], 'di_Voltage'=>$line[12], 'di_Offset'=>$line[13] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function ReadDeviceInfoDC( $de_no, $channel )
	{
		$info = false;
		$query = sprintf( "select di_DeviceInfoNo,di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,
				di_Hysteresis,di_Temperature,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset from 
				deviceinfo where di_DeviceNo=%d and di_IOChannel=%d", $de_no, $channel );

		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = array( 'di_DeviceInfoNo'=>$line[0], 'di_DeviceNo'=>$line[1],  
							'di_IOChannel'=>$line[2], 'di_IOName'=>$line[3], 'di_IOType'=>$line[4], 
							'di_OnPeriod'=>$line[5], 'di_StartTime'=>$line[6], 'di_Hysteresis'=>$line[7],
							'di_Temperature'=>$line[8], 'di_Weekdays'=>$line[9], 'di_AnalogType'=>$line[10],
							'di_CalcFactor'=>$line[11], 'di_Voltage'=>$line[12], 'di_Offset'=>$line[13] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function UpdateDeviceInfoTable( $di_no, $de_no, $channel, $name, $type, $period, $stime, $hyst, $temp, $days, $atype, $factor, $voltage, $offset )
	{
		if ( $di_no == 0 )
		{	// insert
			$query = sprintf( "insert into deviceinfo (di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,
					di_Hysteresis,di_Temperature,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset)
					values(%d,%d,'%s',%d,%d,%d,%d,%.1f,'%s','%s',%.3f,%.1f,%.3f)",
					$de_no, $channel, addslashes($name), $type, $period, $stime, $hyst, $temp, $days, $atype, $factor, $voltage, $offset );
		}
		else
		{
			$query = sprintf( "update deviceinfo set di_DeviceNo=%d,di_IOChannel=%d,di_IOName='%s',di_IOType=%d,
					di_OnPeriod=%d,di_StartTime=%d,di_Hysteresis=%d,di_Temperature=%.1f,di_Weekdays='%s',di_AnalogType='%s',
					di_CalcFactor=%.3f,di_Voltage=%.1f,di_Offset=%.3f  
					where di_DeviceInfoNo=%d",
					$de_no, $channel, addslashes($name), $type, $period, $stime, $hyst, $temp, $days, $atype, $factor, $voltage, $offset, 
					$di_no );
		}
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) >= 0 )
		{	// success
			$this->TouchTriggerFile();
			return true;
		}

		return false;
	}

	function DeleteDeviceInfo( $di_no, $de_no, $io_ch )
	{
		$ok = true;
	
		// check if we can delete the deviceinfo
		$query = sprintf( "select il_LinkNo from iolinks where (il_InDeviceNo=%d and il_InChannel=%d) or
				(il_OutDeviceNo=%d and il_OutChannel=%d) limit 1", $de_no, $io_ch, $de_no, $io_ch );
		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$ok = false;
		}
		$this->FreeQuery($result);
		
		if ( $ok )
		{
			$query = sprintf( "delete from deviceinfo where di_DeviceInfoNo=%d limit 1", $di_no );
			$result = $this->RunQuery( $query );
			if ( mysqli_affected_rows($this->db_link) == 1 )
			{	// success
				return true;
			}
		}
	
		return false;
	}
	

	//*******************************************
	//
	//	iolinks table
	//
	//*******************************************
	function ReadIOLinksTable()
	{
		$info = array();
		$query = sprintf( "select il_LinkNo,il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,
				il_LinkDeviceNo,il_LinkChannel,il_LinkTest,il_LinkValue from 
				iolinks order by il_OutDeviceNo,il_OutChannel" );
				 
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'il_LinkNo'=>$line[0], 'il_InDeviceNo'=>$line[1], 'il_InChannel'=>$line[2],
							'il_OutDeviceNo'=>$line[3], 'il_OutChannel'=>$line[4], 'il_EventType'=>$line[5],
							'il_OnPeriod'=>$line[6], 'il_LinkDeviceNo'=>$line[7], 'il_LinkChannel'=>$line[8],
							'il_LinkTest'=>$line[9], 'il_LinkValue'=>$line[10] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function ReadIOLinks( $de_no )
	{
		$info = false;
		$query = sprintf( "select il_LinkNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,
				il_LinkDeviceNo,il_LinkChannel,il_LinkTest,il_LinkValue from 
				iolinks where il_DeviceNo=%d", $de_no );

		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'il_LinkNo'=>$line[0], 'il_InDeviceNo'=>$line[1], 'il_InChannel'=>$line[2], 
							'il_OutDeviceNo'=>$line[3], 'il_OutChannel'=>$line[4], 'il_EventType'=>$line[5],
							'il_OnPeriod'=>$line[6], 'il_LinkDeviceNo'=>$line[7], 'il_LinkChannel'=>$line[8],
							'il_LinkTest'=>$line[9], 'il_LinkValue'=>$line[10] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function UpdateIOLinksTable( $il_no, $inde_no, $inch, $outde_no, $outch, $ev, $op, $link_deno, $link_ch, $link_test, $link_val )
	{
		if ( $il_no == 0 )
		{	// insert
			$query = sprintf( "insert into iolinks (il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,
					il_LinkDeviceNo,il_LinkChannel,il_LinkTest,il_LinkValue)
					values(%d,%d,%d,%d,%d,%d,%d,%d,'%s',%.1f)",
					$inde_no, $inch, $outde_no, $outch, $ev, $op, $link_deno, $link_ch, $link_test, $link_val );
		}
		else
		{
			$query = sprintf( "update iolinks set il_InDeviceNo=%d,il_InChannel=%d,il_OutDeviceNo=%d,il_OutChannel=%d,
					il_EventType=%d,il_OnPeriod=%d,il_LinkDeviceNo=%d,il_LinkChannel=%d,il_LinkTest='%s',il_LinkValue=%.1f
					where il_LinkNo=%d",
					$inde_no, $inch, $outde_no, $outch, $ev, $op, $link_deno, $link_ch, $link_test, $link_val, $il_no );
		}
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) >= 0 )
		{	// success
			$this->TouchTriggerFile();
			return true;
		}

		return false;
	}

	function DeleteIOLinks( $il_no )
	{
		$ok = true;
	
		// TODO: check if we can delete the iolink
	
		if ( $ok )
		{
			$query = sprintf( "delete from iolinks where il_LinkNo=%d limit 1", $il_no );
			$result = $this->RunQuery( $query );
			if ( mysqli_affected_rows($this->db_link) == 1 )
			{	// success
				return true;
			}
		}
	
		return false;
	}
	
	function RefactorIOLinksIn( $de_no, $ch )
	{
		$query = sprintf( "delete from iolinks where il_InDeviceNo=%d and il_InChannel=%d" );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) >= 0 )
		{	// success
			return true;
		}
		
		return false;
	}
	
	//*******************************************
	//
	//	events table
	//
	//*******************************************
	function ReadEventCount( $de_no )
	{
		$info = false;
		$query = sprintf( "select count(*) from events where ev_DeviceNo=%d", $de_no );

		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = $line[0];
		}

		$this->FreeQuery($result);

		return $info;
	}
	
	function GetTableRecordCount()
	{
	    $info = array();
	    $tables = array();
	    $tables[] = "devices";
	    $tables[] = "deviceinfo";
	    $tables[] = "iolinks";
	    $tables[] = "events";
	    
	    foreach ( $tables as $table )
	    {
	        //$query = sprintf( "select count(*) from %s", $table );
	        $query = sprintf( "select table_rows from information_schema.tables where table_name='%s' and table_schema='nimrod'", $table );
	        $result = $this->RunQuery( $query );
	        if ( $line = mysqli_fetch_row($result) )
	        {
	            $info[] = array( 'table'=>$table,  'count'=>$line[0] );
	        }
	        else
	        {
	            $info[] = array( 'table'=>$table, 'count'=>-1 ); 
	        }
	    }
	    
	    return $info;
	}
	
	function GetDeviceFailures( $de_no )
	{
		$info = array();
		$hours = 24;
		$query = sprintf( "select ev_EventNo,ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description 
				from events where ev_DeviceNo=%d and ev_EventType=%d and
				ev_Timestamp>=date_sub(now(), interval %d hour)
				order by ev_Timestamp desc", $de_no, E_ET_DEVICE_NG, $hours ); 
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'ev_EventNo'=>$line[0], 'ev_Timestamp'=>$line[1], 'ev_DeviceNo'=>$line[2],
						'ev_IOChannel'=>$line[3], 'ev_EventType'=>$line[4], 'ev_Value'=>$line[5], 
						'ev_Description'=>stripslashes($line[6]) );
		}

		$this->FreeQuery($result);
		
		return $info;
	}

	function GetLatestTemperatures( $hours, $datetime )
	{
		$info = array();
		$devices = array();

		// get list of temperature devices
		$query = sprintf( "select di_DeviceNo,di_IOChannel,di_IOName from deviceinfo where di_IOType in (%d,%d,%d)", E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_TEMP_MONITOR );
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$devices[] = array( 'di_DeviceNo'=>$line[0], 'di_IOChannel'=>$line[1], 'di_IOName'=>$line[2] );
		}
		
		$this->FreeQuery($result);
		
		foreach ( $devices as $dd )
		{
			$name = $dd['di_IOName'];
			$de_no = $dd['di_DeviceNo'];
			$ch = 0;
			$atype = "V";
			$tstamp = 0;
			$tinterval = 300;	// 5 minutes
			$list = array();
			$query = sprintf( "select ev_Value,ev_Timestamp,unix_timestamp(ev_Timestamp),di_IOName,di_DeviceNo,di_IOChannel,di_AnalogType from events,deviceinfo 
					where ev_DeviceNo=di_DeviceNo and ev_IOChannel=di_IOChannel and ev_EventType=%d and
					ev_DeviceNo=%d and ev_IOChannel=%d and
					ev_Timestamp>=date_sub(from_unixtime(%d), interval %d hour) and ev_Timestamp<=from_unixtime(%d)
					order by ev_Timestamp desc", 
					E_ET_TEMPERATURE, $dd['di_DeviceNo'], $dd['di_IOChannel'], $datetime, $hours, $datetime );
			$result = $this->RunQuery( $query );
			while ( $line = mysqli_fetch_row($result) )
			{
				$name = stripslashes($line[3]);
				$de_no = $line[4];
				$ch = $line[5];
				$atype = $line[6];
				
				if ( $tstamp == 0 || $tstamp - $tinterval >= $line[2] )
				{	// data point every 5 minutes
					$tstamp = $line[2];
					$list[] = array( 'ev_Value'=>$line[0], 'ev_Timestamp'=>$line[1], 'unixTimestamp'=>$line[2] );
				}
			}

			$this->FreeQuery($result);
			
			$info[] = array( 'di_IOName'=>$name, 'di_DeviceNo'=>$de_no, 'di_IOChannel'=>$ch, 'di_AnalogType'=>$atype, 'data'=>$list );
		}

		return $info;
	}
	
	function GetCurrentValue( $device_no, $channel )
	{
		$info = false;
		$query = sprintf( "select ev_Value,ev_Timestamp from events where ev_DeviceNo=%d and ev_IOChannel=%d order by ev_Timestamp desc limit 1",
			$device_no, $channel );
		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = array( 'ev_Value'=>$line[0], 'ev_Timestamp'=>$line[1] );
		}
		
		$this->FreeQuery($result);
		
		return $info;
	}
	
	function GetLatestVoltages( $hours, $datetime )
	{
		$info = array();
		$devices = array();

		// get list of voltage devices
		$query = sprintf( "select di_DeviceNo,di_IOChannel from deviceinfo where di_IOType in (%d,%d,%d,%d)", 
				E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_VOLT_MONITOR, E_IO_VOLT_DAYNIGHT );
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$devices[] = array( 'di_DeviceNo'=>$line[0], 'di_IOChannel'=>$line[1] );
		}
		
		$this->FreeQuery($result);
		
		foreach ( $devices as $dd )
		{
			$name = "?";
			$de_no = 0;
			$ch = 0;
			$atype = "V";
			$tstamp = 0;
			$tinterval = 300;	// 5 minutes
			if ( $hours == 3 )
			    $tinterval = 60; // 1 minute
			else if ( $hours == 1 )
			    $tinterval = 2;     // 10 seconds
			$list = array();
			$query = sprintf( "select ev_Value,ev_Timestamp,unix_timestamp(ev_Timestamp),di_IOName,di_DeviceNo,di_IOChannel,di_AnalogType from events,deviceinfo 
					where ev_DeviceNo=di_DeviceNo and ev_IOChannel=di_IOChannel and ev_EventType=%d and
					ev_DeviceNo=%d and ev_IOChannel=%d and
					ev_Timestamp>=date_sub(from_unixtime(%d), interval %d hour) and ev_Timestamp<=from_unixtime(%d)
					order by ev_Timestamp desc", 
					E_ET_VOLTAGE, $dd['di_DeviceNo'], $dd['di_IOChannel'], $datetime, $hours, $datetime );
			//printf( "<div>".$query."</div>" );
			$result = $this->RunQuery( $query );
			while ( $line = mysqli_fetch_row($result) )
			{
				$name = stripslashes($line[3]);
				$de_no = $line[4];
				$ch = $line[5];
				$atype = $line[6];
				
				if ( $tstamp == 0 || $tstamp - $tinterval >= $line[2] )
				{	// data point every 5 minutes
					$tstamp = $line[2];
					$list[] = array( 'ev_Value'=>$line[0], 'ev_Timestamp'=>$line[1], 'unixTimestamp'=>$line[2] );
				}
			}

			$this->FreeQuery($result);
			
			$info[] = array( 'di_IOName'=>$name, 'di_DeviceNo'=>$de_no, 'di_IOChannel'=>$ch, 'di_AnalogType'=>$atype, 'data'=>$list );
		}

		return $info;
	}
	
	function DeleteEventNo( $ev_no )
	{
		$query = sprintf( "delete from events where ev_EventNo=%d limit 1", $ev_no );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) == 1 )
		{	// success
			return true;
		}
		
		return false;
	}

	function DeleteAllEventNo( $ev_no )
	{
		$de_no = -1;
		$di_no = -1;
		
		$query = sprintf( "select ev_DeviceNo,ev_IOChannel from events where ev_EventNo=%d limit 1", $ev_no );
		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$de_no = $line[0];
			$di_no = $line[1];
		}
		
		if ( $de_no > 0 && $di_no >= 0 )
		{
			$query = sprintf( "delete from events where ev_DeviceNo=%d and ev_IOChannel=%d and ev_EventType=%d", $de_no, $di_no, E_ET_DEVICE_NG );
			$result = $this->RunQuery( $query );
			if ( mysqli_affected_rows($this->db_link) == 1 )
			{	// success
				return true;
			}
		}
		
		return false;
	}
	
	function CleanupEventsTable()
	{
		$query = sprintf( "DELETE FROM events WHERE ev_Timestamp < DATE_SUB(NOW(), INTERVAL 13 MONTH)" );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) > 0 )
		{
			$query = sprintf( "optimize table events" );
			$result = $this->RunQuery( $query );
				
			return true;
		}
		
		return false;
	}

	function TouchTriggerFile()
	{
//		touch( CONFIG_TRIGGER_FILE );

		$query = sprintf( "update events set ev_Timestamp=now() where ev_DeviceNo=-1" );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) < 1 )
		{	// failed
			$query = sprintf( "insert into events (ev_DeviceNo,ev_Timestamp,ev_Description) values(-1,now(),'Last config update')" );
			$result = $this->RunQuery( $query );
			if ( mysqli_affected_rows($this->db_link) < 1 )
			{	// failed
			
			}
		}
	}
}

// convert yyyy-mm-dd hh:mm:ss to hh:mm dd/mm
function func_convert_timestamp( $tt )
{
	$expl = explode( " ", $tt );
	if ( isset( $expl[0]) && isset($expl[1]) )
	{
		$expld = explode( "-", $expl[0] );
		$explt = explode( ":", $expl[1] );
		
		$out = sprintf( "%02d:%02d %d/%d", $explt[0], $explt[1], $expld[2], $expld[1] );
	}
	else 
	{
		$out = $tt;
	}
	
	return $out;
}

function func_calc_temperature( $cc )
{
	$temp = "?";
	if ( $cc == -1 )
	{	// not connected
		$temp = "N/A";
	}
	else if ( $cc < 10000 )
	{	// temp is > 0
		$temp = sprintf( "%.1f", ($cc / 10 ) );
	}	
	else 
	{	// temp is < 0
		$temp = sprintf( "%.1f", (($cc - 10000) / 10 ) );
	}
	
	return $temp;
}

function func_calc_voltage( $cc, $atype )
{
	$temp = "?";
	if ( $cc == -1 )
	{	// not connected
		$temp = "N/A";
	}
	else
	{	
		if ( $atype == "V" )
			$temp = sprintf( "%.1f", ($cc / 1000) );
		else if ( $cc != 0 )
			$temp = sprintf( "%.2f", ($cc / 1000) );
	}
	// TODO: handle negative voltages	
	
	return $temp;
}


function func_create_click_file( $de_no, $ch )
{
	$file = sprintf( "/tmp/NimrodClick-%d-%d", $de_no, $ch );
	
	touch( $file );
	chmod( $file, 0777 );
}

function func_get_device_type_desc( $dt )
{
	$desc = "?";
	
	switch ( $dt )
	{
		default:
		case E_DT_UNUSED:
			$desc = E_DTD_UNUSED;
			break;
		case E_DT_DIGITAL_IO:
			$desc = E_DTD_DIGITAL_IO;
			break;
		case E_DT_TEMPERATURE:
			$desc = E_DTD_TEMPERATURE;
			break;
		case E_DT_TIMER:
			$desc = E_DTD_TIMER;
			break;
		case E_DT_VOLTAGE:
			$desc = E_DTD_VOLTAGE;
			break;
	}
	
	return $desc;
}

function func_get_device_type( $desc )
{
	if ( $desc == E_DTD_DIGITAL_IO )
		return E_DT_DIGITAL_IO;
	else if ( $desc == E_DTD_TEMPERATURE )
		return E_DT_TEMPERATURE;
	else if ( $desc == E_DTD_TIMER )
		return E_DT_TIMER;
	else if ( $desc == E_DTD_VOLTAGE )
		return E_DT_VOLTAGE;
	
	return E_DT_UNUSED;
}

function func_get_io_type_desc( $io )
{
	$desc = "?";
	switch ( $io )
	{
		default:
		case E_IO_UNUSED:
			$desc = E_IOD_UNUSED;
			break;
		case E_IO_ON_OFF:
			$desc = E_IOD_ON_OFF;
			break;
		case E_IO_ON_TIMER:
			$desc = E_IOD_TIMER;
			break;
		case E_IO_TOGGLE:
			$desc = E_IOD_TOGGLE;
			break;
		case E_IO_ON_OFF_TIMER:
			$desc = E_IOD_ON_OFF_TIMER;
			break;
		case E_IO_OUTPUT:
			$desc = E_IOD_OUTPUT;
			break;
		case E_IO_TEMP_HIGH:
			$desc = E_IOD_TEMP_HIGH;
			break;
		case E_IO_TEMP_LOW:
			$desc = E_IOD_TEMP_LOW;
			break;
		case E_IO_VOLT_HIGH:
			$desc = E_IOD_VOLT_HIGH;
			break;
		case E_IO_VOLT_LOW:
			$desc = E_IOD_VOLT_LOW;
			break;
		case E_IO_TMEP_MONITOR:
			$desc = E_IOD_TEMP_MONITOR;
			break;
		case E_IO_VOLT_MONITOR:
			$desc = E_IOD_VOLT_MONITOR;
			break;
		case E_IO_VOLT_DAYNIGHT:
			$desc = E_IOD_VOLT_DAYNIGHT;
			break;
	}	
	
	return $desc;
}

function func_get_daynight_desc( $dn )
{
	$desc = "?";
	switch ( $dn )
	{
	default:
	case E_DN_NIGHT:
		$desc = E_DND_NIGHT;
		break;
	case E_DN_DAWNDUSK:
		$desc = E_DND_DAWNDUSK;
		break;
	case E_DN_OVERCAST:
		$desc = E_DND_OVERCAST;
		break;
	case E_DN_DAY:
		$desc = E_DND_DAY;
		break;
	}

	return $desc;
}
	
function func_get_io_type( $desc )
{
	if ( $desc == E_IOD_ON_OFF )
		return E_IO_ON_OFF;
	else if ( $desc == E_IOD_ON_TIMER )
		return E_IO_ON_TIMER;
	else if ( $desc == E_IOD_TOGGLE )
		return E_IO_TOGGLE;
	else if ( $desc == E_IOD_ON_OFF_TIMER )
		return E_IO_ON_OFF_TIMER;
	else if ( $desc == E_IOD_OUTPUT )
		return E_IO_OUTPUT;
	else if ( $desc == E_IOD_TEMP_HIGH )
		return E_IO_TEMP_HIGH;
	else if ( $desc == E_IOD_TEMP_LOW )
		return E_IO_TEMP_LOW;
	else if ( $desc == E_IOD_VOLT_HIGH )
		return E_IO_VOLT_HIGH;
	else if ( $desc == E_IOD_VOLT_LOW )
		return E_IO_VOLT_LOW;
	else if ( $desc == E_IOD_TEMP_MONITOR )
		return E_IO_TEMP_MONITOR;
	else if ( $desc == E_IOD_VOLT_MONITOR )
		return E_IO_VOLT_MONITOR;
	else if ( $desc == E_IOD_VOLT_DAYNIGHT )
		return E_IO_VOLT_DAYNIGHT;
	
	return 0;
}

function func_get_daynight( $desc )
{
	if ( $desc == E_DND_NIGHT )
		return E_DN_NIGHT;
	else if ( $desc == E_DND_DAWNDUSK )
		return E_DN_DAWNDUSK;
	else if ( $desc == E_DND_OVERCAST )
		return E_DN_OVERCAST;
	else if ( $desc == E_DND_DAY )
		return E_DN_DAY;
	
	return 0;
}
	
function func_get_eventtype_desc( $ev )
{
	$desc = E_ETD_CLICK;
	switch ( $ev )
	{
		default:
		case E_ET_CLICK:
			$desc = E_ETD_CLICK;
			break;
		case E_ET_DBLCLICK:
			$desc = E_ETD_DBLCLICK;
			break;
		case E_ET_LONGPRESS:
			$desc = E_ETD_LONGPRESS;
			break;
		case E_ET_TIMER:
			$desc = E_ETD_TIMER;
			break;
		case E_ET_TEMPERATURE:
			$desc = E_ETD_TEMPERATURE;
			break;
		case E_ET_DEVICE_NG:
			$desc = E_ETD_DEVICE_NG;
			break;
		case E_ET_DEVICE_OK:
			$desc = E_ETD_DEVICE_OK;
			break;
	}

	return $desc;
}

function func_get_eventtype( $desc )
{
	$ev = E_ET_CLICK;
	switch ( $desc )
	{
		default:
			$ev = "";
			break;
		case E_ETD_CLICK:
			$ev = E_ET_CLICK;
			break;
		case E_ETD_DBLCLICK:
			$ev = E_ET_DBLCLICK;
			break;
		case E_ETD_LONGPRESS:
			$ev = E_ET_LONGPRESS;
			break;
		case E_ETD_TIMER:
			$ev = E_ET_TIMER;
			break;
		case E_ETD_TEMPERATURE:
			$ev = E_ET_TEMPERATURE;
			break;
		case E_ETD_DEVICE_NG:
			$ev = E_ET_DEVICE_NG;
			break;
		case E_ETD_DEVICE_OK:
			$ev = E_ET_DEVICE_OK;
			break;
	}

	return $ev;
}

function func_get_device_status_desc( $st )
{
	$desc = "Unknown";
	switch ( $st )
	{
		default:
			break;
		case E_DS_ALIVE:
			$desc = E_DSD_ALIVE;
			break;
		case E_DS_DEAD:
			$desc = E_DSD_DEAD;
			break;
		case E_DS_BURIED:
			$desc = E_DSD_BURIED;
			break;
	}
	
	return $desc;
}

function func_get_device_status_img( $st )
{
	$img = "";
	switch ( $st )
	{
		default:
			$img = "./images/questionmark.png";
			break;
		case E_DS_ALIVE:
			$img = "./images/green_tick.png";
			break;
		case E_DS_DEAD:
			$img = "./images/dead_robot.png";
			break;
		case E_DS_BURIED:
			$img = "./images/buried_robot.png";
			break;
	}

	return $img;
}

function func_get_device_failures_img( $failures )
{
	$img = "";
	if ( count($failures) == 0 )
		$img = "./images/smile.png";
	else
		$img = "./images/warning.png";

	return $img;
}


// hh:mm xm to nnnn
// nnnn to hh:mm xm
function func_convert_time( $tt )
{
	$out = $tt;
	$expl = explode( ":", $tt );
	if ( substr($tt,2,1) == ':' || substr($tt,1,1) == ':' )
	{	// convert to nnnn
		$out = $expl[0] * 60 + intval($expl[1]);
		if ( intval($expl[0]) < 13 && (strtolower(substr($tt,5,1)) == 'p' || strtolower(substr($tt,6,1)) == 'p') )
			$out += 12*60;
	}
	else
	{	// convert to hh:mm xm
		$hh = intval($tt/60);
		$mm = intval($tt) - 60*$hh;
		$ampm = "am";
		if ( $hh > 12 )
		{
			$hh -= 12;
			$ampm = "pm";
		}
		$out = sprintf( "%02d:%02d %s", $hh, $mm, $ampm );
	}

	return $out;
}

// convert nnn to n.n s/m/h
function func_get_on_period( $sec )
{
	if ( $sec > 180 )
		return sprintf( "%.1f m", $sec / 60 );
	else if ( $sec > 5400 )
		return sprintf( "%.1f h", $sec / 3600 );
	else if ( $sec == 0 )
		return "";
	else
		return sprintf( "%d s", $sec );
}

// convert n.n s/m/h to number of seconds
function func_get_duration( $dd )
{
	$dur = 0;
	
	if ( strstr( strtolower($dd), "s" ) !== false )
	{	// seconds
		$dur = doubleval($dd);
	}
	else if ( strstr( strtolower($dd), "m" ) !== false )
	{	// minutes
		$dur = doubleval($dd) * 60;
	}
	else if ( strstr( strtolower($dd), "h" ) !== false )
	{	// hours
		$sep = "";
		if ( strstr( strtolower($dd), ":") !== false )
			$sep = ":";
		else if ( strstr( strtolower($dd), ".") !== false )
			$sep = ".";
		
		if ( $sep != "" )
		{
			$expl = explode( $sep, $dd );
			$dur = doubleval($expl[0]) * 3600 + doubleval($expl[1]) * 60;
		}
		else
		{
			$dur = doubleval($dd) * 3600;
		}
	}
	else
	{	// assume seconds
		$dur = doubleval($dd);
	}

	return $dur;
}
	
function func_get_build_number()
{
	$ver = "?";
	
	$fh = fopen( "version.txt", "rt" );
	if ( $fh != false )
	{
		$ver = fgets( $fh );
		
		fclose( $fh );
	}
	
	return $ver;
}

function func_read_camera_files( $camera_dir, $year, $month, $day )
{
	$info = array();
	
	$dir = sprintf( "/cctv/%s/record", $camera_dir );
	if ( $handle = opendir( $dir ) ) 
	{
		$mask = sprintf( "MDalarm_%4d%02d%02d", $year, $month, $day );
		while ( ($entry = readdir($handle)) !== false )
		{
			if ( strstr( $entry, $mask ) != false )
			{
				$info[] = $entry;
			}
		}
		
		closedir( $handle );
	}	
	else
	{
		$info[] = $dir;
	}
	
	sort( $info );
	
	return $info;
}

?>
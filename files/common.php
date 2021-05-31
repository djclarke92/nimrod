<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
// 	Nimrod - Neanderthal butler in the Doctor Who tale Ghost Light
//--------------------------------------------------------------------------------------


if ( !include_once( "site_config.php" ) )
{
	die("Configuration file 'files/site_config.php' not found !");
}

define( "THIS_DATABASE_VERSION", 110 );
define( "MAX_IO_PORTS", 16 );   // see mb_devices.h
define( "MAX_CONDITIONS", 10 ); // see mb_devices.h

define( "SECURITY_LEVEL_NONE", 0 );
define( "SECURITY_LEVEL_GUEST", 1 );
define( "SECURITY_LEVEL_USER", 2 );
define( "SECURITY_LEVEL_ADMIN", 9 );

// define user options access
define( "E_UF_CAMERAS", 0 );
define( "E_UFD_CAMERAS", "Camera Access" );
define( "E_UF_UPGRADE", 1 );
define( "E_UFD_UPGRADE", "Software Upgrade" );
define( "E_UF_HOMECAMERAS", 2 );
define( "E_UFD_HOMECAMERAS", "Show Cameras on Home Page" );

define( "MAX_CAMERAS", 9 );


define( "E_DT_UNUSED", 0 );
define( "E_DT_DIGITAL_IO", 1 );		// digital input and/or output
define( "E_DT_TEMPERATURE_DS", 2 );	// temperature, DS18B20 
define( "E_DT_TIMER", 3 );			// timer 
define( "E_DT_VOLTAGE", 4 );		// voltage 
define( "E_DT_TEMPERATURE_K1", 5 );	// temperature, PD3064 K thermocouple
define( "E_DT_LEVEL_K02", 6 );	     // level, K02 module
define( "E_DT_LEVEL_HDL", 7 );	     // level, HDL300 sensor
define( "E_DTD_UNUSED", "Unused" );
define( "E_DTD_DIGITAL_IO", "Digital IO" );
define( "E_DTD_TEMPERATURE_DS", "Temperature DS" );	
define( "E_DTD_TIMER", "Timer" );
define( "E_DTD_VOLTAGE", "Voltage" );
define( "E_DTD_TEMPERATURE_K1", "Temperature K" );
define( "E_DTD_LEVEL_K02", "Level K02" );
define( "E_DTD_LEVEL_HDL", "Level HDL" );


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
define( "E_IO_TEMP_HIGHLOW", 13 );		// 13:	temperature too high or too low
define( "E_IO_VOLT_HIGHLOW", 14 );		// 14:	temperature too high or too low
define( "E_IO_LEVEL_MONITOR", 15 );     // 15:  level measurement K02
define( "E_IO_LEVEL_HIGH", 16 );        // 16:  level measurement K02 too hig
define( "E_IO_LEVEL_LOW", 17 );         // 17:  level measurement K02 too low
define( "E_IO_LEVEL_HIGHLOW", 18 );     // 18:  level measurement K02 too high or too low
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
define( "E_IOD_TEMP_HIGHLOW", "Temperature Too High or Low" );
define( "E_IOD_VOLT_HIGHLOW", "Temperature Too High or Low" );
define( "E_IOD_LEVEL_MONITOR", "Level Monitor Only" );
define( "E_IOD_LEVEL_HIGH", "Level Too High" );
define( "E_IOD_LEVEL_LOW", "Level Too Low" );
define( "E_IOD_LEVEL_HIGHLOW", "Level Too High or Low" );

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
define( "E_ET_LEVEL", 9 );		    // 9: level
define( "E_ETD_LEVEL", "Level" );

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
	if ( !isset($_SESSION['us_AuthLevel']) )
		$_SESSION['us_AuthLevel'] = "";
	if ( !isset($_SESSION['remote_addr']) )
		$_SESSION['remote_addr'] = "";
	if ( !isset($_SESSION['page_mode']) )
		$_SESSION['page_mode'] = "";

	if ( !isset($_SESSION['us_Username']) )
		$_SESSION['us_Username'] = "";
	if ( !isset($_SESSION['us_Name']) )
		$_SESSION['us_Name'] = "";
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
		for ( $i = 1; $i <= MAX_CAMERAS; $i++ )
		{
		    $key = sprintf( "CAMERA_%02d", $i );
		    if ( defined($key) )
		    {
		        $expl = explode( ":", constant($key) );
                $_SESSION['camera_list'][] = array( 'addr'=>$expl[0], 'name'=>$expl[1], 'ptz'=>($expl[2] == "true" ? true : false), 'directory'=>$expl[3] );
		    }
		}
	}
	if ( !isset($_SESSION['show_camera_list']) )
	{
		$_SESSION['show_camera_list'] = array();
		for ( $i = 0; $i < MAX_CAMERAS; $i++ )
		{
			$_SESSION['show_camera_list'][] = "";
		}	
	}
	if ( !isset($_SESSION['ShowCameraFile']) )
	{
		$_SESSION['ShowCameraFile'] = "";
	}
	if ( !isset($_SESSION['MonitorPeriod']) )
	    $_SESSION['MonitorPeriod'] = 0.5;
	if ( !isset($_SESSION['MonitorRefreshPeriod']) )
	    $_SESSION['MonitorRefreshPeriod'] = 15;
    if ( !isset($_SESSION['MonitorRefreshEnabled']) )
        $_SESSION['MonitorRefreshEnabled'] = true;
	if ( !isset($_SESSION['MonitorDate']) )
	    $_SESSION['MonitorDate'] = "";
	if ( !isset($_SESSION['MonitorTime']) )
	    $_SESSION['MonitorTime'] = "";
	if ( !isset($_SESSION['monitor_audio']) )
	    $_SESSION['monitor_audio'] = true;
	
	date_default_timezone_set( 'Pacific/Auckland' );
}

// check if we need to add new tables or columns
function func_check_database( $db )
{
    $version = false;
    $query = sprintf( "select ev_Value from events where ev_DeviceNo=-2" );
    $result = $db->RunQuery( $query );
    if ( $line = mysqli_fetch_row($result) )
    {
        $version = $line[0];
    }
    $db->FreeQuery($result);
    
    if ( $version === false || $version < 100 )
    {   // we have some work to do
        
        // add us_Features column
        $query = sprintf( "alter table users add us_Features char(10) not null default 'NNNNNNNNNN'" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to add us_Features column", $db->db_link );
        }
        

        $version = func_update_database_version( $db, 100 );
    }
 
    if ( $version === false || $version < 107 )
    {   // we have some work to do
        
        // add conditions table
        $query = sprintf( "create TABLE IF NOT EXISTS `conditions` (
            	`co_ConditionNo` int(10) unsigned NOT NULL auto_increment,	# unique record number
            	`co_LinkNo` int(10) NOT NULL default '0',					# link to iolinks table
            	`co_LinkDeviceNo` int(10) NOT NULL default '0',
            	`co_LinkChannel` int(10) NOT NULL default '0',
            	`co_LinkTest` varchar(5) NOT NULL default ' ',				# LT, GT, LE, GE, EQ, NE
            	`co_LinkValue` decimal(6,1) NOT NULL default '0.0', 
            	PRIMARY KEY (`co_ConditionNo`),
            	KEY `co_linkno_index` (`co_LinkNo`) )" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to add conditions table", $db->db_link );
        }

        // drop il_LinkDeviceNo column
        $query = sprintf( "alter table iolinks drop il_LinkDeviceNo" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to drop il_LinkDeviceNo column", $db->db_link );
        }
        
        // drop il_LinkChannel column
        $query = sprintf( "alter table iolinks drop il_LinkChannel" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to drop il_LinkChannel column", $db->db_link );
        }
        
        // drop il_LinkTest column
        $query = sprintf( "alter table iolinks drop il_LinkTest" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to drop il_LinkTest column", $db->db_link );
        }
        
        // drop il_LinkValue column
        $query = sprintf( "alter table iolinks drop il_LinkValue" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to drop il_LinkValue column", $db->db_link );
        }
        
        
        $version = func_update_database_version( $db, 107 );
    }
    
    if ( $version === false || $version < 108 )
    {   // we have some work to do
    
        // update di_MonitorHi
        $query = sprintf( "update deviceinfo set di_MonitorHi=di_Voltage where di_IOType=%d", E_IO_VOLT_HIGH );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to update di_MonitorLo=di_Voltage column", $db->db_link );
        }
        
        // update di_MonitorHi
        $query = sprintf( "update deviceinfo set di_MonitorHi=di_Temperature where di_IOType=%d", E_IO_TEMP_HIGH );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to update di_MonitorHi=di_Temperature column", $db->db_link );
        }
        
        // update di_MonitorLo
        $query = sprintf( "update deviceinfo set di_MonitorLo=di_Voltage where di_IOType=%d", E_IO_VOLT_LOW );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to update di_MonitorLo=di_Voltage column", $db->db_link );
        }
        
        // update di_MonitorLo
        $query = sprintf( "update deviceinfo set di_MonitorLo=di_Temperature where di_IOType=%d", E_IO_TEMP_LOW );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to update di_MonitorLo=di_Temperature column", $db->db_link );
        }
        
        // drop di_Voltage column
        $query = sprintf( "alter table deviceinfo drop di_Voltage" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to drop di_Voltage column", $db->db_link );
        }
        
        // drop di_Temperature column
        $query = sprintf( "alter table deviceinfo drop di_Temperature" );
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to drop di_Temperature column", $db->db_link );
        }
        
        
        $version = func_update_database_version( $db, 108 );
    }
    
    if ( $version === false || $version < 109 )
    {   // we have some work to do
        // create the cameras table
        $query = "create table IF NOT EXISTS `cameras` (
            `ca_CameraNo` int(10) unsigned NOT NULL auto_increment,		# unique record number
            `ca_Name` varchar(50) NOT NULL default ' ',					# camera name
            `ca_IPAddress` varchar(15) NOT NULL default ' ',			# camera IPv4 address
            `ca_PTZ` char(1) NOT NULL default 'N',						# PTZ capable Y/N
            `ca_Encoding` varchar(10) NOT NULL default ' ',				# H.264 / H.265 encoding
            `ca_Directory` varchar(250) NOT NULL default ' ',			# file system directory from root (/)
        	`ca_UserId` varchar(20) NOT NULL default ' ',				# camera user id
        	`ca_Password` varchar(50) NOT NULL DEFAULT ' ',				# camera password (encrypted)
        	`ca_Model` varchar(20) NOT NULL default ' ',				# camera model, e.g. FI9853EP
            `ca_MJpeg` char(1) NOT NULL default 'N',					# supports MJpeg streaming Y/N/H (H means supports mjpeg over https)
            PRIMARY KEY (`ca_CameraNo`),
            KEY `ca_name_index` (`ca_Name`)
            )";
        
        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to create the cameras table", $db->db_link );
        }
        
        // add records
        foreach ( $_SESSION['camera_list'] as $camera )
        {
            $name = $camera['name'];
            $addr = $camera['addr'];
            $ptz = "N";
            if ( $camera['ptz'] )
                $ptz = "Y";
            $encoding = "H.264";
            $dir = sprintf( "/var%s/record", $camera['directory'] );
            $query = sprintf( "insert into cameras (ca_Name,ca_IPAddress,ca_PTZ,ca_Encoding,ca_Directory,ca_UserId,ca_Password) values('%s','%s','%s','%s','%s','%s','%s')",
                addslashes($name), $addr, $ptz, $encoding, $dir, CAMERA_USER, addslashes(base64_encode(CAMERA_PWD)) );
            $result = $db->RunQuery( $query );
            if ( func_db_warning_count($db) != 0 )
            {   // error
                ReportDBError(sprintf("Failed to add camera record for %s", $camera['addr']), $db->db_link );
            }
        }
        
        $version = func_update_database_version( $db, 109 );
    }

    if ( $version === false || $version < 110 )
    {   // we have some work to do
        // create the plcstates table
        $query = "create table IF NOT EXISTS `plcstates` (
        	`pl_StateNo` int(10) unsigned NOT NULL auto_increment,			# unique record number
        	`pl_Operation` varchar(50) NOT NULL default '',					# operation name, e.g. 'Setup Mode' or 'Run Mode'
        	`pl_StateName` varchar(50) NOT NULL default '',					# state name, e.g. 'WAITING' or 'MOVING_LEFT'
        	`pl_StateIsActive` char(1) NOT NULL default 'N',				# remember which state is active
        	`pl_StateTimestamp` timestamp NOT NULL default '0000-00-00',	# when did this state become active
        	`pl_RuleType` char(1) NOT NULL default '',						# I=Init, E=Event
        	`pl_DeviceNo` int(10) NOT NULL default 0,						# link to the deviceinfo table
        	`pl_IOChannel` int(10) NOT NULL default 0,						# link to the deviceinfo table
        	`pl_Value` int(10) NOT NULL default 0,							# data value
        	`pl_Test` varchar(5) NOT NULL default '',						# LT, GT, LE, GE, EQ, NE
        	`pl_NextStateName` varchar(50) NOT NULL default '',				# link to the next state
        	`pl_Order` int(10) NOT NULL default 0,							# optional execution order
            `pl_DelayTime` int(10) NOT NULL default 0,						# time delay in seconds to ignore the event
        	PRIMARY KEY (`pl_StateNo`),
        	KEY `pl_statename_index` (`pl_StateName`)
            )";

        $result = $db->RunQuery( $query );
        if ( func_db_warning_count($db) != 0 )
        {   // error
            ReportDBError("Failed to create the cameras table", $db->db_link );
        }
        
        $version = func_update_database_version( $db, 110 );
    }
}

function func_db_warning_count( $db )
{
    $count = -1;
    $query = sprintf( "select @@warning_count" );
    $result = $db->RunQuery( $query );
    if ( $line = mysqli_fetch_row($result) )
    {
        $count = $line[0];
    }
    $db->FreeQuery($result);
    
    return $count;
}
function func_update_database_version( $db, $ver )
{
    // update the database version
    $query = sprintf( "update events set ev_value=%d where ev_DeviceNo=-2", $ver );
    $result = $db->RunQuery( $query );
    if ( mysqli_affected_rows($db->db_link) <= 0 )
    {	// error
        $query = sprintf( "insert into events (ev_DeviceNo,ev_Value) values(-2,%d)", $ver );
        $result = $db->RunQuery( $query );
        if ( mysqli_affected_rows($db->db_link) < 0 )
        {	// error
            ReportDBError("Failed to update database version", $db->db_link );
        }
    }
    
    return $ver;
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

	$_SESSION['us_AuthLevel'] = SECURITY_LEVEL_NONE;

	$_SESSION['us_Name'] = "";
	$_SESSION['us_Username'] = "";
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
	if ( strncmp( $_SERVER['REMOTE_ADDR'], "192.168.", 8 ) == 0 || strncmp( $_SERVER['REMOTE_ADDR'], "127.0.0", 7 ) == 0 || $_SERVER['REMOTE_ADDR'] == "::1" )
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

	function DeleteDevice( $de_no, &$msg )
	{
		$ok = true;
		
		// TODO: check if we can delete the device
		if ( ($info = $this->ReadDeviceInfoByDevice( $de_no )) !== false )
		{
			$ok = false;
			$msg = "deviceinfo records exist";
		}
		else if ( ($info = $this->ReadIOLinks( $de_no )) !== false )
		{
			$ok = false;
			$msg = "iolinks records exist";
		}
		else if ( ($info = $this->ReadEventCount( $de_no )) > 0 )
		{
			$ok = false;
			$msg = "events records exist";
		}
		
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
			$ss = sprintf( "and di_IOType in (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", E_IO_ON_OFF, E_IO_ON_TIMER, E_IO_TOGGLE, E_IO_ON_OFF_TIMER, 
					 E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_TEMP_MONITOR, E_IO_VOLT_MONITOR,
			         E_IO_TEMP_HIGHLOW, E_IO_VOLT_HIGHLOW, E_IO_LEVEL_MONITOR, E_IO_LEVEL_HIGH, E_IO_LEVEL_LOW, E_IO_LEVEL_HIGHLOW );
		else if ( $out )
			$ss = sprintf( "and di_IOType in (%d)", E_IO_OUTPUT );
		
		if ( $in && $out )
			$ss = sprintf( "and di_IOType in (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", E_IO_ON_OFF, E_IO_ON_TIMER, E_IO_TOGGLE, E_IO_ON_OFF_TIMER, E_IO_OUTPUT, 
					 E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_TEMP_MONITOR, E_IO_VOLT_MONITOR,
			         E_IO_TEMP_HIGHLOW, E_IO_VOLT_HIGHLOW, E_IO_LEVEL_MONITOR, E_IO_LEVEL_HIGH, E_IO_LEVEL_LOW, E_IO_LEVEL_HIGHLOW );
			
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
				di_Hysteresis,di_OutOnStartTime,di_OutOnPeriod,di_Weekdays,di_AnalogType,di_CalcFactor,di_Offset,di_MonitorPos,
                di_MonitorHi,di_MonitorLo,di_ValueRangeHi,di_ValueRangeLo,
                de_Address,de_Hostname from 
				deviceinfo,devices where di_DeviceNo=de_DeviceNo order by de_Address,di_IOChannel" ); 
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'di_DeviceInfoNo'=>$line[0], 'di_DeviceNo'=>$line[1],
							'di_IOChannel'=>$line[2], 'di_IOName'=>stripslashes($line[3]), 'di_IOType'=>$line[4],
							'di_OnPeriod'=>$line[5], 'di_StartTime'=>$line[6], 'di_Hysteresis'=>$line[7], 
							'di_OutOnStartTime'=>$line[8], 'di_OutOnPeriod'=>[9], 
			                 'di_Weekdays'=>$line[10], 'di_AnalogType'=>$line[11], 
							'di_CalcFactor'=>$line[12], 'di_Offset'=>$line[13], 
							'di_MonitorPos'=>$line[14], 'di_MonitorHi'=>$line[15], 'di_MonitorLo'=>$line[16],
			                'di_ValueRangeHi'=>$line[17], 'di_ValueRangeLo'=>$line[18],
			                'de_Address'=>$line[19], 'de_Hostname'=>$line[20] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function ReadDeviceInfoByDevice( $de_no )
	{
	   $info = false;
	   $query = sprintf( "select di_DeviceInfoNo from deviceinfo where di_DeviceNo=%d", $de_no );
	   $result = $this->RunQuery( $query );
	   
	   if ( $line = mysqli_fetch_row($result) )
	   {
	       $info = array( 'di_DeviceInfoNo'=>$line[0] );
	   }
	   
	   $this->FreeQuery($result);
	   
	   return $info;
	}
	
	function ReadDeviceInfo( $di_no )
	{
		$info = false;
		$query = sprintf( "select di_DeviceInfoNo,di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,
				di_OutOnStartTime,di_OutOnPeriod,di_Weekdays,di_AnalogType,di_CalcFactor,di_Offset,di_MonitorPos,di_MonitorHi,di_MonitorLo,
                di_ValueRangeHi,di_ValueRangeLo from
                deviceinfo where di_DeviceInfoNo=%d", $di_no );

		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = array( 'di_DeviceInfoNo'=>$line[0], 'di_DeviceNo'=>$line[1], 
							'di_IOChannel'=>$line[2], 'di_IOName'=>$line[3], 'di_IOType'=>$line[4], 
							'di_OnPeriod'=>$line[5], 'di_StartTime'=>$line[6], 'di_Hysteresis'=>$line[7],
							'di_OutOnStartTime'=>$line[8], 'di_OutOnPeriod'=>$line[9], 
			                 'di_Weekdays'=>$line[10], 'di_AnalogType'=>$line[11],
							'di_CalcFactor'=>$line[12], 'di_Offset'=>$line[13],
			                'di_MonitorPos'=>$line[14], 'di_MonitorHi'=>$line[15], 'di_MonitorLo'=>$line[16],
			                'di_ValueRangeHi'=>$line[17], 'di_ValueRangeLo'=>$line[18] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function ReadDeviceInfoDC( $de_no, $channel )
	{
		$info = false;
		$query = sprintf( "select di_DeviceInfoNo,di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,
				di_Hysteresis,di_Weekdays,di_AnalogType,di_CalcFactor,di_Offset,di_MonitorPos,
                di_MonitorHi,di_MonitorLo,di_ValueRangeHi,di_ValueRangeLo from 
				deviceinfo where di_DeviceNo=%d and di_IOChannel=%d", $de_no, $channel );

		$result = $this->RunQuery( $query );
		if ( $line = mysqli_fetch_row($result) )
		{
			$info = array( 'di_DeviceInfoNo'=>$line[0], 'di_DeviceNo'=>$line[1],  
							'di_IOChannel'=>$line[2], 'di_IOName'=>$line[3], 'di_IOType'=>$line[4], 
							'di_OnPeriod'=>$line[5], 'di_StartTime'=>$line[6], 'di_Hysteresis'=>$line[7],
							'di_Weekdays'=>$line[8], 'di_AnalogType'=>$line[9],
							'di_CalcFactor'=>$line[10], 'di_Offset'=>$line[11],
			                'di_MonitorPos'=>$line[12], 'di_MonitorHi'=>$line[13], 'di_MonitorHi'=>$line[14],
			                'di_ValueRangeHi'=>$line[15], 'di_ValueRangeLo'=>$line[16] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function UpdateDeviceInfoTable( $di_no, $de_no, $channel, $name, $type, $period, $stime, $hyst, $days, $atype, $factor, $offset, $monitor_pos, $monitor_hi, $monitor_lo, $value_hi, $value_lo )
	{
		if ( $di_no == 0 )
		{	// insert
			$query = sprintf( "insert into deviceinfo (di_DeviceNo,di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,
					di_Hysteresis,di_Weekdays,di_AnalogType,di_CalcFactor,di_Offset,di_MonitorPos,
                    di_MonitorHi,di_MonitorLo,di_ValueRangeHi,di_ValueRangeLo)
					values(%d,%d,'%s',%d,%d,%d,%d,'%s','%s',%.3f,%.3f,'%s',%.1f,%.1f,'%s','%s')",
					$de_no, $channel, addslashes($name), $type, $period, $stime, $hyst, $days, $atype, $factor, $offset, $monitor_pos, $monitor_hi, $monitor_lo, $value_hi, $value_lo );
		}
		else
		{
			$query = sprintf( "update deviceinfo set di_DeviceNo=%d,di_IOChannel=%d,di_IOName='%s',di_IOType=%d,
					di_OnPeriod=%d,di_StartTime=%d,di_Hysteresis=%d,di_Weekdays='%s',di_AnalogType='%s',
					di_CalcFactor=%.3f,di_Offset=%.3f,di_MonitorPos='%s',di_MonitorHi=%.1f,di_MonitorLo=%.1f,
                    di_ValueRangeHi='%s',di_ValueRangeLo='%s'  
					where di_DeviceInfoNo=%d",
					$de_no, $channel, addslashes($name), $type, $period, $stime, $hyst, $days, $atype, $factor, $offset, $monitor_pos,
			        $monitor_hi, $monitor_lo, $value_hi, $value_lo,
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
	
	function GetMonitorDevices( $pos )
	{
	    $devices = array();
	    
	    $search = sprintf( "(%d,%d,%d,%d,%d,%d)", E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_TEMP_MONITOR, E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_VOLT_MONITOR );
	    $query = sprintf( "select di_DeviceNo,di_IOChannel,di_IOName from deviceinfo where di_IOType in %s and di_MonitorPos like '%%%s%%'",
	        $search, $pos );
	    
	    $result = $this->RunQuery( $query );
	    while ( $line = mysqli_fetch_row($result) )
	    {
	        $devices[] = array( 'di_DeviceNo'=>$line[0], 'di_IOChannel'=>$line[1], 'di_IOName'=>$line[2] );
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $devices;
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
				di_IOName,di_OutOnStartTime,di_OutOnPeriod from 
				iolinks,deviceinfo where il_OutDeviceNo=di_DeviceNo and il_OutChannel=di_IOChannel order by il_OutDeviceNo,il_OutChannel" );
				 
		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'il_LinkNo'=>$line[0], 'il_InDeviceNo'=>$line[1], 'il_InChannel'=>$line[2],
							'il_OutDeviceNo'=>$line[3], 'il_OutChannel'=>$line[4], 'il_EventType'=>$line[5],
							'il_OnPeriod'=>$line[6], 'di_IOName'=>$line[7],
			                 'di_OutOnStartTime'=>$line[8], 'di_OutOnPeriod'=>$line[9] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function ReadIOLinks( $de_no )
	{
		$info = false;
		$query = sprintf( "select il_LinkNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,
				from iolinks where il_InDeviceNo=%d", $de_no );

		$result = $this->RunQuery( $query );
		while ( $line = mysqli_fetch_row($result) )
		{
			$info[] = array( 'il_LinkNo'=>$line[0], 'il_InDeviceNo'=>$line[1], 'il_InChannel'=>$line[2], 
							'il_OutDeviceNo'=>$line[3], 'il_OutChannel'=>$line[4], 'il_EventType'=>$line[5],
							'il_OnPeriod'=>$line[6] );
		}

		$this->FreeQuery($result);

		return $info;
	}

	function UpdateIOLinksTable( $il_no, $inde_no, $inch, $outde_no, $outch, $ev, $op )
	{
		if ( $il_no == 0 )
		{	// insert
			$query = sprintf( "insert into iolinks (il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod)
					values(%d,%d,%d,%d,%d,%d)",
					$inde_no, $inch, $outde_no, $outch, $ev, $op );
		}
		else
		{
			$query = sprintf( "update iolinks set il_InDeviceNo=%d,il_InChannel=%d,il_OutDeviceNo=%d,il_OutChannel=%d,
					il_EventType=%d,il_OnPeriod=%d where il_LinkNo=%d",
					$inde_no, $inch, $outde_no, $outch, $ev, $op, $il_no );
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
		$query = sprintf( "delete from iolinks where il_InDeviceNo=%d and il_InChannel=%d", $de_no, $ch );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) >= 0 )
		{	// success
			return true;
		}
		
		return false;
	}

	//*******************************************
	//
	//	conditions table
	//
	//*******************************************
	function ReadConditionsTable( $link_no )
	{
	    $info = array();
	    $query = sprintf( "select co_ConditionNo,co_LinkNo,co_LinkDeviceNo,co_LinkChannel,co_LinkTest,co_LinkValue,di_IOName from
				conditions,deviceinfo where co_LinkNo=%d and co_LinkDeviceNo=di_DeviceNo and co_LinkChannel=di_IOChannel 
                order by co_LinkDeviceNo", $link_no );
	    
	    $result = $this->RunQuery( $query );
	    while ( $line = mysqli_fetch_row($result) )
	    {
	        $info[] = array( 'co_ConditionNo'=>$line[0], 'co_LinkNo'=>$line[1], 'co_LinkDeviceNo'=>$line[2], 
	            'co_LinkChannel'=>$line[3], 'co_LinkTest'=>$line[4], 'co_LinkValue'=>$line[5], 'di_IOName'=>stripslashes($line[6]) );
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	function ConditionExists( $link_no, $device_no, $channel_no )
	{
	    $exists = false;
	    $query = sprintf( "select co_ConditionNo from conditions where co_LinkNo=%d and co_LinkDeviceNo=%d and co_LinkChannel=%d",
	        $link_no, $device_no, $channel_no );
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $exists = true;
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $exists;
	}
	
	function UpdateConditionsTable( $co_no, $link_no, $link_deno, $link_ch, $link_test, $link_val )
	{
	    if ( $co_no == 0 )
	    {	// insert
	        $query = sprintf( "insert into conditions (co_LinkNo,co_LinkDeviceNo,co_LinkChannel,co_LinkTest,co_LinkValue)
					values(%d,%d,%d,'%s',%.1f)",
	            $link_no, $link_deno, $link_ch, $link_test, $link_val );
	    }
	    else
	    {
	        $query = sprintf( "update conditions set co_LinkNo=%d,co_LinkDeviceNo=%d,co_LinkChannel=%d,co_LinkTest='%s',co_LinkValue=%.1f
					where co_ConditionNo=%d",
	            $link_no, $link_deno, $link_ch, $link_test, $link_val, $co_no );
	    }
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) >= 0 )
	    {	// success
	        $this->TouchTriggerFile();
	        return true;
	    }
	    
	    return false;
	}

	function DeleteConditions( $co_no )
	{
	    $ok = true;
	    
	    // TODO: check if we can delete the condition
	    
	    if ( $ok )
	    {
	        $query = sprintf( "delete from conditions where co_ConditionNo=%d limit 1", $co_no );
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
	
	function SaveUserLoginAttempt( $user, $success )
	{
	    $query = sprintf( "insert into events (ev_DeviceNo,ev_Timestamp,ev_Description,ev_Value) values(-3,now(),'%s',%d)",
	        addslashes($user), $success );
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) < 1 )
	    {	// failed
	    }
	}
	
	function NotifyPlcStatesTableChange()
	{
	    $found = false;
	    $query = sprintf( "select ev_EventNo from events where ev_DeviceNo=-4" );
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $found = true;
	    }
	    
	    $this->FreeQuery($result);
	    
	    if ( $found )
    	    $query = sprintf( "update events set ev_Timestamp=now(),ev_Description='%s' where ev_DeviceNo=-4", "plcstates table changed" );
	    else
	        $query = sprintf( "insert into events (ev_DeviceNo,ev_Timestamp,ev_Description) values(-4,now(),'%s')", "plcstates table changed" );

        $result = $this->RunQuery( $query );
        //echo $query;
        if ( mysqli_affected_rows($this->db_link) < 1 )
        {  // failed
        }
	}
	
	function NotifyPlcStatesScreenButton( $state_no )
	{
	    $found = false;
	    $query = sprintf( "select ev_EventNo from events where ev_DeviceNo=-5" );
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $found = true;
	    }
	    
	    $this->FreeQuery($result);
	    
	    if ( $found )
	        $query = sprintf( "update events set ev_Value=%d,ev_Timestamp=now(),ev_Description='%s' where ev_DeviceNo=-5", $state_no, "plcstates screen button" );
        else
            $query = sprintf( "insert into events (ev_DeviceNo,ev_Value,ev_Timestamp,ev_Description) values(-5,%d,now(),'%s')", $state_no, "plcstates screen button" );
            
        $result = $this->RunQuery( $query );
        //echo $query;
        if ( mysqli_affected_rows($this->db_link) < 1 )
        {  // failed
        }            
	}
	
	function GetDeviceFailures( $de_no )
	{
		$info = array();
		$hours = 24;
		if ( $de_no == -3 )
		{ // login failures - 48 hours worth
		    $query = sprintf( "select ev_EventNo,ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description
				from events where ev_DeviceNo=%d and ev_Value=%d and
				ev_Timestamp>=date_sub(now(), interval %d hour)
				order by ev_Timestamp desc", $de_no, 0, $hours*2 );
		}
		else
		{
    		$query = sprintf( "select ev_EventNo,ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description 
				from events where ev_DeviceNo=%d and ev_EventType=%d and
				ev_Timestamp>=date_sub(now(), interval %d hour)
				order by ev_Timestamp desc", $de_no, E_ET_DEVICE_NG, $hours );
		}
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
	
	// TODO: validate the date and time format ?
	function ReadEventsTable( $sdate, $stime, $duration, $di_no )
	{
	    $info = array();
	    
	    $datetime = time();
	    $expl1 = explode( "/", $sdate );
	    $expl2 = explode( ":", $stime );
	    if ( isset($expl1[0]) && isset($expl1[1]) && isset($expl1[2]) && isset($expl2[0]) && isset($expl2[1]) )
	    {
	        $year = $expl1[2];
	        $mon = $expl1[1];
	        $day = $expl1[0];
	        $hour = $expl2[0];
	        $min = $expl2[1];
    	    $datetime = mktime( $hour, $min, 0, $mon, $day, $year );
	    }
	    
	    $seconds = func_get_duration( $duration );
	    if ( $di_no < 0 )
	    {
	        $query = sprintf( "select ev_EventNo,ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description
	   			from events where ev_Timestamp>=from_unixtime(%d) and ev_Timestamp<=from_unixtime(%d) and ev_DeviceNo=%d
		  		order by ev_Timestamp desc", $datetime, $datetime+$seconds, $di_no );
	        
	        $result = $this->RunQuery( $query );
	        while ( $line = mysqli_fetch_row($result) )
	        {
	            $info[] = array( 'ev_EventNo'=>$line[0], 'ev_Timestamp'=>$line[1], 'ev_DeviceNo'=>$line[2],
	                'ev_IOChannel'=>$line[3], 'ev_EventType'=>$line[4], 'ev_Value'=>$line[5],
	                'ev_Description'=>stripslashes($line[6]), 'di_IOName'=>"", 'di_AnalogType'=>"" );
	        }
	    }
	    else
	    {
    	    if ( $di_no > 0 )
    	    {
    	        $de_no = 0;
    	        $ch_no = 0;
        	    $query = sprintf( "select di_DeviceNo,di_IOChannel from deviceinfo where di_DeviceInfoNo=%d", $di_no );
        	    $result = $this->RunQuery( $query );
        	    if ( $line = mysqli_fetch_row($result) )
        	    {
        	       $de_no = $line[0];
        	       $ch_no = $line[1];
        	    }
        	    $this->FreeQuery($result);
        	    
        	    $query = sprintf( "select ev_EventNo,ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description,di_IOName,di_AnalogType
    				from events,deviceinfo where ev_Timestamp>=from_unixtime(%d) and ev_Timestamp<=from_unixtime(%d) and
                    ev_DeviceNo=%d and ev_IOChannel=%d and ev_DeviceNo=di_DeviceNo and ev_IOChannel=di_IOChannel
    				order by ev_Timestamp desc", $datetime, $datetime+$seconds, $de_no, $ch_no );
    	    }
    	    else
    	    {  // all devices
        	    $query = sprintf( "select ev_EventNo,ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description,di_IOName,di_AnalogType
    	   			from events,deviceinfo where ev_Timestamp>=from_unixtime(%d) and ev_Timestamp<=from_unixtime(%d) and
                    ev_DeviceNo=di_DeviceNo and ev_IOChannel=di_IOChannel
    		  		order by ev_Timestamp desc", $datetime, $datetime+$seconds );
    	    }
    	    $result = $this->RunQuery( $query );
    	    while ( $line = mysqli_fetch_row($result) )
    	    {
    	        $info[] = array( 'ev_EventNo'=>$line[0], 'ev_Timestamp'=>$line[1], 'ev_DeviceNo'=>$line[2],
    	            'ev_IOChannel'=>$line[3], 'ev_EventType'=>$line[4], 'ev_Value'=>$line[5],
    	            'ev_Description'=>stripslashes($line[6]), 'di_IOName'=>stripslashes($line[7]), 'di_AnalogType'=>$line[8] );
    	    }
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	
	function GetLatestData( $hours, $datetime, $search, $event_type )
	{
	    $info = array();
	    $devices = array();
	    
	    // get list of temperature devices
	    $query = sprintf( "select di_DeviceNo,di_IOChannel,di_IOName from deviceinfo where di_IOType in %s", $search );
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
	        $tt = 0.0;
	        $mon_hi = 0.0;
	        $mon_lo = 0.0;
	        $val_hi = "null";
	        $val_lo = "null";
	        $list = array();
	        $query = sprintf( "select ev_Value,ev_Timestamp,unix_timestamp(ev_Timestamp),di_IOName,di_DeviceNo,di_IOChannel,di_AnalogType,
                    di_MonitorHi,di_MonitorLo,di_ValueRangeHi,di_ValueRangeLo from events,deviceinfo
					where ev_DeviceNo=di_DeviceNo and ev_IOChannel=di_IOChannel and ev_EventType=%d and
					ev_DeviceNo=%d and ev_IOChannel=%d and
					ev_Timestamp>=date_sub(from_unixtime(%d), interval %d minute) and ev_Timestamp<=from_unixtime(%d)
					order by ev_Timestamp",
	            $event_type, $dd['di_DeviceNo'], $dd['di_IOChannel'], $datetime, 60*$hours, $datetime );
	        $result = $this->RunQuery( $query );
	        while ( $line = mysqli_fetch_row($result) )
	        {
	            $name = stripslashes($line[3]);
	            $de_no = $line[4];
	            $ch = $line[5];
	            $atype = $line[6];
	            $mon_hi = $line[7];
	            $mon_lo = $line[8];
	            $val_hi = $line[9];
	            $val_lo = $line[10];
	            
	            $list[] = array( 'ev_Value'=>$line[0], 'ev_Timestamp'=>$line[1], 'unixTimestamp'=>$line[2] );
	        }
	        
	        $this->FreeQuery($result);
	        
	        $info[] = array( 'di_IOName'=>$name, 'di_DeviceNo'=>$de_no, 'di_IOChannel'=>$ch, 'di_AnalogType'=>$atype,
	            'di_MonitorHi'=>$mon_hi, 'di_MonitorLo'=>$mon_lo, 'di_ValueRangeHi'=>$val_hi, 'di_ValueRangeLo'=>$val_lo, 'data'=>$list );
	    }
	    
	    return $info;
	}
	
	function GetLatestTemperatures( $hours, $datetime )
	{
	    $search = sprintf( "(%d,%d,%d,%d)", E_IO_TEMP_HIGH, E_IO_TEMP_LOW, E_IO_TEMP_MONITOR, E_IO_TEMP_HIGHLOW );
	    return $this->GetLatestData( $hours, $datetime, $search, E_ET_TEMPERATURE );
	}
	
	function GetLatestVoltages( $hours, $datetime )
	{
	    $search = sprintf( "(%d,%d,%d,%d)", E_IO_VOLT_HIGH, E_IO_VOLT_LOW, E_IO_VOLT_MONITOR, E_IO_VOLT_HIGHLOW );
	    return $this->GetLatestData( $hours, $datetime, $search, E_ET_VOLTAGE );
	}
	
	function GetLatestLevels( $hours, $datetime )
	{
	    $search = sprintf( "(%d,%d,%d,%d)", E_IO_LEVEL_HIGH, E_IO_LEVEL_LOW, E_IO_LEVEL_MONITOR, E_IO_LEVEL_HIGHLOW );
	    return $this->GetLatestData( $hours, $datetime, $search, E_ET_LEVEL );
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
		
		$query = sprintf( "select ev_DeviceNo,ev_IOChannel from events where ev_EventNo=%d", $ev_no );
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
		$query = sprintf( "update events set ev_Timestamp=now() where ev_DeviceNo=-1" );
		$result = $this->RunQuery( $query );
		if ( mysqli_affected_rows($this->db_link) < 1 )
		{	// failed - row does not exist
			$query = sprintf( "insert into events (ev_DeviceNo,ev_Timestamp,ev_Description) values(-1,now(),'Last config update')" );
			$result = $this->RunQuery( $query );
			if ( mysqli_affected_rows($this->db_link) < 1 )
			{	// failed
			
			}
		}
	}

	//*******************************************
	//
	//	users table
	//
	//*******************************************
	function ReadUsers()
	{
	    $info = array();
	    $query = sprintf( "Select us_Username,us_name,us_Password,us_AuthLevel,us_Features from users" );
	    $result = $this->RunQuery( $query );
	    while ( $line = mysqli_fetch_row($result) )
	    {
	        $info[] = array( 'us_Username'=>stripslashes($line[0]), 'us_Name'=>stripslashes($line[1]), 'us_Password'=>$line[2], 'us_AuthLevel'=>$line[3],
	            'us_Features'=>$line[4] );
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	function DeleteUser( $username )
	{
	    $count = 0;
	    $query = sprintf( "select count(*) from users" );
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $count = $line[0];
	    }
	    
	    $this->FreeQuery($result);
	    
	    // cannot delete the last user
	    if ( $count <= 1 )
	    {
	        return false;
	    }
	    
	    $query = sprintf( "DELETE FROM users WHERE us_Username='%s'", addslashes($username) );
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) == 1 )
	    {
	        return true;
	    }
	    
	    return false;
	}
	
	function UpdateUserTable( $newuser, $username, $name, $password, $auth_level, $features )
	{
	    $hash = hash( "sha256", $password, FALSE );
	    
	    if ( $newuser )
	    {
	        $query = sprintf( "insert into users (us_Username,us_Name,us_Password,us_AuthLevel,us_Features) values('%s','%s','%s',%d,'%s')",
	            addslashes($username), addslashes($name), $hash, $auth_level, $features );
	    }
	    else if ( $password != "" )
	    {
	        $query = sprintf( "update users set us_Name='%s',us_Password='%s',us_AuthLevel=%d,us_Features='%s' where us_Username='%s'",
	            $name, $hash, $auth_level, $features, $username );
	    }
	    else
	    {
	        $query = sprintf( "update users set us_Name='%s',us_AuthLevel=%d,us_Features='%s' where us_Username='%s'",
	            $name, $auth_level, $features, $username );
	    }
	    
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) >= 0 )
	    {	// success
	        return true;
	    }
	    
	    return false;
	}
	
	function SelectUser( $username )
	{
	    $info = false;
	    
        $query = sprintf( "select us_Username,us_Name,us_Password,us_AuthLevel,us_Features from users where us_Username='%s'", addslashes($username) );
        $result = $this->RunQuery( $query );
        if ( $line = mysqli_fetch_row($result) )
        {  
           $info = array( 'us_Username'=>stripslashes($line[0]), 'us_Name'=>stripslashes($line[1]), 'us_Password'=>$line[2], 'us_AuthLevel'=>$line[3],
               'us_Features'=>$line[4] ); 
        }
        
        $this->FreeQuery($result);
	    
	    return $info;
	}

	
	//*******************************************
	//
	// cameras table
	//
	//*******************************************
	function FindCameraByNameOrIPAddress( $name, $ipaddr )
	{
	    $rc = false;
	    $query = sprintf( "select ca_CameraNo from cameras where ca_Name='%s' or ca_IPAddress='%s'", $name, $ipaddr );    
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $rc = true;
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $rc;
	}
	   
	function ReadCameraList()
	{
	    $info = array();
	    
	    $query = sprintf( "select ca_CameraNo,ca_Name,ca_IPAddress,ca_PTZ,ca_Encoding,ca_Directory,ca_UserId,ca_Password,ca_Model,ca_MJpeg from cameras order by ca_Name" );
	    $result = $this->RunQuery( $query );
	    while ( $line = mysqli_fetch_row($result) )
	    {
	        $info[] = array( 'ca_CameraNo'=>$line[0], 'ca_Name'=>stripslashes($line[1]), 'ca_IPAddress'=>$line[2], 'ca_PTZ'=>$line[3], 'ca_Encoding'=>$line[4], 
	            'ca_Directory'=>$line[5], 'ca_UserId'=>stripslashes($line[6]), 'ca_Password'=>stripslashes(base64_decode($line[7])),
	            'ca_Model'=>stripslashes($line[8]), 'ca_MJpeg'=>$line[9] );
	    }
	
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	function SaveCameraRecord( $ca_no, $name, $ipaddr, $ptz, $encoding, $dir, $userid, $pwd, $model, $mjpeg )
	{
	   if ( $ca_no == 0 )
	   {   // insert
	       $query = sprintf( "insert into cameras (ca_Name,ca_IPAddress,ca_PTZ,ca_Encoding,ca_Directory,ca_UserId,ca_Password,ca_Model,ca_MJpeg) 
                values('%s','%s','%s','%s','%s','%s','%s','%s','%s')",
	           addslashes($name), $ipaddr, $ptz, $encoding, $dir, addslashes($userid), addslashes(base64_encode($pwd)), addslashes($model),
	           $mjpeg );
	   }
	   else
	   {   // update
	       if ( $pwd == "" )
	       {   // no password
    	       $query = sprintf( "update cameras set ca_Name='%s',ca_IPAddress='%s',ca_PTZ='%s',ca_Encoding='%s',ca_Directory='%s',ca_UserId='%s',
                    ca_Model='%s',ca_MJpeg='%s' where ca_CameraNo=%d",
    	           addslashes($name), $ipaddr, $ptz, $encoding, $dir, addslashes($userid), addslashes($model), $mjpeg,  
    	           $ca_no );
	       }
	       else
	       {
    	       $query = sprintf( "update cameras set ca_Name='%s',ca_IPAddress='%s',ca_PTZ='%s',ca_Encoding='%s',ca_Directory='%s',ca_UserId='%s',
                    ca_Password='%s',ca_Model='%s',ca_MJpeg='%s' where ca_CameraNo=%d",
    	           addslashes($name), $ipaddr, $ptz, $encoding, $dir, addslashes($userid), addslashes(base64_encode($pwd)), addslashes($model), $mjpeg,
    	           $ca_no );
	       }
	   }

	   $result = $this->RunQuery( $query );
	   if ( mysqli_affected_rows($this->db_link) >= 0 )
	   {	// success
	       return true;
	   }
	   
	   return false;
	}
	
	function DeleteCamera( $ca_no )
	{
	    $query = sprintf( "delete from cameras where ca_CameraNo=%d", $ca_no );
	
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) == 1 )
	    {
	        return true;
	    }
	    
	    return false;
	}
	
	
	
	//*******************************************
	//
	//	plcstates table
	//
	//*******************************************
	function ReadPlcStatesTable( $state_no, $op )
	{
	    $info = array();
	    
	    if ( $state_no == 0 )
	    {
	        if ( $op == "" )
	        {  // all operations
        	    $query = sprintf( "select pl_StateNo,pl_Operation,pl_StateName,pl_StateIsActive,pl_StateTimestamp,
        	        pl_RuleType,pl_DeviceNo,pl_IOChannel,pl_Value,pl_Test,pl_NextStateName,pl_Order,pl_DelayTime  
        	        from plcstates order by pl_Operation,pl_StateName,pl_RuleType,pl_Order,pl_NextStateName" );
	        }
	        else
	        {
	            $query = sprintf( "select pl_StateNo,pl_Operation,pl_StateName,pl_StateIsActive,pl_StateTimestamp,
        	        pl_RuleType,pl_DeviceNo,pl_IOChannel,pl_Value,pl_Test,pl_NextStateName,pl_Order,pl_DelayTime 
        	        from plcstates where pl_Operation='%s' order by pl_Operation,pl_StateName,pl_RuleType,pl_Order,pl_NextStateName",
	                addslashes($op) );
	        }
	    }
	    else
	    {
	        $query = sprintf( "select pl_StateNo,pl_Operation,pl_StateName,pl_StateIsActive,pl_StateTimestamp,
    	        pl_RuleType,pl_DeviceNo,pl_IOChannel,pl_Value,pl_Test,pl_NextStateName,pl_Order,pl_DelayTime
    	        from plcstates where pl_StateNo=%d", $state_no );
	    }
    	    
	    $result = $this->RunQuery( $query );
	    while ( $line = mysqli_fetch_row($result) )
	    {
	        $info[] = array( 'pl_StateNo'=>$line[0], 'pl_Operation'=>stripslashes($line[1]), 'pl_StateName'=>stripslashes($line[2]), 
	               'pl_StateIsActive'=>$line[3], 'pl_StateTimestamp'=>$line[4], 'pl_RuleType'=>$line[5], 'pl_DeviceNo'=>$line[6],
	               'pl_IOChannel'=>$line[7], 'pl_Value'=>$line[8], 'pl_Test'=>$line[9], 'pl_NextStateName'=>$line[10], 'pl_Order'=>$line[11],
	               'pl_DelayTime'=>$line[12] );
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	function DeletePlcStateRecord( $state_no )
	{
	    $query = sprintf( "delete from plcstates where pl_StateNo=%d", $state_no );
	    
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) == 1 )
	    {
	        return true;
	    }
	    
	    return false;
	}
	
	function SavePlcState( $state_no, $op, $state_name, $state_active, $state_timestamp, $rule_type, $device_no, $iochannel, $value, $test, $next_state_name, $order, $delay )
	{
        if ( $state_timestamp == "" )
            $state_timestamp = "0000-00-00";
	            
	    if ( $state_no == 0 )
	    {  // insert
	        $query = sprintf( "insert into plcstates (pl_Operation,pl_StateName,pl_StateIsActive,pl_StateTimestamp,pl_RuleType,
	               pl_DeviceNo,pl_IOChannel,pl_Value,pl_Test,pl_NextStateName,pl_Order,pl_DelayTime) values ('%s','%s','%s','%s','%s',%d,%d,%d,'%s','%s',%d,%d)",
	               addslashes($op), addslashes($state_name), $state_active, $state_timestamp, $rule_type,
	               $device_no, $iochannel, $value, $test, addslashes($next_state_name), $order, $delay_time );
	    }
	    else
	    {  // update
	        $query = sprintf( "update plcstates set pl_Operation='%s',pl_StateName='%s',pl_StateIsActive='%s',
	               pl_StateTimestamp='%s',pl_RuleType='%s',pl_DeviceNo=%d,pl_IOChannel=%d,pl_Value=%d,pl_Test='%s',pl_NextStateName='%s',pl_Order=%d,
                   pl_DelayTime=%d   
	               where pl_StateNo=%d",
	               addslashes($op), addslashes($state_name), $state_active, $state_timestamp, $rule_type, $device_no, $iochannel,
	               $value, $test, addslashes($next_state_name), $order, $delay,   
	               $state_no );	        
	    }
	    
	    //echo $query;
	    $result = $this->RunQuery( $query );
	    if ( mysqli_affected_rows($this->db_link) >= 0 )
	    {	// success
	        if ( $state_active == "Y" )
	        {  // make other states inactive
	            $this->PlcStateClearActive( $op, $state_no );
	        }
	        $this->NotifyPlcStatesTableChange();
	        
	        return true;
	    }
	    
	    
	    return false;
	}
	
	function PlcGetActiveStateName( $op )
	{
	    $info = false;
	    
	    $query = sprintf( "select pl_StateName from plcstates where pl_Operation='%s' and pl_StateIsActive='Y' and pl_RuleType=''", addslashes($op) );
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $info = $line[0];
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $info;
	}
	
	function PlcStateClearActive( $op, $state_no )
	{
	   $query = sprintf( "update plcstates set pl_StateIsActive='N' where pl_Operation='%s' and pl_StateNo!=%d", addslashes($op), $state_no );    
	   $result = $this->RunQuery( $query );
	   if ( mysqli_affected_rows($this->db_link) >= 0 )
	   {	// success
	       return true;
	   }
	   
	   return false;
	}
	
	function OperationStateNameExists( $op, $state_name )
	{
	    $rc = false;
	    
	    $query = sprintf( "select pl_StateNo from plcstates where pl_Operation='%s' and pl_StateName='%s' and pl_DeviceNo=0 && pl_IOChannel=0", $op, $state_name );    
	    $result = $this->RunQuery( $query );
	    if ( $line = mysqli_fetch_row($result) )
	    {
	        $rc = $line[0];
	    }
	   
	    $this->FreeQuery($result);
	    
	    return $rc;
	}
	
	function CountOperationStateName( $op, $state_name )
	{
	    $rc = 0;
	    
	    $query = sprintf( "select pl_StateNo from plcstates where pl_Operation='%s' and pl_StateName='%s'", $op, $state_name );
	    $result = $this->RunQuery( $query );
	    while ( $line = mysqli_fetch_row($result) )
	    {
	        $rc += 1;
	    }
	    
	    $this->FreeQuery($result);
	    
	    return $rc;
	}
	
	function SelectPlcOperations()
	{
	   $info = array();
	   
	   $query = sprintf( "select distinct pl_Operation from plcstates" );
	   $result = $this->RunQuery( $query );
	   while ( $line = mysqli_fetch_row($result) )
	   {
	       $info[] = array( 'pl_Operation'=>stripslashes($line[0]) );
	   }
	   
	   $this->FreeQuery($result);
	   
	   return $info;
	}

	
	
	
	
	//*******************************************
	//
	//	misc database functions
	//
	//*******************************************
	function GetTableRecordCount()
	{
	    $info = array();
	    $tables = array();
	    $tables[] = "devices";
	    $tables[] = "deviceinfo";
	    $tables[] = "iolinks";
	    $tables[] = "users";
	    $tables[] = "events";
	    
	    foreach ( $tables as $table )
	    {
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
	
}

// convert yyyy-mm-dd hh:mm:ss to hh:mm dd/mm
function func_convert_timestamp( $tt )
{
	$expl = explode( " ", $tt );
	if ( isset( $expl[0]) && isset($expl[1]) )
	{
		$expld = explode( "-", $expl[0] );
		$explt = explode( ":", $expl[1] );
		
		$out = sprintf( "%02d:%02d %02d/%02d", $explt[0], $explt[1], $expld[2], $expld[1] );
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
	    if ( $cc > 999 )
	      	$temp = sprintf( "%.0f", ($cc / 10 ) );
	    else
    		$temp = sprintf( "%.1f", ($cc / 10 ) );
	}	
	else 
	{	// temp is < 0
		$temp = sprintf( "-%.1f", (($cc - 10000) / 10 ) );
	}
	
	return $temp;
}

function func_calc_level( $cc )
{
    $temp = "?";
    if ( $cc == -1 )
    {	// not connected
        $temp = "N/A";
    }
    else
    {	// value is a percentage x 10: 0 - 1000 
        $temp = sprintf( "%.1f", $cc/10 );
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
		{
		    if ( $cc > 100000 )
    			$temp = sprintf( "%.0f", ($cc / 1000) );
		    else
    			$temp = sprintf( "%.1f", ($cc / 1000) );
		}
		else if ( $cc != 0 )
		{
			$temp = sprintf( "%.2f", ($cc / 1000) );
		}
	}
	// TODO: handle negative voltages	
	
	return $temp;
}

function func_create_click_file( $db, $de_no, $ch_no )
{
    $rc = true;
    $query = sprintf( "insert into events (ev_DeviceNo,ev_IOChannel,ev_Timestamp,ev_Description) values(%d,%d,now(),'Web click event')", -(1000 + $de_no), $ch_no );
    $result = $db->RunQuery( $query );
    if ( mysqli_affected_rows($db->db_link) < 1 )
    {	// failed
        $rc = false;
    }
    
	return $rc;
}

// 0123456789012345678901234567
// foscam
// MDAlarm_yyyymmdd_hhmmss.mkv
function func_get_date_from_video( $fname )
{
    $year = substr( $fname, 8, 4 );
    $month = substr( $fname, 12, 2 );
    $day = substr( $fname, 14, 2 );
    $hour = substr( $fname, 17, 2 );
    $min = substr( $fname, 19, 2 );
    $sec = substr( $fname, 21, 2 );

    return sprintf( "%02d/%02d/%d %02d:%02d:%02d", $day, $month, $year, $hour, $min, $sec );
}

function func_user_feature_enabled( $feature )
{
    if ( substr( $_SESSION['us_Features'], $feature, 1 ) == 'Y' )
        return true;
    else
        return false;
}

function func_get_user_feature_desc( $feature )
{
    $desc = "?";
    switch ( $feature )
    {
    default:
        break;
    case E_UF_CAMERAS:
        $desc = E_UFD_CAMERAS;
        break;
    case E_UF_UPGRADE:
        $desc = E_UFD_UPGRADE;
        break;
    case E_UF_HOMECAMERAS:
        $desc = E_UFD_HOMECAMERAS;
        break;
    }
    
    return $desc;
}

function func_get_security_level_desc( $sec )
{
    $desc = "?";
    switch ( $sec )
    {
    default:
        break;
    case SECURITY_LEVEL_NONE:
        $desc = "None";
        break;
    case SECURITY_LEVEL_GUEST:
        $desc = "Guest";
        break;
    case SECURITY_LEVEL_USER:
        $desc = "User";
        break;
    case SECURITY_LEVEL_ADMIN:
        $desc = "Admin";
        break;
    }
    
    return $desc;
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
	case E_DT_TEMPERATURE_DS:
		$desc = E_DTD_TEMPERATURE_DS;
		break;
	case E_DT_TIMER:
		$desc = E_DTD_TIMER;
		break;
	case E_DT_VOLTAGE:
		$desc = E_DTD_VOLTAGE;
		break;
	case E_DT_TEMPERATURE_K1:
	    $desc = E_DTD_TEMPERATURE_K1;
	    break;
	case E_DT_LEVEL_K02:
	    $desc = E_DTD_LEVEL_K02;
	    break;
	case E_DT_LEVEL_K02:
	    $desc = E_DTD_LEVEL_K02;
	    break;
	case E_DT_LEVEL_HDL:
	    $desc = E_DTD_LEVEL_HDL;
	    break;
	}
	
	return $desc;
}

function func_get_device_type( $desc )
{
	if ( $desc == E_DTD_DIGITAL_IO )
		return E_DT_DIGITAL_IO;
	else if ( $desc == E_DTD_TEMPERATURE_DS )
		return E_DT_TEMPERATURE_DS;
	else if ( $desc == E_DTD_TIMER )
		return E_DT_TIMER;
	else if ( $desc == E_DTD_VOLTAGE )
		return E_DT_VOLTAGE;
    else if ( $desc == E_DTD_TEMPERATURE_K1 )
	    return E_DT_TEMPERATURE_K1;
	else if ( $desc == E_DTD_LEVEL_K02 )
	    return E_DT_LEVEL_K02;
    else if ( $desc == E_DTD_LEVEL_HDL )
        return E_DT_LEVEL_HDL;
	        
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
		case E_IO_TEMP_HIGHLOW:
		    $desc = E_IOD_TEMP_HIGHLOW;
		    break;
		case E_IO_VOLT_HIGHLOW:
		    $desc = E_IOD_VOLT_HIGHLOW;
		    break;
		case E_IO_LEVEL_MONITOR:
		    $desc = E_IOD_LEVEL_MONITOR;
		    break;
		case E_IO_LEVEL_HIGH:
		    $desc = E_IOD_LEVEL_HIGH;
		    break;
		case E_IO_LEVEL_LOW:
		    $desc = E_IOD_LEVEL_LOW;
		    break;
		case E_IO_LEVEL_HIGHLOW:
		    $desc = E_IOD_LEVEL_HIGHLOW;
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
	else if ( $desc == E_IOD_TEMP_HIGHLOW )
	    return E_IO_TEMP_HIGHLOW;
	else if ( $desc == E_IOD_VOLT_HIGHLOW )
	    return E_IO_VOLT_HIGHLOW;
	else if ( $desc == E_IOD_LEVEL_MONITOR )
	    return E_IO_LEVEL_MONITOR;
	else if ( $desc == E_IOD_LEVEL_HIGH )
	    return E_IO_LEVEL_HIGH;
	else if ( $desc == E_IOD_LEVEL_LOW )
	    return E_IO_LEVEL_LOW;
	else if ( $desc == E_IOD_LEVEL_HIGHLOW )
	    return E_IO_LEVEL_HIGHLOW;
	                
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
		case E_ET_VOLTAGE:
		    $desc = E_ETD_VOLTAGE;
		    break;
		case E_ET_STARTUP:
		    $desc = E_ETD_STARTUP;
		    break;
		case E_ET_LEVEL:
		    $desc = E_ETD_LEVEL;
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
		case E_ETD_VOLTAGE:
		    $ev = E_ET_VOLTAGE;
		    break;
		case E_ETD_STARTUP:
		    $ev = E_ET_STARTUP;
		    break;
		case E_ETD_LEVEL:
		    $ev = E_ET_LEVEL;
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
	else if ( $tt != "" )
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
		{
		    $expl = explode( ":", $dd );
		    $dur = doubleval($expl[0]) * 3600 + doubleval($expl[1]) * 60;
		}
		else if ( strstr( strtolower($dd), ".") !== false )
		{
			$expl = explode( ".", $dd );
			$dur = doubleval($expl[0]) * 3600 + doubleval($expl[1]) * 30;
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

function func_get_package_number()
{
    $ver = "?";
    
    $fh = fopen( "package.txt", "rt" );
    if ( $fh != false )
    {
        $ver = fgets( $fh );
        
        fclose( $fh );
    }
    
    return $ver;
}

// /var/cctv/...
// /var/www/html/cctv
function func_make_camera_web_dir( $top, $camera_dir )
{
    return sprintf( "%s/%s", $top, substr( $camera_dir, 5 ) );
}

// file format is
// 012345678901234567890123456789
// MDalarm_yyyymmdd_hhmmss.mkv
function func_read_camera_files( $camera_dir, $year, $month, $day )
{
	$info = array();
	$info2 = array();
	
	//$mask = sprintf( "MDalarm_%4d%02d%02d", $year, $month, $day );
	$mask = sprintf( "%4d%02d%02d", $year, $month, $day );
	$dir = sprintf( "%s", $camera_dir );
	if ( is_dir($dir) )
	{
	   $list = scandir( $dir );
	   foreach( $list as $entry )
	   {
	       if ( strstr( $entry, $mask ) != false && strstr( $entry, "mp4" ) == false )
	       {
	           $info[] = array( 'file'=>$entry, 'dir'=>'' );
	       }
	   }
	   
       if ( count($info) == 0 && count($list) > 0 )
	   {   // funky date issue - camera time not being updated by ntp
	       foreach( $list as $entry )
	       {
	           if ( substr( $entry, 0, 8 ) == "MDalarm_" )
	           {
    	           $info2[] = array( 'file'=>$entry, 'dir'=>'' );
	           }
	       }
	       
	       // check the archive directory
	       $dir = sprintf( "%s/archive/", $camera_dir );
	       if ( is_dir($dir) )
	       {
	           $list = scandir( $dir );
	           foreach( $list as $entry )
	           {
	               if ( strstr( $entry, $mask ) != false && strstr( $entry, "mp4" ) == false )
	               {
	                   $info[] = array( 'file'=>$entry, 'dir'=>'archive/' );
	               }
	           }
	       }
	   
	       if ( count($info) == 0 && count($info2) > 0 )
	       {
	           $info = $info2;
	       }
       }
	}
	
	sort( $info );
	
	return $info;
}

function func_disabled_non_admin()
{
    if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
    {
        return "";
    }
    else
    {
        return "disabled";
    }
}

function func_disabled_non_user()
{
    if ( $_SESSION['us_AuthLevel'] >= SECURITY_LEVEL_USER )
    {
        return "";
    }
    else
    {
        return "disabled";
    }
}

//****************************************************************************
//
//  Graph Functions
//
//****************************************************************************
function func_get_graph_data( $temperatures, $voltages, $levels, $devices )
{
    $data = array();
    foreach ( $devices as $gg )
    {
        $atype = "V";
        $gname = "?";
        $gvoltage = false;
        
        $found = false;
        foreach ( $temperatures as $tt )
        {
            if ( $tt['di_DeviceNo'] == $gg['di_DeviceNo'] && $tt['di_IOChannel'] == $gg['di_IOChannel'] )
            {
                $found = true;
                $atype = "T";
                $myarray = $tt;
                $gname = $tt['di_IOName'];
                break;
            }
        }
        if ( $found == false )
        {
            $gvoltage = true;
            foreach ( $voltages as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['di_DeviceNo'] && $tt['di_IOChannel'] == $gg['di_IOChannel'] )
                {
                    $found = true;
                    $myarray = $tt;
                    $gname = $tt['di_IOName'];
                    $atype = $tt['di_AnalogType'];  // V or A
                    break;
                }
            }
        }
        if ( $found == false )
        {
            $gvoltage = false;
            foreach ( $levels as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['di_DeviceNo'] && $tt['di_IOChannel'] == $gg['di_IOChannel'] )
                {
                    $found = true;
                    $myarray = $tt;
                    $gname = $tt['di_IOName'];
                    $atype = "L";
                    break;
                }
            }
        }
        
        if ( $found )
        {
            $alert = false;
            if ( $myarray['di_MonitorLo'] != 0 && $myarray['di_MonitorHi'] != 0 )
            {   // check if the last value is in the alert state
                $val = $myarray['data'][count($myarray['data'])-1]['ev_Value'];
                if ( $atype == "T" && func_calc_temperature($val) < $myarray['di_MonitorLo'] || func_calc_temperature($val) > $myarray['di_MonitorHi'] )
                {
                    $alert = true;
                }
                else if ( $atype == "V" && func_calc_voltage($val,"V") < $myarray['di_MonitorLo'] || func_calc_voltage($val,"V") > $myarray['di_MonitorHi'] )
                {
                    $alert = true;
                }
                else if ( $atype == "L" && func_calc_level($val) < $myarray['di_MonitorLo'] || func_calc_level($val) > $myarray['di_MonitorHi'] )
                {
                    $alert = true;
                }
                else
                {
                    // TODO
                }
            }
            
            $data[] = array( 'data'=>$myarray['data'], 'name'=>$gname, 'voltage'=>$gvoltage, 'atype'=>$atype, 'alert'=>$alert, 'di_MonitorLo'=>$myarray['di_MonitorLo'], 'di_MonitorHi'=>$myarray['di_MonitorHi'],
                'di_ValueRangeLo'=>$myarray['di_ValueRangeLo'], 'di_ValueRangeHi'=>$myarray['di_ValueRangeHi'], 'SeqNo'=>0 );
            
        }
    }
    
    $cc = 0;
    foreach ( $data as &$dd )
    {
        $dd['SeqNo'] = $cc;
        
        $cc = $cc + 1;
    }
    
    return $data;
}

function func_create_graph( $gdata, $divname )
{
    printf( "<script type='text/javascript'>" );
    printf( "g = new Dygraph(" );
    
    // containing div
    printf( "document.getElementById('%s'),", $divname );
    
    $num_lines = count($gdata);
    
    $combined = array();
    foreach ( $gdata as $graph )
    {
        foreach ( $graph['data'] as $data )
        {
            if ( $graph['atype'] == "T" )
                $combined[] = array( 'ev_Timestamp'=>$data['ev_Timestamp'], 'value'=>func_calc_temperature($data['ev_Value']), 'SeqNo'=>$graph['SeqNo'] );
            else if ( $graph['atype'] == "V" )
                $combined[] = array( 'ev_Timestamp'=>$data['ev_Timestamp'], 'value'=>func_calc_voltage($data['ev_Value'],"V"), 'SeqNo'=>$graph['SeqNo'] );
            else if ( $graph['atype'] == "L" )
                $combined[] = array( 'ev_Timestamp'=>$data['ev_Timestamp'], 'value'=>func_calc_level($data['ev_Value']), 'SeqNo'=>$graph['SeqNo'] );
            else
                $combined[] = array( 'ev_Timestamp'=>$data['ev_Timestamp'], 'value'=>func_calc_voltage($data['ev_Value'],"A"), 'SeqNo'=>$graph['SeqNo'] );
        }
    }
    array_multisort( $combined );
    
    $combined2 = array();
    $cc = 0;
    while ( $cc < count($combined) )
    {
        $vv = array();
        for ( $i = 0; $i < $num_lines; $i++ )
        {
            $vv[$i] = "null";
        }
        $vv[$combined[$cc]['SeqNo']] = $combined[$cc]['value'];
        
        $ts = $combined[$cc]['ev_Timestamp'];
        $n = 1;
        while ( $cc+1 < count($combined) && isset($combined[$cc+$n]['ev_Timestamp']) && $ts == $combined[$cc+$n]['ev_Timestamp'] )
        {
            $cc = $cc + 1;
            $vv[$combined[$cc]['SeqNo']] = $combined[$cc]['value'];
            
            $n = $n + 1;
        }
        
        $combined2[] = array( 'ev_Timestamp'=>$ts, 'values'=>$vv );
        
        $cc = $cc + 1;
    }
    
    printf( "[" );
    $count = 0;
    foreach ( $combined2 as $data )
    {
        printf( "[new Date('%s')", $data['ev_Timestamp'] );
        
        foreach ( $data['values'] as $vv )
        {
            if ( $vv == "null" )
                printf( ",null" );
            else
                printf( ",%.1f", $vv );
        }
        
        printf( "]" );
        
        $count += 1;
        if ( $count < count($combined2) )
        {
            printf( "," );
        }
    }
    
    printf("],");
    
    printf( "{ labels: ['Date', '%s' ", $gdata[0]['name'] );
    $count = 1;
    while ( $count < $num_lines )
    {
        printf( ", '%s' ", $gdata[$count]['name'] );
        $count += 1;
    }
    printf( "], " );
    
    $valRangeLo = "";
    $valRangeHi = "";
    foreach ( $gdata as $gg )
    {
        if ( $gg['di_ValueRangeLo'] != "" )
        {
            if ( $valRangeLo == "" && $gg['di_ValueRangeLo'] == "null" )
            {
                $valRangeLo = "null";
            }
            else if ( floatval($valRangeLo) > floatval($gg['di_ValueRangeLo']) )
            {
                $valRangeLo = floatval($gg['di_ValueRangeLo']);
            }
            //else
            //{
            //    $valRangeLo = floatval($gg['di_ValueRangeLo']);
            //}
        }
        if ( $gg['di_ValueRangeHi'] != "" )
        {
            if ( $valRangeHi == "" && $gg['di_ValueRangeHi'] == "null" )
            {
                $valRangeHi = "null";
            }
            else if ( floatval($valRangeHi) < floatval($gg['di_ValueRangeHi']) )
            {
                $valRangeHi = floatval($gg['di_ValueRangeHi']);
            }
            //else
            //{
            //    $valRangeHi = floatval($gg['di_ValueRangeHi']);
            //}
        }
    }
    
    //printf( "  title: 'Temperature (C)', " );
    printf( "  legend: 'always', " );
    printf( "  connectSeparatedPoints: true, " );
    //printf( "  showRangeSelector: true, " );
    //printf( "  rangeSelectorPlotFillColor: 'MediumSlateBlue', " );
    //printf( "  rangeSelectorPlotFillGradientColor: 'rgba(123, 104, 238, 0)', " );
    //printf( "  colorValue: 0.9, " );
    //printf( "  fillAlpha: 0.4, " );
    if ( $valRangeLo != "" || $valRangeHi != "" )
    {
        if ( $valRangeLo == "null" && $valRangeHi == "null" )
            printf( "  valueRange: [%s,%s],", $valRangeLo, $valRangeHi );
        else if ( $valRangeLo == "null" )
            printf( "  valueRange: [%s,%.1f],", $valRangeLo, $valRangeHi );
        else
            printf( "  valueRange: [%.1f,%.1f],", $valRangeLo, $valRangeHi );
    }
    printf( "  drawPoints: true, " );
    printf( "  underlayCallback: function(canvas, area, g) {" );
    
    printf( "    canvas.fillStyle = 'rgba(255, 179, 179, 0.5)';" );
    printf( "    function highlight_period(x_start, x_end) {" );
    printf( "      var canvas_left_x = g.toDomXCoord(x_start);" );
    printf( "      var canvas_right_x = g.toDomXCoord(x_end);" );
    printf( "      var canvas_width = canvas_right_x - canvas_left_x;" );
    printf( "      canvas.fillRect(canvas_left_x, area.y, canvas_width, area.h);" );
    printf( "    }" );
    
    printf( "    var min_data_x;" );
    printf( "    var max_data_x;" );
    printf( "    var start_x_highlight;" );
    printf( "    var end_x_highlight;" );
    printf( "    var w;" );
    printf( "    var didx = 1;" );
    
    $count = 0;
    while ( $count < $num_lines )
    {
        
        printf( "    w = 0;" );
        printf( "    min_data_x = g.getValue(w,0);" );
        printf( "    while ( min_data_x == null ) {" );
        printf( "       w += 1;" );
        printf( "       min_data_x = g.getValue(w,0);" );
        printf( "    }" );
        
        printf( "    w = g.numRows()-1;" );
        printf( "    max_data_x = g.getValue(w,0);" );
        printf( "    while ( max_data_x == null ) {" );
        printf( "       w -= 1;" );
        printf( "       max_data_x = g.getValue(w,0);" );
        printf( "    }" );
        
        printf( "    w = 0;" );
        printf( "    start_x_highlight = 0;" );
        printf( "    end_x_highlight = 0;" );
        printf( "    if ( %f != 0.0 && %f != 0.0 ) {", $gdata[$count]['di_MonitorLo'], $gdata[$count]['di_MonitorHi'] );
        printf( "      while (w < g.numRows()) {" );
        
        printf( "        if ( g.getValue(w,didx) == null || g.getValue(w,0) == null ) {" );
        printf( "        } else if ( g.getValue(w,didx) < %f || g.getValue(w,didx) > %f ) {", $gdata[$count]['di_MonitorLo'], $gdata[$count]['di_MonitorHi'] );
        printf( "          /* value is out of range */" );
        printf( "          if ( start_x_highlight == 0 ) {" );
        printf( "            if ( w > 0 ) {" );
        printf( "              start_x_highlight = g.getValue(w-1,0);" );
        printf( "              if ( start_x_highlight == null )" );
        printf( "                start_x_highlight = g.getValue(w,0);" );
        printf( "            } else" );
        printf( "              start_x_highlight = g.getValue(w,0);" );
        printf( "            if ( start_x_highlight < min_data_x )" );
        printf( "              start_x_highlight = min_data_x;" );
        printf( "            end_x_highlight = start_x_highlight;" );
        printf( "          }" );
        printf( "        } else {" );
        printf( "          /* value is within range again */" );
        printf( "          end_x_highlight = g.getValue(w-1,0);" );
        printf( "          if ( start_x_highlight > 0 ) {" );
        printf( "            /* highlight the previous out of range block */" );
        printf( "            highlight_period(start_x_highlight,end_x_highlight);" );
        printf( "            start_x_highlight = 0;" );
        printf( "            end_x_highlight = 0;" );
        printf( "          }" );
        printf( "        }" );
        
        printf( "        if ( w+1 >= g.numRows() && start_x_highlight > 0 ) {" );
        printf( "          end_x_highlight = g.getValue(g.numRows()-1,0);" );
        printf( "          highlight_period(start_x_highlight,end_x_highlight);" );
        printf( "        }" );
        
        printf( "        w += 1;" );
        printf( "      }" );
        printf( "    }" );
        
        printf( "    didx = didx + 1;" );
        
        $count += 1;
    }
    
    printf( "  }" );    // end callback function
    
    printf( "}" );
    
    printf( ").resize();" );
    printf( "</script>" );
}

function func_draw_graph_div( $bs, $div_name, $graph_per_line, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $div_data )
{
    $graph_width = (100 / $graph_per_line) - ($alert_width / $graph_per_line);
    $al_width = $alert_width / $graph_per_line;
    $a_bgcolor = $graph_bgcolor;
    $a_name = "";
    $a_nameok = "";
    $a_info = "";

    if ( $bs )
    {
        printf( "<div class='row'>" );
        printf( "<div class='col-sm-10'>" );
        printf( "<div id='%s' style='height:300px; width:100%%; background-color:%s; position:relative;'></div>", $div_name, $graph_bgcolor );
    }
    else
    {
        printf( "<td width='%d%%'>", $graph_width );
        printf( "<div id='%s' style='height:100%%; width:100%%; background-color:%s; position:relative;'></div>", $div_name, $graph_bgcolor );
        printf( "</td>" );
    }
    
    if ( count($div_data) != 0 )
    {   // set alert color
        $hh = "h2";
        if ( count($div_data) > 2 )
            $hh = "h3";
        foreach ( $div_data as $dat )
        {
            $a_bgcolor = $alert_okcolor;
            $val = 0;
            if ( isset($dat['data'][count($dat['data'])-1]) )
                $val = $dat['data'][count($dat['data'])-1]['ev_Value'];
            if ( $dat['alert'] )
            {
                $a_bgcolor = $alert_ngcolor;
            }
            if ( $dat['atype'] == "T" )
            {
                $a_info .= sprintf( "<table width='100%%' height='%d%%' style='background-color: %s; text-align: center;'><tr><td><%s>%s<br><div class='small'><b>%s</b>&#8451</div></%s></td></tr></table>",
                    100/count($div_data), $a_bgcolor, $hh, $dat['name'], func_calc_temperature($val), $hh );
            }
            else if ( $dat['atype'] == "V" )
            {
                $a_info .= sprintf( "<table width='100%%' height='%d%%' style='background-color: %s; text-align: center;'><tr><td><%s>%s<br><div class='small'><b>%s</b>V</div></%s></td></tr></table>",
                    100/count($div_data), $a_bgcolor, $hh, $dat['name'], func_calc_voltage($val,"V"), $hh );
            }
            else if ( $dat['atype'] == "L" )
            {
                $a_info .= sprintf( "<table width='100%%' height='%d%%' style='background-color: %s; text-align: center;'><tr><td><%s>%s<br><div class='small'><b>%s</b>%%</div></%s></td></tr></table>",
                    100/count($div_data), $a_bgcolor, $hh, $dat['name'], func_calc_level($val), $hh );
            }
            else
            {
                $a_info .= sprintf( "<table width='100%%' height='%d%%' style='background-color: %s; text-align: center;'><tr><td><%s>%s<br><div class='small'><b>%s</b>A</div></%s></td></tr></table>",
                    100/count($div_data), $a_bgcolor, $hh, $dat['name'], func_calc_voltage($val,"A"), $hh );
            }
        }
    }
    else
    {
        $a_info = sprintf( "<table style='background-color:%s;' width='100%%' height='100%%'><tr><td>&nbsp;</td></tr></table>", $graph_bgcolor );
    }

    if ( $bs )
    {
        printf( "</div>" );
        printf( "<div class='col-sm-2'>" );
        printf( "%s", $a_info );
        printf( "</div>" );
        printf( "</div>" );
    }
    else
    {
        printf( "<td width='%d%%' style='text-align: center;'>%s</td>", $al_width, $a_info );
    }
    
}

// file format is
// 012345678901234567890123456789
// MDalarm_yyyymmdd_hhmmss.mkv
function func_create_camera_graph( $gdata, $divname, $camera_addr )
{
    printf( "<script type='text/javascript'>" );
    
    printf( "/* Darken a color */" );
    printf( "function darkenColor(colorStr) {" );
    printf( "    /* Defined in dygraph-utils.js */" );
    printf( "    var color = Dygraph.toRGB_(colorStr);" );
    printf( "    color.r = Math.floor((255 + color.r) / 2);" );
    printf( "    color.g = Math.floor((255 + color.g) / 2);" );
    printf( "    color.b = Math.floor((255 + color.b) / 2);" );
    printf( "    return 'rgb(' + color.r + ',' + color.g + ',' + color.b + ')';" );
    printf( "}" );
    
    printf( "function barChartPlotter(e) {" );
    printf( "    var ctx = e.drawingContext;" );
    printf( "    var points = e.points;" );
    printf( "    var y_bottom = e.dygraph.toDomYCoord(0);" );
    printf( "    ctx.fillStyle = darkenColor(e.color);" );
    printf( "    /* Find the minimum separation between x-values. */" );
    printf( "    /* This determines the bar width. */" );
    printf( "    var min_sep = Infinity;" );
    printf( "    for (var i = 1; i < points.length; i++) {" );
    printf( "        var sep = points[i].canvasx - points[i - 1].canvasx;" );
    printf( "        if (sep < min_sep) min_sep = sep;" );
    printf( "    }" );
    printf( "    var bar_width = Math.floor(2.0 / 3 * min_sep);" );
    printf( "    /* Do the actual plotting.*/" );
    printf( "    for (var i = 0; i < points.length; i++) {" );
    printf( "        var p = points[i];" );
    printf( "        var center_x = p.canvasx;" );
            
    printf( "        ctx.fillRect(center_x - bar_width / 2, p.canvasy," );
    printf( "            bar_width, y_bottom - p.canvasy);" );
            
    printf( "        ctx.strokeRect(center_x - bar_width / 2, p.canvasy," );
    printf( "            bar_width, y_bottom - p.canvasy);" );
    printf( "    }" );
    printf( "}" );
    
    printf( "g = new Dygraph(" );
    
    // containing div
    printf( "document.getElementById('%s'),", $divname );
    
    printf( "[" );
    $partname = "";
    $count = 0;
    foreach ( $gdata as $data )
    {
        $year = substr( $data['file'], 8, 4 );
        $mon = substr( $data['file'], 12, 2 );
        $day = substr( $data['file'], 14, 2 );
        $hour = substr( $data['file'], 17, 2 );
        $min = substr( $data['file'], 19, 2 );
        $sec = substr( $data['file'], 21, 2 );
        $msec = 0;
        printf( "[new Date('%d-%02d-%02dT%02d:%02d:%02d'),1]", $year, $mon, $day, $hour, $min, $sec );
        
                
        $count += 1;
        if ( $count < count($gdata) )
        {
            printf( "," );
        }
    }
    
    printf("],");

    printf( "{ labels: ['Time', 'Video'], " );
    printf( "  legend: 'always', " );
    printf( "  includeZero: true, " );
    printf( "  plotter: barChartPlotter, " );
    //printf( "  drawPoints: true " );
    printf( "  drawAxis: { y: false }, " );
    printf( "  axes: {" );
    printf( "    y: {" );
    printf( "        drawGrid: false" );
    printf( "    }" );
    printf( "  }," );
    
    
    printf( "interactionModel:{
        willDestroyContextMyself: true,
        mousedown: function (event, g, context) {
            Dygraph.defaultInteractionModel.mousedown(event, g, context);
        },
        mousemove: function (event, g, context) {
            // no call to defaultInteractionModel needed as a handler for this event is bound in mousedown.
        },
        mouseup: function(event, g, context) {
            // no call to defaultInteractionModel needed as a handler for this event is bound in mousedown.
        },
        mouseout: function(event, g, context) {
            // no call to defaultInteractionModel needed as this event is not used. (mouseout is detected using the dynamically bound mousemove event)
        },
        dblclick: function(event, g, context) {
            Dygraph.defaultInteractionModel.dblclick(event, g, context);
        },
        mousewheel: function(event, g, context) {
            // no call to defaultInteractionModel needed as this event is not used.
        },
        //touchstart: newDygraphTouchstart,
        touchstart: function touchstart(event, g, context) {
            DygraphInteraction.startTouch(event, g, context);
        },
        touchmove: function touchmove(event, g, context) {
            DygraphInteraction.moveTouch(event, g, context);
        },
        touchend: function touchend(event, g, context) {
            DygraphInteraction.endTouch(event, g, context);
        },
       }" );
    
    printf( "}" );
    printf( ");" );
    
    printf( "g.updateOptions( {" );
    printf( "  pointClickCallback: function(event, p) {" );
    printf( "    var dd = new Date(p.xval);" );
    printf( "    var html = '';" );
    printf( "    html += 'MDalarm_';" );
    printf( "    html += dd.getFullYear();" );
    printf( "    html += ('0' + (dd.getMonth()+1)).slice(-2);" );
    printf( "    html += ('0' + dd.getDate()).slice(-2);" );
    printf( "    html += '_';" );
    printf( "    html += ('0' + dd.getHours()).slice(-2);" );
    printf( "    html += ('0' + dd.getMinutes()).slice(-2);" );
    printf( "    html += ('0' + dd.getSeconds()).slice(-2);" );
    printf( "    html += '.mkv';" );
    printf( "    document.getElementById('cameragraphclick').innerHTML = html;" );
    printf( "    var href = '?CameraNo=%s&CameraFile=';", $camera_addr );
    printf( "    href += html;" );
    printf( "    document.getElementById('cameragraphclick').href = href;" );
    printf( "    document.getElementById('cameragraphclick').click()" );
    printf( "  }" );
    printf( "});" );
    
    printf( "</script>" );
}


?>
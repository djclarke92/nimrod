<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Flat Cat IT Ltd
//	Author: Dave Clarke
//
//--------------------------------------------------------------------------------------

include_once( "common.php" );

if ( !isset($_SESSION['auth_level']) )
{	// access not via main page - access denied
	func_unauthorisedaccess();
	return;
}

function func_find_graph_device( $de_no, $ch )
{
	$rc = false;
	
	foreach( $_SESSION['GraphDevices'] as $gg )
	{
		if ( $gg['GraphDeviceNo'] == $de_no && $gg['GraphIOChannel'] == $ch )
		{
			$rc = true;
			break;
		}
	}
	
	return $rc;
}

function func_set_graph_datetime( &$dd, &$tt, $inc )
{
	if ( $dd == "" || $tt == "" )
	{	// set date/time now
		$when = getdate();
		
		$dd = sprintf( "%02d/%02d/%4d", $when['mday'], $when['mon'], $when['year'] );
		$tt = sprintf( "%02d:%02d", $when['hours'], $when['minutes'] );
	}
	
	$expl = explode( "/", $dd );
	$year = $expl[2];
	$mon = $expl[1];
	$mday = $expl[0];
	
	$expl = explode( ":", $tt );
	$hours = $expl[0];
	$minutes = $expl[1];
	
	$now = time();
	$mktime = mktime( $hours, $minutes, 0, $mon, $mday, $year );
	
	$mktime += ($inc * 3600);
	
	if ( $mktime <= $now )
	{
		$when = getdate( $mktime );	
	
		$dd = sprintf( "%02d/%02d/%4d", $when['mday'], $when['mon'], $when['year'] );
		$tt = sprintf( "%02d:%02d", $when['hours'], $when['minutes'] );
	}
	else
	{
		$dd = "";
		$tt = "";
	}
}

function func_get_graph_datetime()
{
	if ( $_SESSION['GStartDate'] == "" || $_SESSION['GStartTime'] == "" )
	{
		return time();
	}
	else
	{
		$expl = explode( "/", $_SESSION['GStartDate'] );
		$year = $expl[2];
		$mon = $expl[1];
		$mday = $expl[0];
		
		$expl = explode( ":", $_SESSION['GStartTime'] );
		$hours = $expl[0];
		$minutes = $expl[1];
		
		$now = time();
		$mktime = mktime( $hours, $minutes, 0, $mon, $mday, $year );
		
		return $mktime;
	}
}

function func_is_temp( $io_type )
{
    $rc = false;
    
    switch ( $io_type )
    {
    default:
        break;   
    case E_IO_TEMP_HIGH:
    case E_IO_TEMP_LOW:
    case E_IO_TEMP_MONITOR:
        $rc = true;
        break;
    }
    
    return $rc;
}

$gdevice_no = 0;
$gio_channel = 0;
$delete_event_no = 0;
$delete_all_event_no = 0;
$set_camera_mode = false;


if ( !isset($_SESSION['GraphHours']) )
	$_SESSION['GraphHours'] = 6;
if ( !isset($_SESSION['GraphDevices']) )
	$_SESSION['GraphDevices'] = array();
if ( !isset($_SESSION['GStartDate']) )
	$_SESSION['GStartDate'] = "";
if ( !isset($_SESSION['GStartTime']) )
	$_SESSION['GStartTime'] = "";
if ( !isset($_SESSION['ShowCameraNo']) )
	$_SESSION['ShowCameraNo'] = 0;
if ( !isset($_SESSION['ShowCameraFile']) )
	$_SESSION['ShowCameraFile'] = "";



if ( isset($_GET['DeleteEventNo']) )
	$delete_event_no = $_GET['DeleteEventNo'];
if ( isset($_GET['DeleteAllEventNo']) )
	$delete_all_event_no = $_GET['DeleteAllEventNo'];
if ( isset($_GET['Hours']) )
	$_SESSION['GraphHours'] = $_GET['Hours'];
if ( isset($_GET['GraphDeviceNo']) )
{
	$gdevice_no = $_GET['GraphDeviceNo'];
	$_SESSION['ShowCameraNo'] = 0;
	$_SESSION['ShowCameraFile'] = "";
}
if ( isset($_GET['GraphIOChannel']) )
	$gio_channel = $_GET['GraphIOChannel'];
if ( isset($_POST['GStartDate']) )
	$_SESSION['GStartDate'] = $_POST['GStartDate'];
if ( isset($_POST['GStartTime']) )
	$_SESSION['GStartTime'] = $_POST['GStartTime'];
if ( isset($_GET['CameraNo']) )
{
	if ( $_SESSION['ShowCameraNo'] != $_GET['CameraNo'] )
	{	// switch cameras
		$_SESSION['ShowCameraNo'] = $_GET['CameraNo'];
		$_SESSION['ShowCameraFile'] = "";
		$set_camera_mode = true;
	}
	else if ( $_SESSION['ShowCameraFile'] != "" )
	{	// go back to real time view
		$_SESSION['ShowCameraFile'] = "";
	}
	else if ( !isset($_GET['CameraFile']) )
	{	// remove camera display
		$_SESSION['ShowCameraNo'] = 0;
	}
	$gdevice_no = 0;
	$gio_channel = 0;
	$_SESSION['GraphDevices'] = array();
}
if ( isset($_GET['CameraFile']) )
{
	$_SESSION['ShowCameraFile'] = $_GET['CameraFile'];
}


if ( $gdevice_no != 0 )
{
    $di = $db->ReadDeviceInfoDC( $gdevice_no, $gio_channel );
    
    $found = false;
	foreach ( $_SESSION['GraphDevices'] as &$gdev )
	{
		if ( $gdev['GraphDeviceNo'] == $gdevice_no && $gdev['GraphIOChannel'] == $gio_channel )
		{	// remove it from the list
			$found = true;
			$gdev['GraphDeviceNo'] = 0;
			$gdev['GraphIOChannel'] = 0;
			$gdev['GraphIOType'] = 0;
			break;
		}
		else if ( (func_is_temp($di['di_IOType']) && !func_is_temp($gdev['GraphIOType'])) ||
		    (!func_is_temp($di['di_IOType']) && func_is_temp($gdev['GraphIOType'])) )
		{   // different type - remove it from the list
		    $gdev['GraphDeviceNo'] = 0;
		    $gdev['GraphIOChannel'] = 0;
		    $gdev['GraphIOType'] = 0;
		}
	}
	if ( $found == false )
	{	// add a new device to the graph list
		$_SESSION['GraphDevices'][] = array( 'GraphDeviceNo'=>$gdevice_no, 'GraphIOChannel'=>$gio_channel, 'GraphIOType'=>$di['di_IOType'] );
	}

	// remove holes in the list
	$temp = $_SESSION['GraphDevices'];
	$_SESSION['GraphDevices'] = array();
	foreach ( $temp as $gg )
	{
		if ( $gg['GraphDeviceNo'] != 0 )
		{
			$_SESSION['GraphDevices'][] = array( 'GraphDeviceNo'=>$gg['GraphDeviceNo'], 'GraphIOChannel'=>$gg['GraphIOChannel'], 'GraphIOType'=>$gg['GraphIOType'] );
		}
	}
}




if ( $delete_event_no != 0 )
{
	$db->DeleteEventNo( $delete_event_no );
}
else if ( $delete_all_event_no != 0 )
{
	$db->DeleteAllEventNo( $delete_all_event_no );
}
else if ( isset($_POST['CurrentGraph']) )
{
	$_SESSION['GStartDate'] = "";
	$_SESSION['GStartTime'] = "";
}
else if ( isset($_POST['GraphPlusHour']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], 1 );
}
else if ( isset($_POST['GraphMinusHour']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], -1 );
}
else if ( isset($_POST['GraphPlusDay']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], 24 );
}
else if ( isset($_POST['GraphMinusDay']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], -24 );
}


$devices = $db->ReadDevicesTable();

$datetime = func_get_graph_datetime();

$temperatures = $db->GetLatestTemperatures( $_SESSION['GraphHours'], $datetime );
$voltages = $db->GetLatestVoltages( $_SESSION['GraphHours'], $datetime );

$db_size = $db->GetDatabaseSize();

$now = getdate();
$camera_files = array();
foreach ( $_SESSION['camera_list'] as $camera )
{
	if ( $_SESSION['ShowCameraNo'] == $camera['addr'] )
	{
		$camera_files = func_read_camera_files( $camera['directory'], $now['year'], $now['mon'], $now['mday'] );
		break;
	}
}


printf( "<tr>" );
printf( "<td colspan='4'>" );

printf( "<table width='100%%'>" );
printf( "<tr valign='top'>" );
printf( "<td>" );

// display devices list
printf( "<table class='style-small-table'>" );
printf( "<tr><th>Name</th><th>Status</th></tr>" );
foreach ( $devices as $dd )
{
	$failures = $db->GetDeviceFailures( $dd['de_DeviceNo'] );
	
	printf( "<tr>" );
	printf( "<td><b>%s (%d)</b></td>", $dd['de_Name'], $dd['de_Address'] );
	printf( "<td><img src='%s' height='25px'> <img src='%s' height='25px'></td>", func_get_device_status_img( $dd['de_Status'] ),
			func_get_device_failures_img( $failures ) );
	printf( "</tr>" );
}
printf( "</table>" );

printf( "<br>" );

// display last device failure
$printed = false;
printf( "<table class='style-small-table'>" );
printf( "<tr><th>Name</th><th>Last Failure</th></tr>" );
foreach ( $devices as $dd )
{
	$failures = $db->GetDeviceFailures( $dd['de_DeviceNo'] );
	
	if ( count($failures) > 0 )
	{
		$printed = true;
		printf( "<tr>" );
		
		printf( "<td><b>%s</b></td>", $dd['de_Name'] );
		
		$onclick = sprintf( "return confirm(\"Are you sure you want to delete this event ?\")" );
		printf( "<td>" );
		printf( "<a href='index.php?DeleteEventNo=%d' onclick='%s;'>%s</a>", $failures[0]['ev_EventNo'], $onclick, $failures[0]['ev_Timestamp'] );
		
		printf( "&nbsp;&nbsp;" );
		
		$onclick = sprintf( "return confirm(\"Are you sure you want to delete all failure event ?\")" );
		printf( "<a href='index.php?DeleteAllEventNo=%d' onclick='%s;'>All</a>", $failures[0]['ev_EventNo'], $onclick );
		printf( "</td>" );
		
		printf( "</tr>" );
	}
}
if ( !$printed )
{
	printf( "<tr>" );
	printf( "<td colspan='2'>No recent failures</td>" );
	printf( "</tr>" );
}
printf( "</table>" );

printf( "<br>" );

// display table record counts
$records = $db->GetTableRecordCount();

printf( "<table class='style-small-table'>" );

printf( "<tr><td colspan=2>" );
printf( "<table class='style-small-table'>" );
printf( "<tr><td><b>Nimrod</b></td><td>%.1f MBytes</td></tr>", $db_size );
printf( "</table>" );
printf( "</td></tr>" );

printf( "<tr><th>Table</th><th>Records</th></tr>" );

foreach( $records as $record )
{
    printf( "<tr>" );
    
    printf( "<td>%s</td><td>%d</td>", $record['table'], $record['count'] );
    
    printf( "</tr>" );
}

printf( "</table>" );


printf( "</td>" );
printf( "<td>" );

// display temperatures
printf( "<table class='style-small-table'>" );
printf( "<tr><th>Name</th><th>Temperature</th><th>Graph</th></tr>" );
foreach ( $temperatures as $tt )
{
	printf( "<tr>" );
	
	if ( isset($tt['data'][0]) )
	{
		printf( "<td><a href='index.php?GraphDeviceNo=%d&GraphIOChannel=%d'>%s</a></td>", $tt['di_DeviceNo'], $tt['di_IOChannel'], $tt['di_IOName'] );
		printf( "<td>%s deg C</td>", func_calc_temperature( $tt['data'][0]['ev_Value'] ) );
		printf( "<td>%s</td>", func_convert_timestamp( $tt['data'][0]['ev_Timestamp'] ) );
	}
	else
	{
		printf( "<td>%s</td>", $tt['di_IOName'] );
		printf( "<td>? deg C</td>" );
		printf( "<td></td>" );
	}
	
	$img = "&nbsp;&nbsp;&nbsp;";
	if ( func_find_graph_device( $tt['di_DeviceNo'], $tt['di_IOChannel'] ) )
	{
		$img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
	}
	printf( "<td>%s</td>", $img );
	
	printf( "</tr>" );
}
printf( "</table>" );

// display voltages
printf( "<table class='style-small-table'>" );
printf( "<tr><th>Name</th><th>Voltage</th><th>Graph</th></tr>" );
foreach ( $voltages as $tt )
{
	printf( "<tr>" );
	printf( "<td><a href='index.php?GraphDeviceNo=%d&GraphIOChannel=%d'>%s</a></td>", $tt['di_DeviceNo'], $tt['di_IOChannel'], $tt['di_IOName'] );
	printf( "<td>%s %s</td>", func_calc_voltage( $tt['data'][0]['ev_Value'], $tt['di_AnalogType'] ), $tt['di_AnalogType'] );
	printf( "<td>%s</td>", func_convert_timestamp( $tt['data'][0]['ev_Timestamp'] ) );
	
	$img = "&nbsp;&nbsp;&nbsp;";
	if ( func_find_graph_device( $tt['di_DeviceNo'], $tt['di_IOChannel'] ) )
	{
		$img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
	}
	printf( "<td>%s</td>", $img );
	
	printf( "</tr>" );
}
printf( "</table>" );

// display cameras
printf( "<table class='style-small-table'>" );
printf( "<tr>" );

printf( "<td valign='top'>" );
printf( "<table class='style-small-table'>" );
printf( "<tr><th>Camera</th><th></th></tr>" );

foreach ( $_SESSION['camera_list'] as $camera )
{
	printf( "<tr>" );
	printf( "<td><a href='index.php?CameraNo=%d'>%s</a></td>", $camera['addr'], $camera['name'] );
	$img = "&nbsp;&nbsp;&nbsp;";
	if ( $_SESSION['ShowCameraNo'] == $camera['addr'] )
	{
		$img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
	}
	printf( "<td>%s</td>", $img );
	printf( "</tr>" );
}

printf( "</table>" );
printf( "</td>" );

printf( "<td valign='top'>" );
printf( "<table class='style-small-table'>" );
printf( "<tr><th>Files</th><th></th></tr>" );

foreach ( $camera_files as $file )
{
	printf( "<tr><td><a href='index.php?CameraNo=%d&CameraFile=%s'>%s</a></td>", $_SESSION['ShowCameraNo'], $file, substr($file,8,strlen($file)-4-8) );
	$img = "&nbsp;&nbsp;&nbsp;";
	if ( $_SESSION['ShowCameraFile'] == $file )
	{
		$img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
	}
	printf( "<td>%s</td>", $img );
	printf( "</tr>" );
}

printf( "</table>" );
printf( "</td>" );

printf( "</tr>" );
printf( "</table>" );


printf( "</td>" );
printf( "<td colspan='2' width='44%%'>" );


if ( $_SESSION['ShowCameraNo'] != 0 )
{	// display camera stream
	if ( $set_camera_mode )
	{	// set MJpeg stream
		$cmd = sprintf( "curl --silent \"http://192.168.1.%d:88/cgi-bin/CGIProxy.fcgi?cmd=setSubStreamFormat&format=1&usr=%s&pwd=%s\"", $_SESSION['ShowCameraNo'], CAMERA_USER, CAMERA_PWD );
		$res = exec( $cmd, $out, $ret );
	}
	
	if ( $_SESSION['ShowCameraFile'] != "" )
	{
		$file = $_SESSION['ShowCameraFile'];
		foreach ( $_SESSION['camera_list'] as $camera )
		{
			if ( $_SESSION['ShowCameraNo'] == $camera['addr'] )
			{
				$file = sprintf( "/cctv/%s/record/%s", $camera['directory'], $_SESSION['ShowCameraFile'] );
				break;
			}
		}
		printf( "%s", $file );

		printf( "<video id='nimrod-player' class='video-js' controls preload='auto' poster='' data-setup='{}'>" );
		printf( "<source src='file://%s' type='video/webm'></source>", $file );
		printf( "<p class='vjs-no-js'>" );
		printf( "To view this video please enable JavaScript, and consider upgrading to a web browser that supports HTML5 video</p>" );
		printf( "</video>" );
		                
		printf( "<video width='400' height='225' controls>" );
		//printf( "<source src='file://%s' type='video/webm'>", $file );	
		printf( "<source src='file://%s' controls>", $file );
		printf( "Your browser does not support the video tag." );
		printf( "</video>" );
		printf( "Camera File" );
	}
	else if ( func_is_external_connection() )
	{	// external web connection
		$cmd = sprintf( "curl --silent \"http://192.168.1.%d:88/cgi-bin/CGIProxy.fcgi?cmd=snapPicture2&usr=%s&pwd=%s\"", $_SESSION['ShowCameraNo'], CAMERA_USER, CAMERA_PWD );

		echo "<img src='data:image/jpeg;base64,";
		
		ob_start();
		passthru( $cmd, $out );
		$var = ob_get_contents();
		ob_end_clean();
		echo base64_encode($var);
		
		echo "' alt='no snap shot' width='400'>";
		printf( "Camera Snapshot" );
	}
	else 
	{
		$dest = sprintf( "192.168.1.%d:88", $_SESSION['ShowCameraNo'] );
		printf( "<img src='http://%s/cgi-bin/CGIStream.cgi?cmd=GetMJStream&usr=%s&pwd=%s' alt='no mjpeg stream (%s)' width='400'>", 
			$dest, CAMERA_USER, CAMERA_PWD, $dest );
		printf( "Camera Stream" );
	}
}
else 
{	// display graph
	$factor = 0;
	$max = 100;
	$max_points = 0;
	$min_points = 1000;
	$_SESSION['graph_data'] = array();
	foreach ( $_SESSION['GraphDevices'] as $gg )
	{
		$atype = "V";
		$gname = "?";
		$gformat = "?";
		$gvoltage = false;
	
		$found = false;
		foreach ( $temperatures as $tt )
		{
			if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
			{
				$found = true;
				$myarray = $tt;
				$gname = $tt['di_IOName'];
				$gformat = "degrees";
				break;
			}
		}
		if ( $found == false )
		{
			$gvoltage = true;
			foreach ( $voltages as $tt )
			{
				if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
				{
					$found = true;
					$myarray = $tt;
					$gname = $tt['di_IOName'];
					$gformat = "V";
					$atype = $tt['di_AnalogType'];
					if ( $atype == "A" )
					{
						$gformat = "A";
					}
					break;
				}
			}
		}
		
		if ( $found )
		{
			$ff = intval(floor(count($myarray['data']) / $max));
			if ( $ff > $factor )
			{	// get the biggest factor value
				$factor = $ff;
			}
			$mp = 1 + count($myarray['data']) / ($factor + 1);
			if ( $mp > $max_points )
			{
				$max_points = $mp;
			}
			if ( count($myarray['data']) < $min_points )
			{
				$min_points = count($myarray['data']);
			}
			
			$_SESSION['graph_data'][] = array( 'data'=>array(), 'name'=>$gname, 'myarray'=>$myarray, 'factor'=>0, 'voltage'=>$gvoltage, 'atype'=>$atype );
		}
	}

	$removed = 0;
	if ( $min_points < $max )
	{
		$max_points = $min_points;
		foreach ( $_SESSION['graph_data'] as &$graph )
		{
			$remove = count($graph['myarray']['data']) - $max_points;
			if ( $remove > 0 )
			{
			    $removed = $remove;
				$data = array();
				foreach ( array_reverse($graph['myarray']['data']) as $dd )
				{
					if ( $remove <= 0 )
					{
						$data[] = $dd;
					}
					$remove -= 1;
				}
				
				$graph['myarray']['data'] = array_reverse($data);
			}
		}
	}

	foreach ( $_SESSION['graph_data'] as &$graph )
	{
		$graph['factor'] = $factor;
	
		$i = 0;
		$data = array();

		// limit data 
		$pcount = 0;
		$total = count($graph['myarray']['data']);
		foreach ( array_reverse($graph['myarray']['data']) as $dd )
		{
			$pcount += 1;
			$expl = explode( " ", $dd['ev_Timestamp'] );
			if ( $gvoltage )
				$temp = func_calc_voltage( $dd['ev_Value'], $graph['atype'] );
			else
				$temp = func_calc_temperature( $dd['ev_Value'] );

			if ( $i == 0 )
			{
				$val = $temp;
				$data[substr($expl[1],0,5)] = $temp;
			}
			$i += 1;
			if ( $i > $factor )
			{
				$i = 0;
			}
			$numleft = $total - $pcount;
			if ( $numleft < $max_points - count($data) )
			{	// show all points from now on
				$i = 0;
			}
		}

		$graph['data'] = $data;
//		printf( "(%s) data out %d", $graph['name'], count($data) ); 
	}

	$xinterval = 0;
	if ( count($_SESSION['GraphDevices']) != 0 )
	{
		$xinterval = intval( $max_points / 20 );
		printf( "<img src='./files/create_graph.php?XInterval=%d&Format=%s'>", $xinterval, $gformat );

		printf( "<br>" );
		printf( "<a href='index.php?Hours=1'><input type='button' name='Hours1' value='1 Hour' style='%s'></a>", ($_SESSION['GraphHours'] == 1 ? "font-weight: bold;" : "") );
		printf( "<a href='index.php?Hours=3'><input type='button' name='Hours3' value='3 Hours' style='%s'></a>", ($_SESSION['GraphHours'] == 3 ? "font-weight: bold;" : "") );
		printf( "<a href='index.php?Hours=6'><input type='button' name='Hours6' value='6 Hours' style='%s'></a>", ($_SESSION['GraphHours'] == 6 ? "font-weight: bold;" : "") );
		printf( "<a href='index.php?Hours=12'><input type='button' name='Hours12' value='12 Hours' style='%s'></a>", ($_SESSION['GraphHours'] == 12 ? "font-weight: bold;" : "") );
		printf( "<a href='index.php?Hours=24'><input type='button' name='Hours24' value='24 Hours' style='%s'></a>", ($_SESSION['GraphHours'] == 24 ? "font-weight: bold;" : "") );
		printf( "<a href='index.php?Hours=48'><input type='button' name='Hours48' value='48 Hours' style='%s'></a>", ($_SESSION['GraphHours'] == 48 ? "font-weight: bold;" : "") );
	
		printf( "<br>" );
		printf( "Historic: Date <input type='text' name='GStartDate' value='%s' size='8'>", $_SESSION['GStartDate'] );
		printf( "Time <input type='text' name='GStartTime' value='%s' size='5'>", $_SESSION['GStartTime'] );
		printf( "<input type='submit' name='GraphGo' value='Go'>" );
		printf( "&nbsp;&nbsp;&nbsp;<input type='submit' name='CurrentGraph' value='Graph Now'>" );
	
		printf( "<br>" );
		printf( "<input type='submit' name='GraphMinusHour' value='-1 hrs'>" );
		printf( "<input type='submit' name='GraphMinusDay' value='-24 hrs'>" );
		printf( "<input type='submit' name='GraphPlusDay' value='+24 hrs'>" );
		printf( "<input type='submit' name='GraphPlusHour' value='+1 hrs'>" );
	
		printf( "<br><div id='info'>interval %d, %d, %d</div>", $xinterval, $max_points, $removed );

		foreach ( $_SESSION['GraphDevices'] as $gg )
		{
			printf( "(%d,%d,%d)<br>", $gg['GraphDeviceNo'], $gg['GraphIOChannel'], $gg['GraphIOType'] );
		}
		foreach ( $_SESSION['graph_data'] as $dd )
		{
			printf( "%s: %d,%d,%d<br>", $dd['name'], count($dd['myarray']['data']), count($dd['data']), $dd['factor'] );
		}
	}
}

printf( "</td>" );
printf( "</tr>" );
printf( "</table>" );

printf( "</td>" );
printf( "</tr>" );

printf( "<tr><td><a href='index.php?Refresh=1'><input type='button' name='Refresh' id='Refresh' value='Refresh'></a>" );
printf( "</td>" );
printf( "</tr>" );

printf( "<tr><td>&nbsp;</td></tr>" );



?>


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


function func_get_mdalarm_count($dir)
{
    $count = -1;
    
    $file = sprintf( "%s/file_count.txt", $dir );
    $fp = fopen( $file, "r" );
    if ( $fp !== false )
    {
        $count = fgets( $fp );
        fclose( $fp );
    }
    
    return $count;
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

function func_get_graph_devices( $temperatures, $voltages, $levels, $currents, $powers, $frequencies )
{
    $g_devices = array();
    foreach ( $_SESSION['GraphDevices'] as $gg )
    {
        $gname = "?";
        
        $found = false;
        foreach ( $temperatures as $tt )
        {
            if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
            {
                $found = true;
                $gname = $tt['di_IOName'];
                break;
            }
        }
        if ( $found == false )
        {
            foreach ( $voltages as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
                {
                    $found = true;
                    $gname = $tt['di_IOName'];
                    break;
                }
            }
        }
        if ( $found == false )
        {
            foreach ( $levels as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
                {
                    $found = true;
                    $gname = $tt['di_IOName'];
                    break;
                }
            }
        }
        if ( $found == false )
        {
            foreach ( $currents as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
                {
                    $found = true;
                    $gname = $tt['di_IOName'];
                    break;
                }
            }
        }
        if ( $found == false )
        {
            foreach ( $powers as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
                {
                    $found = true;
                    $gname = $tt['di_IOName'];
                    break;
                }
            }
        }
        if ( $found == false )
        {
            foreach ( $frequencies as $tt )
            {
                if ( $tt['di_DeviceNo'] == $gg['GraphDeviceNo'] && $tt['di_IOChannel'] == $gg['GraphIOChannel'] )
                {
                    $found = true;
                    $gname = $tt['di_IOName'];
                    break;
                }
            }
        }
        
        if ( $found )
        {
            $g_devices[] = array( 'di_DeviceNo'=>$gg['GraphDeviceNo'], 'di_IOChannel'=>$gg['GraphIOChannel'], 'di_IOName'=>$gname );
        }
    }
    
    return $g_devices;
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
	$_SESSION['ShowCameraNo'] = "";
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
	$_SESSION['ShowCameraNo'] = "";
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
	else if ( !isset($_GET['CameraFile']) )
	{	// remove camera display
		$_SESSION['ShowCameraNo'] = "";
		$_SESSION['ShowCameraFile'] = "";
	}
	$gdevice_no = 0;
	$gio_channel = 0;
	$_SESSION['GraphDevices'] = array();
}
if ( isset($_GET['CameraFile']) )
{
   	$_SESSION['ShowCameraFile'] = $_GET['CameraFile'];
}

// reset the list of graph devices
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
else if ( isset($_GET['ClearGraph']) )
{
    $_SESSION['GraphDevices'] = array();
    $_SESSION['GStartDate'] = "";
    $_SESSION['GStartTime'] = "";
}
else if ( isset($_GET['CurrentGraph']) )
{
	$_SESSION['GStartDate'] = "";
	$_SESSION['GStartTime'] = "";
}
else if ( isset($_GET['GraphPlusHour']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], 1 );
}
else if ( isset($_GET['GraphMinusHour']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], -1 );
}
else if ( isset($_GET['GraphPlusDay']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], 24 );
}
else if ( isset($_GET['GraphMinusDay']) )
{
	func_set_graph_datetime( $_SESSION['GStartDate'], $_SESSION['GStartTime'], -24 );
}

$records = $db->GetTableRecordCount();

$devices = $db->ReadDevicesTable();
$deviceinfo_list = $db->ReadDeviceInfoTable();
$iolinks_list = $db->ReadIOLinksTable();

$datetime = func_get_graph_datetime();

$temperatures = $db->GetLatestTemperatures( $_SESSION['GraphHours'], $datetime );
$voltages = $db->GetLatestVoltages( $_SESSION['GraphHours'], $datetime );
$levels = $db->GetLatestLevels( $_SESSION['GraphHours'], $datetime );
$currents = $db->GetLatestCurrents( $_SESSION['GraphHours'], $datetime );
$powers = $db->GetLatestPowers( $_SESSION['GraphHours'], $datetime );
$frequencies = $db->GetLatestFrequencies( $_SESSION['GraphHours'], $datetime );

$db_size = $db->GetDatabaseSize();

// check for WebClick
$done = false;
foreach ( $devices as $device )
{
    $dno = $device['de_DeviceNo'];
    for ( $cno = 1; $cno <=16; $cno++ )
    {
        $wc = sprintf( "WebClick%02d%02d", $dno, $cno );
        if ( isset($_GET[$wc]) )
        {
            $done = true;
            func_create_click_file( $db, $dno, $cno );
            
            // sleep for 2 seconds
            sleep(1);
            // read the new status
            $deviceinfo_list = $db->ReadDeviceInfoTable();
            $iolinks_list = $db->ReadIOLinksTable();
            break;
        }
    }
    if ( $done )
    {
        break;
    }
}


$now = getdate();
$camera_list = $db->ReadCameraList();
$camera_files = array();
foreach ( $camera_list as $camera )
{
	if ( $_SESSION['ShowCameraNo'] == $camera['ca_IPAddress'] )
	{
		$camera_files = func_read_camera_files( $camera['ca_Directory'], $now['year'], $now['mon'], $now['mday'] );
		break;
	}
}


?>


<div class="container" style="margin-top:30px">
	<!-- *************************************************************************** -->
	<div class="row">
		<div class="col-sm-4">
			<div class="progress">
  				<div class="progress-bar bg-info" id='refresh-progress-bar' style="width:0%;height:10px">Refresh</div>
  			</div>
		</div>
		<div class="col-sm-1">
			<div>
				<input class=form-check-input type='checkbox' id='RefreshEnabled' name='RefreshEnabled' value='Refresh'>
			</div>
		</div>
	</div>

    <?php
    if ( count($devices) > 1 || (count($devices) == 1 && $devices[0]['de_Type'] != E_DT_TIMER) )
    {   // only display device details if there are more devices than just the timer

    $show_row_dvt = "";
    if ( $_SESSION['ShowCameraFile'] == "" && $_SESSION['ShowCameraNo'] == "" && count($_SESSION['GraphDevices']) == 0 )
    {
        $show_row_dvt = "show";
    }
    ?>	
			
	<!-- *************************************************************************** -->
	<div class="row">
        
		<!-- *************************************************************************** -->
		<div class="col-sm-4">
			<div class='row'>
				<div class='col'>
    				<h3>Status</h3>
    			</div>
    			<div class='col'>
					<a href='#row_dvt' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
				</div>
			</div>

			<div id="row_dvt" class="collapse  <?php echo $show_row_dvt; ?>">
            
    		<table class='table table-striped'>
    		<thead class="thead-light">
              <tr>
              <th>Name</th>
              <th>Status</th>
              </tr>
            </thead>
            <?php 
            foreach ( $devices as $dd )
            {
                $failures = $db->GetDeviceFailures( $dd['de_DeviceNo'] );
                
                printf( "<tr>" );
                printf( "<td><b>%s (%d)</b></td>", $dd['de_Name'], $dd['de_Address'] );
                printf( "<td><img src='%s' height='25px'> <img src='%s' height='25px'></td>", func_get_device_status_img( $dd['de_Status'] ),
                    func_get_device_failures_img( $failures ) );
                printf( "</tr>" );
            }
            ?>
            </table>
            
    		</div>
		</div>


		<!-- *************************************************************************** -->
        <div class="col-sm-4">
		<div id="row_dvt" class="collapse  <?php echo $show_row_dvt; ?>">
			<?php
			$title = "Voltages";
			foreach ( $voltages as $tt )
			{
			    if ( $tt['di_AnalogType'] == 'A' )
			    {
			        $title = "Voltage &amp; Current";
			        break;
			    }
			}
			printf( "<h3>%s</h3>", $title );
			?>
            
            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Name</th>
              <th>Value</th>
              <th>Date</th>
              <th>Graph</th>
              </tr>
            </thead>
    
    		<?php
    		$datalist = array( $voltages, $currents, $powers, $frequencies );
    		foreach ( $datalist as $dtype )
    		{
    		    if ( count($dtype) > 0 )
    		    {
                    foreach ( $dtype as $tt )
                    {
                        $units = "";
                        $val = 0;
                        if ( count($tt['data']) > 0 )
                        {
                            $units = $tt['di_AnalogType'];
                            switch ( $tt['di_AnalogType'] )
                            {
                            default:
                            case 'V':
                                $val = func_calc_voltage( $tt['data'][count($tt['data'])-1]['ev_Value'], $tt['di_AnalogType'] );
                                break;
                            case 'A':
                                $val = func_calc_current( $tt['data'][count($tt['data'])-1]['ev_Value'] );
                                break;
                            case 'W':
                                $val = func_calc_power( $tt['data'][count($tt['data'])-1]['ev_Value'] );
                                break;
                            case 'F':
                                $units = "Hz";
                                $val = func_calc_frequency( $tt['data'][count($tt['data'])-1]['ev_Value'] );
                                break;
                            }
                        }
                        $class = "";
                        if ( $tt['di_MonitorLo'] != 0.0 && $tt['di_MonitorHi'] != 0.0 && ($val < $tt['di_MonitorLo'] || $val > $tt['di_MonitorHi']) )
                            $class = "table-danger";
                        printf( "<tr class='%s'>", $class );
                    	printf( "<td><div class='text-nowrap'><a href='?GraphDeviceNo=%d&GraphIOChannel=%d'>%s</a></div></td>", $tt['di_DeviceNo'], $tt['di_IOChannel'], $tt['di_IOName'] );
                    	printf( "<td>%s%s</td>", $val, $units );
                    	if ( count($tt['data']) > 0 )
                        	printf( "<td><div class='timestamp text-nowrap'>%s</div></td>", func_convert_timestamp( $tt['data'][count($tt['data'])-1]['ev_Timestamp'] ) );
                    	else
                    	    printf( "<td><div class='timestamp text-nowrap'>?</div></td>" );
                    	
                    	$img = "&nbsp;&nbsp;&nbsp;";
                    	if ( func_find_graph_device( $tt['di_DeviceNo'], $tt['di_IOChannel'] ) )
                    	{
                    		$img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
                    	}
                    	printf( "<td>%s</td>", $img );
                    	
                    	printf( "</tr>" );
                    }
                }
    		}
            ?>
            
            </table>
            

			<?php
			if ( count($levels) > 0 )
			{
			$title = "Levels";
			printf( "<h3>%s</h3>", $title );
			?>
            
            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Name</th>
              <th>Value</th>
              <th>Date</th>
              <th>Graph</th>
              </tr>
            </thead>
    
    		<?php         
            foreach ( $levels as $tt )
            {
                $val = 0;
                if ( count($tt['data']) > 0 )
                    $val = func_calc_level( $tt['data'][count($tt['data'])-1]['ev_Value'] );
                $class = "";
                if ( $tt['di_MonitorLo'] != 0.0 && $tt['di_MonitorHi'] != 0.0 && ($val < $tt['di_MonitorLo'] || $val > $tt['di_MonitorHi']) )
                    $class = "table-danger";
                printf( "<tr class='%s'>", $class );
            	printf( "<td><div class='text-nowrap'><a href='?GraphDeviceNo=%d&GraphIOChannel=%d'>%s</a></div></td>", $tt['di_DeviceNo'], $tt['di_IOChannel'], $tt['di_IOName'] );
            	printf( "<td>%s%s</td>", $val, "%" );
            	if ( count($tt['data']) > 0 )
                	printf( "<td><div class='timestamp text-nowrap'>%s</div></td>", func_convert_timestamp( $tt['data'][count($tt['data'])-1]['ev_Timestamp'] ) );
            	else
            	    printf( "<td><div class='timestamp text-nowrap'>?</div></td>" );
            	
            	$img = "&nbsp;&nbsp;&nbsp;";
            	if ( func_find_graph_device( $tt['di_DeviceNo'], $tt['di_IOChannel'] ) )
            	{
            		$img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
            	}
            	printf( "<td>%s</td>", $img );
            	
            	printf( "</tr>" );
            }
            
            printf( "</table>" );
            }
            ?>
            
        </div>
        </div>


		<!-- *************************************************************************** -->
        <div class="col-sm-4">
		<div id="row_dvt" class="collapse  <?php echo $show_row_dvt; ?>">
            <h3>Temperatures</h3>
            
            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Name</th>
              <th>Value</th>
              <th>Date</th>
              <th>Graph</th>
              </tr>
            </thead>

            <?php   
            foreach ( $temperatures as $tt )
            {
                $val = 0;
                if ( count($tt['data']) > 0 )
                    $val = func_calc_temperature( $tt['data'][count($tt['data'])-1]['ev_Value'] );
                $class = "";
                if ( $tt['di_MonitorLo'] != 0.0 && $tt['di_MonitorHi'] != 0.0 && ($val < $tt['di_MonitorLo'] || $val > $tt['di_MonitorHi']) )
                    $class = "table-danger";
                    
                printf( "<tr class='%s'>", $class );
            	
            	if ( isset($tt['data'][count($tt['data'])-1]) )
            	{
            		printf( "<td><div class='text-nowrap'><a href='?GraphDeviceNo=%d&GraphIOChannel=%d'>%s</a></div></td>", $tt['di_DeviceNo'], $tt['di_IOChannel'], $tt['di_IOName'] );
            		printf( "<td>%s&#8451</td>", $val );
            		printf( "<td><div class='timestamp text-nowrap'>%s</div></td>", func_convert_timestamp( $tt['data'][count($tt['data'])-1]['ev_Timestamp'] ) );
            	}
            	else
            	{
            		printf( "<td>%s</td>", $tt['di_IOName'] );
            		printf( "<td>? C</td>" );
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
            ?>
                
            </table>
        </div>
        </div>

	</div> <!-- row -->
	
	
	<?php
    
	if ( $_SESSION['ShowCameraNo'] == "" )
	{	// show graph
	    
	    if ( count($_SESSION['GraphDevices']) != 0 )
	    {
	   ?>


        <?php 
        
        // graph goes here
        $alert_width = "8";     // percent
        $graph_bgcolor = "#AABBCC";
        $alert_okcolor = "#58D68D";
        $alert_ngcolor = "#FE2E2E";
        
        $datetime = func_get_graph_datetime();
        
        $temperatures = $db->GetLatestTemperatures( $_SESSION['GraphHours'], $datetime );
        $voltages = $db->GetLatestVoltages( $_SESSION['GraphHours'], $datetime );
        $levels = $db->GetLatestLevels( $_SESSION['GraphHours'], $datetime );
        $currents = $db->GetLatestCurrents( $_SESSION['GraphHours'], $datetime );
        $powers = $db->GetLatestPowers( $_SESSION['GraphHours'], $datetime );
        $frequencies = $db->GetLatestFrequencies( $_SESSION['GraphHours'], $datetime );
        
        $g_devices = func_get_graph_devices( $temperatures, $voltages, $levels, $currents, $powers, $frequencies );
        
        $g_data = func_get_graph_data( $temperatures, $voltages, $levels, $currents, $powers, $frequencies, $g_devices );
        func_draw_graph_div( true, "graphdiv", 1, $alert_width, $graph_bgcolor, $alert_okcolor, $alert_ngcolor, $g_data );
        func_create_graph( $g_data, "graphdiv" );
        ?>
		<div class="row">

            <!-- ***************************************************************************
            -->
			<div class="col-sm-12">

            <?php 
            printf( "<p>" );
            printf( "<a href='?Hours=1'><input class='btn btn-outline-dark %s' type='button' name='Hours1' value='1 Hour'></a> ", ($_SESSION['GraphHours'] == 1 ? "btn-info" : "") );
            printf( "<a href='?Hours=3'><input class='btn btn-outline-dark %s' type='button' name='Hours3' value='3 Hours'></a> ", ($_SESSION['GraphHours'] == 3 ? "btn-info" : "") );
            printf( "<a href='?Hours=6'><input class='btn btn-outline-dark %s' type='button' name='Hours6' value='6 Hours'></a> ", ($_SESSION['GraphHours'] == 6 ? "btn-info" : "") );
            printf( "<a href='?Hours=12'><input class='btn btn-outline-dark %s' type='button' name='Hours12' value='12 Hours'></a> ", ($_SESSION['GraphHours'] == 12 ? "btn-info" : "") );
            printf( "<a href='?Hours=24'><input class='btn btn-outline-dark %s' type='button' name='Hours24' value='24 Hours'></a> ", ($_SESSION['GraphHours'] == 24 ? "btn-info" : "") );
            printf( "<a href='?Hours=48'><input class='btn btn-outline-dark %s' type='button' name='Hours48' value='48 Hours'></a> ", ($_SESSION['GraphHours'] == 48 ? "btn-info" : "") );
            printf( "<a href='?GraphMinusHour'><input class='btn btn-outline-dark' type='button' name='GraphMinusHour' value='-1 hrs'></a> " );
            printf( "<a href='?GraphMinusDay'><input class='btn btn-outline-dark' type='button' name='GraphMinusDay' value='-24 hrs'></a> " );
            printf( "<a href='?GraphPlusDay'><input class='btn btn-outline-dark' type='button' name='GraphPlusDay' value='+24 hrs'></a> " );
            printf( "<a href='?GraphPlusHour'><input class='btn btn-outline-dark' type='button' name='GraphPlusHour' value='+1 hrs'></a> " );
            printf( "</p>" );
            
            printf( "<p>" );
            printf( "Historic: Date <input class='form-control' type='text' name='GStartDate' value='%s' size='8'> ", $_SESSION['GStartDate'] );
            printf( "Time <input class='form-control' type='text' name='GStartTime' value='%s' size='5'> ", $_SESSION['GStartTime'] );
            printf( "<a href='?GraphGo'><input class='btn btn-outline-dark' type='submit' name='GraphGo' value='Go'></a> " );
            printf( "<a href='?CurrentGraph'><input class='btn btn-outline-dark' type='button' name='CurrentGraph' value='Graph Now'></a> " );
            printf( "<a href='?ClearGraph&RefreshEnabled'><input class='btn btn-outline-dark' type='button' name='ClearGraph' value='Clear'></a> " );
            printf( "</p>" );
            ?>

			</div>
		</div> <!-- row -->

	<?php
	    }
	}
	?>
	
    <?php
    $show_row_cfd = "";
    if ( $_SESSION['ShowCameraFile'] == "" && $_SESSION['ShowCameraNo'] == "" && count($_SESSION['GraphDevices']) == 0 )
    {
        $show_row_cfd = "show";
    }
    ?>	
	

	<!-- *************************************************************************** -->
	<div class="row">

		<!-- *************************************************************************** -->
        <div class="col-sm-4" id='Control'>
			<div class='row'>
				<div class='col'>
    				<h3>Control</h3>
    			</div>
    			<div class='col'>
					<a href='#row_cfd' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
				</div>
			</div>

			<div id="row_cfd" class="collapse  <?php echo $show_row_cfd; ?>">

            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Name</th>
              <th>Output</th>
              </tr>
            </thead>
            <?php
            foreach ( $deviceinfo_list as $dinfo )
            {
                $ok = false;
                $on = false;
                $outname = "";
                if ( isset($_POST['WebClick']) )
                    $outname = "!";
                switch ( $dinfo['di_IOType'] )
                {
                default:
                    break;
                case E_IO_ON_TIMER:
                case E_IO_TOGGLE:
                case E_IO_ON_OFF_TIMER:
                    //$ok = true;
                    
                    // find any io links
                    foreach ( $iolinks_list as $ilink )
                    {
                        if ( $ilink['il_InDeviceNo'] == $dinfo['di_DeviceNo'] && $ilink['il_InChannel'] == $dinfo['di_IOChannel'] )
                        {
                            if ( $outname != "" )
                                $outname .= ",";
                            $outname .= $ilink['di_IOName'];
                            
                            if ( $ilink['di_OutOnStartTime'] != 0 )
                                $on = true;
                            
                            $ok = true;
                        }
                    }
                    break;
                }
                
                if ( $ok )
                {
                    printf( "<tr>" );
                    
                    printf( "<td><a class='btn btn-outline-dark %s' href='?WebClick%02d%02d=%d#Control'>%s</a></td>", 
                        ($on ? "btn-success" : ""), $dinfo['di_DeviceNo'], $dinfo['di_IOChannel'], time(), $dinfo['di_IOName'] );
                    printf( "<td>%s<br><div class='small %s'>%s</div></td>", $outname, ($on ? "text-success" : "text-warning"), ($on ? "(on)" : "(off)") );
                    
                    printf( "</tr>" );
                }
            }
            ?>
            </table>

        </div>
        </div>

		<!-- *************************************************************************** -->
        <div class="col-sm-4">
		<div id="row_cfd" class="collapse  <?php echo $show_row_cfd; ?>">
            <h3>Failures</h3>
            
            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Name</th>
              <th>Last Failure</th>
              </tr>
            </thead>

            <?php 
            $printed = false;
            $devices2 = $devices;
            $devices2[] = array( 'de_DeviceNo'=>-3, 'de_Name'=>'Failed Logins' );
            foreach ( $devices2 as $dd )
            {
                $failures = $db->GetDeviceFailures( $dd['de_DeviceNo'] );
                
            	if ( count($failures) > 0 )
            	{
            		$printed = true;
            		printf( "<tr>" );
            		
            		printf( "<td><b>%s %d</b></td>", $dd['de_Name'], count($failures) );
            		
            		$onclick = sprintf( "return confirm(\"Are you sure you want to delete this event ?\")" );
            		printf( "<td>" );
            		printf( "<a href='?DeleteEventNo=%d' onclick='%s;'>%s</a>", $failures[0]['ev_EventNo'], $onclick, $failures[0]['ev_Timestamp'] );
            		
            		printf( "&nbsp;&nbsp;" );
            		
            		$onclick = sprintf( "return confirm(\"Are you sure you want to delete all failure events ?\")" );
            		printf( "<a href='?DeleteAllEventNo=%d' onclick='%s;'>All</a>", $failures[0]['ev_EventNo'], $onclick );
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
            ?>
            
            </table>
        </div>
        </div>

		<!-- *************************************************************************** -->
        <div class="col-sm-4">
		<div id="row_cfd" class="collapse  <?php echo $show_row_cfd; ?>">
            <h3>Database</h3>
            
            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Table</th>
              <th>Records</th>
              </tr>
            </thead>

            <?php 
            foreach( $records as $record )
            {
                printf( "<tr>" );
                
                printf( "<td>%s</td><td>%d</td>", $record['table'], $record['count'] );
                
                printf( "</tr>" );
            }
            ?>

            </table>
        </div>
        </div>
	
	</div> <!-- row -->
	
	
    <?php 
    }   // end of if ( count($devices) == 0 )

    if ( count($camera_list) > 0 && func_user_feature_enabled(E_UF_CAMERAS) && func_user_feature_enabled(E_UF_HOMECAMERAS) )
    {
        printf( "<!-- *************************************************************************** -->" );
    	printf( "<div class='row'>" );
    	printf( "<h3>Cameras</h3><br>" );
    	printf( "</div> <!-- row -->" );

    	printf( "<!-- *************************************************************************** -->" );
    	printf( "<div class='row'>" );
    	
        foreach( $camera_list as $camera )
        {
            printf( "<div class='col'>" );
            
            $file = sprintf( "%s/latest_snapshot.jpg", func_make_camera_web_dir( "", $camera['ca_Directory'] ) );
            
            $count = func_get_mdalarm_count($camera['ca_Directory']);
            
            $stat = stat( sprintf( "%s/latest_snapshot.jpg", $camera['ca_Directory'] ) );
            if ( $stat !== false )
            {
                $class = "";
                if ($stat['mtime'] + 60*3 < time() )
                {
                    $class = "text-danger";
                }
                $class2 = "";
                if ( $count <= 0 )
                {
                    $class2 = "text-danger";
                }
                $date = getdate( $stat['mtime'] );
                printf( "%s: <span class='%s'>%02d/%02d/%d %02d:%02d:%02d</span> - </span class='%s'>%d files today</span><br>", $camera['ca_Name'], $class, 
                    $date['mday'], $date['mon'], $date['year'], $date['hours'], $date['minutes'], $date['seconds'], $class2, $count );
            }
            printf( "<img src='%s' width='400px' alt='No snapshot from %s'>", $file, $camera['ca_Name'] );
        
		    printf( "</div>" );
        }
    	printf( "</div> <!-- row -->" );
    }
    
    if ( count($camera_list) > 0 )
    {
    ?>
	<!-- *************************************************************************** -->
	<div class="row">

		<!-- *************************************************************************** -->
        <div class="col-sm-2">
            <h3>Cameras</h3>
            
            <table class='table table-striped'>
            <thead class="thead-light">
              <tr>
              <th>Camera</th>
              <th></th>
              </tr>
            </thead>

            <?php 
            $camera_dir = "";
            foreach ( $camera_list as $camera )
            {
                printf( "<tr>" );
                if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )    
                    printf( "<td><a href='?CameraNo=%s'>%s</a></td>", $camera['ca_IPAddress'], $camera['ca_Name'] );
                else
                    printf( "<td><div class='text-muted'>%s</div></td>", $camera['ca_Name'] );
                $img = "&nbsp;&nbsp;&nbsp;";
                if ( $_SESSION['ShowCameraNo'] == $camera['ca_IPAddress'] )
                {
                    $camera_dir = $camera['ca_Directory'];
                    $img = sprintf( "<img src='./images/green_tick.png' height='15px'>" );
                }
                printf( "<td>%s</td>", $img );
                printf( "</tr>" );
            }
            ?>

            </table>
        </div>

        <?php
        if ( $_SESSION['ShowCameraNo'] != "" )
        {
        ?>
		<!-- *************************************************************************** -->
        <div class="col-sm-6">
            <h3>Video Files</h3>
    		<?php
    		if ( count($camera_files) > 0 )
    		{
                printf( "<div id='cameragraph' class='chart'></div><br><a href='' id='cameragraphclick'></a>" );
                func_create_camera_graph( $camera_files, "cameragraph", $_SESSION['ShowCameraNo'] );
                
                if ( $_SESSION['ShowCameraFile'] != "" )
                {
                    printf( "%s <div class='small'>(%s)</div><br>", func_get_date_from_video($_SESSION['ShowCameraFile']), $_SESSION['ShowCameraFile'] );
                    $filemkv = sprintf( "%s/%s%s", $camera_dir, $camera_files[0]['dir'], $_SESSION['ShowCameraFile'] );
                    $expl = explode(".", $_SESSION['ShowCameraFile'] );
                    $basemp4 = sprintf( "%s.mp4", $expl[0] );
                    $camera_webdir = func_make_camera_web_dir( $_SERVER['DOCUMENT_ROOT'], $camera_dir );
                    $filemp4 = sprintf( "%s/%s%s", $camera_webdir, $camera_files[0]['dir'], $basemp4 );
                    if ( !file_exists($filemp4) )
                    {
                        $cmd = sprintf( "ffmpeg -hide_banner -loglevel warning -i %s -codec copy %s", $filemkv, $filemp4 );
                        system( $cmd );
                        //printf( $cmd );
                    }
                    $filemp4x = sprintf( "%s/%s%s", substr($camera_webdir,strlen($_SERVER['DOCUMENT_ROOT'])), $camera_files[0]['dir'], $basemp4 );
                    
                    printf( "<video width='400' height='225' controls type='video/mp4'>" );
        		    printf( "<source src='./%s'>", $filemp4x );
        		    printf( "</video>" );
                }
    		}
    		else
    		{
    		    printf( "No video files available" );
    		}
    		?>
        </div>
        <?php
        }
        ?>
	
	</div> <!-- row -->
    <?php 
    }
    ?>
	
</div>





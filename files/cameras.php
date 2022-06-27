<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------

// ptz commands
// cgi-bin/CGIProxy.fcgi?cmd=ptzMoveDown&usr=camuser&pwd=*****
// ptzMoveDown
// ptzMoveUp
// ptzMoveLeft
// ptzMoveRight
// ptzMoveTopLeft
// ptzMoveTopRight
// ptzMoveBottomLeft
// ptzMoveBottomRight
// ptzReset
// ptzStopRun
// ptzSetPTZSpeed&speed=x		where speed = 0 (v. slow) to 4 (v. fast)


include_once( "common.php" );

if ( !isset($_SESSION['us_AuthLevel']) || !func_user_feature_enabled(E_UF_CAMERAS) )
{	// access not via main page - access denied
    func_unauthorisedaccess();
    return;
}


function func_clear_ca_array( &$ca_array )
{
    $ca_array['ca_CameraNo'] = 0;
    $ca_array['ca_Name'] = "";
    $ca_array['ca_IPAddress'] = "";
    $ca_array['ca_PTZ'] = "N";
    $ca_array['ca_Encoding'] = "";
    $ca_array['ca_Directory'] = "";
    $ca_array['ca_UserId'] = "";
    $ca_array['ca_Password'] = "";
    $ca_array['ca_Password2'] = "";
    $ca_array['ca_Model'] = "";
    $ca_array['ca_MJpeg'] = "";
    $ca_array['info_msg'] = "";
    $ca_array['error_msg'] = "";
}

function func_check_ca_array(&$ca_array)
{
    $ca_array['error_msg'] = "";
    $ca_array['info_msg'] = "";
    
    if ($ca_array['ca_Name'] === "") 
    {
        $ca_array['error_msg'] = "You must enter the camera name.";
        return false;
    }
    else if ($ca_array['ca_IPAddress'] === "")
    {
        $ca_array['error_msg'] = "You must enter the camera IP Address.";
        return false;
    }
    else if ($ca_array['ca_Encoding'] === "")
    {
        $ca_array['error_msg'] = "You must enter the camera video encoding.";
        return false;
    }
    else if ($ca_array['ca_Directory'] === "")
    {
        $ca_array['error_msg'] = "You must enter the camera file system directory name.";
        return false;
    }
    else if ($ca_array['ca_UserId'] === "")
    {
        $ca_array['error_msg'] = "You must enter the camera User Id.";
        return false;
    }
    else if ( ($ca_array['ca_Password'] === "" && $ca_array['ca_Password2'] !== "") ||
        ($ca_array['ca_Password'] !== "" && $ca_array['ca_Password2'] === "" ) )
    {
        $ca_array['error_msg'] = "You must enter both the Password and Confirmation or leave them both blank.";
        return false;
    }
    else if ( ($ca_array['ca_Password'] === "" || $ca_array['ca_Password2'] === "" || $ca_array['ca_Password'] != $ca_array['ca_Password2']) && $ca_array['ca_CameraNo'] == 0 )
    {
        $ca_array['error_msg'] = "You must enter both the Password and Confirmation to add an new camera record.";
        return false;
    }
    
    return true;
}

function func_supports_mjpeg( $camera_list, $ip )
{
    $rc = "N";
    
    foreach ( $camera_list as $camera )
    {
        if ( $camera['ca_IPAddress'] == $ip )
        {
            $rc = $camera['ca_MJpeg'];
            break;
        }
    }
    
    return $rc;
}

function func_find_camera( $camera_list, $ip )
{
    foreach ( $camera_list as $camera )
    {
        if ( $camera['ca_IPAddress'] == $ip )
        {
            return $camera;
        }
    }
    
    return false;
}

function func_is_camera_showing( $camera_ip )
{
    $found = false;
    foreach ( $_SESSION['show_camera_list'] as $ip )
    {
        if ( $ip == $camera_ip )
        {
            $found = true;
            break;
        }
    }
    
    return $found;
}

function func_total_cameras_showing()
{
    $count = 0;
    for ( $i = 0; $i < MAX_CAMERAS; $i++ )
    {
        if ( $_SESSION['show_camera_list'][$i] != "" )
        {
            $count += 1;
        }
    }
    
    return $count;
}

function func_display_camera( $camera_ip, $width, $ptz, $camuser, $campwd, $supports_mjpeg )
{
    $height = $width * 9 / 16;
	$set_camera_mode = true;
	
	if ( $set_camera_mode )
	{	// set MJpeg stream
	    // format=1 for MJStream, format=0 for H264
	    $parms = sprintf( "cmd=setSubStreamFormat&format=1&usr=%s&pwd=%s", $camuser, $campwd );
	    $uparms = urlencode( $parms );
	    $cmd = sprintf( "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\"", $camera_ip, $uparms );
		$res = exec( $cmd, $out, $ret );
	}
	
	if ( $supports_mjpeg == "N" || ($supports_mjpeg == "Y" && $_SERVER['HTTPS'] == "on") || func_is_external_connection() )
	{	// external web connection
	    $parms = sprintf( "cmd=snapPicture2&usr=%s&pwd=%s", $camuser, $campwd );
	    $uparms = urlencode( $parms );
	    $cmd = sprintf( "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\"", $camera_ip, $uparms );
		//echo $cmd;
	
		if ( $ptz )
		{
			//printf( "<a href='?MoveCamera'><img src='data:image/jpeg;base64," );
			printf( "<img usemap='#cameramap' src='data:image/jpeg;base64," );
		}
		else
		{
			printf( "<img src='data:image/jpeg;base64," );
		}
		
		ob_start();
		passthru( $cmd, $out );
		$var = ob_get_contents();
		ob_end_clean();
		echo base64_encode($var);
	
		if ( $ptz )
		{
			//printf( "' alt='no snap shot' width='%d'></a>", $width );
			printf( "' alt='no snap shot (%s)' width='%d'>", $camera_ip, $width );
			printf( "<map name='cameramap'>" );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveLeft'>", 0, $height*0.25, $width*0.5, $height*0.75 );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveRight'>", $width*0.5, $height*0.25, $width, $height*0.75 );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveUp'>", $width*0.25, 0, $width*0.75, $height*0.25 );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveDown'>", $width*0.25, $height*0.75, $width*0.75, $height );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveTopLeft'>", 0, 0, $width*0.25, $height*0.25 );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveBottomLeft'>", 0, $height*0.75, $width*0.25, $height );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveTopRight'>", $width*0.75, 0, $width, $height*0.25 );
			printf( "<area shape='rect' coords='%d,%d,%d,%d' alt='camera click' href='index.php?MoveCamera=ptzMoveBottomRight'>", $width*0.75, $height*0.75, $width, $height );
			printf( "</map>" );
		}
		else
		{
			printf( "' alt='no snap shot (%s)' width='%d'>", $camera_ip, $width );
		}
	}
	else
	{   // MJStream on works for some foscam cameras if we are not using https
	    // when using https we cannot display http images from other hosts due to CORS browser security restrictions
		$dest = sprintf( "%s:88", $camera_ip );
		if ( $_SERVER['HTTPS'] == "on" )
		{ 
		    $dest = sprintf( "%s:88", $camera_ip );
		}
		
		if ( $ptz )
		{
		    $parms = sprintf( "cmd=getMJStream&usr=%s&pwd=%s", $camuser, $campwd );
		    $uparms = urlencode( $parms ) ;
		    printf( "<a href='?MoveCamera'><img src='http://%s/cgi-bin/CGIStream.cgi?%s' alt='no mjpeg stream (%s)' width='%d' ismap></a>",
			    $dest, $parms, $dest, $width );
		}
		else
		{
		    $parms = sprintf( "cmd=getMJStream&usr=%s&pwd=%s", $camuser, $campwd );
		    $uparms = urlencode( $parms );
		    $src = sprintf( "http://%s/cgi-bin/CGIStream.cgi?%s", $dest, $parms );
		    printf( "<img src='%s' alt='no mjpeg stream (%s)' width='%d'>",
			    $src, $dest, $width );
		    printf( "%s", $src );
		}
	}
}

if ( !isset($_SESSION['CameraSearch']) )
    $_SESSION['CameraSearch'] = false;
if ( !isset($_SESSION['SearchCameraFile']) )
    $_SESSION['SearchCameraFile'] = "";
if ( !isset($_SESSION['CameraDate']) )
    $_SESSION['CameraDate'] = getdate();
        
    
$ca_array = array();
func_clear_ca_array( $ca_array );

$ptz_x = 0;
$ptz_y = 0;

$new_camera = false;
$camera_list = $db->ReadCameraList();

    
if ( isset($_POST['ca_CameraNo']) )
    $ca_array['ca_CameraNo'] = $_POST['ca_CameraNo'];
if ( isset($_POST['ca_Name']) )
    $ca_array['ca_Name'] = $_POST['ca_Name'];
if ( isset($_POST['ca_IPAddress']) )
    $ca_array['ca_IPAddress'] = $_POST['ca_IPAddress'];
if ( isset($_POST['ca_PTZ']) )
    $ca_array['ca_PTZ'] = "Y";
if ( isset($_POST['ca_Encoding']) )
    $ca_array['ca_Encoding'] = $_POST['ca_Encoding'];
if ( isset($_POST['ca_Directory']) )
    $ca_array['ca_Directory'] = $_POST['ca_Directory'];
if ( isset($_POST['ca_UserId']) )
    $ca_array['ca_UserId'] = $_POST['ca_UserId'];
if ( isset($_POST['ca_Password']) )
    $ca_array['ca_Password'] = $_POST['ca_Password'];
if ( isset($_POST['ca_Password2']) )
    $ca_array['ca_Password2'] = $_POST['ca_Password2'];
if ( isset($_POST['ca_Model']) )
    $ca_array['ca_Model'] = $_POST['ca_Model'];
if ( isset($_POST['ca_MJpeg']) )
    $ca_array['ca_MJpeg'] = substr($_POST['ca_MJpeg'],0,1);
        
if ( isset($_GET['CameraSearch']) )
{
    $_SESSION['CameraSearch'] = true;
    $_SESSION['CameraDate'] = getdate();
}
else if ( isset($_GET['HideCameraSearch']) )
{
    $_SESSION['CameraSearch'] = false;
    $_SESSION['SearchCameraFile'] = "";
}

if ( isset($_GET['CameraFile']) )
{
    $_SESSION['SearchCameraFile'] = $_GET['CameraFile'];
}


if ( isset($_GET['CameraCycle']) )
{
    if ( $_SESSION['show_camera_list'][1] != "" )
    {
        $info_msg = "The 'Camera Cycle' option is only valid when displaying a single camera image";
    }
    else
    {
        $found = 0;
        foreach ( $camera_list as $camera )
        {
            if ( $found == 1 )
            {
                $found = 2;
                $_SESSION['show_camera_list'][0] = $camera['ca_IPAddress'];
                break;
            }
            else if ( $camera['ca_IPAddress'] == $_SESSION['show_camera_list'][0] )
            {
                $found = 1;
            }
        }
        if ( $found != 2 )
        {
            $_SESSION['show_camera_list'][0] = $camera_list[0]['ca_IPAddress'];
        }
    }
}
else if ( isset( $_POST['SelCamera']) )
{
	$camera_ip = "";
	$name = $_POST['SelCamera'];

	// find the camera
	foreach ( $camera_list as $camera )
	{
		if ( $camera['ca_Name'] == $name )
		{
		    $camera_ip = $camera['ca_IPAddress'];
			break;
		}
	}
	
	if ( $_SESSION['CameraSearch'] )
	{  // only show one camera when searching
	    $_SESSION['SearchCameraFile'] = "";
	    if ( $camera_ip == $_SESSION['show_camera_list'][0] )
	    {
	        $_SESSION['CameraSearch'] = false;
	    }
	    else
	    {
    	    for ( $i = 0; $i < MAX_CAMERAS; $i++ )
	        {
	            $_SESSION['show_camera_list'][$i] = "";
	        }
	    }
	}
	
	if ( $camera_ip != "" )
	{
		$found = false;
		for ( $i = 0; $i < MAX_CAMERAS; $i++ )
		{
			if ( $found )
			{	// remove holes in the list
				if ( $i+1 < MAX_CAMERAS )
				{
					$_SESSION['show_camera_list'][$i] = $_SESSION['show_camera_list'][$i+1];
				}
			}
			else if ( $_SESSION['show_camera_list'][$i] == $camera['ca_IPAddress'] )
			{	// already in the list - remove it
				$found = true;
				$_SESSION['show_camera_list'][$i] = "";
				if ( $i+1 < MAX_CAMERAS )
				{
					$_SESSION['show_camera_list'][$i] = $_SESSION['show_camera_list'][$i+1];
				}
				$_SESSION['CameraSearch'] = false;
			}
			else if ( $_SESSION['show_camera_list'][$i] == "" )
			{	// empty slot - add the camera
				$_SESSION['show_camera_list'][$i] = $camera_ip;
				break;
			}
		}
	}
}
else if ( isset($_GET['ClearCameras']) )
{
	for ( $i = 0; $i < MAX_CAMERAS; $i++ )
	{
		$_SESSION['show_camera_list'][$i] = "";
	}
}
else if ( isset($_POST['CameraDate']) )
{
    $expl = explode( "/", $_POST['CameraDate'] );
    if ( isset($expl[0]) && isset($expl[1]) && isset($expl[2]) )
    {
        $_SESSION['CameraDate']['year'] = $expl[2];
        $_SESSION['CameraDate']['mon'] = $expl[1];
        $_SESSION['CameraDate']['mday'] = $expl[0];
    }
    $_SESSION['SearchCameraFile'] = "";
}
else if ( isset($_POST['AddCamera']) )
{
    $new_camera = true;
}
else if ( isset($_POST['CancelCamera']) )
{
    func_clear_ca_array( $ca_array );
}
else if ( isset($_POST['RebootCamera']) )
{
    $ip = $_SESSION['show_camera_list'][0];
    $camera = func_find_camera( $camera_list, $ip );
    
    $parms = sprintf( "cmd=rebootSystem&format=1&usr=%s&pwd=%s", $camera['ca_UserId'], $camera['ca_Password'] );
    $uparms = urlencode( $parms );
    $cmd = sprintf( "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\"", $camera['ca_IPAddress'], $uparms );
    $res = exec( $cmd, $out, $ret );
    
    //echo $cmd;
}
else if ( isset($_POST['EditCamera']) )
{
    $ip = $_SESSION['show_camera_list'][0];
    $camera = func_find_camera( $camera_list, $ip );
    
    func_clear_ca_array( $ca_array );
    
    $ca_array['ca_CameraNo'] = $camera['ca_CameraNo'];
    $ca_array['ca_Name'] = $camera['ca_Name'];
    $ca_array['ca_IPAddress'] = $camera['ca_IPAddress'];
    $ca_array['ca_PTZ'] = $camera['ca_PTZ'];
    $ca_array['ca_Encoding'] = $camera['ca_Encoding'];
    $ca_array['ca_Directory'] = $camera['ca_Directory'];
    $ca_array['ca_UserId'] = $camera['ca_UserId'];
    $ca_array['ca_Password'] = "";
    $ca_array['ca_Password2'] = "";
    $ca_array['ca_Model'] = $camera['ca_Model'];
    $ca_array['ca_MJpeg'] = $camera['ca_MJpeg'];
}
else if ( isset($_POST['UpdateCamera']) | isset($_POST['NewCamera']) )
{
    $ok = true;
    if ( isset($_POST['NewCamera']) )
    {
        if ( $db->FindCameraByNameOrIPAddress( $ca_array['ca_Name'], $ca_array['ca_IPAddress'] ) )
        {   // camera name or ip address already exists
            $ok = false;
            $ca_array['error_msg'] = sprintf( "A camera with the name '%s' or IP Address '%s' already exists.", $ca_array['ca_Name'], $ca_array['ca_IPAddress'] );
        }
    }
    
    if ( $ok && func_check_ca_array( $ca_array) )
    {
        if ( $db->SaveCameraRecord( $ca_array['ca_CameraNo'], $ca_array['ca_Name'], $ca_array['ca_IPAddress'], $ca_array['ca_PTZ'], $ca_array['ca_Encoding'],   
            $ca_array['ca_Directory'], $ca_array['ca_UserId'], $ca_array['ca_Password'], $ca_array['ca_Model'], $ca_array['ca_MJpeg'] ) )
        {   // success
            $ca_array['info_msg'] = sprintf( "Camera record saved successfully" );
        
            $camera_list = $db->ReadCameraList();
        }
        else
        {
            $ca_array['error_msg'] = sprintf( "Failed to save the camera record !" );   
        }
    }
    else if ( $ca_array['ca_CameraNo'] == 0 )
    {
        $new_camera = true;
    }
}
else if ( isset($_POST['DeleteCamera']) )
{
    $db->DeleteCamera( $ca_array['ca_CameraNo'] );
    
    func_clear_ca_array( $ca_array );
    
    $_SESSION['show_camera_list'][0] = "";
    
    $camera_list = $db->ReadCameraList();
}

if ( isset($_GET['MoveCamera']) )
{	// move the camera
	$camera_ip = "";
	foreach ( $_SESSION['show_camera_list'] as $ip )
	{
		if ( $ip != "" )
		{
			$camera_ip = $ip;
			break;
		}
	}
	
	// image is 900 x 680
	$move = $_GET['MoveCamera'];
	
	if ( $camera_ip != "" )
	{
	    $camera = func_find_camera( $camera_list, $camera_ip );
		$info_msg = $move;
		if ( $move == "ptzReset" )
		{
		    $parms = urlencode( sprintf( "cmd=%s&usr=%s&pwd=%s", $move, $camera['ca_UserId'], $camera['ca_Password'] ));
		    $cmd = sprintf( "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\" &",
			    $camera['ca_IPAddress'], $parms );
		}
		else 
		{
		    $parms = sprintf( "cmd=%s&usr=%s&pwd=%s", $move, $camera['ca_UserId'], $camera['ca_Password'] );
		    $uparms = urlencode( $parms );
		    $parms2 = sprintf( "cmd=%s&usr=%s&pwd=%s", "ptzStopRun", $camera['ca_UserId'], $camera['ca_Password'] );
		    $uparms2 = urlencode( $parms2 );
		    $cmd = sprintf( "(curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\" > /dev/null 2>&1;
						sleep 1 > /dev/null 2>&1;	curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\" > /dev/null 2>&1; sleep 0.2) > /dev/null 2>&1 &", 
			            $camera['ca_IPAddress'], $uparms, $camera['ca_IPAddress'], $uparms2 );
		    //$cmd = sprintf( "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\" > /dev/null 2>&1",
		    //    $camera['ca_IPAddress'], $uparms );
		}
		//echo $cmd;
		$res = system( $cmd, $ret );
	}
}


$camera_addr = "0";
$camera_dir = "";
$camera_files = array();
foreach ( $camera_list as $camera )
{
    if ( func_is_camera_showing($camera['ca_IPAddress']) )
    {
        $camera_addr = $camera['ca_IPAddress'];
        $camera_dir = $camera['ca_Directory'];
        $camera_files = func_read_camera_files( $camera['ca_Directory'], $_SESSION['CameraDate']['year'], $_SESSION['CameraDate']['mon'], $_SESSION['CameraDate']['mday'] );
        break;
    }
}


?>

<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-2">
			<h3>Cameras</h3>
		</div>

		<div class="col-sm-3">
			<?php
	        if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
	        {
    		?>
			<a href='?ClearCameras'>Clear</a>
			&nbsp;
   			<?php 
    			if ( func_total_cameras_showing() == 1 )
    			{
        			if ( $_SESSION['CameraSearch'] )
        			    printf( "<a href='?HideCameraSearch'>Hide Search</a>" );
        			    else if ( $ca_array['ca_CameraNo'] == 0 && !$new_camera )
        			    printf( "<a href='?CameraSearch'>Search</a>" );
			    }
	        }
	        ?>
		</div>

		<div class="col-sm-3">
        <?php
        if ( $_SESSION['CameraSearch'] )
        {
            printf( "<input type='text' class='form-control' id='CameraDate' name='CameraDate' size='8' placeholder='Search Date' value=%02d/%02d/%d>", 
                $_SESSION['CameraDate']['mday'], $_SESSION['CameraDate']['mon'], $_SESSION['CameraDate']['year'] );
            printf( "<button type='submit' class='btn btn-info' id='SearchDate' name='SearchDate'>Go</button>" );
        }
        else if ( $ca_array['ca_CameraNo'] == 0 && !$new_camera )
        {
        ?>
			<div class="custom-control custom-checkbox">
    			<input type="checkbox" class="custom-control-input" id="CameraRefresh" name="CameraRefresh" <?php if(isset($_GET['CameraRefresh'])) echo "checked"?>>
    			<label class="custom-control-label" for="CameraRefresh">Refresh 10sec</label>
  			</div>
			<div class="custom-control custom-checkbox">
    			<input type="checkbox" class="custom-control-input" id="CameraCycle" name="CameraCycle" <?php if(isset($_GET['CameraCycle'])) echo "checked"?>>
    			<label class="custom-control-label" for="CameraCycle">Cycle</label>
  			</div>
		<?php
        }
		?>
		</div>
	</div>

    <!-- *************************************************************************** -->
	<div class="row">
		<div class="col-sm-6">
		
    		<?php 
    		if ( $ca_array['error_msg'] != "" )
    		    printf( "<div class='text-error'>%s</div>", $ca_array['error_msg'] );
    		else if ( $ca_array['info_msg'] != "" )
    		    printf( "<div class='text-info'>%s</div>", $ca_array['info_msg'] );
    		?>
		</div>
	</div>

    <?php
    if ( $ca_array['ca_CameraNo'] == 0 && !$new_camera )
    {
        printf( "<!-- *************************************************************************** -->" );
		printf( "<div class='row mb-2'>" );
		printf( "<div class='col-sm-6'>" );
	
        $count = 0;
        foreach ( $camera_list as $camera )
        {
        	$count += 1;
        	$class = "";
        	if ( func_is_camera_showing($camera['ca_IPAddress']) )
        	    $class = "btn-info";
        	printf( "<button type='submit' class='btn btn-outline-dark %s' name='SelCamera' id='SelCamera' value='%s' %s>%s</button>", $class, $camera['ca_Name'], func_disabled_non_admin(), 
        	    $camera['ca_Name'] );
       		printf( "&nbsp;&nbsp;" );
        }
        
        if ( !$_SESSION['CameraSearch'] )
        {
            $class = "";
            printf( "&nbsp;&nbsp;" );
            if ( func_total_cameras_showing() == 0 )    
                printf( "<button type='submit' class='btn btn-outline-dark %s' name='AddCamera' id='AddCamera' value='Add New Camera' %s>Add New Camera</button>", $class, func_disabled_non_admin() );
            else if ( func_total_cameras_showing() == 1 )
            {
                printf( "<button type='submit' class='btn btn-outline-dark %s' name='EditCamera' id='EditCamera' value='Edit Camera' %s>Edit Camera</button>", $class, func_disabled_non_admin() );
                printf( "&nbsp;&nbsp;" );
                printf( "<button type='submit' class='btn btn-outline-dark %s' name='RebootCamera' id='RebootCamera' value='Reboot Camera' %s>Reboot Camera</button>", $class, func_disabled_non_admin() );
            }
        }
            	
		printf( "</div>" );
    	printf( "</div>" );    // end of row
    }
    

    if ( $ca_array['ca_CameraNo'] != 0 || $new_camera )
    {
        printf( "<!-- *************************************************************************** -->" );
        printf( "<div class='row mb-2'>" );
        printf( "<div class='col-sm-8'>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_Name'>Name: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='text' class='form-control' size='15' name='ca_Name' id='ca_Name' value='%s'>", $ca_array['ca_Name'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_IPAddress'>IP Address: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='text' class='form-control' size='15' name='ca_IPAddress' id='ca_IPAddress' value='%s'>", $ca_array['ca_IPAddress'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_PTZ'>PTZ Capable: </label>" );
        printf( "</div>" );
        printf( "<div class='col form-check-inline'>" );
        printf("<label><input type='checkbox' name='ca_PTZ' id='ca_PTZ' %s></label>", ($ca_array['ca_PTZ'] == "Y" ? "checked" : ""));
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_Encoding'>Encoding: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf("<select class='custom-select' size='1' name='ca_Encoding' id='ca_Encoding'>");
        printf("<option> ");
        printf("<option %s>%s", ($ca_array['ca_Encoding'] == "H.264" ? "selected" : ""), "H.264" );
        printf("<option %s>%s", ($ca_array['ca_Encoding'] == "H.265" ? "selected" : ""), "H.265" );
        printf("</select>");
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_Directory'>Directory: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='text' class='form-control' size='45' name='ca_Directory' id='ca_Directory' value='%s'>", $ca_array['ca_Directory'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_UserId'>User Id: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='text' class='form-control' size='10' name='ca_UserId' id='ca_UserId' value='%s'>", $ca_array['ca_UserId'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_Password'>Password: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='password' class='form-control' size='15' name='ca_Password' id='ca_Password' value='%s'>", $ca_array['ca_Password'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_Password2'>Confirmation: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='password' class='form-control' size='15' name='ca_Password2' id='ca_Password2' value='%s'>", $ca_array['ca_Password2'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_Model'>Model: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<input type='text' class='form-control' size='10' name='ca_Model' id='ca_Model' value='%s'>", $ca_array['ca_Model'] );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row'>" );
        printf( "<div class='col'>" );
        printf( "<label for='ca_MJpeg'>MJpeg Streaming: </label>" );
        printf( "</div>" );
        printf( "<div class='col'>" );
        printf( "<select class='form-control custom-select' size='1' name='ca_MJpeg' id='ca_MJpeg'>" );
        printf( "<option %s>N. MJpeg not supported</option>", ($ca_array['ca_MJpeg'] == "N" ? "selected" : "") );
        printf( "<option %s>Y. MJpeg over HTTP</option>", ($ca_array['ca_MJpeg'] == "Y" ? "selected" : "") );
        printf( "<option %s>H. MJpeg over HTTPS</option>", ($ca_array['ca_MJpeg'] =="H" ? "selected" : "") );
        printf( "</select>" );
        printf( "</div>" );
        printf( "</div>" );
        
        printf( "<div class='form-row mb-2 mt-2'>" );
        printf( "<p>" );
        printf("<input type='hidden' class='form-control' name='ca_CameraNo' value='%d'>", $ca_array['ca_CameraNo']);
        
        printf("<button type='submit' class='btn btn-outline-dark' name='UpdateCamera' id='UpdateCamera' %s>Update</button>", ($ca_array['ca_CameraNo'] == 0 ? "disabled" : ""));
        printf("&nbsp;&nbsp;");
        printf("<button type='submit' class='btn btn-outline-dark' name='NewCamera' id='NewCamera' %s>New</button>", ($ca_array['ca_CameraNo'] != 0 ? "disabled" : "") );
        printf("&nbsp;&nbsp;");
        $onclick = sprintf("return confirm(\"Are you sure you want to delete Camera with name '%s' ?\")", $ca_array['ca_Name'] );
        printf("<button type='submit' class='btn btn-outline-dark' name='DeleteCamera' id='DeleteCamera' onclick='%s' %s>Delete</button>", $onclick, ($ca_array['ca_CameraNo'] == 0 ? "disabled" : "") );
        printf("&nbsp;&nbsp;");
        printf("<button type='submit' class='btn btn-outline-dark' name='CancelCamera' id='CancelCamera'>Cancel</button>");
        printf( "</div>" );
            
        
        printf( "</div>" );
        printf( "</div>" ); // end of row
    }
    else if ( $_SESSION['CameraSearch'] )
    {
    ?>
    <!-- *************************************************************************** -->
	<div class="row">
		<div class="col-sm-6">
		
    		<?php
    		if ( count($camera_files) > 0 )
    		{
    		    printf( "%d files on %02d/%02d/%d", count($camera_files), $_SESSION['CameraDate']['mday'], $_SESSION['CameraDate']['mon'], $_SESSION['CameraDate']['year'] );
                printf( "<div id='cameragraph' class='chart'></div><br><a href='' id='cameragraphclick'></a>" );
                func_create_camera_graph( $camera_files, "cameragraph", $camera_addr );
                
                if ( $_SESSION['SearchCameraFile'] != "" )
                {
                    $filemkv = sprintf( "%s/%s%s", $camera_dir, $camera_files[0]['dir'], $_SESSION['SearchCameraFile'] );
                    $expl = explode(".", $_SESSION['SearchCameraFile'] );
                    $basemp4 = sprintf( "%s.mp4", $expl[0] );
                    $camera_webdir = func_make_camera_web_dir( $_SERVER['DOCUMENT_ROOT'], $camera_dir );
                    $filemp4 = sprintf( "%s/%s%s", $camera_dir, $camera_files[0]['dir'], $basemp4 );
                    if ( !file_exists($filemp4) )
                    {
                        $cmd = sprintf( "ffmpeg -hide_banner -loglevel warning -i %s -codec copy %s", $filemkv, $filemp4 );
                        system( $cmd );
                        //printf( $cmd );
                    }
                    $filemp4x = sprintf( "%s/%s%s", substr($camera_webdir,strlen($_SERVER['DOCUMENT_ROOT'])), $camera_files[0]['dir'], $basemp4 );
                    
                    printf( "%s <div class='small'>(<a href='%s' download>%s</a>)</div><br>", func_get_date_from_video($_SESSION['SearchCameraFile']), $filemp4x, $_SESSION['SearchCameraFile'] );
                    
                    printf( "<video width='1100' controls type='video/mp4'>" );
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
	</div>

	<?php
    }
    else
    {
    ?>    
    
    <!-- *************************************************************************** -->
	<div class="row">
		<div class="col-sm-12">
            <?php 
            // how many cameras to display
            $img_width = 1100;  //992;
            $camera_ip = "";
            $total = 0;
            foreach ( $_SESSION['show_camera_list'] as $ip )
            {
            	if ( $ip != "" )
            	{
            	    $camera_ip = $ip;
            		$total += 1;
            	}
            }
            
            if ( $total == 1 )
            {	// single camera view
                $supports_mjpeg = func_supports_mjpeg( $camera_list, $camera_ip );
                
                $camera = func_find_camera( $camera_list, $camera_ip );
            	func_display_camera( $camera['ca_IPAddress'], $img_width, $camera['ca_PTZ'], $camera['ca_UserId'], $camera['ca_Password'], $supports_mjpeg );
            }
            else if ( $total > 1 )
            {
            	$count = 0;
            	$width = $img_width / 2;
            	if ( $total > 4 )
            	{
            		$width = $img_width / 3;
            	}
            	            
            	foreach ( $_SESSION['show_camera_list'] as $ip )
            	{
            		if ( $ip != "" )
            		{
            			$count += 1;
            			$supports_mjpeg = func_supports_mjpeg( $camera_list, $ip );
            			
            			$camera = func_find_camera( $camera_list, $ip );
            			
            			func_display_camera( $camera['ca_IPAddress'], $width, $camera['ca_PTZ'], $camera['ca_UserId'], $camera['ca_Password'], $supports_mjpeg );
            		}
            	}
            	
            }
            ?>

		</div>
	</div>
    <?php
    if ( $ca_array['ca_CameraNo'] == 0 && !$new_camera )
    {
        printf( "<!-- *************************************************************************** -->" );
        printf( "<div class='row mb-2 mt-2'>" );
        printf( "<div class='col-sm-6'>" );

        printf( "Full Screen<br>" );
        foreach ( $camera_list as $camera )
        {
            $class = "";
            printf( "<a href='?PageMode=CameraFS&CameraIP=%s' target='_blank'><button type='button' class='btn btn-outline-dark %s' name='CameraFS' id='CameraFS' value='%s' %s>%s</button></a>", 
                $camera['ca_IPAddress'], $class, $camera['ca_Name'], func_disabled_non_admin(), $camera['ca_Name'] );
            printf( "&nbsp;&nbsp;" );
        }
        
        printf( "</div>" );
        printf( "</div>" );    // end of row
    }
    }
    ?>
	
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


function func_get_ptz( $camera_ip )
{
	$ptz = false;
	
	foreach ( $_SESSION['camera_list'] as $camera )
	{
		if ( $camera['addr'] == $camera_ip )
		{
			$ptz = $camera['ptz'];
			break;
		}
	}
	
	return $ptz;
}

function func_is_camera_showing( $camera_ip )
{
    $found = false;
    foreach ( $_SESSION['show_camera_list'] as $camera )
    {
        if ( $camera == $camera_ip )
        {
            $found = true;
            break;
        }
    }
    
    return $found;
}

function func_display_camera( $camera_ip, $width, $ptz )
{
    $height = $width * 9 / 16;
	$set_camera_mode = false;
	
	if ( $set_camera_mode )
	{	// set MJpeg stream
	    // format=1 for MJStream, format=0 for H264
	    $parms = urlencode( sprintf( "cmd=setSubStreamFormat&format=0&usr=%s&pwd=%s", CAMERA_USER, CAMERA_PWD ));
		$cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\"", $camera_ip, $parms );
		$res = exec( $cmd, $out, $ret );
	}
	
	if ( true || func_is_external_connection() )
	{	// external web connection
	    $parms = urlencode( sprintf( "cmd=snapPicture2&usr=%s&pwd=%s", CAMERA_USER, CAMERA_PWD ));
		$cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\"", $camera_ip, $parms );
	
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
			printf( "' alt='no snap shot' width='%d'>", $width );
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
			printf( "' alt='no snap shot' width='%d'>", $width );
		}
	}
	else
	{  // MJStream not working with foscam cameras
		$dest = sprintf( "%s:443", $camera_ip );
		if ( $ptz )
		{
		    $parms = urlencode( sprintf( "cmd=GetMJStream&usr=%s&pwd=%s", CAMERA_USER, CMAERA_PWD )) ;
			printf( "<a href='?MoveCamera'><img src='https://%s/cgi-bin/CGIStream.cgi?%s' alt='no mjpeg stream (%s)' width='%d' ismap></a>",
			    $dest, $parms, $dest, $width );
		}
		else
		{
		    $parms = urlencode( sprintf( "cmd=GetMJStream&usr=%s&pwd=%s", CAMERA_USER, CAMERA_PWD ));
			printf( "<img src='http://%s/cgi-bin/CGIStream.cgi?%s' alt='no mjpeg stream (%s)' width='%d'>",
			    $dest, $parms, $dest, $width );
			
			$url = sprintf( "https://%s/cgi-bin/CGIStream.cgi?%s", $dest, $params );
			printf( "<video controls><source src='%s' type='video/mp4'>unsupported</video>", $url );
		}
	}
}


$error_msg = "";
$info_msg = "";

$ptz_x = 0;
$ptz_y = 0;

if ( isset($_GET['RebootCamera']) )
{
    if ( count($_SESSION['camera_list']) == 1 )
    {
        $camera_ip = "";
        
        // find the camera
        foreach ( $_SESSION['camera_list'] as $camera )
        {
            if ( $camera['name'] == $name )
            {
                $camera_ip = $camera['addr'];
                break;
            }
        }
        
        if ( $camera_ip != "" )
        {
            $cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=rebootSystem&usr=%s&pwd=%s\"", $camera_ip, CAMERA_USER, CAMERA_PWD );
            $res = exec( $cmd, $out, $ret );
        }
        else
        {
            $error_msg = "Could not find the camera ip address to reboot";
        }
    }
    else
    {
        $error_msg = "You must be displaying only a single camera for the reboot to work";
    }
}
else if ( isset($_GET['CameraCycle']) )
{
    if ( $_SESSION['show_camera_list'][1] != "" )
    {
        $info_msg = "The 'Camera Cycle' option is only valid when displaying a single camera image";
    }
    else
    {
        $found = 0;
        foreach ( $_SESSION['camera_list'] as $camera )
        {
            if ( $found == 1 )
            {
                $found = 2;
                $_SESSION['show_camera_list'][0] = $camera['addr'];
                break;
            }
            else if ( $camera['addr'] == $_SESSION['show_camera_list'][0] )
            {
                $found = 1;
            }
        }
        if ( $found != 2 )
        {
            $_SESSION['show_camera_list'][0] = $_SESSION['camera_list'][0]['addr'];
        }
    }
}
else if ( isset( $_POST['SelCamera']) )
{
	$camera_ip = "";
	$name = $_POST['SelCamera'];

	// find the camera
	foreach ( $_SESSION['camera_list'] as $camera )
	{
		if ( $camera['name'] == $name )
		{
			$camera_ip = $camera['addr'];
			break;
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
			else if ( $_SESSION['show_camera_list'][$i] == $camera_ip )
			{	// already in the list - remove it
				$found = true;
				$_SESSION['show_camera_list'][$i] = "";
				if ( $i+1 < MAX_CAMERAS )
				{
					$_SESSION['show_camera_list'][$i] = $_SESSION['show_camera_list'][$i+1];
				}
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

if ( isset($_GET['MoveCamera']) )
{	// move the camera
	$camera_ip = "";
	foreach ( $_SESSION['show_camera_list'] as $camera )
	{
		if ( $camera != 0 )
		{
			$camera_ip = $camera;
			break;
		}
	}
	
	// image is 900 x 680
	$move = $_GET['MoveCamera'];
	
	if ( $camera_ip != "" )
	{
		$info_msg = $move;
		if ( $move == "ptzReset" )
		{
			$cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=%s&usr=%s&pwd=%s\" &",
			    $camera_ip, $move, CAMERA_USER, CAMERA_PWD );
		}
		else 
		{
			$cmd = sprintf( "(curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=%s&usr=%s&pwd=%s\" > /dev/null 2>&1;
						sleep 1 > /dev/null 2>&1;	curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=ptzStopRun&usr=%s&pwd=%s\" > /dev/null 2>&1) > /dev/null 2>&1 &", 
			    $camera_ip, $move, CAMERA_USER, CAMERA_PWD, $camera_ip, CAMERA_USER, CAMERA_PWD );
		}
		$res = system( $cmd, $ret );
	}
}



?>

<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-2">
			<h3>Cameras</h3>
		</div>

		<div class="col-sm-4">
			<?php
	        if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
	        {
    		?>
			<a href='?ClearCameras'>Clear Cameras</a>
			&nbsp;&nbsp;&nbsp;
			<a href='?RebootCamera'>Reboot Camera</a>
			<?php 
	        }
	        ?>
		</div>

		<div class="col-sm-2">
			<div class="custom-control custom-checkbox">
    			<input type="checkbox" class="custom-control-input" id="CameraRefresh" name="CameraRefresh" <?php if(isset($_GET['CameraRefresh'])) echo "checked"?>>
    			<label class="custom-control-label" for="CameraRefresh">Refresh</label>
  			</div>
			<div class="custom-control custom-checkbox">
    			<input type="checkbox" class="custom-control-input" id="CameraCycle" name="CameraCycle" <?php if(isset($_GET['CameraCycle'])) echo "checked"?>>
    			<label class="custom-control-label" for="CameraCycle">Cycle</label>
  			</div>
		</div>
		
	</div>

    <!-- *************************************************************************** -->
	<div class="row">
		<div class="col-sm-6">
		
    		<?php 
    		if ( $error_msg != "" )
    		    printf( "<div class='text-error'>%s</div>", $error_msg );
    		else if ( $info_msg != "" )
    		    printf( "<div class='text-info'>%s</div>", $info_msg );
    		?>
		</div>
	</div>

    <!-- *************************************************************************** -->
	<div class="row mb-2">
		<div class="col-sm-6">
	
			<?php 
	        $count = 0;
            foreach ( $_SESSION['camera_list'] as $camera )
            {
            	$count += 1;
            	$class = "";
            	if ( func_is_camera_showing($camera['addr']) )
            	    $class = "btn-info";
            	printf( "<button type='submit' class='btn btn-outline-dark %s' name='SelCamera' id='SelCamera' value='%s' %s>%s</button>", $class, $camera['name'], func_disabled_non_admin(), $camera['name'] );
           		printf( "&nbsp;&nbsp;" );
            }
            ?>
            	
		</div>
	</div>

    <!-- *************************************************************************** -->
	<div class="row">
		<div class="col-sm-12">
            <?php 
            // how many cameras to display
            $img_width = 1100;  //992;
            $total = 0;
            $camera_ip = "";
            $ptz = false;
            foreach ( $_SESSION['show_camera_list'] as $camera )
            {
            	if ( $camera != "" )
            	{
            		$total += 1;
            		$camera_ip = $camera;
            	}
            }
            
            if ( $total == 1 )
            {	// single camera view
            	$ptz = func_get_ptz( $camera_ip );
            	func_display_camera( $camera_ip, $img_width, $ptz );
            }
            else if ( $total > 1 )
            {
            	$count = 0;
            	$width = $img_width / 2;
            	if ( $total > 4 )
            	{
            		$width = $img_width / 3;
            	}
            	            
            	foreach ( $_SESSION['show_camera_list'] as $camera_ip )
            	{
            		if ( $camera_ip != "" )
            		{
            			$count += 1;
            		
            			$ptz = func_get_ptz( $camera_ip );
            			func_display_camera( $camera_ip, $width, false );
            		}
            	}
            	
            }
            ?>

		</div>
	</div>

	
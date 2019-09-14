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

function func_display_camera( $camera_ip, $width, $ptz )
{
	$set_camera_mode = false;
	
	if ( $set_camera_mode )
	{	// set MJpeg stream
		$cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=setSubStreamFormat&format=1&usr=%s&pwd=%s\"", $camera_ip, CAMERA_USER, CAMERA_PWD );
		$res = exec( $cmd, $out, $ret );
	}
	
	if ( func_is_external_connection() )
	{	// external web connection
		$cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=snapPicture2&usr=%s&pwd=%s\"", $camera_ip, CAMERA_USER, CAMERA_PWD );
	
		if ( $ptz )
			printf( "<a href='index.php?MoveCamera'><img src='data:image/jpeg;base64," );
		else
			printf( "<img src='data:image/jpeg;base64," );
		
		ob_start();
		passthru( $cmd, $out );
		$var = ob_get_contents();
		ob_end_clean();
		echo base64_encode($var);
	
		if ( $ptz )
			printf( "' alt='no snap shot' width='%d'></a>", $width );
		else
			printf( "' alt='no snap shot' width='%d'>", $width );
	}
	else
	{
		$dest = sprintf( "%s:88", $camera_ip );
		if ( $ptz )
		{
			printf( "<a href='index.php?MoveCamera'><img src='http://%s/cgi-bin/CGIStream.cgi?cmd=GetMJStream&usr=%s&pwd=%s' alt='no mjpeg stream (%s)' width='%d' ismap></a>",
			    $dest, CAMERA_USER, CAMERA_PWD, $dest, $width );
		}
		else
		{
			printf( "<img src='http://%s/cgi-bin/CGIStream.cgi?cmd=GetMJStream&usr=%s&pwd=%s' alt='no mjpeg stream (%s)' width='%d'>",
			    $dest, CAMERA_USER, CAMERA_PWD, $dest, $width );
		}
	}
}


$error_msg = "";
$info_msg = "";
$changed = false;

$ptz_x = 0;
$ptz_y = 0;

// check for ptz movement
if ( isset($_SERVER['QUERY_STRING']) && strstr( $_SERVER['QUERY_STRING'], "MoveCamera" ) !== false )
{	// MoveCamera?xx,yy
	$exp = explode( "?", $_SERVER['QUERY_STRING'] );
	if ( isset($exp[1]) )
	{
		$info_msg = $exp[1];
		$exp = explode( ",", $exp[1] );
		if ( isset($exp[0]) && isset($exp[1]) )
		{
			$ptz_x = intval($exp[0]);
			$ptz_y = intval($exp[1]);
		}
	}
}

if ( isset( $_POST['SelCamera']) )
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
	
	if ( $camera_ip != "" && isset($_POST['ca_reboot_camera']) )
	{
	    $cmd = sprintf( "curl --silent \"http://%s:88/cgi-bin/CGIProxy.fcgi?cmd=rebootSystem&usr=%s&pwd=%s\"", $camera_ip, CAMERA_USER, CAMERA_PWD );
	    $res = exec( $cmd, $out, $ret );
	}
	else if ( $camera_ip != "" )
	{
		$changed = true;
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
	$changed = true;
	for ( $i = 0; $i < MAX_CAMERAS; $i++ )
	{
		$_SESSION['show_camera_list'][$i] = "";
	}
}
if ( $ptz_x > 0 && $ptz_y > 0 )
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
	$move = "ptzReset";
	if ( $ptz_x < 200 )
	{
		if ( $ptz_y < 200 )
			$move = "ptzMoveTopLeft";
		else if ( $ptz_y > 480 )
			$move = "ptzMoveBottomLeft";
		else
			$move = "ptzMoveLeft";
	}
	else if ( $ptz_x > 700 )
	{
		if ( $ptz_y < 200 )
			$move = "ptzMoveTopRight";
		else if ( $ptz_y > 480 )
			$move = "ptzMoveBottomRight";
		else
			$move = "ptzMoveRight";
	}
	else if ( $ptz_y < 200 )
	{
		$move = "ptzMoveUp";
	}
	else if ( $ptz_y > 480 )
	{
		$move = "ptzMoveDown";
	}
	
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



printf( "<tr>" );
printf( "<td>" );
printf( "<b>Cameras</b>" );
printf( "&nbsp;&nbsp;&nbsp;<i><a href='index.php?ClearCameras'>Clear List</a></i>" );
printf( "&nbsp;&nbsp;&nbsp;<input type='checkbox' name='ca_reboot_camera'>Reboot" );
printf( "</td>" );
printf( "<td colspan='3' align='right'>" );
if ( $error_msg != "" )
	printf( "<div class='style-error'>%s</div>", $error_msg );
else if ( $info_msg != "" )
	printf( "<div class='style-info'>%s</div>", $info_msg );
printf( "</td>" );
printf( "</tr>" );


printf( "<tr valign='top'>" );
printf( "<td colspan='3'>" );
	
$count = 0;
foreach ( $_SESSION['camera_list'] as $camera )
{
	$count += 1;
	printf( "<input type='submit' name='SelCamera' value='%s'>", $camera['name'] );
	if ( $count == 10 )
		printf( "<br>" );
	else
		printf( "&nbsp;&nbsp;&nbsp;" );
}

printf( "</td>" );
printf( "</tr>" );


printf( "<tr>" );
printf( "<td colspan='3'>" );
	
// how many cameras to display
$total = 0;
$camera_ip = "";
$ptz = false;
foreach ( $_SESSION['show_camera_list'] as $camera )
{
	if ( $camera != 0 )
	{
		$total += 1;
		$camera_ip = $camera;
	}
}


if ( $total == 1 )
{	// single camera view
	$ptz = func_get_ptz( $camera_ip );
	func_display_camera( $camera_ip, 900, $ptz );
}
else if ( $total > 1 )
{
	$count = 0;
	$width = 450;
	if ( $total > 4 )
	{
		$width = 300;
	}
	
	printf( "<table border='1' width='100%%'>" );
	
	printf( "<tr>" );

	foreach ( $_SESSION['show_camera_list'] as $camera_ip )
	{
		if ( $camera_ip != "" )
		{
			$count += 1;
		
			printf( "<td>" );
		
			$ptz = func_get_ptz( $camera_ip );
			func_display_camera( $camera_ip, $width, false );
		
			printf( "</td>" );
		}
	
		if ( $total > 4 )
		{	// more than 4 cameras
			if ( $count == 3 || $count == 6 )
			{
				printf( "<tr>" );
				printf( "</tr>" );
			}
		}
		else
		{	// 2-4 cameras
			if ( $count == 2 )
			{
				printf( "<tr>" );
				printf( "</tr>" );
			}
		}
	}
	
	
	printf( "</tr>" );
	
	printf( "</table>" );
}


printf( "</td>" );
printf( "</tr>" );




?>

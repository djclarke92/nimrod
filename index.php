<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Flat Cat IT Ltd
//	Author: Dave Clarke
//
//--------------------------------------------------------------------------------------

session_start();
include_once( "files/common.php" );
include_once( "files/commonhtml.php" );
include_once( "files/class.email.php" );



func_session_init();


	

// database
$db = new MySQLDB();
if ( $db->Open( DB_HOST_NAME, DB_USER_NAME, DB_PASSWORD, DB_DATABASE_NAME ) === false )
{
	func_unauthorisedaccess();
	return;
}




$device_no = 0;
$my_email = "";
$my_password = "";
$my_login_msg = "";


if ( isset($_GET['PageMode']) )
{
	$_SESSION['page_mode'] = $_GET['PageMode'];
}

	
	
	
if ( isset($_POST['my_logout']) || isset($_GET['Logout']) )
{
	func_user_logout();
}
else if ( isset($_POST['my_login']) )
{
	if ( isset($_POST['my_email']) )
		$my_email = trim($_POST['my_email']);
	if ( isset($_POST['my_password']) )
		$my_password = trim($_POST['my_password']);
		
	if ( $my_email != "" && $my_password != "" )
	{
		// TODO: login
		if ( $my_email == "djclarke@flatcatit.co.nz" && $my_password == "passw0rd" )
		{	// success
			$_SESSION['user_email'] = $my_email;
			$_SESSION['auth_level'] = SECURITY_LEVEL_ADMIN;
			$_SESSION['user_no'] = 1;
		}
		else
		{	// error
			$my_login_msg = sprintf( "The email address entered does not exist in the database." );
		}
	}
	else
	{	// error
		$my_login_msg = sprintf( "You must enter your email address and password to login." );
	}
}
else if ( $_SERVER['HTTPS'] != "" && $_SERVER['PHP_AUTH_USER'] == "djclarke@flatcatit.co.nz" )
{
    $_SESSION['user_email'] = $_SERVER['PHP_AUTH_USER'];
	if ( strncmp( $_SERVER['REMOTE_ADDR'], "192.168.1", 9 ) != 0 )
	{	// external web connection
		$_SESSION['auth_level'] = SECURITY_LEVEL_GUEST;
	}
	else
	{ 
		$_SESSION['auth_level'] = SECURITY_LEVEL_ADMIN;
	}
	$_SESSION['user_no'] = 1;
}

$fs_redirect_timeout = AUTO_REFRESH_LOGOUT;
$fs_redirect_url = sprintf( "index.php?Logout=1&PageMode=Home", $_SERVER['PHP_SELF'] );



	
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>

<head>
	<meta http-equiv="Content-Language" content="en-nz">
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<title>Nimrod - the Butler</title>
	<meta name="description" content="Nimrod - the butler - home automation">
	<meta name="keywords" content="home automation">
	<link rel="icon" href="favicon.ico" type="image/x-icon">
	<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
	<link rel="stylesheet" href="./files/styles.css">
	

	<link href="//vjs.zencdn.net/6.2.7/video-js.min.css" rel="stylesheet">
	<script src="//vjs.zencdn.net/6.2.7/video.min.js"></script>


	<META HTTP-EQUIV="Refresh" CONTENT="<?php echo $fs_redirect_timeout;?>; URL=javascript:window.open('<?php echo $fs_redirect_url;?>','_parent');"> 
	
	<script language="javascript">
		<?php
		if ( isset($_POST['counter']) )
			printf( "var counter = %d;", $_POST['counter'] );
		else 
			printf( "var counter = 0;" );
		?>
		
		function logoutTimer()
		{
		<?php
		if ( $_SESSION['user_email'] != "" )
		{	// someone is logged in
			$timeout_text = "'Logout in '";
			?>				
			var timeout_div = document.getElementById('fs_div_timeout');
			if ( timeout_div ) {
				var val = <?php echo $fs_redirect_timeout; ?> - counter;
				var valm = Math.floor(val / 60);
				var vals = val - (60 * valm);
				var space = "";
				if ( vals < 10 ) {
					space = "0";
				}
				if ( val > 180 ) {
					timeout_div.innerHTML = <?php echo $timeout_text; ?> + valm + ":" + space + vals + " sec";
				} else {
					timeout_div.innerHTML = <?php echo $timeout_text; ?> + val + " sec";
				}

				if ( val <= 0 ) {
					// time for a redirect
					window.location="<?php echo $fs_redirect_url; ?>";
				}
			}
			
			counter += 1;
			
			<?php 
			if ( $_SESSION['page_mode'] == "Home" )
			{
				?>
				if ( (counter % 30) == 0 )
				{
					document.getElementById('counter').value = counter;
					document.getElementById('Refresh').click();
				}
				<?php 
			}
			?>
			
		<?php
		}
		?>	

			setTimeout( "logoutTimer()", 1000 );
		}
	</script>
	<script language="javascript">
		function cursorFocus()
		{
			var field1 = document.getElementById('my_email');	// login
			if ( field1 ) {
				field1.focus();
			}
		}
	</script>

	
</head>


<body onLoad="logoutTimer();cursorFocus();" background='images/background.jpg'>
<div id="content" style="width: 900px; height: 600px;}">
<form action='index.php' method='post'>

<table border="1" height="100%" width='100%'>
<tr valign='top'>
	<td class="style-normal-np" colspan="3">
		<table border="0" width="100%">
		<tr valign="top">
			<?php
			printf( "<td class='style-normal' colspan='2'>" );
			printf( "<img src='images/inductor2.png' height='80px'>" );
			printf( "</td>" );

			printf( "<td><b>Nimrod Admin Console</b></td>" );
				
			if ( $_SESSION['user_no'] != 0 )
			{
				printf( "<td align='right'>%s<br><br><a href='index.php?Logout'>Logout</a><br><br><div id='fs_div_timeout'></div></td>",
					$_SESSION['user_email'] );
			}
			?>
		</tr>

		
		</table>
	</td>
</tr>

<tr valign='top'>
	<td class="style-normal-np" colspan="3">
		<input type='hidden' name='counter' id='counter' value=''>
		<table border="0" width="100%">
	
<?php 
if ( $_SESSION['auth_level'] <= SECURITY_LEVEL_NONE )
{
	printf( "<tr>" );
	printf( "<td><b>Email</b></td>" );
	printf( "<td colspan='3'><input type='text' size='20' name='my_email'></td>");
	printf( "</tr>" );
	printf( "<tr>" );
	printf( "<td><b>Password</b></td>" );
	printf( "<td colspan='3'><input type='password' size='10' name='my_password'></td>");
	printf( "</tr>" );
	printf( "<tr>" );
	printf( "<td></td>" );
	printf( "<td colspan='3'><input type='submit' name='my_login' value='Login'></td>");
	printf( "</tr>" );

	include("./files/intro.php");
}
else
{
	printf( "<tr>" );
	printf( "<td colspan='4'>" );
	printf( "<table width='100%%'>" );
	printf( "<tr>" );
	printf( "<td><a href='index.php?PageMode=Home'><input type='button' name='PageMode' value='Home' style='display:block;width:100%%'></a></td>" );
	printf( "<td><a href='index.php?PageMode=Cameras'><input type='button' name='PageMode' value='Cameras' style='display:block;width:100%%'></a></td>" );
	if ( $_SESSION['auth_level'] > SECURITY_LEVEL_GUEST )
	{
		printf( "<td><a href='index.php?PageMode=Devices' style='display:block;'><input type='button' name='PageMode' value='Devices' style='display:block;width:100%%'></a></td>" );
		printf( "<td><a href='index.php?PageMode=DeviceInfo' style='display:block;'><input type='button' name='PageMode' value='Device Info' style='display:block;width:100%%'></a></td>" );
		printf( "<td><a href='index.php?PageMode=IOLinks' style='display:block;'><input type='button' name='PageMode' value='IO Links' style='display:block;width:100%%'></a></td>");
	}
	printf( "<td><a href='index.php?PageMode=Events' style='display:block;'><input type='button' name='PageMode' value='Events' style='display:block;width:100%%'></a></td>");
	printf( "</tr>" );
	printf( "</table>" );
	printf( "</td>" );
	printf( "</tr>" );
	
	switch ( $_SESSION['page_mode'] )
	{
	default:
	case "Home":
		$_SESSION['page_mode'] = "Home";
		include("./files/home.php");
		break;
			
	case "Cameras":
		include("./files/cameras.php");
		break;
		
	case "Devices":
		if ( $_SESSION['auth_level'] > SECURITY_LEVEL_GUEST )
		{
			include("./files/devices.php");
		}
		break;

	case "DeviceInfo":
		if ( $_SESSION['auth_level'] > SECURITY_LEVEL_GUEST )
		{
			include("./files/deviceinfo.php");
		}
		break;

	case "IOLinks":
		if ( $_SESSION['auth_level'] > SECURITY_LEVEL_GUEST )
		{
			include("./files/iolinks.php");
		}
		break;
	}
	
}
?>
		</table>
	</td>
</tr>



<tr valign='bottom'>
	<td class="style-normal-np" colspan="5">
	<table width="100%">
	<tr>
		<td class="style-small" align="left" width="33%">
			<?php 
			printf( "Version 1.%s", func_get_build_number() ); 
			?>
		</td>
		<td class="style-small" align="center" width="33%">
		
		<?php 
		printf( "%s - %s", $_SERVER['REMOTE_ADDR'], $_SERVER['HTTP_USER_AGENT'] );
		?>
		
		</td>
		<td class="style-small" align="right">
			<?php 
			printf( "Design by FlatCatIT" );
			?>
		</td>
	</tr>
	</table>
	</td>
</tr>
</table>
</form>
</div>
</body>

<?php
$db->Close();
?>

</html>

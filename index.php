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


$_SESSION['bootstrap'] = true;


if ( isset($_SERVER['HTTP_AUTHORIZATION']) )
{
    list($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']) = explode(':' , base64_decode(substr($_SERVER['HTTP_AUTHORIZATION'], 6)));
}


function ValidateUser( $db, $username, $password, &$my_login_msg )
{
    $ok = false;
    
    $_SESSION['us_Username'] = "";
    $_SESSION['us_AuthLevel'] = "";
    $_SESSION['us_Features'] = "";
    
    $my_login_msg = "";
    
    $list = $db->ReadUsers();
    foreach ( $list as $user )
    {
        if ( $username == $user['us_Username'] )
        {
            $hash = hash( "sha256", $password, FALSE );
            if ( strcmp( $user['us_Password'], $hash ) == 0 )
            {
                $ok = true;
                $_SESSION['us_Username'] = $user['us_Username'];
                $_SESSION['us_AuthLevel'] = $user['us_AuthLevel'];
                $_SESSION['us_Features'] = $user['us_Features'];
            }
            else
            {  // wrong password
                $my_login_msg = sprintf( "The entered password is not correct." );
            }
            break;
        }
    }
    
    
    if ( $_SESSION['us_Username'] == "" && $my_login_msg == "" )
    {	// error
        $my_login_msg = sprintf( "The Username entered does not exist in the database." );
    }
    
    return $ok;
}


// database
$db = new MySQLDB();
if ( $db->Open( DB_HOST_NAME, DB_USER_NAME, DB_PASSWORD, DB_DATABASE_NAME ) === false )
{
    func_unauthorisedaccess();
    return;
}

func_check_database( $db );

$my_email = "";
$my_password = "";
$my_login_msg = "";
$new_login = false;
$from_refresh = false;
$display_mode = "";
$monitor_page = 1;


if ( isset($_GET['PageMode']) )
{
    if ( $_GET['PageMode'] == "Monitor1" )
    {
        $display_mode = "Monitor";
        $monitor_page = 1;
    }
    else if ( $_GET['PageMode'] == "Monitor2" )
    {
        $display_mode = "Monitor";
        $monitor_page = 2;
    }
    else
    {
        $_SESSION['page_mode'] = $_GET['PageMode'];
    }
}
if ( isset($_POST['MonitorPage']) )
{
    $monitor_page = $_POST['MonitorPage'];
}
else if ( isset($_GET['MonitorPage']) )
{
    $monitor_page = $_GET['MonitorPage'];
}
if ( isset($_POST['AudioToggle']) )
{
    if ( $_SESSION['monitor_audio'] )
    {
        $_SESSION['monitor_audio'] = false;
    }
    else
    {
        $_SESSION['monitor_audio'] = true;
        
        // play the warning audio
        //system( "sounds/warning1.mp3" );
    }
}
if ( isset($_POST['MonitorPeriod']) )
    $_SESSION['MonitorPeriod'] = floatval($_POST['MonitorPeriod']);
if ( isset($_POST['MonitorRefreshPeriod']) )
{
    $_SESSION['MonitorRefreshPeriod'] = intval($_POST['MonitorRefreshPeriod']);
    if ( $_SESSION['MonitorRefreshPeriod'] <= 4 )
        $_SESSION['MonitorRefreshPeriod'] = 15;
}
if ( isset($_POST['MonitorRefreshEnabled']) )
    $_SESSION['MonitorRefreshEnabled'] = true;
else
    $_SESSION['MonitorRefreshEnabled'] = false;
if ( isset($_POST['MonitorDate']) )
    $_SESSION['MonitorDate'] = $_POST['MonitorDate'];
if ( isset($_POST['MonitorTime']) )
    $_SESSION['MonitorTime'] = $_POST['MonitorTime'];
if ( isset($_POST['MonitorNow']) )
{
    $_SESSION['MonitorDate'] = "";
    $_SESSION['MonitorTime'] = "";
}
if ( isset($_POST['MonitorNow']) || isset($_POST['MonitorGo']) )
{
    $display_mode = "Monitor";
}
if ( isset($_GET['RefreshEnabled']) )
    $from_refresh = true;
    
    
if ( isset($_POST['my_logout']) || isset($_GET['Logout']) )
{
    func_user_logout();
}
else if ( isset($_POST['Username']) && isset($_POST['Password']) )
{
    $my_email = $_POST['Username'];
    $my_password = $_POST['Password'];
    if ( $my_email != "" && $my_password != "" )
    {
        if ( ValidateUser( $db, $my_email, $my_password, $my_login_msg ) )
        {
            $new_login = true;
            $_SESSION['page_mode'] = "Home";
        }
    }
}

if ( $_SESSION['us_AuthLevel'] <= SECURITY_LEVEL_NONE )
{
    $_SESSION['page_mode'] = "Intro";
}

$fs_redirect_timeout = AUTO_REFRESH_LOGOUT;
$fs_redirect_url = sprintf( "?Logout=1&PageMode=Home", $_SERVER['PHP_SELF'] );
                    

?>

<!DOCTYPE html>
<html lang="en">
<head>
  <title>Nimrod Admin Console</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>
  
	<meta http-equiv="Content-Language" content="en-nz">
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta name="description" content="Nimrod - the butler - home automation">
	<meta name="keywords" content="home automation">
	<link rel="icon" href="favicon.ico" type="image/x-icon">
	<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
	<link rel="stylesheet" src="./files/dygraph.css" />
	<script type="text/javascript" src="./files/dygraph.js"></script>
  
  <style>
  <?php
  if ( $display_mode == "" )
  {
  ?>
  body {
    padding-top: 30px;
  }
  @media (max-width: 979px) {
    body {
      padding-top: 30px;
    }
  }
  <?php
  }
  ?>
  </style>
  
  <script language="javascript">
		var counter = 0;

		function refreshTimer()
		{
			<?php
			printf( "   if ( document.getElementById('MonitorRefreshEnabled').checked ) {" );
		    printf( "	  document.getElementById('MonitorNow').click();" );
		    printf( "   }" );
		    printf( "   setTimeout( 'refreshTimer()', %d );", $_SESSION['MonitorRefreshPeriod'] * 1000 );
		    ?>
		}

		function firstTime()
		{
			<?php
			if ( $new_login || $from_refresh )
			{
			    printf( "if ( document.getElementById('RefreshEnabled') != null ) {" );
			    printf( "  document.getElementById('RefreshEnabled').checked = true;" );
			    printf( "}" );
			}
			?>
		}
		
		function homeTimer()
		{
		    var progressBar = $("#refresh-progress-bar");
		    var refreshCheck = document.getElementById('RefreshEnabled');
		    var cameraRefresh = document.getElementById('CameraRefresh');
			if ( refreshCheck != null && progressBar != null ) {
			  if ( refreshCheck.checked ) { 
    	        counter = counter + 1;
			  }
              progressBar.css("width", (100 - (3.3 * counter)) + "%");
			}
			else if ( cameraRefresh != null ) {
				counter = counter + 1;
			}

    		<?php
    		if ( $display_mode == "Monitor" )
    		{
    		    printf( "   document.getElementById('MonitorRefreshEnabled').checked = true;" );
    		    printf( "   setTimeout( 'refreshTimer()', %d );", $_SESSION['MonitorRefreshPeriod'] * 1000 );
    		}
    		else 
    		{	
    			if ( $_SESSION['page_mode'] == "Home" )
    			{
    			    printf( "if ( counter >= 30 ) {" );
    			    printf( "  if ( document.getElementById('RefreshEnabled').checked ) {" );
    				printf( "    window.location.href = '%s?RefreshEnabled';", $_SERVER['PHP_SELF'] );
    				printf( "  }" );
    				printf( "}" );
    			}
    			else if ( $_SESSION['page_mode'] == "Cameras" )
    			{
    			    printf( "if ( counter >= 10 ) {" );
    			    printf( "  if ( document.getElementById('CameraRefresh').checked && document.getElementById('CameraCycle').checked ) {" );
    			    printf( "    window.location.href = '%s?CameraRefresh&CameraCycle';", $_SERVER['PHP_SELF'] );
    			    printf( "  } else if ( document.getElementById('CameraRefresh').checked ) {" );
    			    printf( "    window.location.href = '%s?CameraRefresh';", $_SERVER['PHP_SELF'] );
    			    printf( "  }" );
    			    printf( "}" );
    			}
    			
    			
    
    			printf( "setTimeout( 'homeTimer()', 1000 );" );
    		}
    		?>
    	}
	</script>
	<script>
		$(document).ready(function()
		{
  			$('[data-toggle="tooltip"]').tooltip();
		});
	</script>
</head>



<?php
printf( "<body onLoad='firstTime();homeTimer();' style='background-color: #F5F5F5;' %s>", ($display_mode == "" ? "" : "background='images/background.jpg'") );

if ( $display_mode == "" )
{
?>
<nav class="navbar navbar-expand-sm bg-dark navbar-dark fixed-top">
  <a class="navbar-brand" href="?PageMode=Home">Nimrod</a>
  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#collapsibleNavbar">
    <span class="navbar-toggler-icon"></span>
  </button>
  <div class="collapse navbar-collapse" id="collapsibleNavbar">
    <ul class="navbar-nav">
      <?php
      if ( $_SESSION['us_Username'] != "" )
      {
      ?>
      <li class="nav-item">
        <a class="nav-link" href="?PageMode=Devices">Devices</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" href="?PageMode=DeviceInfo">Device Info</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" href="?PageMode=IOLinks">IO Links</a>
      </li> 
      <li class="nav-item">
        <a class="nav-link" href="?PageMode=Cameras">Cameras</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" href="?PageMode=Events">Events</a>
      </li>    
      <li class="nav-item">
        <a class="nav-link" href="?PageMode=Users">Users</a>
      </li>    
      <?php
      }

      if ( !func_is_external_connection() || $_SESSION['us_Username'] != "" )
      {
      ?>
      <li class='nav-item'>
        <a class='nav-link' href='?PageMode=Monitor1'>Monitor 1</a>
      </li>    
      <li class='nav-item'>
        <a class='nav-link' href='?PageMode=Monitor2'>Monitor 2</a>
      </li>
      <?php
      }
      
      if ( $_SESSION['us_Username'] != "" )
      {
        printf( "<li class='nav-item'>" );
        $tip = sprintf( "You are logged in as %s", $_SESSION['us_Username'] );
        printf( "<a class='nav-link' href='?Logout=1' data-toggle='tooltip' data-html='true' title='%s'>Logout</a>", $tip );
        printf( "</li>" );    
      }
      ?>
    </ul>
  </div>  
</nav>
<?php
$mm = sprintf( "?MonitorPage=%d", $monitor_page );
printf( "<form action='%s%s' method='post' class='form-inline'>", $_SERVER['PHP_SELF'], ($display_mode == "Monitor" ? $mm : "") );

}
else 
{   // monitor mode
    $mm = sprintf( "?MonitorPage=%d", $monitor_page );
    printf( "<form action='%s%s' method='post' >", $_SERVER['PHP_SELF'], ($display_mode == "Monitor" ? $mm : "") );
    
    printf( "<table width='100%%' border='0'>" );
    printf( "<tr>" );
    printf( "<td><a href='index.php?PageMode=Home'><input type='button' name='PageMode' value='Home'></a></td>" );
    //printf( "<td><a href='index.php?PageMode=Home'><input type='button' name='PageMode' value='Home' style='display:block;width:100%%'></a></td>" );
    if ( $display_mode == "Monitor" )
    {
        printf( "<td>" );
        printf( "<input type='hidden' name='MonitorPage' value='%d'>", $monitor_page );
        printf( "<b>Monitor</b> Period <input type='text' name='MonitorPeriod' value='%.1f' size='4'> (hours)", $_SESSION['MonitorPeriod'] );
        printf( "&nbsp;&nbsp;&nbsp;" );
        printf( "Date <input type='text' name='MonitorDate' value='%s' size='10'> (dd/mm/yyyy)", $_SESSION['MonitorDate'] );
        printf( "&nbsp;&nbsp;&nbsp;" );
        printf( "Time <input type='text' name='MonitorTime' value='%s' size='5'> (hh:mm)", $_SESSION['MonitorTime'] );
        printf( "&nbsp;&nbsp;&nbsp;" );
        printf( "<input type='submit' name='MonitorGo' value='Go'>" );
        printf( "&nbsp;&nbsp;&nbsp;" );
        printf( "<input type='submit' name='MonitorNow' id='MonitorNow' value='Now'>" );
        printf( "&nbsp;&nbsp;&nbsp;" );
        printf( "Refresh Interval <input type='text' name='MonitorRefreshPeriod' id='MonitorRefreshPeriod' size='3' value='%d'> sec", $_SESSION['MonitorRefreshPeriod'] );
        printf( "&nbsp;&nbsp;&nbsp;" );
        printf( "<input type='checkbox' name='MonitorRefreshEnabled' id='MonitorRefreshEnabled' value='MonitorRefreshEnabled' %s>", ($_SESSION['MonitorRefreshEnabled'] ? "checked" : "") );
        printf( "<label for='MonitorRefreshEnabled'>Refresh Enabled</label>" );
        printf( "<div id='MonitorRefreshCount'></div>" );
        //printf( "<input type='submit' name='AudioToggle' value='%s'>", ($_SESSION['monitor_audio'] ? "Audio On" : "Audio Muted") );
        printf( "</td>" );
    }
    
    printf( "</tr>" );
    printf( "</table>" );
}


//printf( "%s,%s,%s", $_SESSION['page_mode'], $_SESSION['us_Username'], $_SESSION['us_AuthLevel'] );
if ( $display_mode == "Monitor" )
{
    include("./files/monitor.php");
}
else
{
    switch ( $_SESSION['page_mode'] )
    {
        default:
        case "Intro":
            include("./files/intro.php");
            break;
            
        case "Home":
            include("./files/home.php");
            break;
    
        case "Devices":
            include("./files/devices.php");
            break;
            
        case "DeviceInfo":
            include("./files/deviceinfo.php");
            break;
            
        case "IOLinks":
            include("./files/iolinks.php");
            break;
            
        case "Cameras":
            include("./files/cameras.php");
            break;
            
        case "Users":
            include("./files/users.php");
            break;
            
        case "Events":
            include("./files/events.php");
            break;
    }
}
?>


</form>

<?php 
if ( $display_mode == "" )
{
?>
<div class="container bg-info small" style="margin-bottom:0;">
  	<div class="row">
  		<div class="col-sm-6">
		<?php 
		printf( "Nimrod Build %s, %s", trim(func_get_build_number()), func_get_package_number() );
		printf( "<br>" );
		printf( "Design by FlatCatIT @ %s", date("Y-m-d H:i:s") );
		?>
		</div>
		<div class="col-sm-6">
		<?php 		
		printf( "%s - %s", $_SERVER['REMOTE_ADDR'], (isset($_SERVER['HTTP_USER_AGENT']) ? $_SERVER['HTTP_USER_AGENT'] : "Unknown") );
		?>
		</div>
	</div>
</div>

<?php 
}
?>

</body>
</html>


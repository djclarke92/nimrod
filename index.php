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


function ValidateUser( $db, $username, $password, &$my_login_msg, $autologin_username )
{
    $ok = false;
    
    $_SESSION['us_Username'] = "";
    $_SESSION['us_AuthLevel'] = "";
    $_SESSION['us_Features'] = "";
    
    $my_login_msg = "";
    
    $list = $db->ReadUsers();
    foreach ( $list as $user )
    {
        if ( strtolower($username) == strtolower($user['us_Username']) )
        {
            $hash = hash( "sha256", $password, FALSE );
            if ( $autologin_username != "" )
            {   // fake the login
                $user['us_Password'] = $hash;
            }
            if ( strcmp( $user['us_Password'], $hash ) == 0)
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
    
    if ( !$ok )
    {   // delay to stop robot attacks
        sleep( 4 );
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
    else if ( $_GET['PageMode'] == "PlcState" )
    {
        $display_mode = "PlcState";
    }
    else if ( $_GET['PageMode'] == "CameraFS" )
    {
        $display_mode = "Camera";
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
else if ( isset($POST['PlcStateRefresh']) || isset($_GET['PlcStateFinished']) )
{
    $display_mode = "PlcState";
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
    if ( $_SESSION['MonitorPeriod'] >= 3600 )
    {
        if ( $_SESSION['MonitorRefreshPeriod'] <= 4 )
            $_SESSION['MonitorRefreshPeriod'] = 15;
    }
    else
    {
        if ( $_SESSION['MonitorRefreshPeriod'] <= 1 )
            $_SESSION['MonitorRefreshPeriod'] = 2;
    }
}
if ( isset($_POST['MonitorRefreshEnabled']) )
    $_SESSION['MonitorRefreshEnabled'] = true;
else
    $_SESSION['MonitorRefreshEnabled'] = false;
if ( isset($_POST['MonitorDate']) )
    $_SESSION['MonitorDate'] = $_POST['MonitorDate'];
if ( isset($_POST['MonitorTime']) )
    $_SESSION['MonitorTime'] = $_POST['MonitorTime'];
if ( isset($_POST['MonitorFaster']) )
{
    $display_mode = "Monitor";
    $_SESSION['MonitorDate'] = "";
    $_SESSION['MonitorTime'] = "";
    
    if ( $_SESSION['MonitorRefreshPeriod'] > 30)
        $_SESSION['MonitorRefreshPeriod'] = 30;
    else if ( $_SESSION['MonitorRefreshPeriod'] > 15)
        $_SESSION['MonitorRefreshPeriod'] = 15;
    else if ( $_SESSION['MonitorRefreshPeriod'] > 5)
        $_SESSION['MonitorRefreshPeriod'] = 5;
    else if ( $_SESSION['MonitorRefreshPeriod'] > 2)
    {
        $_SESSION['MonitorRefreshPeriod'] = 2;
        $_SESSION['MonitorPeriod'] = 0.5;
    }
}
if ( isset($_POST['MonitorSlower']) )
{
    $display_mode = "Monitor";
    $_SESSION['MonitorDate'] = "";
    $_SESSION['MonitorTime'] = "";
    
    if ( $_SESSION['MonitorRefreshPeriod'] < 5 )
        $_SESSION['MonitorRefreshPeriod'] = 5;
    else if ( $_SESSION['MonitorRefreshPeriod'] < 15)
        $_SESSION['MonitorRefreshPeriod'] = 15;
    else if ( $_SESSION['MonitorRefreshPeriod'] < 30)
        $_SESSION['MonitorRefreshPeriod'] = 30;
}
if ( isset($_POST['MonitorNow']) )
{
    $_SESSION['MonitorDate'] = "";
    $_SESSION['MonitorTime'] = "";
}
if ( isset($_POST['MonitorNow']) || isset($_POST['MonitorGo']) )
{
    $display_mode = "Monitor";
}
if ( $_SESSION['MonitorDate'] != "" || $_SESSION['MonitorTime'] != "" )
{
    $_SESSION['MonitorRefreshEnabled'] = false;
}
if ( isset($_GET['RefreshEnabled']) || isset($_GET['MonitorRefreshEnabled']) )
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
        if ( ValidateUser( $db, $my_email, $my_password, $my_login_msg, "" ) )
        {
            $new_login = true;
            $_SESSION['page_mode'] = "Home";
            
            $db->SaveUserLoginAttempt( $my_email, 1 );
        }
        else   
        {   // failed
            $db->SaveUserLoginAttempt( $my_email, 0 );
        }
    }
}

if ( $_SESSION['us_AuthLevel'] <= SECURITY_LEVEL_NONE )
{
    $_SESSION['page_mode'] = "Intro";
}

$fs_redirect_timeout = AUTO_REFRESH_LOGOUT;
$fs_redirect_url = sprintf( "?Logout=1&PageMode=Home", $_SERVER['PHP_SELF'] );
$fs_autologin_username = "";
if ( defined("AUTOLOGIN_USERNAME") && ($_SERVER['REMOTE_ADDR'] == "127.0.0.1" || $_SERVER['REMOTE_ADDR'] == "10.166.1.119") )
{
    $fs_autologin_username = AUTOLOGIN_USERNAME;
}

if ( $fs_autologin_username != "" && $_SESSION['us_AuthLevel'] <= SECURITY_LEVEL_NONE )
{   // not already logged in
    if ( ValidateUser( $db, $fs_autologin_username, "", $my_login_msg, $fs_autologin_username ) )
    {
        $new_login = true;
        $_SESSION['page_mode'] = "Home";
        
        $db->SaveUserLoginAttempt( $my_email, 1 );
    }
}
                    

?>

<!DOCTYPE html>
<html lang="en">
<head>
  	<title>Nimrod Admin Console</title>
  	<meta charset="utf-8">
  	<meta name="viewport" content="width=device-width, initial-scale=1.0">
  	<script src="./files/jquery-3.6.0.min.js"></script>
  	<script src="./files/popper-2.11.5.min.js"></script>
  	<link rel="stylesheet" href="./files/bootstrap-5.1.3.min.css">
  	<script src="./files/bootstrap-5.1.3.bundle.min.js"></script>
  
	<meta http-equiv="Content-Language" content="en-nz">
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta name="description" content="Nimrod - the butler - home automation">
	<meta name="keywords" content="home automation">
	<link rel="icon" href="favicon.ico" type="image/x-icon">
	<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
	<link rel="stylesheet" src="./files/dygraph-2.1.0.css" />
	<script type="text/javascript" src="./files/dygraph-2.1.0.min.js"></script>
  
  <style>
  <?php
  if ( $display_mode == "" )
  {
      printf( "body { padding-top: 30px; }" );
      
      if ( $_SESSION['page_mode'] != "Cameras" )
      {
//          printf( "@media (max-width: 979px) { body { padding-top: 30px; } }" );
      }
  }
  ?>
  </style>
  
  <script language="javascript">
		var counter = 0;
        var monRefreshChecked = false;

        function monitorRefreshClicked() {
            var elem = document.getElementById('MonitorRefreshEnabled');
            if ( elem != null ) {
                if ( elem.checked )
                    monRefreshChecked = true;
                else
                    monRefreshChecked = false;
            }
        }

            function firstTime()
		{
			<?php
			if ( $new_login || $from_refresh )
			{
			    printf( "if ( document.getElementById('RefreshEnabled') != null ) {" );
			    printf( "  document.getElementById('RefreshEnabled').checked = true;" );
			    printf( "}" );
			    printf( "if ( document.getElementById('MonitorRefreshEnabled') != null ) {" );
                printf( "  document.getElementById('MonitorRefreshEnabled').checked = true;" );
                printf( "  monRefreshChecked = true;" );
                printf( "}" );
            }
			?>
		}
		
		function homeTimer()
		{
		    var progressBar = document.getElementById('refresh-progress-bar');
		    var monitorBar = document.getElementById('monitor-refresh-progress-bar');
		    var refreshCheck = document.getElementById('RefreshEnabled');
		    var monitorCheck = document.getElementById('MonitorRefreshEnabled');
		    var cameraRefresh = document.getElementById('CameraRefresh');
			if ( refreshCheck != null && progressBar != null ) {
			  if ( refreshCheck.checked ) { 
    	        counter = counter + 1;
                progressBar.innerHTML = '<span class="spinner-border spinner-border-sm"></span>&nbsp;&nbsp;' + (counter < 10 ? '&nbsp;' : '') + (30-counter);
			  } else {
                progressBar.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' + (counter < 10 ? '&nbsp;' : '') + (30-counter);
              }
			}
			else if ( monitorCheck != null && monitorBar != null ) {
			  if ( monitorCheck.checked ) { 
    	        counter = counter + 1;
                monitorBar.innerHTML = '<span class="spinner-border spinner-border-sm"></span>&nbsp;&nbsp;' + (counter < 10 ? '&nbsp;' : '') + (<?php echo $_SESSION['MonitorRefreshPeriod'];?>-counter);
			  } else {
                monitorBar.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' + (counter < 10 ? '&nbsp;' : '') + (<?php echo $_SESSION['MonitorRefreshPeriod'];?>-counter);
              }
			}
			else if ( cameraRefresh != null ) {
				counter = counter + 1;
			}

    		<?php
    		if ( $display_mode == "Monitor" )
    		{
                if ( $_SESSION['MonitorDate'] == "" && $_SESSION['MonitorTime'] == "" )
                {
                    printf( "if ( document.getElementById('MonitorRefreshEnabled') != null && monRefreshChecked ) {" );
                    printf( "   document.getElementById('MonitorRefreshEnabled').checked = true;" );
                    printf( "}" );
                }

                ?>
                if ( counter >= <?php echo $_SESSION['MonitorRefreshPeriod'];?> ) {
                    counter = 0;
                    if ( document.getElementById('MonitorRefreshEnabled').checked ) {
                      window.location.href = "<?php printf("%s?PageMode=Monitor%d&MonitorRefreshEnabled", $_SERVER['PHP_SELF'], $monitor_page ); ?>"; 
                    }
                }
    		    setTimeout( 'homeTimer()', 1000 );
                <?php
    		}
    		else if ( $display_mode == "Camera" )
    		{
    		}
    		else if ( $display_mode == "PlcState" )
    		{
    		}
    		else 
    		{	
    			if ( $_SESSION['page_mode'] == "Home" )
    			{
    			    ?>
    			    if ( counter >= 30 ) {
        			  counter = 0;
    			      if ( document.getElementById('RefreshEnabled').checked ) {
    				    window.location.href = "<?php echo $_SERVER['PHP_SELF']; ?>?RefreshEnabled"; 
    				  }
    				} else {
            			var all = document.getElementsByClassName('timestamp');
            			for (var i = 0; i < all.length; i++) {
                		  // 01234567890
                		  // hh:mm dd/mm
                		  var dd = all[i].textContent;
                		  var now = new Date();
                		  var last = new Date();

                		  last.setFullYear( now.getFullYear() );
                		  last.setMonth( parseInt(dd.substr(9,2))-1 );
                		  last.setDate( parseInt(dd.substr(6,2)) );
                		  last.setHours( parseInt(dd.substr(0,2)) );
                		  last.setMinutes( parseInt(dd.substr(3,2)) );
                		  last.setSeconds( 0 );
                		   
                		  if ( last.getTime() + 16*60*1000 < now.getTime() ) {
                			if ( (now.getSeconds() % 2) == 0 ) {
                    			all[i].style.color = 'white';
                    			all[i].style.backgroundColor = 'red';
                			} else {
                    			all[i].style.color = 'red';
                				all[i].style.backgroundColor = 'transparent';
                			}
                		  } else if ( last.getTime() + 6*60*1000 < now.getTime() ) {
              			    all[i].style.color = 'red';
            				all[i].style.backgroundColor = 'transparent';
                		  } else {
                			all[i].style.color = 'black';
            				all[i].style.backgroundColor = 'transparent';
                		  }
            			}
    				}
    				<?php 
    			}
    			else if ( $_SESSION['page_mode'] == "Cameras" )
    			{
    			    printf( "if ( counter >= 10 ) {" );
    			    printf( "  counter = 0;" );
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
  			var tooltipTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="tooltip"]'));
            var tooltipList = tooltipTriggerList.map(function (tooltipTriggerEl) {
              return new bootstrap.Tooltip(tooltipTriggerEl)
            });
		});
	</script>
	
	<?php 
	if ( $display_mode == "PlcState" )
	{
    ?>
		<script type = "text/javascript">
		var wshost = "<?php printf( "%s://%s:%d", (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != "" ? "wss" : "ws"), ($_SERVER['SERVER_PORT'] == 8080 ? "127.0.0.1" : $_SERVER['SERVER_ADDR']), ($_SERVER['SERVER_PORT'] == 8080 ? 8081 : 8000) ); ?>";
  	  	var socket = new WebSocket( wshost, "nimrod-protocol" );
  	  	var plcCounter = 0;

  	  	function plcTimer() {
  	  	  	plcCounter += 1;

  	  	  	if ( plcCounter >= <?php printf( "%d", ($_SESSION['plc_Operation'] == "" ? 30 : 5) ); ?> ) {
	  	    	window.location.replace('index.php?PageMode=PlcState'); 
  	  		}

  	  	  	setTimeout( 'plcTimer()', 1000 );   
  	  	}

        function plcChangeVsdFrequency( msg ) {
            console.log(msg);
            socket.send( msg );
        }

        function plcButtonVsdFreq( msg, val ) {
            console.log(msg);
            msg += '_';
            msg += val;
            plcChangeVsdFrequency( msg );
        }
  	  	
	  	function update(msg) { 
		  	if ( msg.length > 0 ) 
		  	{   // 01234567890123
                // VVc_dd_ii:nnnn
                if ( msg.substr(3,1) != '_' && msg.substr(6,1) != '_' && msg.substr(9,1) != ':') {
                    var wsm = document.getElementById("ws_message");
                    if ( wsm != null ) {
                        wsm.innerHTML = msg;
                    }
        
                    var select = document.getElementById('ws_list');
                    if ( select != null ) {
                        var date = new Date();
            
                        var dateStr =
                        ("00" + (date.getMonth() + 1)).slice(-2) + "/" +
                        ("00" + date.getDate()).slice(-2) + "/" +
                        date.getFullYear() + " " +
                        ("00" + date.getHours()).slice(-2) + ":" +
                        ("00" + date.getMinutes()).slice(-2) + ":" +
                        ("00" + date.getSeconds()).slice(-2);
            
                        var msg2 = dateStr + ': ' + msg;
                        
                        var opt = document.createElement('option');
                        opt.value = msg2;
                        opt.innerHTML = msg2;
                        select.add(opt,0)
                        
                        // only allow 20 items in the list
                        if ( select.length >= 20 )
                        {
                            select.removeChild( select.options[20] );
                        }
            
                        //select.scrollTo( 0, select.length - 4 ); 
                    }

                    var stateName = '<?php echo $_SESSION['plc_StateName'];?>';
                    
                    if ( msg.substring(0,6) == 'State:' ) {
                        if ( msg.substring(6) != stateName && stateName != '' ) {
                        window.location.replace('index.php?PageMode=PlcState');
                        }
                    } else if ( msg == "Refresh" ) {
                        window.location.replace('index.php?PageMode=PlcState'); 
                    } else if ( msg == "wss closed" ) {
                        setTimeout( 'plcTimer()', 1000 );   
                    } else {
                        plcCounter  = 0;
                    }
    		  	} else if ( msg.substr(3,1) == '_' && msg.substr(6,1) == '_' && msg.substr(9,1) == ':') {

                    //console.log(msg);
                    var elem = document.getElementById(msg.substr(0,9));
                    if ( elem != null ) {
                        elem.innerHTML = msg.substr(10);
                    } else {
                        //console.log("element not found '"+msg.substr(0.9)+"'");
                    }
                }
            }
		}
		
	  	socket.onopen = function() { console.log("socket open"); update("wss open"); }
	  	socket.onclose = function() { console.log("socket close"); update("wss closed"); }
	  	socket.onmessage = function(msg) { console.log("socket message: " + msg.data); update(msg.data); }
	  	socket.error = function(error){ console.log( "socket error: :" + error.message); update(error.message); }    
	  	</script>
	<?php 
	} else if ( $display_mode == "" && $_SESSION['page_mode'] == "Home" ) {
        ?>
		<script type = "text/javascript">
		var wshost = "<?php printf( "%s://%s:%d", (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != "" ? "wss" : "ws"), ($_SERVER['SERVER_PORT'] == 8080 ? "127.0.0.1" : $_SERVER['SERVER_NAME']), ($_SERVER['SERVER_PORT'] == 8080 ? 8081 : 8000) ); ?>";
  	  	var socket = new WebSocket( wshost, "nimrod-protocol" );
        var wssCounter = 0;
        var wssConnected = 0;

        setTimeout( 'wssTimer()', 1000 );   
        setupWebSocket();

        function wssTimer() {
            wssCounter += 1;

            if ( wssCounter >= 5 ) {
                //window.location.replace('index.php?PageMode=Home'); 
                wssCounter = 0;

                if ( wssConnected == 0 ) {
                    socket = null;
                    socket = new WebSocket( wshost, "nimrod-protocol" );
                    setupWebSocket();
                } else {

                }
            }

            setTimeout( 'wssTimer()', 1000 );   
        }

        function update(msg) {
            if ( msg.length > 0) {
                // format of update messages
                // cc = {VV|TT}, nn = DeviceNo, xx = IOChannel
                // 01234567890
                // CCc_nn_xx:value
                if ( (msg.substr(3,1) == '_' && msg.substr(6,1) == '_' && msg.substr(9,1) == ':') || msg.substr(0,3) == 'wss') {
                    var wsm = document.getElementById("ws_message");
                    if ( wsm != null ) {
                        wsm.innerHTML = msg;
                    }

                    var date = new Date();
    
                    var dateStr =
                        ("00" + date.getHours()).slice(-2) + ":" +
                        ("00" + date.getMinutes()).slice(-2) + " " +
                        ("00" + date.getDate()).slice(-2) + "/" +
                        ("00" + (date.getMonth() + 1)).slice(-2);

                    if ( msg.substr(0,2) == 'VV' || msg.substr(0,2) == 'TT' || msg.substr(0,2) == 'LL' ) {
                        // voltages
                        var val = document.getElementById(msg.substr(0,9));
                        if ( val != null ) {
                            val.innerHTML = msg.substr(10);
                        } else {
                            //console.log("element not found '"+msg.substr(0,9)+"'");
                        }

                        var dd = document.getElementById(msg.substr(0,9)+'_DT');
                        if ( dd != null ) {
                            dd.innerHTML = dateStr;
                        }
                    } else if ( msg.substr(0,3) == 'STT' ) {
                        var val = document.getElementById(msg.substr(0,9));
                        if ( val != null ) {
                            if ( msg.substr(10) == '1' )
                                val.src = './images/green_tick.png';
                            else
                                val.src = './images/dead_robot.png';
                        } else {
                            //console.log("element not found '"+msg.substr(0,9)+"'");
                        }
                    } else if ( msg.substr(0,3) == 'CNG' ) {
                        var val = document.getElementById(msg.substr(0,9));
                        if ( val != null ) {
                            if ( msg.substr(10) == '1' )
                                val.innerHTML = 'Certificate Error';
                            else
                                val.innerHTML = '';
                        } else {
                            console.log("element not found '"+msg.substr(0,9)+"'");
                        }
                    }
                }
            }
        }

        function setupWebSocket() {
            socket.onopen = function() { console.log("socket open"); update("wss open"); wssConnected = 1; }
            socket.onclose = function() { console.log("socket close"); update("wss closed"); wssConnected = 0; }
            socket.onmessage = function(msg) { console.log("socket message: '" + msg.data + "'"); update(msg.data); }
            socket.error = function(error){ console.log( "socket error: :" + error.message); update(error.message); wssConnected = 0; }    
        }
        </script>
	<?php 
	} else if ( $display_mode == "Monitor" ) {
        ?>
		<script type = "text/javascript">
		var wshost = "<?php printf( "%s://%s:%d", (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != "" ? "wss" : "ws"), ($_SERVER['SERVER_PORT'] == 8080 ? "127.0.0.1" : $_SERVER['SERVER_NAME']), ($_SERVER['SERVER_PORT'] == 8080 ? 8081 : 8000) ); ?>";
  	  	var socket = new WebSocket( wshost, "nimrod-protocol" );
        var monCounter = 0;
        var monConnected = 0;

        setTimeout( 'monTimer()', 1000 );   
        setupWebSocket();

        function monTimer() {
            monCounter += 1;

            if ( monCounter >= 5 ) {
                //window.location.replace('index.php?PageMode=Home'); 
                monCounter = 0;

                if ( monConnected == 0 ) {
                    socket = null;
                    socket = new WebSocket( wshost, "nimrod-protocol" );
                    setupWebSocket();
                } else {

                }
            }

            setTimeout( 'monTimer()', 1000 );   
        }

        function update(msg) {
            if ( msg.length > 0) {
                // format of update messages
                // cc = {VV|TT|LL}, nn = DeviceNo, xx = IOChannel
                // 01234567890
                // CCc_nn_xx:value
                if ( (msg.substr(3,1) == '_' && msg.substr(6,1) == '_' && msg.substr(9,1) == ':') || msg.substr(0,3) == 'wss') {
                    var wsm = document.getElementById("ws_message");
                    if ( wsm != null ) {
                        wsm.innerHTML = msg;
                    }

                    if ( msg.substr(0,2) == 'VV' || msg.substr(0,2) == 'TT' || msg.substr(0,2) == 'LL' ) {
                        // voltages
                        var val = document.getElementById(msg.substr(0,9));
                        if ( val != null ) {
                            val.innerHTML = msg.substr(10);
                        } else {
                            console.log("element not found '"+msg.substr(0,9)+"'");
                        }
                    }
                }
            }
        }

        function setupWebSocket() {
            socket.onopen = function() { console.log("socket open"); update("wss open"); monConnected = 1; }
            socket.onclose = function() { console.log("socket close"); update("wss closed"); monConnected = 0; }
            socket.onmessage = function(msg) { console.log("socket message: '" + msg.data + "'"); update(msg.data); }
            socket.error = function(error){ console.log( "socket error: :" + error.message); update(error.message); monConnected = 0; }    
        }
        </script>
        <?php
    }
	?>      
</head>



<?php
$bg = "";
if ( $display_mode == "Monitor" || $display_mode == "PlcState" )
{
    $bg = "background='images/background.jpg'";
}
printf( "<body onLoad='firstTime();homeTimer();' style='background-color: #F5F5F5;' %s>", $bg );

$devices = $db->ReadDevicesTable();

if ( $display_mode == "" )
{
?>
<nav class="navbar navbar-expand-lg bg-dark navbar-dark fixed-top">
 <div class="container-fluid">
  <a class="navbar-brand" href="?PageMode=Home">Nimrod</a>
  <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#collapsibleNavbar">
    <span class="navbar-toggler-icon"></span>
  </button>
  <div class="collapse navbar-collapse" id="collapsibleNavbar">
    <ul class="navbar-nav">
      <?php
      if ( $_SESSION['us_Username'] != "" )
      {
          printf( "<li class='nav-item'>" );
          printf( "  <a class='nav-link' href='?PageMode=Devices'>Devices</a>" );
          printf( "</li>" );
          if ( count($devices) > 1 || (count($devices) == 1 && $devices[0]['de_Type'] != E_DT_TIMER) )
          {
              printf( "<li class='nav-item'>" );
              printf( "  <a class='nav-link' href='?PageMode=DeviceInfo'>Device Info</a>" );
              printf( "</li>" );
              printf( "<li class='nav-item'>" );
              printf( "  <a class='nav-link' href='?PageMode=IOLinks'>IO Links</a>" );
              printf( "</li>" ); 
              if ( SHOW_PLC == 1 )
              {
                  printf( "<li class='nav-item'>" );
                  printf( "  <a class='nav-link' href='?PageMode=PLC'>PLC</a>" );
                  printf( "</li>" );
              }
          }
          if ( func_user_feature_enabled(E_UF_CAMERAS) )
          {
              printf( "<li class='nav-item'>" );
              printf( "  <a class='nav-link' href='?PageMode=Cameras'>Cameras</a>" );
              printf( "</li>" );
          }
          printf( "<li class='nav-item'>" );
          printf( "  <a class='nav-link' href='?PageMode=Events'>Events</a>" );
          printf( "</li>" );    
          printf( "<li class='nav-item'>" );
          printf( "  <a class='nav-link' href='?PageMode=Users'>Users</a>" );
          printf( "</li>" );    
      }

      if ( (count($devices) > 1 || (count($devices) == 1 && $devices[0]['de_Type'] != E_DT_TIMER)) && (!func_is_external_connection() || $_SESSION['us_Username'] != "") )
      {
          printf( "<li class='nav-item'>" );
          printf( "  <a class='nav-link' href='?PageMode=Monitor1'>Monitor 1</a>" );
          printf( "</li>" );    
          printf( "<li class='nav-item'>" );
          printf( "  <a class='nav-link' href='?PageMode=Monitor2'>Monitor 2</a>" );
          printf( "</li>" );
          if ( SHOW_PLC == 1 )
          {
             printf( "<li class='nav-item'>" );
             printf( "  <a class='nav-link' href='?PageMode=PlcState' target='_blank'>PlcState</a>" );
             printf( "</li>" );
          }
      }
      
      if ( $_SESSION['us_Username'] != "" )
      {
        printf( "<li class='nav-item'>" );
        $tip = sprintf( "You are logged in as %s", $_SESSION['us_Username'] );
        printf( "<a class='nav-link' href='?Logout=1' data-bs-toggle='tooltip' data-bs-html='true' title='%s'>Logout</a>", $tip );
        printf( "</li>" );    
      }
      ?>
    </ul>
  </div>  
 </div>
</nav>
<?php
$mm = sprintf( "?MonitorPage=%d", $monitor_page );
printf( "<form action='%s%s' enctype='multipart/form-data' method='post' class='form-inline'>", $_SERVER['PHP_SELF'], ($display_mode == "Monitor" ? $mm : "") );

}
else if ( $display_mode == "Monitor" )
{   // monitor mode
    $mm = sprintf( "?MonitorPage=%d", $monitor_page );
    printf( "<form action='%s%s' enctype='multipart/form-data' method='post' >", $_SERVER['PHP_SELF'], ($display_mode == "Monitor" ? $mm : "") );
    
    printf( "<table width='100%%' border='0'>" );
    printf( "<tr>" );
    printf( "<td><a href='index.php?PageMode=Home'><input type='button' name='PageMode' value='Home'></a></td>" );
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
        printf( "<button class='btn btn-outline-primary btn-sm' id='monitor-refresh-progress-bar' name='monitor-refresh-progress-bar' disabled>" );
        printf( "<span class='spinner-border spinner-border-sm'></span>" );
        printf( "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" );
        printf( "</button>" );
        printf( "<input type='checkbox' name='MonitorRefreshEnabled' id='MonitorRefreshEnabled' value='MonitorRefreshEnabled' onclick='monitorRefreshClicked();'%s>", ($_SESSION['MonitorRefreshEnabled'] ? "checked" : "") );
        printf( "<label for='MonitorRefreshEnabled'>Refresh Enabled</label>" );
        printf( "&nbsp;&nbsp;&nbsp;" );

        $disabled = "";
        if ( $_SESSION['MonitorRefreshPeriod'] <= 2 )
            $disabled = "disabled";
        printf( "<input type='submit' name='MonitorFaster' value='Faster' %s>", $disabled );
        printf( "&nbsp;&nbsp;&nbsp;" );

        $disabled = "";
        if ( $_SESSION['MonitorRefreshPeriod'] >= 30 )
            $disabled = "disabled";
        printf( "<input type='submit' name='MonitorSlower' value='Slower' %s>", $disabled );
        printf( "&nbsp;%s: <span id='ws_message'></span>", ($_SERVER['HTTPS'] != "" ? "wss" : "ws") );
        printf( "</td>" );
    }
    
    printf( "</tr>" );
    printf( "</table>" );
}
else if ( $display_mode == "PlcState" )
{   // plcstate mode
    printf( "<form action='%s?PageMode=PlcState' enctype='multipart/form-data' method='post' >", $_SERVER['PHP_SELF'] );
    
    printf( "<table width='100%%' border='0'>" );
    printf( "<tr>" );
    printf( "<td>" );
    printf( "<a href='index.php?PageMode=Home'><input type='button' name='PageMode' value='Home'></a>" );
    $list = glob( sprintf( "%s/nimrod-*.tgz", $_SERVER['DOCUMENT_ROOT'] ) );
    if ( count($list) != 0 ) 
    {
        printf( "&nbsp;&nbsp;<i>Upgrade pending...</i>" );
    }
    printf( "</td>" );
    
    printf( "<td>" );
    printf( "<a href='index.php?PlcStateFinished=1'>Finished</a>&nbsp;&nbsp;" );
    printf( "&nbsp;&nbsp;&nbsp;" );
    printf( "<input type='submit' name='PlcStateRefresh' id='PlcStateRefresh' value='Refresh'>" );
    printf( "</td>" );
    
    printf( "</tr>" );
    printf( "</table>" );
}


//printf( "%s,%s,%s", $_SESSION['page_mode'], $_SESSION['us_Username'], $_SESSION['us_AuthLevel'] );
if ( $display_mode == "Monitor" )
{
    include("./files/monitor.php");
}
else if ( $display_mode == "PlcState" )
{
    include("./files/plcstate.php");
}
else if ( $display_mode == "Camera" )
{
    include("./files/camerafs.php");
}
else
{
    printf( "<div class='container-fluid'>" );
    
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
        
    case "PLC":
        include("./files/plc.php");
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
    
    printf( "</div>" );
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

$db->Close();
?>

</body>
</html>


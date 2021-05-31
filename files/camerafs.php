<?php 
// display full screen camera mjpeg stream using curl passthru


$camera_ip = "";
$camuser = "";
$campwd = "";
$camname = $camera_ip;
    
if ( isset($_GET['CameraIP']) )
{
    $camera_ip = $_GET['CameraIP'];
    
    
    $camera_list = $db->ReadCameraList();
    
   
    foreach ( $camera_list as $camera )
    {
        if ( $camera['ca_IPAddress'] == $camera_ip )
        {
            $camuser = $camera['ca_UserId'];
            $campwd = $camera['ca_Password'];
            $camname = $camera['ca_Name'];
            break;
        }
    }
    
    $parms = sprintf( "cmd=getMJStream&usr=%s&pwd=%s", $camuser, $campwd );
    $uparms = urlencode( $parms );
    
    $parms = sprintf( "cmd=snapPicture2&usr=%s&pwd=%s", $camuser, $campwd );
    $uparms = urlencode( $parms );
    $cmd = sprintf( "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\"", $camera_ip, $uparms );
}



if ( $camera_ip != "" && $camuser != "" )
{
    printf( "<table border='0' width='100%%'><tr>" );
    printf( "<td width=50%%><a href='index.php?PageMode=Home'>Return to Home page</a></td>" );
    printf( "<td><input type='checkbox' name='refreshCamera' id='refreshCamera'>Stop Refresh</td>" );
    printf( "</tr></table><br>" );
    printf( "<img src='data:image/jpeg;base64," );
    
    ob_start();
    passthru( $cmd, $out );
    $var = ob_get_contents();
    ob_end_clean();
    echo base64_encode($var);
    
    printf( "' alt='no snap shot (%s)'>", $camera_ip );
    
    ?>
    <script language='javascript'>
    function refresh()
    {
    	var chkBox = document.getElementById('refreshCamera');
        if (chkBox.checked)
        {
            setTimeout( refresh, 5000 );
        }
        else
        {
		    window.location.reload();
        }
    }
    setTimeout( refresh, 1000 );
    </script>
    <?php 
}
else
{
    printf( "You must pass the camera ip address as a parameter" );    
}


?>

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


function func_clear_us_array( &$us_array )
{
	$us_array['us_Username'] = "";
	$us_array['us_Name'] = "";
	$us_array['us_Password'] = "";
	$us_array['us_Password2'] = "";
	$us_array['us_AuthLevel'] = "0";
	$us_array['us_UF_Cameras'] = "N";
	$us_array['us_UF_Upgrade'] = "N";
	$us_array['us_UF_HomeCameras'] = "N";
	$us_array['us_UF_AccountingUser'] = 'N';
	$us_array['us_CardNumber'] = "";
	$us_array['us_CardPin'] = "";
	$us_array['us_CardPin2'] = "";
	$us_array['us_CardEnabled'] = 'N';
	$us_array['us_PinFailCount'] = "";
	$us_array['us_TruckRego'] = "";
	$us_array['us_TrailerRego'] = "";
	$us_array['us_BillingName'] = "";
	$us_array['us_BillingAddr1'] = "";
	$us_array['us_BillingAddr2'] = "";
	$us_array['us_BillingAddr3'] = "";
	$us_array['us_BillingEmail'] = "";
	$us_array['us_TruckTare'] = "";
	$us_array['us_TrailerTare'] = "";
	$us_array['us_TruckLoad'] = "";
	$us_array['us_TrailerLoad'] = "";
	$us_array['error_msg'] = "";
	$us_array['info_msg'] = "";
}

function func_check_us_array( &$us_array )
{
	$us_array['error_msg'] = "";
	$us_array['info_msg'] = "";
	
	if ( $us_array['us_Password'] != "" && $us_array['us_Password2'] == "" )
	{  // handle autocomplete
	    $us_array['us_Password'] = "";
	}
	if ( $us_array['us_CardPin'] != "" && $us_array['us_CardPin2'] == "" )
	{  // handle autocomplete
	    $us_array['us_CardPin'] = "";
	}
	
	if ( $us_array['us_Username'] == "" )
	{
		$us_array['error_msg'] = "You must enter the Username.";
		return false;
	}
	else if ( strstr($us_array['us_Username'], "@" ) == false || strstr($us_array['us_Username'], "." ) == false )
	{
	    $us_array['error_msg'] = "The Username must be an email address containing '@' and '.'.";
	    return false;
	}
	else if ( $us_array['us_Password'] != "" && $us_array['us_Password'] != $us_array['us_Password2'] )
	{
		$us_array['error_msg'] = "The password and confirmation must match.";
		return false;
	}
	else if ( $us_array['us_Password'] != "" && strlen($us_array['us_Password']) < 6 )
	{
	    $us_array['error_msg'] = "The password must be at least 6 characters in length.";
	    return false;
	}
	else if ( $us_array['us_Name'] == "" )
	{
		$us_array['error_msg'] = "You must enter the Name.";
		return false;
	}
	else if ( $us_array['us_AuthLevel'] == "" )
	{
		$us_array['error_msg'] = "Your must select the Security Level.";
		return false;
	}
	else if ( $us_array['us_CardNumber'] != "" && (strlen($us_array['us_CardNumber']) < 6 || strlen($us_array['us_CardNumber']) > 10) )
	{
	    $us_array['error_msg'] = "The Card Number must be from 6 to 10 digits long";
	    return false;
	}
	else if ( $us_array['us_CardPin'] != "" && (strlen($us_array['us_CardPin']) < 4 || strlen($us_array['us_CardPin']) > 6) )
	{
	    $us_array['error_msg'] = "The Card Pin must be from 4 to 6 digits long";
	    return false;
	}
	else if ( $us_array['us_CardPin'] != "" && $us_array['us_CardPin'] != $us_array['us_CardPin2'] )
	{
	    $us_array['error_msg'] = "The PIN and confirmation must match.";
	    return false;
	}
	else if ( ($us_array['us_TruckRego'] != "" || $us_array['us_TrailerRego'] != "") && ($us_array['us_BillingName'] == "" || $us_array['us_BillingAddr1'] == "" || $us_array['us_BillingEmail'] == "") )
	{
	    $us_array['error_msg'] = "The Billing Name, Address and Email must be entered if a Rego is entered.";
	    return false;
	}
	
	return true;
}


$us_array = array();
func_clear_us_array( $us_array );
$new_user = false;



if ( isset( $_GET['Username']) )
	$us_array['us_Username'] = $_GET['Username'];
if ( isset( $_POST['us_Username']) )
	$us_array['us_Username'] = $_POST['us_Username'];
if ( isset( $_POST['us_Name']) )
	$us_array['us_Name'] = $_POST['us_Name'];
if ( isset( $_POST['us_Password']) )
	$us_array['us_Password'] = $_POST['us_Password'];
if ( isset( $_POST['us_Password2']) )
	$us_array['us_Password2'] = $_POST['us_Password2'];
if ( isset( $_POST['us_AuthLevel']) )
    $us_array['us_AuthLevel'] = substr($_POST['us_AuthLevel'],0,1);
if ( isset( $_POST['us_UF_Cameras']) )
    $us_array['us_UF_Cameras'] = "Y";
if ( isset( $_POST['us_UF_Upgrade']) )
    $us_array['us_UF_Upgrade'] = "Y";
if ( isset( $_POST['us_UF_HomeCameras']) )
    $us_array['us_UF_HomeCameras'] = "Y";
if ( isset( $_POST['us_UF_AccountingUser']) )
    $us_array['us_UF_AccountingUser'] = "Y";
if ( isset( $_POST['us_CardNumber']) )
    $us_array['us_CardNumber'] = $_POST['us_CardNumber'];
if ( isset( $_POST['us_CardPin']) )
    $us_array['us_CardPin'] = $_POST['us_CardPin'];
if ( isset( $_POST['us_CardPin2']) )
    $us_array['us_CardPin2'] = $_POST['us_CardPin2'];
if ( isset( $_POST['us_CardEnabled']) )
    $us_array['us_CardEnabled'] = "Y";
if ( isset( $_POST['us_TruckRego']) )
	$us_array['us_TruckRego'] = $_POST['us_TruckRego'];
if ( isset( $_POST['us_TrailerRego']) )
	$us_array['us_TrailerRego'] = $_POST['us_TrailerRego'];
if ( isset( $_POST['us_BillingName']) )
	$us_array['us_BillingName'] = $_POST['us_BillingName'];
if ( isset( $_POST['us_BillingAddr1']) )
	$us_array['us_BillingAddr1'] = $_POST['us_BillingAddr1'];
if ( isset( $_POST['us_BillingAddr2']) )
	$us_array['us_BillingAddr2'] = $_POST['us_BillingAddr2'];
if ( isset( $_POST['us_BillingAddr3']) )
	$us_array['us_BillingAddr3'] = $_POST['us_BillingAddr3'];
if ( isset( $_POST['us_BillingEmail']) )
	$us_array['us_BillingEmail'] = $_POST['us_BillingEmail'];
if ( isset( $_POST['us_TruckTare']) )
	$us_array['us_TruckTare'] = $_POST['us_TruckTare'];
if ( isset( $_POST['us_TrailerTare']) )
	$us_array['us_TrailerTare'] = $_POST['us_TrailerTare'];
if ( isset( $_POST['us_TruckLoad']) )
	$us_array['us_TruckLoad'] = $_POST['us_TruckLoad'];
if ( isset( $_POST['us_TrailerLoad']) )
	$us_array['us_TrailerLoad'] = $_POST['us_TrailerLoad'];
        
if ( isset( $_FILES['us_FileName']) )        
{
    $us_array['info_msg'] = "set";
    if ( isset($_FILES['us_FileName']['error']) )
    {
        $ok = false;
        switch ($_FILES['us_FileName']['error']) 
        {
        case UPLOAD_ERR_OK:
            $ok = true;
            break;
        case UPLOAD_ERR_NO_FILE:
            $us_array['error_msg'] = sprintf('No file sent.');
            break;
        case UPLOAD_ERR_INI_SIZE:
            $us_array['error_msg'] = sprintf('Exceeded ini filesize limit.');
            break;
        case UPLOAD_ERR_FORM_SIZE:
            $us_array['error_msg'] = sprintf('Exceeded form filesize limit.');
            break;
        default:
            $us_array['error_msg'] = sprintf("Unknown error");
            break;
        }
        
        if ( $ok )
        {
            $name = basename($_FILES['us_FileName']['name']);
            $dest = sprintf( "/var/www/html/uploads/%s", $name );
            $type = mime_content_type($_FILES['us_FileName']['tmp_name']);
            $us_array['info_msg'] = sprintf( "File uploaded: '%s', %d bytes, type '%s'", $_FILES['us_FileName']['tmp_name'], $_FILES['us_FileName']['size'], $type );
            
            if ( $type == "application/x-gzip" && strstr($name,"nimrod") !== false )
            {
                if ( move_uploaded_file( $_FILES['us_FileName']['tmp_name'], $dest ) )
                {   // success
                    $us_array['info_msg'] = sprintf( "Uploaded file moved to '%s'", $dest );
                    
                    chmod( $dest, 0666 );
                }
                else
                {
                    $us_array['error_msg'] = sprintf( "Failed to move uploaded file '%s' to '%s'", $_FILES['us_FileName']['tmp_name'], $dest );
                }
            }
            else
            {
                $us_array['error_msg'] = sprintf( "Invalid upload file type '%s'", $type );
            }
        }
    }
    else
    {
        $us_array['error_msg'] = sprintf( "File upload error" );
    }
}


if ( isset($_GET['Username']) )
{
	if ( ($line=$db->GetFields( 'users', 'us_Username', $us_array['us_Username'], "us_Username,us_Name,us_AuthLevel,us_Features,us_CardNumber,us_CardPin,us_CardEnabled,us_PinFailCount,
			us_TruckRego,us_TrailerRego,us_BillingName,us_BillingAddr1,us_BillingAddr2,us_BillingAddr3,us_BillingEmail,us_TruckTare,us_TrailerTare,us_TruckLoad,us_TrailerLoad" )) !== false )
	{	// success
		$us_array['us_Username'] = stripslashes($line[0]);
		$us_array['us_Name'] = stripslashes($line[1]);
		$us_array['us_AuthLevel'] = $line[2];
		$us_array['us_UF_Cameras'] = substr( $line[3], E_UF_CAMERAS, 1 );
		$us_array['us_UF_Upgrade'] = substr( $line[3], E_UF_UPGRADE, 1 );
		$us_array['us_UF_HomeCameras'] = substr( $line[3], E_UF_HOMECAMERAS, 1 );
		$us_array['us_UF_AccountingUser'] = substr( $line[3], E_UF_ACCOUNTINGUSER, 1 );
		$us_array['us_CardNumber'] = $line[4];
		$us_array['us_CardPin'] = $line[5];
		$us_array['us_CardPin2'] = $us_array['us_CardPin'];
		$us_array['us_CardEnabled'] = $line[6];
		$us_array['us_PinFailCount'] = $line[7];
		$us_array['us_TruckRego'] = $line[8];
		$us_array['us_TrailerRego'] = $line[9];
		$us_array['us_BillingName'] = $line[10];
		$us_array['us_BillingAddr1'] = $line[11];
		$us_array['us_BillingAddr2'] = $line[12];
		$us_array['us_BillingAddr3'] = $line[13];
		$us_array['us_BillingEmail'] = $line[14];
		$us_array['us_TruckTare'] = $line[15];
		$us_array['us_TrailerTare'] = $line[16];
		$us_array['us_TruckLoad'] = $line[17];
		$us_array['us_TrailerLoad'] = $line[18];
	}
	else
	{
		$us_array['error_msg'] = sprintf( "Failed to read users table for Username=%s", $us_array['us_Username'] );
	}
}
else if ( isset($_POST['DeleteUser']) )
{
	$us_array['us_Username'] = $_POST['us_Username']; 
	if ( $db->DeleteUser( $us_array['us_Username'] ) )
	{
		$us_array['info_msg'] = sprintf( "User deleted" );
		func_clear_us_array( $us_array );
		$new_user = false;
	}
	else 
	{
		$us_array['error_msg'] = sprintf( "Failed to delete user with Username=%s", $us_array['us_Username'] );
	}
}
else if ( isset($_GET['AddNewUser']) )
{
    func_clear_us_array( $us_array );
    $new_user = true;
}
else if ( isset($_POST['UploadFile']) )
{
    $us_array['info_msg'] .= sprintf( "UploadFile" );   
}
else if ( isset($_POST['NewUser']) || isset($_POST['UpdateUser']) )
{
	if ( isset($_POST['NewUser']) )
	{
	    $new_user = true;
	}
	
	if ( $new_user && $us_array['us_Password'] == "" && $us_array['us_AuthLevel'] > SECURITY_LEVEL_NONE )
	{
	    $us_array['error_msg'] = "The password and confirmation must be entered for new users.";
	}
	else if ( $new_user && $db->SelectUser($us_array['us_Username']) !== false )
	{
	    $us_array['error_msg'] = sprintf( "Username %s already exists in the database.", $us_array['us_Username'] );
	}
	else if ( $us_array['us_CardNumber'] != "" && $db->SelectCardNumber($us_array['us_CardNumber'], $us_array['us_Username']) !== false )
	{
	    $us_array['error_msg'] = sprintf( "The Card Number '%s' already exists in the database.", $us_array['us_CardNumber'] );
	}
	else if ( func_check_us_array( $us_array ) )
	{
	    if ( $us_array['us_CardEnabled'] == "Y" )
	        $us_array['us_PinFailCount'] = 0;
	    
	    $features = "";
	    $features .= $us_array['us_UF_Cameras'];
	    $features .= $us_array['us_UF_Upgrade'];
	    $features .= $us_array['us_UF_HomeCameras'];
		$features .= $us_array['us_UF_AccountingUser'];
	    $features .= "NNNNNN";
		if ( $db->UpdateUserTable( $new_user, $us_array['us_Username'], $us_array['us_Name'], $us_array['us_Password'], $us_array['us_AuthLevel'], $features, $us_array['us_CardNumber'], 
		    $us_array['us_CardPin'], $us_array['us_CardEnabled'], $us_array['us_PinFailCount'], $us_array['us_TruckRego'], $us_array['us_TrailerRego'], $us_array['us_BillingName'],
			$us_array['us_BillingAddr1'], $us_array['us_BillingAddr2'], $us_array['us_BillingAddr3'], $us_array['us_BillingEmail'], $us_array['us_TruckTare'], $us_array['us_TrailerTare'],
			$us_array['us_TruckLoad'], $us_array['us_TrailerLoad'] ) )
		{	// success
			//func_clear_us_array( $us_array );
			
		    if ( $us_array['us_Username'] == $_SESSION['us_Username'] )
		    { // update the auth details
		        $_SESSION['us_Features'] = $features;
		    }

			$us_array['info_msg'] = "User details saved successfully.";
			$new_user = false;
		}
		else
		{
			$us_array['error_msg'] = sprintf( "Failed to update User record %s", $us_array['us_Username'] );
		}
	}
}
else if ( isset($_POST['ClearUser']) )
{
	func_clear_us_array( $us_array );
}


$user_list = $db->ReadUsers();


?>

<div class="container" style="margin-top:30px">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-5">
			<h3>User Management</h3>
		</div>
		<div class="col-sm-1">
			<a href='#userslist' data-toggle='collapse' class='small'><i>Hide/Show</i></a>
        </div>
    </div>

	<div id="userslist" class="collapse <?php ($new_user || $us_array['us_Username'] != "" ? printf("") : printf("show"))?>">

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-6">
    		<table class='table table-striped'>
    		<thead class="thead-light">
              <tr>
              <th>Username</th>
              <th>Name</th>
              <th>Access</th>
              </tr>
            </thead>
 			<tbody>
 			
            <?php 
            foreach ( $user_list as $info )
            {
                if ( $_SESSION['us_AuthLevel'] != SECURITY_LEVEL_ADMIN && $_SESSION['us_Username'] != $info['us_Username'] )
                    continue;
                
                printf( "<tr>" );
                
                printf( "<td><a href='?Username=%s'>%s</a></td>", $info['us_Username'], $info['us_Username'] );
                printf( "<td><a href='?Username=%s'>%s</a></td>", $info['us_Username'], $info['us_Name'] );
                printf( "<td><a href='?Username=%s'>%s</a></td>", $info['us_Username'], func_get_security_level_desc($info['us_AuthLevel']) );
                
                printf( "</tr>" );
            }
            ?>
			</tbody>
			</table>
			
			<?php 
            if ( $_SESSION['us_AuthLevel'] == SECURITY_LEVEL_ADMIN )
            {
                printf( "<p><a href='?AddNewUser'>Add New User</a></p>" );
            }
            ?>

		</div>

	</div>	<!-- end of row -->
	</div>
	
	<?php
    if ( $us_array['us_Username'] != "" || $new_user )
    {
    ?>

    <!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-6">
			<h3>User Detail</h3>

            <?php 
            if ( $us_array['error_msg'] != "" )
                printf( "<p class='text-danger'>%s</p>", $us_array['error_msg'] );
            else if ( $us_array['info_msg'] != "" )
                printf( "<p class='text-info'>%s</p>", $us_array['info_msg'] );
            

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_Username'>Username: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_Username' id='us_Username' size='25' value='%s'> ", $us_array['us_Username'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_Name'>Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_Name' id='us_Name' size='25' value='%s'> ", $us_array['us_Name'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_Password'>Password: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<input type='password' class='form-control' name='us_Password' id='us_Password' size='12' value='%s'> ", $us_array['us_Password'] );
    		printf( "</div>" );

			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_Password2'>Confirmation: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<input type='password' class='form-control' name='us_Password2' id='us_Password2' size='12' value='%s'> ", $us_array['us_Password2'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_AuthLevel'>Security Level: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
            printf( "<select class='form-control custom-select' size='1' name='us_AuthLevel' id='us_AuthLevel' %s>", func_disabled_non_admin() );
            printf( "<option ></option>" );
            printf( "<option %s>%d. None</option>", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_NONE ? "selected" : ""), SECURITY_LEVEL_NONE );
            printf( "<option %s>%d. Guest</option>", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_GUEST ? "selected" : ""), SECURITY_LEVEL_GUEST );
            printf( "<option %s>%d. Admin</option>", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_ADMIN ? "selected" : ""), SECURITY_LEVEL_ADMIN );
            printf( "</select>" );
    		printf( "</div>" );
			if ( $new_user )
			{
	    		printf( "<div class='col-sm-4'>" );
				printf( "Password is reqd if secuity level > NONE");
    			printf( "</div>" );
			}
    		printf( "</div>" );

    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_Name'>Card Number: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<input type='text' class='form-control' name='us_CardNumber' id='us_CardNumber' size='8' value='%s'> ", $us_array['us_CardNumber'] );
			printf( "</div>");
    		printf( "<div class='col-sm-3'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='us_CardEnabled' id='us_CardEnabled' %s> ", ($us_array['us_CardEnabled'] == "Y" ? "checked" : "") );
    		printf( "Card Enabled" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_CardPin'>PIN: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='password' class='form-control' name='us_CardPin' id='us_CardPin' size='6' value='%s'> ", $us_array['us_CardPin'] );
    		printf( "</div>" );
    		
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_CardPin2'>Confirmation: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='password' class='form-control' name='us_CardPin2' id='us_CardPin2' size='6' value='%s'> ", $us_array['us_CardPin2'] );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_PinFailCount'>Pin Fail Count: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-2'>" );
    		printf( "<input type='test' class='form-control' name='us_PinFailCount' id='us_PinFailCount' size='6' value='%s'> ", $us_array['us_PinFailCount'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_TruckRego'>Vehicle Rego/Id: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-5'>" );
    		printf( "<input type='text' class='form-control' name='us_TruckRego' id='us_TruckRego' size='10' value='%s'> ", $us_array['us_TruckRego'] );
    		printf( "</div>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "15 Characters max" );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_TruckTare'>Vehicle Tare KG: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-4'>" );
    		printf( "<input type='text' class='form-control' name='us_TruckTare' id='us_TruckTare' size='6' value='%s'>", $us_array['us_TruckTare'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_TruckLoad'>Vehicle Load KG: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col-sm-4'>" );
    		printf( "<input type='text' class='form-control' name='us_TruckLoad' id='us_TruckLoad' size='6' value='%s'>", $us_array['us_TruckLoad'] );
    		printf( "</div>" );
			printf( "<div class='col-sm-3'>" );
    		printf( "Max Weight %d KG", $us_array['us_TruckTare'] + $us_array['us_TruckLoad'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_BillingName'>Billing Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_BillingName' id='us_BillingName' size='30' value='%s'> ", $us_array['us_BillingName'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_BillingAddr1'>Billing Address #1: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_BillingAddr1' id='us_BillingAddr1' size='30' value='%s'> ", $us_array['us_BillingAddr1'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_BillingAddr2'>Billing Address #2: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_BillingAddr2' id='us_BillingAddr2' size='30' value='%s'> ", $us_array['us_BillingAddr2'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_BillingAddr3'>Billing Address #3: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_BillingAddr3' id='us_BillingAddr3' size='30' value='%s'> ", $us_array['us_BillingAddr3'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='row'>" ); 
			printf( "<div class='col-sm-3'>" );
    		printf( "<label for='us_BillingEmail'>Billing Email: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_BillingEmail' id='us_BillingEmail' size='30' value='%s'> ", $us_array['us_BillingEmail'] );
    		printf( "</div>" );
    		printf( "</div>" );


    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "</div>" );
    		printf( "<div class='col form-check'>" );
    		printf( "<label class='form-check-label'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='us_UF_HomeCameras' id='us_UF_HomeCameras' %s> ", ($us_array['us_UF_HomeCameras'] == "Y" ? "checked" : "") );
    		printf( "Show Cameras on Home Page</label>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "</div>" );
    		printf( "<div class='col form-check'>" );
    		printf( "<label class='form-check-label'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='us_UF_Cameras' id='us_UF_Cameras' %s> ", ($us_array['us_UF_Cameras'] == "Y" ? "checked" : "") );
    		printf( "Camera Access</label>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "</div>" );
    		printf( "<div class='col form-check'>" );
    		printf( "<label class='form-check-label'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='us_UF_Upgrade' id='us_UF_Upgrade' %s> ", ($us_array['us_UF_Upgrade'] == "Y" ? "checked" : "") );
    		printf( "Software Upgrade</label>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		printf( "<div class='row'>" );
    		printf( "<div class='col-sm-3'>" );
    		printf( "</div>" );
    		printf( "<div class='col form-check'>" );
    		printf( "<label class='form-check-label'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='us_UF_AccountingUser' id='us_UF_AccountingUser' %s> ", ($us_array['us_UF_AccountingUser'] == "Y" ? "checked" : "") );
    		printf( "Copy invoices to this user</label>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
    		if ( $us_array['us_UF_Upgrade'] == 'Y' )
    		{
        		printf( "<div class='row'>" );
        		printf( "<div class='col-sm-3'>" );
        		printf( "</div>" );
        		printf( "<div class='col'>" );
        		printf( "<input type='hidden' name='MAX_FILE_SIZE' value='4096000'>" );
        		printf( "<input type='file' class='form-control-file border' name='us_FileName' id='us_FileName'>" );
        		printf( "</div>" );
        		printf( "</div>" );
    		}
        		
    		printf( "<div class='row mb-2 mt-2'>" ); 
			printf( "<div class='col'>" );
            printf( "<button type='submit' class='btn btn-outline-dark' name='UpdateUser' id='UpdateUser' value='Update' %s>Update</button>", ($us_array['us_Username'] == "" ? "disabled" : "") );
            printf( "&nbsp;&nbsp;&nbsp;" );
            printf( "<button type='submit' class='btn btn-outline-dark' name='NewUser' id='NewUser' value='New' %s>New</button>", ($us_array['us_Username'] != "" || func_disabled_non_admin() ? "disabled" : "") );
            printf( "&nbsp;&nbsp;&nbsp;" );
            $onclick = sprintf( "return confirm(\"Are you sure you want to delete user with username %s ?\")", $us_array['us_Username'] );
            printf( "<button type='submit' class='btn btn-outline-dark' name='DeleteUser' id='DeleteUser' value='Delete' onclick='%s' %s>Delete</button>", $onclick, 
                ($us_array['us_Username'] == "" || func_disabled_non_admin() != "" || $us_array['us_Username'] == $_SESSION['us_Username'] ? "disabled" : "") );
            printf( "&nbsp;&nbsp;&nbsp;" );
            printf( "<button type='submit' class='btn btn-outline-dark' name='ClearUser' id='ClearUser' value='Clear'>Clear</button>" );
            if ( $us_array['us_UF_Upgrade'] == "Y" )
            {
                printf( "&nbsp;&nbsp;&nbsp;" );
                printf( "<button type='submit' class='btn btn-outline-dark' name='UploadFile' id='UploadFile' value='Upload File'>Upload File</button>" );
            }
            printf( "</div>" );
    		printf( "</div>" );
            ?>

		</div>
	</div>	<!-- end of row -->

	<?php 
    }
    ?>
    
</div>


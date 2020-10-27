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
	$us_array['error_msg'] = "";
	$us_array['info_msg'] = "";
}

function func_check_us_array( &$us_array )
{
	$us_array['error_msg'] = "";
	$us_array['info_msg'] = "";
	
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
	    


if ( isset($_GET['Username']) )
{
	if ( ($line=$db->GetFields( 'users', 'us_Username', $us_array['us_Username'], "us_Username,us_Name,us_AuthLevel,us_Features" )) !== false )
	{	// success
		$us_array['us_Username'] = stripslashes($line[0]);
		$us_array['us_Name'] = stripslashes($line[1]);
		$us_array['us_AuthLevel'] = $line[2];
		$us_array['us_UF_Cameras'] = substr( $line[3], E_UF_CAMERAS, 1 );
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
else if ( isset($_POST['NewUser']) || isset($_POST['UpdateUser']) )
{
	if ( isset($_POST['NewUser']) )
	{
	    $new_user = true;
	}
	
	if ( $new_user && $us_array['us_Password'] == "" )
	{
	    $us_array['error_msg'] = "The password and confirmation must be entered for new users.";
	}
	else if ( $new_user && $db->SelectUser($us_array['us_Username']) !== false )
	{
	    $us_array['error_msg'] = sprintf( "Username %s already exists in the database.", $us_array['us_Username'] );
	}
	else if ( func_check_us_array( $us_array ) )
	{
	    $features = $us_array['us_UF_Cameras'];
	    $features .= "NNNNNNNNN";
		if ( $db->UpdateUserTable( $new_user, $us_array['us_Username'], $us_array['us_Name'], $us_array['us_Password'], $us_array['us_AuthLevel'], $features ) )
		{	// success
			func_clear_us_array( $us_array );
			
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

		<div class="col-sm-5">
			<h3>User Detail</h3>

            <?php 
            if ( $us_array['error_msg'] != "" )
                printf( "<p class='text-danger'>%s</p>", $us_array['error_msg'] );
            else if ( $us_array['info_msg'] != "" )
                printf( "<p class='text-info'>%s</p>", $us_array['info_msg'] );
            

			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='us_Username'>Username: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_Username' id='us_Username' size='25' value='%s'> ", $us_array['us_Username'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='us_Name'>Name: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='text' class='form-control' name='us_Name' id='us_Name' size='25' value='%s'> ", $us_array['us_Name'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='us_Password'>Password: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='password' class='form-control' name='us_Password' id='us_Password' size='12' value='%s'> ", $us_array['us_Password'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='us_Password2'>Confirmation: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
    		printf( "<input type='password' class='form-control' name='us_Password2' id='us_Password2' size='12' value='%s'> ", $us_array['us_Password2'] );
    		printf( "</div>" );
    		printf( "</div>" );

			printf( "<div class='form-row'>" ); 
			printf( "<div class='col'>" );
    		printf( "<label for='us_AuthLevel'>Security Level: </label>" );
    		printf( "</div>" );
    		printf( "<div class='col'>" );
            printf( "<select class='form-control custom-select' size='1' name='us_AuthLevel' id='us_AuthLevel' %s>", func_disabled_non_admin() );
            printf( "<option ></option>" );
            printf( "<option %s>%d. None</option>", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_NONE ? "selected" : ""), SECURITY_LEVEL_NONE );
            printf( "<option %s>%d. Guest</option>", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_GUEST ? "selected" : ""), SECURITY_LEVEL_GUEST );
            printf( "<option %s>%d. Admin</option>", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_ADMIN ? "selected" : ""), SECURITY_LEVEL_ADMIN );
            printf( "</select>" );
    		printf( "</div>" );
    		printf( "</div>" );

    		printf( "<div class='form-row'>" );
    		//printf( "<div class='col'>" );
    		//printf( "</div>" );
    		printf( "<div class='col form-check'>" );
    		printf( "<label class='form-check-label'>" );
    		printf( "<input type='checkbox' class='form-check-input' name='us_UF_Cameras' id='us_UF_Cameras' %s> ", ($us_array['us_UF_Cameras'] == "Y" ? "checked" : "") );
    		printf( "Camera Access</label>" );
    		printf( "</div>" );
    		printf( "</div>" );
    		
			printf( "<div class='form-row mb-2 mt-2'>" ); 
			printf( "<p>" );
            printf( "<button type='submit' class='btn btn-outline-dark' name='UpdateUser' id='UpdateUser' value='Update' %s>Update</button>", ($us_array['us_Username'] == "" ? "disabled" : "") );
            printf( "&nbsp;&nbsp;&nbsp;" );
            printf( "<button type='submit' class='btn btn-outline-dark' name='NewUser' id='NewUser' value='New' %s>New</button>", ($us_array['us_Username'] != "" || func_disabled_non_admin() ? "disabled" : "") );
            printf( "&nbsp;&nbsp;&nbsp;" );
            $onclick = sprintf( "return confirm(\"Are you sure you want to delete user with username %s ?\")", $us_array['us_Username'] );
            printf( "<button type='submit' class='btn btn-outline-dark' name='DeleteUser' id='DeleteUser' value='Delete' onclick='%s' %s>Delete</button>", $onclick, 
                ($us_array['us_Username'] == "" || func_disabled_non_admin() != "" || $us_array['us_Username'] == $_SESSION['us_Username'] ? "disabled" : "") );
            printf( "&nbsp;&nbsp;&nbsp;" );
            printf( "<button type='submit' class='btn btn-outline-dark' name='ClearUser' id='ClearUser' value='Clear'>Clear</button>" );
    		printf( "</p>" );
    		printf( "</div>" );
            ?>

		</div>
	</div>	<!-- end of row -->

	<?php 
    }
    ?>
    
</div>


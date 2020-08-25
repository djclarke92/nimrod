<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------


function func_clear_us_array( &$us_array )
{
	$us_array['us_Username'] = "";
	$us_array['us_Name'] = "";
	$us_array['us_Password'] = "";
	$us_array['us_Password2'] = "";
	$us_array['us_AuthLevel'] = "0";
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
	    


if ( isset($_GET['Username']) )
{
	if ( ($line=$db->GetFields( 'users', 'us_Username', $us_array['us_Username'], "us_Username,us_Name,us_AuthLevel" )) !== false )
	{	// success
		$us_array['us_Username'] = stripslashes($line[0]);
		$us_array['us_Name'] = stripslashes($line[1]);
		$us_array['us_AuthLevel'] = $line[2];
	}
	else
	{
		$us_array['error_msg'] = sprintf( "Failed to read users table for Username=%s", $us_array['us_Username'] );
	}
}
else if ( isset($_GET['DeleteUser']) )
{
	$us_array['us_Username'] = $_GET['DeleteUser']; 
	if ( $db->DeleteUser( $us_array['us_Username'] ) )
	{
		$us_array['info_msg'] = sprintf( "User deleted" );
	}
	else 
	{
		$us_array['error_msg'] = sprintf( "Failed to delete user with Username=%s", $us_array['us_Username'] );
	}
}
else if ( isset($_POST['NewUser']) || isset($_POST['UpdateUser']) )
{
    $newuser = false;
	if ( isset($_POST['NewUser']) )
	{
	    $newuser = true;
	}
	
	if ( $newuser && $us_array['us_Password'] == "" )
	{
	    $us_array['error_msg'] = "The password and confirmation must be entered for new users.";
	}
	else if ( $newuser && $db->SelectUser($us_array['us_Username']) !== false )
	{
	    $us_array['error_msg'] = sprintf( "Username %s already exists in the database.", $us_array['us_Username'] );
	}
	else if ( func_check_us_array( $us_array ) )
	{
		if ( $db->UpdateUserTable( $newuser, $us_array['us_Username'], $us_array['us_Name'], $us_array['us_Password'], $us_array['us_AuthLevel'] ) )
		{	// success
			func_clear_us_array( $us_array );
			
			$us_array['info_msg'] = "New user saved successfully.";
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


printf( "<tr>" );
printf( "<td>" );
printf( "<b>User Management</b>" );
printf( "</td>" );
printf( "<td colspan='3' align='right'>" );
if ( $us_array['error_msg'] != "" )
	printf( "<div class='style-error'>%s</div>", $us_array['error_msg'] );
else if ( $us_array['info_msg'] != "" )
	printf( "<div class='style-info'>%s</div>", $us_array['info_msg'] );
printf( "</td>" );
printf( "</tr>" );


printf( "<tr valign='top'>" );
printf( "<td colspan='2'>" );
	
$user_list = $db->ReadUsers();
	
$table_info = array();
$table_info[] = array( 120, "Username" );
$table_info[] = array( 120, "Name" );
$table_info[] = array( 50, "Auth Level" );
$table_info[] = array( 44, "&nbsp;" );
func_html_header_table( $table_info );

printf( "<table cellspacing='0' cellpadding='2' class='style-table' border='0' width='%d' height='310'>",
func_html_get_table_width( $table_info ) );
func_html_set_col_widths( $table_info );
printf( "<tbody class='style-tbody'>" );
$count = 0;
foreach ( $user_list as $info )
{
	$style = "alternateRow";
	if ( $info['us_Username'] == $us_array['us_Username'] )
		$style = "selectedRow";
	else if ( ($count % 2) == 0 )
		$style = "normalRow";
	printf( "<tr class='%s'>", $style );
	printf( "<td><a href='index.php?Username=%s' class='style-tablelink'>%s</a></td>",
		$info['us_Username'], $info['us_Username'] );
	printf( "<td align='center'><a href='index.php?Username=%s' class='style-tablelink'>%s</a></td>",
		$info['us_Username'], $info['us_Name'] );
	printf( "<td align='center'><a href='index.php?Username=%s' class='style-tablelink'>%s</a></td>",
		$info['us_Username'], $info['us_AuthLevel'] );
	
	$onclick = sprintf( "return confirm(\"Are you sure you want to delete user with username %s ?\")", $info['us_Username'] );
	printf( "<td><a href='index.php?DeleteUsername=%d' onclick='%s;'>delete</a></td>", $info['us_Username'], $onclick );
	printf( "</tr>" );
	$count += 1;
}

func_html_extend_table( $table_info, $count, 16 );
printf( "</tbody>" );
printf( "</table>" );

printf( "<br><input type='submit' name='Refresh' value='Refresh'>" );
printf( "</td>" );
	
printf( "<td colspan='2'>" );
	
printf( "<table>" );
	
printf( "<tr>" );
printf( "<td><b>Username</b></td>" );
printf( "<td><input type='text' size='50' name='us_Username' value='%s'></td>", $us_array['us_Username'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Name</b></td>" );
printf( "<td><input type='text' size='50' name='us_Name' value='%s'></td>", $us_array['us_Name'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Password</b></td>" );
printf( "<td><input type='password' size='15' name='us_Password' value='%s'></td>", $us_array['us_Password'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Confirmation</b></td>" );
printf( "<td><input type='password' size='15' name='us_Password2' value='%s'></td>", $us_array['us_Password2'] );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td><b>Security Level</b></td>" );
printf( "<td><select size='1' name='us_AuthLevel'>" );
printf( "<option >" );
printf( "<option %s>%d. None", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_NONE ? "selected" : ""), SECURITY_LEVEL_NONE );
printf( "<option %s>%d. Guest", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_GUEST ? "selected" : ""), SECURITY_LEVEL_GUEST );
printf( "<option %s>%d. Admin", ($us_array['us_AuthLevel'] == SECURITY_LEVEL_ADMIN ? "selected" : ""), SECURITY_LEVEL_ADMIN );
printf( "</select>" );
printf( "<td>" );
printf( "</tr>" );

printf( "<tr>" );
printf( "<td colspan='2'>" );
printf( "<input type='submit' name='UpdateUser' value='Update'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='NewUser' value='New'>" );
printf( "&nbsp;&nbsp;&nbsp;" );
printf( "<input type='submit' name='ClearUser' value='Clear'>" );
printf( "</td>" );
printf( "</tr>" );

printf( "</table>" );

printf( "</td>" );
printf( "</tr>" );




?>
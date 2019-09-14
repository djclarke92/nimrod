<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Dave Clarke
//
//--------------------------------------------------------------------------------------




function func_get_email_styles()
{
	$style = "";
	$style .= "<style type=\"text/css\">";
	$style .= "* { font-family: Verdana, Arial, Helvetica, sans-serif;}";
	$style .= "style-normal { font-family: Verdana, Arial, Helvetica, sans-serif;}";
	$style .= "</style>";
	
	return $style;
}

function func_send_mail( $to, $body, $subject, $fromaddress, $fromname, $attachments = false )
{
	$eol="\n";
	$mime_boundary = md5(time());
	
	# Common Headers
	$headers = "";
	$headers .= "From: ".$fromname."<".$fromaddress.">".$eol;
	$headers .= "Reply-To: ".$fromname."<".$fromaddress.">".$eol;
	$headers .= "Return-Path: ".$fromname."<".$fromaddress.">".$eol;    // these two to set reply address
	$headers .= "Message-ID: <".time()."-".$fromaddress.">".$eol;
	$headers .= "X-Mailer: PHP v".phpversion().$eol;          // These two to help avoid spam-filters

	# Boundry for marking the split & Multitype Headers
	$headers .= "MIME-Version: 1.0".$eol;
	$headers .= "Content-Type: multipart/mixed; boundary=\"".$mime_boundary."\"".$eol.$eol;

	# Open the first part of the mail
	$msg = "--".$mime_boundary.$eol;
 
	$htmlalt_mime_boundary = $mime_boundary."_htmlalt"; //we must define a different MIME boundary for this section
	# Setup for text OR html -
	$msg .= "Content-Type: multipart/alternative; boundary=\"".$htmlalt_mime_boundary."\"".$eol.$eol;

	# Text Version
	$msg .= "--".$htmlalt_mime_boundary.$eol;
	$msg .= "Content-Type: text/plain; charset=us-ascii".$eol;
	$msg .= "Content-Transfer-Encoding: 8bit".$eol.$eol;
	$msg .= wordwrap(strip_tags(str_replace("<br>", "\n", substr($body, (strpos($body, "<body>")+6)))),70).$eol.$eol;

	# HTML Version
	$msg .= "--".$htmlalt_mime_boundary.$eol;
	$msg .= "Content-Type: text/html; charset=us-ascii".$eol;
//	$msg .= "Content-Transfer-Encoding: 8bit".$eol.$eol;
//	$msg .= $body.$eol.$eol;
	$msg .= "Content-Transfer-Encoding: base64".$eol.$eol;
	$msg .= wordwrap(base64_encode($body),70,"\n",true).$eol.$eol;

	//close the html/plain text alternate portion
	$msg .= "--".$htmlalt_mime_boundary."--".$eol.$eol;

	if ($attachments !== false)
	{
		for($i=0; $i < count($attachments); $i++)
		{
			if (is_file($attachments[$i]["file"]))
			{  
				# File for Attachment
				$file_name = substr($attachments[$i]["file"], (strrpos($attachments[$i]["file"], "/")+1));
       
				$handle=fopen($attachments[$i]["file"], 'rb');
				$f_contents=fread($handle, filesize($attachments[$i]["file"]));
				$f_contents=chunk_split(base64_encode($f_contents));    //Encode The Data For Transition using base64_encode();
				$f_type=filetype($attachments[$i]["file"]);
				fclose($handle);
       
				# Attachment
				$msg .= "--".$mime_boundary.$eol;
				$msg .= "Content-Type: ".$attachments[$i]["content_type"]."; name=\"".$file_name."\"".$eol;  // sometimes i have to send MS Word, use 'msword' instead of 'pdf'
				$msg .= "Content-Transfer-Encoding: base64".$eol;
				$msg .= "Content-Description: ".$file_name.$eol;
				$msg .= "Content-Disposition: attachment; filename=\"".$file_name."\"".$eol.$eol; // !! This line needs TWO end of lines !! IMPORTANT !!
				$msg .= $f_contents.$eol.$eol;
			}
		}
	}

	# Finished
	$msg .= "--".$mime_boundary."--".$eol.$eol;  // finish with two eol's for better security. see Injection.
 
	# SEND THE EMAIL
	ini_set( "sendmail_from", $fromaddress );  // the INI lines are to force the From Address to be used !
	$mail_sent = mail( $to, $subject, $msg, $headers );
 
	ini_restore( "sendmail_from" );
 
	return $mail_sent;
}

function func_email_category_form( $db, $title, $msg )
{
	$emsg = "";
	
	$admin_list = $db->GetUserList( true );
	
	// send email to site admin with totals
	foreach ( $admin_list as $user )
	{
		$dest = sprintf( "%s", $user['us_Email'] );
		$subject = sprintf( "%s", $title );
		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
	
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		$message .= sprintf( "User '%s' with email address <a href=\"mailto:%s\">%s</a> has asked a question.<br><br>", 
				$_SESSION['user_name'], $_SESSION['user_email'], $_SESSION['user_email'] );
		$message .= sprintf( "%s<br>", $msg );
		$message .= sprintf( "Have a great day !<br>%s", $_SESSION['website_admin'] );
		
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );
		
		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
		}
		else
		{	// error
			$emsg .= sprintf( "Failed to send email to admin user '%s'.\n", $dest );
		}
	}
	
	return $emsg;
}

function func_email_forgotten_password( $user, $pwd )
{
	$rc = false;
	
	$dest = sprintf( "%s,%s", $user['us_Email'], $_SESSION['website_email'] );
	$subject = sprintf( "Password for %s", $_SESSION['website_name'] );
			
	$message = "";
	$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
	$message .= sprintf( "<html><title>%s</title>", $subject );
	$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
	$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
	$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
	if ( $_SESSION['website_logo'] != "" )
		$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
	else if ( $_SESSION['website_image'] != "" )
		$message .= sprintf( "%s<br><br>", func_get_website_image() );
	$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
	
	$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
	$message .= sprintf( "You have requested an email to \"%s\" reminding you of your password to the %s web site.<br><br>", $user['us_Email'], $_SESSION['website_name'] );
	$message .= sprintf( "Your password to the %s web site is \"%s\"<br><br>", $_SESSION['website_name'], $pwd );
	$message .= sprintf( "Have a great day !<br>%s", $_SESSION['website_admin'] );
	
	$message .= sprintf( "</span></td></tr></table></body>" );
	$message .= sprintf( "</html>" );
	
	if ( func_send_mail( $dest, $message, $_SESSION['website_name'], $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
	{	// success;
		$rc = true;
	}
	
	return $rc;
}

function func_email_new_product( $db, $pr_array )
{
	$user_list = $db->GetUserList( false );
	$admin_list = $db->GetUserList( true );
	$category_description = $db->GetCategoryDescription( $pr_array['category_no'] );
						
	$count = 0;
	$ecount = 0;
	$emsg = "";
	foreach ( $user_list as $user )
	{
		if ( substr( $user['us_Interests'], INTERESTS_NEW_PRODUCTS, 1 ) == "Y" )
		{	// this user wants to know about new products
			$subject = sprintf( "New Products at %s", $_SESSION['website_name'] );
			$dest = sprintf( "%s", $user['us_Email'] );
			$href = sprintf( "http://%s/%s?ProductNo=%d", $_SESSION['website_domain'], WEB_ROOT_PATH, $pr_array['product_no'] );
								
			$message = "";
			$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
			$message .= sprintf( "<html><title>%s</title>", $subject );
			$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
			$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
			$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
			if ( $_SESSION['website_logo'] != "" )
				$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
			else if ( $_SESSION['website_image'] != "" )
				$message .= sprintf( "%s<br><br>", func_get_website_image() );
			$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
			$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
			$message .= sprintf( "You have requested an email whenever new products are added to the <a href=\"http://%s\">%s</a> web site.<br><br>", $_SESSION['website_domain'], $_SESSION['website_name'] );
			$message .= sprintf( "A new item has been added to the '%s' category, called '<a href=\"%s\">%s</a>'.<br><br>", 
					$category_description, $href, $pr_array['title'] );

			$fn = $db->GetFileName( $pr_array['product_no'], "Y", 0 );
			$extn = func_get_file_extn( $fn );
			if ( func_is_image( $extn ) )
			{
				$ww = 110;
				$hh = $ww * 3 / 4;
				$fn = sprintf( "%s/%sdownloads/%s", $_SERVER['DOCUMENT_ROOT'], WEB_ROOT_PATH, $fn );
				$message .= sprintf( "<a href=\"%s\"><img src=\"http://%s/%sfiles/thumbnail.php?src=%s\" width=\"%d\" height=\"%d\"></a><br><br>", 
						$href, $_SESSION['website_domain'], WEB_ROOT_PATH, $fn, $ww, $hh );
			}
				
			$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
			$message .= sprintf( "</span></td></tr></table></body>" );
			$message .= sprintf( "</html>" );
								
			if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
			{	// success
				$count += 1;
			}
			else
			{	// error
				$ecount += 1;
				$emsg .= sprintf( "Failed to send email to '%s'.\n", $user['us_Email'] );
			}
		}
	}
						
	// send email to site admin with totals
	foreach ( $admin_list as $user )
	{
		$dest = sprintf( "%s", $user['us_Email'] );
		$subject = "New Product Email";
		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
	
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		$message .= sprintf( "A new product has been added in the '%s' category, called %s.<br><br>", 
				$category_description, $pr_array['title'] );
		$message .= sprintf( "An email has been sent to %d registered users about this new product.<br><br>", $count );
		if ( $ecount != 0 )
		{
			$message .= sprintf( "An error occurred while sending %d email(s):<br>%s<br>", $ecount, $emsg );
		}
		$message .= sprintf( "Have a great day !<br>%s", $_SESSION['website_admin'] );
		
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );
		
		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
		}
		else
		{	// error
			$emsg .= sprintf( "Failed to send summary email to admin user '%s'.\n", $dest );
		}
	}
	
	return $emsg;
}

function func_email_test_product( $db, $pr_array )
{
	$category_description = $db->GetCategoryDescription( $pr_array['category_no'] );
		
	$subject = sprintf( "New Products at %s", $_SESSION['website_name'] );
	$dest = sprintf( "%s", $_SESSION['website_email'] );
	$href = sprintf( "http://%s/%s?ProductNo=%d", $_SESSION['website_domain'], WEB_ROOT_PATH, $pr_array['product_no'] );
		
	$message = "";
	$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
	$message .= sprintf( "<html><title>%s</title>", $subject );
	$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
	$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
	$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", 
		func_show_bgcolor(), func_show_bgimage() );
	if ( $_SESSION['website_logo'] != "" )
		$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
	else if ( $_SESSION['website_image'] != "" )
		$message .= sprintf( "%s<br><br>", func_get_website_image() );
	$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
	$message .= sprintf( "Hi,<br><br>" );
	$message .= sprintf( "You have requested a test email for this product on the <a href=\"http://%s\">%s</a> web site.<br><br>", 
		$_SESSION['website_domain'], $_SESSION['website_name'] );
	$message .= sprintf( "A new item has been added to the '%s' category, called '<a href=\"%s\">%s</a>'.<br><br>", 
		$category_description, $href, $pr_array['title'] );

	$fn = $db->GetFileName( $pr_array['product_no'], "Y", 0 );
	$extn = func_get_file_extn( $fn );
	if ( func_is_image( $extn ) )
	{
		$ww = 110;
		$hh = $ww * 3 / 4;
		$fn = sprintf( "%s/%sdownloads/%s", $_SERVER['DOCUMENT_ROOT'], WEB_ROOT_PATH, $fn );
		$message .= sprintf( "<a href=\"%s\"><img src=\"http://%s/%sfiles/thumbnail.php?src=%s\" width=\"%d\" height=\"%d\"></a><br><br>", 
				$href, $_SESSION['website_domain'], WEB_ROOT_PATH, $fn, $ww, $hh );
	}
				
	$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
	$message .= sprintf( "</span></td></tr></table></body>" );
	$message .= sprintf( "</html>" );
								
	if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
	{	// success
		$pr_email_msg = sprintf( "Email: sent to '%s'", $dest );
	}
	else
	{	// error
		$pr_email_msg = sprintf( "Email: send failed to '%s'", $dest );
	}
	
	return $pr_email_msg;
}

function func_send_specials_email( $db, $user_list, $pr_list, &$msg )
{
	$rc = true;
	$msg = "";
	foreach( $user_list as $user )
	{
		$subject = sprintf( "Specials at %s", $_SESSION['website_name'] );
		$dest = sprintf( "%s", $user['us_Email'] );
		
		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", 
			func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		
		$message .= sprintf( "You have requested an email from <a href=\"http://%s\">%s</a> whenever the list of specials changes.<br><br>", 
			$_SESSION['website_domain'], $_SESSION['website_name'] );
	
		foreach( $pr_list as $pr_array )
		{
			$href = sprintf( "http://%s/%s?ProductNo=%d", $_SESSION['website_domain'], WEB_ROOT_PATH, $pr_array['pr_ProductNo'] );
			
			$price = $pr_array['pr_Price'] - $pr_array['pr_Discount'];
			$until = "";
			if ( $pr_array['pr_DiscountEndDate'] != "" )
				$until = sprintf( "until %s", $pr_array['pr_DiscountEndDate'] );
				
			$message .= sprintf( "'<a href=\"%s\">%s</a>' is now on special at $%.2f %s (The normal price $%.2f).<br><br>", 
				$href, $pr_array['pr_Title'], $price, $until, $pr_array['pr_Price'] );

			$fn = $db->GetFileName( $pr_array['pr_ProductNo'], "Y", 0 );
			$extn = func_get_file_extn( $fn );
			if ( func_is_image( $extn ) )
			{
				$ww = 110;
				$hh = $ww * 3 / 4;
				$fn = sprintf( "%s/%sdownloads/%s", $_SERVER['DOCUMENT_ROOT'], WEB_ROOT_PATH, $fn );
				$message .= sprintf( "<a href=\"%s\"><img src=\"http://%s/%sfiles/thumbnail.php?src=%s\" width=\"%d\" height=\"%d\"></a><br><br>", 
						$href, $_SESSION['website_domain'], WEB_ROOT_PATH, $fn, $ww, $hh );
			}
			$message .= sprintf( "<br><br>" );
		}
		
		$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );
								
		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
			$msg .= sprintf( "Email: sent to '%s'. ", $dest );
		}
		else
		{	// error
			$rc = false;
			$msg .= sprintf( "Email: failed for '%s'. ", $dest );
		}
	}
	
	return $rc;
}

function func_email_forum_reply( $user_list, $news_no, $news_title, $user_name, $ne_type, $status )
{
	if ( $news_title == "" )
		$news_title = "title";
		
	$count = 0;
	$ecount = 0;
	$emsg = "";
	foreach ( $user_list as $user )
	{
		$subject = sprintf( "Forum Reply at %s", $_SESSION['website_name'] );
		$dest = sprintf( "%s", $user['us_Email'] );
		$href = sprintf( "http://%s/%s?ShowNewsNo=%d&NewsType=%s", $_SESSION['website_domain'], WEB_ROOT_PATH, $news_no, $ne_type );
								
		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		$message .= sprintf( "You have requested an email whenever your forum questions are updated on the <a href=\"http://%s/%s\">%s</a> web site.<br><br>", 
			$_SESSION['website_domain'], WEB_ROOT_PATH, $_SESSION['website_name'] );
		$message .= sprintf( "Forum question '<a href=\"%s\">%s</a>' has just been updated by '%s'.<br><br>", 
					$href, $news_title, $user_name );
		
		if ( $status == "H" )
		{	// reply is held
			$message .= sprintf( "This %s is held until a moderator releases it for publication.<br><br>", ($ne_type == NEWS_TYPE_NEWS ? "news item" : "forum topic") );
		}

		$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );
								
		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
			$count += 1;
		}
		else
		{	// error
			$ecount += 1;
			$emsg .= sprintf( "Failed to send email to '%s'.\n", $user['us_Email'] );
		}
	}
	
	return $emsg;
}

function func_email_admin_new_user( $user_list, $new_email, $new_name )
{
	$count = 0;
	$ecount = 0;
	$emsg = "";
	foreach ( $user_list as $user )
	{
		$subject = sprintf( "New User at %s", $_SESSION['website_name'] );
		$dest = sprintf( "%s", $user['us_Email'] );
								
		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		$message .= sprintf( "You have requested an email whenever a new user registers on the <a href=\"http://%s/%s\">%s</a> web site.<br><br>", 
			$_SESSION['website_domain'], WEB_ROOT_PATH, $_SESSION['website_name'] );
		$message .= sprintf( "User '%s' with email '%s' has just registered.<br><br>", 
					$new_name, $new_email );

		$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );
								
		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
			$count += 1;
		}
		else
		{	// error
			$ecount += 1;
			$emsg .= sprintf( "Failed to send email to '%s'.\n", $user['us_Email'] );
		}
	}
	
	return $emsg;
}

// email all news moderators and admins
function func_email_admin_forum_topic( $user_list, $title, $user_email, $ne_type, $ne_status )
{
	$count = 0;
	$ecount = 0;
	$emsg = "";
	foreach ( $user_list as $user )
	{
		$subject = sprintf( "New Forum Topic at %s", $_SESSION['website_name'] );
		$dest = sprintf( "%s", $user['us_Email'] );
								
		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		$message .= sprintf( "You have requested an email whenever a new forum topic or news item is created on the <a href=\"http://%s/%s\">%s</a> web site.<br><br>", 
			$_SESSION['website_domain'], WEB_ROOT_PATH, $_SESSION['website_name'] );
		$message .= sprintf( "%s '%s' has just been created by '%s'.<br><br>", 
					($ne_type == NEWS_TYPE_NEWS ? "News item" : "Forum topic"), $title, $user_email );

		if ( $ne_status == "H" )
		{
			$message .= sprintf( "This %s is held until a moderator releases it for publication.<br><br>", ($ne_type == NEWS_TYPE_NEWS ? "news item" : "forum topic") );
		}

		$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );
								
		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
			$count += 1;
		}
		else
		{	// error
			$ecount += 1;
			$emsg .= sprintf( "Failed to send email to '%s'.\n", $user['us_Email'] );
		}
	}
	
	return $emsg;
}

function func_email_admin_new_exam( $user_list, $user_email, $user_name, $result )
{
	$count = 0;
	$ecount = 0;
	$emsg = "";
	foreach ( $user_list as $user )
	{
		if ( $result == "" )
			$subject = sprintf( "New Exam at %s", $_SESSION['website_name'] );
		else
			$subject = sprintf( "Exam Result at %s", $_SESSION['website_name'] );
		$dest = sprintf( "%s", $user['us_Email'] );

		$message = "";
		$message .= "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
		$message .= sprintf( "<html><title>%s</title>", $subject );
		$message .= sprintf( "<head>%s</head>", func_get_email_styles() );	// these styles do not always work ???
		$message .= sprintf( "<body %s %s>", func_show_bgcolor(), func_show_bgimage() );
		$message .= sprintf( "<table width=\"100%%\" height=\"100%%\" %s %s><tr valign=\"top\"><td>", func_show_bgcolor(), func_show_bgimage() );
		if ( $_SESSION['website_logo'] != "" )
			$message .= sprintf( "%s<br><br>", $_SESSION['website_logo'] );
		else if ( $_SESSION['website_image'] != "" )
			$message .= sprintf( "%s<br><br>", func_get_website_image() );
		$message .= sprintf( "<span style=\"font-family: Verdana; font-size: 10pt;\">" );
		$message .= sprintf( "Hi %s,<br><br>", $user['us_Name'] );
		$message .= sprintf( "You have requested an email whenever a user creates or completes an exam on the <a href=\"http://%s/%s\">%s</a> web site.<br><br>",
				$_SESSION['website_domain'], WEB_ROOT_PATH, $_SESSION['website_name'] );
		if ( $result == "" )
		{	// new exam created
			$message .= sprintf( "User '%s' with email '%s' has just created an exam.<br><br>",
				$user_name, $user_email );
		}
		else
		{	// exam completed
			$message .= sprintf( "User '%s' with email '%s' has just completed an exam: %s.<br><br>",
					$user_name, $user_email, $result );
		}	

		$message .= sprintf( "Have a great day !<br>%s<br>", $_SESSION['website_admin'] );
		$message .= sprintf( "</span></td></tr></table></body>" );
		$message .= sprintf( "</html>" );

		if ( func_send_mail( $dest, $message, $subject, $_SESSION['website_email'], $_SESSION['website_admin'] ) ===  true )
		{	// success
			$count += 1;
		}
		else
		{	// error
			$ecount += 1;
			$emsg .= sprintf( "Failed to send email to '%s'.\n", $user['us_Email'] );
		}
	}

	return $emsg;
}





?>
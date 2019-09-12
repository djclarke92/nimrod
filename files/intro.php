<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Flat Cat IT Ltd
//	Author: Dave Clarke
//
//--------------------------------------------------------------------------------------

include_once( "common.php" );

if ( !isset($_SESSION['auth_level']) )
{	// access not via main page - access denied
	func_unauthorisedaccess();
	return;
}




printf( "<tr>" );
printf( "<td>" );
printf( "<b>Documentation</b>" );
printf( "</td>" );
printf( "</tr>" );
printf( "<tr>" );
printf( "<td><a href='http://dev.mysql.com/doc/refman/5.6/en/c-api-functions.html' target='_blank'>MySQL C API</a></td>" );
printf( "<td><a href='http://libmodbus.org/' target='_blank'>libmodbus</a></td>" );
printf( "</tr>" );

printf( "<tr><td>&nbsp;</td></tr>" );

printf( "<tr>" );
printf( "<td>" );
printf( "<b>Modbus Modules</b>" );
printf( "</td>" );
printf( "</tr>" );
printf( "<tr>" );
printf( "<td><img src='images/WP8028ADAM.jpg' alt='WP8028ADAM'></td>" );
printf( "<td><img src='images/WP3082ADAM.jpg' alt='WP3082ADAM'></td>" );
printf( "<td><img src='images/WP8025ADAM.jpg' alt='WP8025ADAM'></td>" );
printf( "<td><img src='images/WP8026ADAM.jpg' alt='WP8026ADAM'></td>" );
printf( "</tr>" );
printf( "<tr>" );
printf( "<td colspan='4'>" );
printf( "DIN rail mounting Modbus modules, approx US$30 each from <a href='http://aliexpress.com' target='_blank'>Aliexpress</a>." );
printf( "</td>" );
printf( "</tr>" );
printf( "<tr>" );
printf( "<td><img src='images/usb-rs485.jpg' alt='usb-rs485' width='120px'></td>" );
printf( "<td><img src='images/raspberry-pi-b.jpg' alt='raspberry-pi-b' width='140px'></td>" );
printf( "</tr>" );
printf( "<tr>" );
printf( "<td colspan='4'>" );
printf( "USB-RS485 Adaptor: $3, Raspberry Pi B $50." );
printf( "</td>" );
printf( "</tr>" );


?>


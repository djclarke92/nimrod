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

?>


<div class="container" style="margin-top:30px">
<?php
if ( $_SESSION['us_AuthLevel'] <= SECURITY_LEVEL_NONE )
{
?>
	<div class="row mb-2">
		<div class="col-sm-12">

			<div class="form-group">
  				<label for="usr">Username:</label>
  				<input type="text" class="form-control" name='Username' id='Username' placeholder="Enter email">
  				&nbsp;
  				<label for="pwd">Password:</label>
  				<input type="password" class="form-control" name='Password' id='Password' placeholder="Enter password">
  				&nbsp;
				<button type="submit" class='btn btn-outline-dark btn-primary' name='Login' Value='Login' id='Login'>Login</button>
			</div>
			
			<?php
			if ( $my_login_msg != "" )
			{
    			printf( "<p class='text-danger'>%s</p>", $my_login_msg );
			}
			?>

		</div>
	</div>	<!-- end of row -->
<?php
}
?>


	<div class="row">
		<div class="col-sm-4">
		
			<h2>About Nimrod</h2>
			<div><img class='img-fluid' src="./images/raspberry-pi-b.jpg"></div>
			<p>A home automation project based on the Raspberry Pi, Modbus IO devices and NodeMCU modules. The control software is written in C++ and the web configuration is in
			PHP7, Bootstrap and JQuery and uses MySQL/MariaDB as the database</p>
			<h3>Some Links</h3>
			<p>The Nimrod source is available on Github.</p>
			<ul class="nav nav-pills flex-column">
				<li class="nav-item">
				<a class="nav-link" href="https://github.com/djclarke92/nimrod">Nimrod on Github</a>
				</li>
				<li class="nav-item">
				<a class="nav-link" href="http://libmodbus.org/">Lib Modbus</a>
				</li>
				<li class="nav-item">
				<a class="nav-link" href="https://www.raspberrypi.org/">Raspberry Pi</a>
				</li>
			</ul>
			<hr class="d-sm-none">
		</div>
		
		<div class="col-sm-8">
			<h2>Modbus Devices</h2>
			
			<div class='row'>
			<div class='col'><img class='img-fluid' src="./images/WP3082ADAM.jpg"></div>
			<div class='col'><img class='img-fluid' src="./images/WP8026ADAM.jpg"></div>
			<div class='col'><img class='img-fluid' src="./images/usb-rs485.jpg"></div>
			<div class='col'><img class='img-fluid' src="./images/esp8266.png"></div>
			</div>
			<p>A number of Modbus RTU devices are supported to interface with digital inputs and outputs, voltage, current and temperature.
			NodeMCU ESP8266 WiFi devices are also supported.</p>
	
			<h2>What can you do with it ?</h2>
			<ul>
				<li>Input events can be switches, temperature values, voltage levels or time of day</li>
				<li>Turn on one or more output ports when an input event occurs</li>
				<li>Input switch events can be a click, double click or long click, each can trigger different output actions</li>
				<li>Input events can be chained together, e.g. if time is 6:30am and temperature sensor #2 is < 15 deg then turn on output #7 for 30 minutes</li>
			</ul>
		</div>
	
	</div>	<!-- end of row -->
	
</div>



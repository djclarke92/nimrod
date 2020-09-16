<?php
// --------------------------------------------------------------------------------------
//
// Nimrod Website
// Copyright (c) 2015 Dave Clarke
//
// --------------------------------------------------------------------------------------

include_once( "common.php" );

if ( !isset($_SESSION['us_AuthLevel']) )
{	// access not via main page - access denied
    func_unauthorisedaccess();
    return;
}













?>


<div class="container" style="margin-top:30px">

	<!-- *************************************************************************** -->
	<div class="row">
	
		<!-- *************************************************************************** -->
		<div class="col-sm-4">
    		<h3>Events</h3>


		</div>
	</div>	<!-- end of row -->
	
	
</div>

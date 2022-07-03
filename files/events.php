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

function func_clear_ev_array(&$ev_array)
{
    $ev_array['ev_StartDate'] = "";
    $ev_array['ev_StartTime'] = "";
    $ev_array['ev_Duration'] = "";
    $ev_array['ev_DIFilter'] = "";
    $ev_array['ev_DeviceInfoNo'] = "";
    $ev_array['error_msg'] = "";
    $ev_array['info_msg'] = "";
}

function func_check_ev_array(&$ev_array)
{
    $ev_array['error_msg'] = "";
    $ev_array['info_msg'] = "";
    
    if ($ev_array['ev_StartDate'] == "" || strlen($ev_array['ev_StartDate']) != 10 ) 
    {
        $ev_array['error_msg'] = "You must enter the search start date as DD/MM/YYYY.";
        return false;
    }
    else if ($ev_array['ev_StartTime'] == "" || strlen($ev_array['ev_StartTime']) != 5 )
    {
        $ev_array['error_msg'] = "You must enter the search start time as HH:MM.";
        return false;
    }
    else if ($ev_array['ev_Duration'] == "")
    {
        $ev_array['error_msg'] = "You must enter the search duration.";
        return false;
    }
    
    return true;
}


$ev_array = array();
func_clear_ev_array($ev_array);

$events_data = array();

if ( isset($_POST['ev_StartDate']) )
    $ev_array['ev_StartDate'] = $_POST['ev_StartDate'];
if ( isset($_POST['ev_StartTime']) )
    $ev_array['ev_StartTime'] = $_POST['ev_StartTime'];
if ( isset($_POST['ev_Duration']) )
    $ev_array['ev_Duration'] = $_POST['ev_Duration'];
if ( isset($_POST['ev_DIFilter']) )
    $ev_array['ev_DeviceInfoNo'] = intval($_POST['ev_DIFilter']);
        

if ( isset($_POST['Search']) )
{
    if ( func_check_ev_array($ev_array) )
    {
        $events_data = $db->ReadEventsTable( $ev_array['ev_StartDate'], $ev_array['ev_StartTime'], $ev_array['ev_Duration'], $ev_array['ev_DeviceInfoNo'] );
    }
}




$di_list = $db->ReadDeviceInfoTable();
$di_list[] = array( 'di_DeviceInfoNo'=>-3, 'di_IOName'=>"User Login Attempts" );

?>


<div class="container" style="margin-top:30px">

	<!-- *************************************************************************** -->
	<div class="row">
	
		<!-- *************************************************************************** -->
		<div class="col-sm-4">
    		<h3>Events</h3>
		</div>
	</div>	<!-- end of row -->
			
	<!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-10">
		
			<?php 
			if ( $ev_array['error_msg'] != "" )
			    printf( "<p class='text-danger'>%s</p>", $ev_array['error_msg'] );
		    else if ( $ev_array['info_msg'] != "" )
		        printf( "<p class='text-info'>%s</p>", $ev_array['info_msg'] );
            ?>
		
		
			<div class='row mb-2'>
				<div class='col-sm-1'>
					<label for='ev_DIFilter'>Device: </label>
				</div>
				<div class='col'>
					<select class='form-control custom-select' size='1' name='ev_DIFilter' id='ev_DIFilter'>
					<option></option>
					<?php
			        foreach ($di_list as $dinfo) 
			        {
    			        printf("<option %s>%02d. %s</option>", ($ev_array['ev_DeviceInfoNo'] == $dinfo['di_DeviceInfoNo'] ? "selected" : ""), $dinfo['di_DeviceInfoNo'], $dinfo['di_IOName']);
			        }
			        ?>
					</select>
				</div>

				<div class='col-sm-1'>
					<label for='ev_StartDate'>Date: </label>
				</div>
				<div class='col'>
	    			<?php printf( "<input type='text' class='form-control' name='ev_StartDate' id='ev_StartDate' size='8' value='%s' placeholder='dd/mm/yyyy'> ", 
	    			    ($ev_array['ev_StartDate'] === "" ? "" : $ev_array['ev_StartDate']) );?>
    			</div>

				<div class='col-sm-1'>
					<label for='ev_StartTime'>Time: </label>
				</div>
				<div class='col'>
    				<?php printf( "<input type='text' class='form-control' name='ev_StartTime' id='ev_StartTime' size='4' value='%s' placeholder='hh:mm'> ", 
    				    ($ev_array['ev_StartTime'] === "" ? "" : $ev_array['ev_StartTime']) );?>
    			</div>

				<div class='col-sm-1'>
					<label for='ev_Duration'>Duration: </label>
				</div>
				<div class='col'>
    				<?php printf( "<input type='text' class='form-control' name='ev_Duration' id='ev_Duration' size='4' value='%s' placeholder='X.X hms'> ", 
    				    ($ev_array['ev_Duration'] === "" ? "" : $ev_array['ev_Duration']) );?>
    			</div>

				<div class='col'>
					<button type='submit' class='btn btn-outline-dark' name='Search' id='Search'>Search Fwd</button>
				</div>
			</div>
		</div>
	</div>	<!-- end of row -->
			
	<!-- *************************************************************************** -->
	<div class="row">

		<div class="col-sm-9">

        <?php
        if ( count($events_data) > 0 )
        {
    		printf( "<table class='table table-striped'>" );
    		
    		printf( "<thead class='thead-light'>" );
            printf( "  <tr>" );
            if ( $ev_array['ev_DeviceInfoNo'] == -3 )
            {
                printf( "  <th>Timestamp</th>" );
                printf( "  <th>User Id</th>" );
                printf( "  <th>Result</th>" );
            }
            else
            {
                printf( "  <th>Timestamp</th>" );
                printf( "  <th>Channel Name</th>" );
                printf( "  <th>Event Type</th>" );
                printf( "  <th>Value</th>" );
                printf( "  <th>Description</th>" );
            }
            printf( "  </tr>" );
            printf( "</thead>" );

			foreach ( $events_data as $info )
			{
			    printf( "<tr>" );

			    if ( $ev_array['ev_DeviceInfoNo'] == -3 )
			    {
			        printf( "<td>%s</td>", $info['ev_Timestamp'] );
			        printf( "<td>%s</td>", $info['ev_Description'] );
			        printf( "<td>%s</td>", ($info['ev_Value'] == 1 ? "Success" : "FAILED") );
			    }
			    else
			    {
    			    printf( "<td>%s</td>", $info['ev_Timestamp'] );
    			    printf( "<td>%s</td>", $info['di_IOName'] );
       			    printf( "<td>%s</td>", func_get_eventtype_desc($info['ev_EventType']) );
    			    
    			    switch ( $info['ev_EventType'] )
    			    {
                    default:
    		  	       printf( "<td>%d</td>", $info['ev_Value'] );
    		  	       break;
                    case E_ET_TEMPERATURE:
                        printf( "<td>%s C</td>", func_calc_temperature($info['ev_Value']) );
                        break;
                    case E_ET_VOLTAGE:
                        printf( "<td>%s V</td>", func_calc_voltage($info['ev_Value'], $info['di_AnalogType']) );
                        break;
    			    }
    			    
    			    printf( "<td>%s</td>", $info['ev_Description'] );
			    }
			    
			    printf( "</tr>" );
			}
			printf( "</table>" );

        }
        else if ( $ev_array['ev_StartDate'] != "" && $ev_array['ev_Duration'] != "" )
        {
            printf( "<p>No events exist for the selected period.</p>" );
        }
		?>   

		</div>
	</div>	<!-- end of row -->

	
	
</div>

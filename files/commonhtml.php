<?php
//--------------------------------------------------------------------------------------
//
//	Nimrod Website
//	Copyright (c) 2015 Flat Cat IT Ltd
//	Author: Dave Clarke
//
//--------------------------------------------------------------------------------------



function func_read_tables_file( $fn )
{
	$info = false;
	if ( file_exists( $fn ) )
	{
		$info = array();
		
		$fh = fopen( $fn, "r" );
		if ( $fh !== false )
		{
			$query = "";
			while ( ($line = fgets( $fh )) !== false )
			{
				$line = trim($line);
				if ( substr( $line, 0, 1 ) != "#" )
				{	// not a comment
				
				
					// strip comment text out of line
					$quote1 = false;
					$quote2 = false;
					$len = strlen($line);
					for ( $i = 0; $i < $len; $i++ )
					{
						$char = substr( $line, $i, 1 );
						if ( $char == "'" )
						{	// single quote
							if ( $quote1 )
								$quote1 = false;
							else
								$quote1 = true;
						}
						else if ( $char == "\"" )
						{	// double quote
							if ( $quote2 )
								$quote2 = false;
							else
								$quote2 = true;
						}
						else if ( $char == "#" )
						{	// comment character
							if ( !$quote1 && !$quote2 )
							{	// its a real comment not part of html code so we can strip it off
								$line = substr( $line, 0, $i );
								break;
							}
						}
					}
					
					$query .= $line;
					
					// is this a complete query line
					if ( strpos( $line, ";" ) !== false )
					{	// query is complete
						
						$info[] = $query;
						$query = "";
					}
				}
			}
		
			fclose( $fh );
		}
	}
	
	return $info;
}

function func_unauthorisedaccess()
{
	?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type"
	content="text/html; charset=windows-1252">
<meta http-equiv="Content-Language" content="en-nz">
<title>HomeAuto</title>
</head>
<body>
	<h1>Unauthorised access is denied !</h1>
</body>
</html>
<?php
}

function func_html_get_table_width( $info )
{
	$width = 16;	// scroll bar width
	foreach ( $info as $col )
	{
		$width += $col[0];
	}

	return $width;
}

function func_html_set_col_widths( $info )
{
	foreach ( $info as $col )
	{
		printf( "<col width='%d'>", $col[0] );
	}
}

function func_html_extend_table( $info, $count, $max_rows = 10 )
{
	while ( $count < $max_rows )
	{	// force the scroll bar to appear
		$style = "alternateRow";
		if ( ($count % 2) == 0 )
			$style = "normalRow";
		printf( "<tr class='%s'>", $style );
		foreach ( $info as $col )
		{
			printf( "<td>&nbsp;</td>" );
		}
		printf( "</tr>" );
		$count += 1;
	}
}

function func_html_header_table( $info )
{
	$sbw = 16;		// scroll bar width

	printf( "<table cellspacing='0' cellpadding='2' class='style-headertable' border='0'>" );

	$i = 0;
	while ( isset($info[$i]) )
	{
		$width = $info[$i][0];
		if ( !isset($info[$i+1]) )
			$width += $sbw;
			
		printf( "<col width='%d'>", $width );
		$i += 1;
	}
	printf( "<tr>" );

	$i = 0;
	while ( isset($info[$i]) )
	{
		$style = "";
		if ( !isset($info[$i+1]) )
			$style = "style='{border-right:1px solid #C0C0C0;}'";
			
		printf( "<td class='style-normal' %s><b>%s</b></td>", $style, $info[$i][1] );
		$i += 1;
	}
	printf( "</tr>" );
	printf( "</table>" );
}



?>
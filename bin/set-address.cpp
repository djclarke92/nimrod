/*
 * Nimrod set-address utility: set the modbus address and baud rate on Wellpro modbus devices
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>

#include <modbus.h>


int gRS485_iBaud = 9600;
char gRS485_cParity = 'N';
int gRS485_iDataBits = 8;
int gRS485_iStopBits = 1;


void SetNewAddress( modbus_t *ctx, int oldAddr, int newAddr, int oldBaud, int newBaud, int type );
void ReadData( modbus_t *ctx, int newAddr, int newBaud, int type );


int main( int argc, char** argv )
{
	int type = 0;
	int oldAddr = 0;
	int newAddr = 0;
	int oldBaud = 9600;
	int newBaud = 9600;
	modbus_t* ctx = NULL;

	printf( "Compiled against libmodbus v%s\n", LIBMODBUS_VERSION_STRING );
	printf( "Linked against libmodbus v%d.%d.%d\n", libmodbus_version_major, libmodbus_version_minor, libmodbus_version_micro );
	printf( "\n" );

	if ( argc == 7 )
	{	// broadcast new address for ALL connected devices

		oldAddr = atoi( argv[2] );
		newAddr = atoi( argv[3] );
		oldBaud = atoi( argv[4] );
		newBaud = atoi( argv[5] );
		type = atoi( argv[6] );

		gRS485_iBaud = oldBaud;

		ctx = modbus_new_rtu( argv[1], gRS485_iBaud, gRS485_cParity, gRS485_iDataBits, gRS485_iStopBits );
		if ( ctx == NULL )
		{
			printf( "Error: modbus ctx in is null, aborting: %s\n", modbus_strerror(errno) );
			exit( 1 );
		}

		modbus_set_debug(ctx, true);

		if ( modbus_connect(ctx) == -1 )
		{
			printf( "Error: modbus_connect() failed, aborting: %s\n", modbus_strerror(errno) );
			exit( 1 );
		}

		printf( "Ctx %p\n", ctx );


		SetNewAddress( ctx, oldAddr, newAddr, oldBaud, newBaud, type );

		if ( oldBaud != newBaud )
		{
			modbus_free( ctx );
			ctx = NULL;

			gRS485_iBaud = newBaud;

			ctx = modbus_new_rtu( argv[1], gRS485_iBaud, gRS485_cParity, gRS485_iDataBits, gRS485_iStopBits );
			if ( ctx == NULL )
			{
				printf( "Error: modbus ctx in is null, aborting: %s\n", modbus_strerror(errno) );
				exit( 1 );
			}

			modbus_set_debug(ctx, true);

			if ( modbus_connect(ctx) == -1 )
			{
				printf( "Error: modbus_connect() failed, aborting: %s\n", modbus_strerror(errno) );
				exit( 1 );
			}

			printf( "Ctx %p\n", ctx );
		}

		ReadData( ctx, newAddr, newBaud, type );


		modbus_free( ctx );
		ctx = NULL;
	}
	else
	{
		printf( "Usage: '%s <com_port> <old_addr> <new_addr> <old_baud> <new_baud> <device_type>' to set comms parameters\n", argv[0] );
		printf( "    where device_type: 0 Digital Input devices\n" );
		printf( "                       1 Digital Output devices\n" );
		printf( "                       2 Voltage device and temperature devices\n" );
		printf( "                       3 Thermocouple devices PD3064\n" );
	}

	return 0;
}


// PD30xx sereies devices (thermocouple)
// function code 0x03 = read, 0x06 = write
// Register 0x10: 0x01 (address) 0x00 (comms)
//		address		0x01 to 250
//		comms	bits 7:5 - 	0x00
//				bits 4:3 - 	00 = no parity
//							01 = even parity
//							10 = odd parity
//							11 = odd parity
//				bits 2:0 -	000=9600 	0
//							001=1200	1
//							010=2400	2
//							011=4800	3
//							100=9600	4
//							101=14400	5
//							110=19200	6
// Register 0x11: 0xff (round values) 0x05 (sample rate)
//		rounding		0xff
//		sample rate		0-255 seconds (0 same as 1)

void SetNewAddress( modbus_t *ctx, int oldAddr, int newAddr, int oldBaud, int newBaud, int type )
{
	bool bBaudOk = true;
	bool bReset = false;
	int modBaud = 2;
	int modBaud2 = 0;
	int modBaud3 = 0;

	// check the baud rate
	switch ( newBaud )
	{
	default:
		bBaudOk = false;
		break;
	case 4800:
		modBaud = 1;
		modBaud2 = 3;
		break;
	case 9600:		// no baud rate change
		modBaud = 2;
		modBaud2 = 0;
		break;
	case 19200:
		modBaud = 3;
		modBaud2 = 6;
		break;
	}

	switch ( oldBaud )
	{
	default:
		break;
	case 4800:
		modBaud3 = 3;
		break;
	case 9600:		// no baud rate change
		modBaud3 = 0;
		break;
	case 19200:
		modBaud3 = 6;
		break;
	}

	if ( newAddr <= 0x00 || newAddr > 0xfe )
	{	// error
		printf( "Error: Invalid new address %d\n", newAddr );
	}
	else if ( !bBaudOk )
	{
		printf( "Error: Invalid new baud rate %d\n", newBaud );
	}
	else if ( newAddr == oldAddr && newBaud == oldBaud )
	{
		printf( "Nothing to set 0x%02x, %d\n", newAddr, newBaud );
	}
	else
	{	// data is valid

		// MODBUS_BROADCAST_ADDRESS;
		if ( modbus_set_slave( ctx, oldAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", oldAddr, modbus_strerror(errno) );
		}


		if ( type == 0 || type == 1 || type == 2 )
		{
			if ( newAddr != oldAddr )
			{
				printf( "Setting new address 0x%02x -> 0x%02x\n", oldAddr, newAddr );

				if ( modbus_write_register( ctx, 0x0064, newAddr ) == -1 )
				{
					printf( "Error: modbus_write_register(0x64) failed: %s\n", modbus_strerror(errno) );
				}
				else
				{
					bReset = true;
					printf( "New slave address 0x%02x set\n", newAddr );

					if ( modbus_set_slave( ctx, newAddr ) == -1 )
					{
						printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
					}
				}
			}
			else
			{
				printf( "Warning: skip setting module address\n" );
			}

			if ( newBaud != oldBaud )
			{
				printf( "Setting new baud rate %d -> %d (%d)\n", oldBaud, newBaud, modBaud );

				if ( modbus_write_register( ctx, 0x0065, modBaud ) == -1 )
				{
					bReset = true;
					printf( "Error: modbus_write_register(0x65) failed: %s\n", modbus_strerror(errno) );
				}
				else
				{
					printf( "New baud rate %d set\n", newBaud );
				}
			}
		}
		else if ( type == 3 )
		{	// thermocouple
			if ( newAddr != oldAddr )
			{

				int val = modBaud3;	// 0x00 = 9600 baud in low byte

				val += (newAddr << 8);

				printf( "Setting new address 0x%02x -> 0x%02x (0x%x)\n", oldAddr, newAddr, val );

				if ( modbus_write_register( ctx, 0x0010, val ) == -1 )
				{
					printf( "Error: modbus_write_register(0x10) failed: %s\n", modbus_strerror(errno) );
				}
				else
				{
					bReset = true;
					printf( "New slave address 0x%02x set\n", newAddr );

					if ( modbus_set_slave( ctx, newAddr ) == -1 )
					{
						printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
					}
				}
			}
			else
			{
				printf( "Warning: skip setting module address\n" );
			}

			if ( newBaud != oldBaud )
			{
				int val = modBaud2;
				val += (newAddr << 8);

				printf( "Setting new baud rate %d -> %d (%d,0x%x)\n", oldBaud, newBaud, modBaud2, val );

				if ( modbus_write_register( ctx, 0x0010, val ) == -1 )
				{
					bReset = true;
					printf( "Error: modbus_write_register(0x65) failed: %s\n", modbus_strerror(errno) );
				}
				else
				{
					printf( "New baud rate %d set\n", newBaud );
				}
			}
		}
		else
		{
			printf( "Error: invalid type = %d\n", type );
		}

		if ( bReset )
		{	// reset the slave
/*
			if ( modbus_write_register( ctx, 0x0001, modBaud ) == -1 )
			{
				bReset = true;
				printf( "Error: modbus_write_register(1) failed: %s\n", modbus_strerror(errno) );
			}
			else
			{
				printf( "New baud rate %d set\n", newBaud );
			}
*/
		}
	}

	usleep( 30000 );
}

void ReadData( modbus_t *ctx, int newAddr, int newBaud, int type )
{
	int rc;
	int addr;
	const int iLen = 8;

	if ( type == 0 )
	{
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		usleep( 30000 );

		// read input bits to see if the device is talking
		addr = 0;
		uint8_t uInputs[iLen];

		for ( int i = 0; i < iLen; i++ )
		{
			uInputs[i] = 0;
		}

		rc = modbus_read_input_bits( ctx, addr, iLen, uInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_input_bits() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read input bits: %u %u %u %u %u %u %u %u\n", uInputs[0], uInputs[1], uInputs[2], uInputs[3], uInputs[4], uInputs[5], uInputs[6], uInputs[7] );
		}
	}
	else if ( type == 1 )
	{	// output
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		usleep( 30000 );

		printf( "modbus_write_bit() 1 -> 1\n" );
		if ( modbus_write_bit( ctx, 1, true ) == -1 )
		{	// failed
			printf( "Error: modbus_write_bit() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Bit 1 turned on\n" );
		}
		usleep( 500000 );

		uint8_t cData[16];
		printf( "modbus_read_bits()\n" );
		if ( modbus_read_bits( ctx, 0, 16, cData ) == -1 )
		{	// error
			printf( "Error: modbus_read_bits() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "modbus_read_bits() %u %u %u %u %u %u %u %u\n", (uint8_t)cData[0], (uint8_t)cData[1], cData[2], cData[3], cData[4], cData[5], cData[6], cData[7] );
		}

		usleep( 30000 );

		printf( "modbus_write_bit() 1 -> 0\n" );
		if ( modbus_write_bit( ctx, 1, false ) == -1 )
		{	// failed
			printf( "Error: modbus_write_bit() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Bit 1 turned off\n" );
		}
	}
	else if ( type == 2 )
	{	// voltage
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		addr = 0x00;
		uint16_t ulInputs[iLen];

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: %u %u %u %u %u %u %u %u\n", ulInputs[0], ulInputs[1], ulInputs[2], ulInputs[3], ulInputs[4], ulInputs[5], ulInputs[6], ulInputs[7] );
		}
	}
	else if ( type == 3 )
	{	// thermocouple
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		addr = 0x20;
		uint16_t ulInputs[iLen];

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: %u %u %u %u %u %u %u %u\n", ulInputs[0], ulInputs[1], ulInputs[2], ulInputs[3], ulInputs[4], ulInputs[5], ulInputs[6], ulInputs[7] );
		}
	}

}



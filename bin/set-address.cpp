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

#include <modbus/modbus.h>


int gRS485_iBaud = 9600;
char gRS485_cParity = 'N';
int gRS485_iDataBits = 8;
int gRS485_iStopBits = 1;


void SetNewAddress( modbus_t *ctx, int oldAddr, int newAddr, int oldBaud, int newBaud, int type );
void ReadData( modbus_t *ctx, int newAddr, int newBaud, int type, int vsdOperation );


int main( int argc, char** argv )
{
	int type = 0;
	int oldAddr = 0;
	int newAddr = 0;
	int oldBaud = 9600;
	int newBaud = 9600;
	int vsdOperation = 0;
	modbus_t* ctx = NULL;

	printf( "Compiled against libmodbus v%s\n", LIBMODBUS_VERSION_STRING );
	printf( "Linked against libmodbus v%d.%d.%d\n", libmodbus_version_major, libmodbus_version_minor, libmodbus_version_micro );
	printf( "\n" );

	if ( argc >= 7 )
	{	// broadcast new address for ALL connected devices

		oldAddr = atoi( argv[2] );
		newAddr = atoi( argv[3] );
		oldBaud = atoi( argv[4] );
		newBaud = atoi( argv[5] );
		type = atoi( argv[6] );

		if ( argc >= 8 )
			vsdOperation = atoi(argv[7] );

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

		ReadData( ctx, newAddr, newBaud, type, vsdOperation );


		modbus_free( ctx );
		ctx = NULL;
	}
	else
	{
		printf( "Usage: '%s <com_port> <old_addr> <new_addr> <old_baud> <new_baud> <device_type> <vsd_op>' to set comms parameters\n", argv[0] );
		printf( "    where device_type: 0 Digital Input devices\n" );
		printf( "                       1 Digital Output devices\n" );
		printf( "                       2 Voltage device and temperature devices\n" );
		printf( "                       3 Thermocouple devices PD3064\n" );
		printf( "                       4 HDL300 water level sensor\n" );
		printf( "                       5 Rotary Encoder MT-5208 series\n" );
		printf( "                       6 PZEM-016 AC V/A Monitor\n" );
		printf( "                       7 NFlixen 9600 VSD\n" );
		printf( "                       8 Power Electronics SD700 VSD\n" );
		printf( "                       9 Toshiba VS11 VSD\n" );
		printf( "    optional   vsd_op: 1 forward\n");
		printf( "                       2 backward\n");
		printf( "                       3 forward jog\n");
		printf( "                       4 backward jog\n");
		printf( "                       5 free stop\n");
		printf( "                       6 speed down stop\n");
		printf( "                       7 fault reset\n");
	}

	return 0;
}

// PZEM-016 AC V/I Monitor
// Does not reply to the broadcast address 0x00
// Uses read_input_registers()
// Only 9600 baud is supported
// Input Register 0x00 voltage, lsb = 0.1V
// Input Register 0x01/2 current, lsb = 0.001A
// Input Register 0x03/4 power, lsb = 0.1W
// Input Register 0x05/6 energy, lsb = 1Wh
// Input Register 0x07 frequency, lsb = 0.1Hz
// Input Register 0x08 power factor, lsb = 0.01
// Input Register 0x09 alarm status
// Register 0x01 power alarm threshold
// Register 0x02 address
//
// MT 5208 Rotary Encoder 12 bit
// function code 0x03 = read
// Register 0x00 angle
// Register 0x01 turns
// Register 0x02 address
// Register 0x03 baud rate (1=9600, 2=19200, 3=38400)
// Register 0x04 parity (1=N, 2=Odd, 3=even)
// Register 0x05 set zero pos (write data 0xff)
// Register 0x06 set counting dir (1=clockwise, 2=anticlockwise)
// Cable red = +12V
//       black = 0V
//       green = rs485A
//       yellow - rs486B
//       white = factory reset (connect to 0V during use)
//
// HDL300 water level sensor
// function code 0x03 = read
// Register 0x00 = modbus address
// Register 0x01 = baud rate
// Register 0x04 = depth in mm
// Cable red = +12V
//       black = 0V
//       white = rs485A
//       blue = rs485B

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
	int modBaud = 2;
	int modBaud2 = 0;
	int modBaud3 = 0;
	int modBaud4 = 0;
	int modBaud5 = 0;
	int reg;

	// check the baud rate
	switch ( newBaud )
	{
	default:
		bBaudOk = false;
		break;
	case 4800:
		modBaud = 1;
		modBaud2 = 3;
		modBaud4 = 2;
		break;
	case 9600:		// no baud rate change
		modBaud = 2;
		modBaud2 = 0;
		modBaud4 = 3;
		modBaud5 = 1;
		break;
	case 19200:
		modBaud = 3;
		modBaud2 = 6;
		modBaud4 = 4;
		modBaud5 = 2;
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

				reg = 0x64;
				if ( modbus_write_register( ctx, reg, newAddr ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
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

				reg = 0x65;
				if ( modbus_write_register( ctx, reg, modBaud ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
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

				reg = 0x10;
				if ( modbus_write_register( ctx, reg, val ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
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

				reg = 0x10;
				if ( modbus_write_register( ctx, reg, val ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
					printf( "New baud rate %d set\n", newBaud );
				}
			}
		}
		else if ( type == 4 )
		{
			if ( newAddr != oldAddr )
			{
				printf( "Setting new address 0x%02x -> 0x%02x\n", oldAddr, newAddr );

				// HDL300 modbus address is in register 0x00
				reg = 0x00;
				if ( modbus_write_register( ctx, reg, newAddr ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
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
				printf( "Setting new baud rate %d -> %d (%d)\n", oldBaud, newBaud, modBaud4 );

				reg = 0x1;
				if ( modbus_write_register( ctx, reg, modBaud4 ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
					printf( "New baud rate %d set\n", newBaud );
				}
			}
		}
		else if ( type == 5 )
		{	// rotary encoder
			if ( newAddr != oldAddr )
			{
				printf( "Setting new address 0x%02x -> 0x%02x\n", oldAddr, newAddr );

				// MT-5208 modbus address is in register 0x02
				reg = 0x02;
				if ( modbus_write_register( ctx, reg, newAddr ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
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
				printf( "Setting new baud rate %d -> %d (%d)\n", oldBaud, newBaud, modBaud5 );

				reg = 0x03;
				if ( modbus_write_register( ctx, reg, modBaud5 ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
					printf( "New baud rate %d set\n", newBaud );
				}
			}
		}
		else if ( type == 6 )
		{	// PZEM-016 AC V/I Monitor
			if ( newAddr != oldAddr )
			{
				printf( "Setting new address 0x%02x -> 0x%02x\n", oldAddr, newAddr );

				// MT-5208 modbus address is in register 0x02
				reg = 0x02;
				if ( modbus_write_register( ctx, reg, newAddr ) == -1 )
				{
					printf( "Error: modbus_write_register(0x%x) failed: %s\n", reg, modbus_strerror(errno) );
				}
				else
				{
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
				printf( "Only supports 9600 baud %d\n", oldBaud );
			}
		}
		else if ( type == 7 || type == 8 || type == 9 )
		{	// NFlixen VSD - baud rate set on the VSD
			// Power Electronics VSD
			// Toshiba VSD
			if ( newBaud != oldBaud )
			{
				printf( "Ignoring baud rate change\n" );
			}
		}
		else
		{
			printf( "Error: invalid type = %d\n", type );
		}
	}

	usleep( 30000 );
}

void ReadData( modbus_t *ctx, int newAddr, int newBaud, int type, int vsdOperation )
{
	int rc;
	int addr;
	int iLen = 8;

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
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", uInputs[i] );
			}
			printf ( "\n" );
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
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", (uint8_t)cData[i] );
			}
			printf ( "\n" );
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
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
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
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
		}
	}
	else if ( type == 4 )
	{	// hdl300 water level
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		addr = 0x04;
		uint16_t ulInputs[iLen];

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
			printf( "Depth = %d mm\n", ulInputs[0] + (ulInputs[1]>>8) );
		}

/*		for ( addr = 0x00; addr < 17 ; addr += 8 )
		{
			rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
			if ( rc == -1 )
			{
				printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
			}
			else
			{
				printf( "Read registers 0x%x: %X %X %X %X %X %X %X %X\n", addr, ulInputs[0], ulInputs[1], ulInputs[2], ulInputs[3], ulInputs[4], ulInputs[5], ulInputs[6], ulInputs[7] );
			}
		} */
	}
	else if ( type == 5 )
	{	// rotary encoder
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		iLen = 5;

		addr = 0x00;
		uint16_t ulInputs[iLen];

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
			printf( "Angle %d (%.1f deg), Turns %d\n", ulInputs[0], 360 * (double)ulInputs[0]/4096, ulInputs[1] );
		}
	}
	else if ( type == 6 )
	{	// PZEM-016 AC V/A Monitor
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		iLen = 10;

		addr = 0x00;
		uint16_t ulInputs[iLen];

		rc = modbus_read_input_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
			printf( "V=%.1fV, I=%.2fA, P=%.0fW, E=%dWh, F=%.1fHz, PF=%.1f, Alrm=%d\n", (double)ulInputs[0] / 10, (double)(ulInputs[1] + (ulInputs[2]>>8)) / 1000, (double)(ulInputs[3] + (ulInputs[4]>>8)) / 10,
					(int)(ulInputs[5] + (ulInputs[6]>>8)), (double)ulInputs[7] / 10, (double)ulInputs[8] / 100, ulInputs[9] );
		}
	}
	else if ( type == 7 )
	{	// NFlixen VSD
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		iLen = 18;

		addr = 0x1001;
		uint16_t ulInputs[iLen];

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
			printf( "F=%.2fHz, BV=%.1fV, MV=%.1fV, MA=%.1fA, MP=%.2fkW, MT=%.1f%%, RS=%.1frpm\n",
					(double)ulInputs[0] / 100,	// 0.01 Hz
					(double)ulInputs[1] / 10, 	// 0.1 V
					(double)ulInputs[2],		// 1 V
					(double)ulInputs[3] / 10, 	// 0.1 A ??
					(double)ulInputs[4] / 100,	// 0.1 kW ??
					(double)ulInputs[5] / 10, 	// 0.1 %
					(double)ulInputs[6] );		// 1
		}
		printf("\n");

		usleep( 50000 );

		// output current
		iLen = 1;
		addr = 0x7004;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
				printf( "Read Output Current: %d %.1f A\n", (int)ulInputs[0], (double)ulInputs[0] / 10 );
		}
		printf("\n");

		usleep( 50000 );

		// status
		iLen = 1;
		addr = 0x3000;

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			if ( ulInputs[0] == 1 )
				printf( "Read status: %d Forward\n", (int)ulInputs[0] );
			else if ( ulInputs[0] == 2 )
				printf( "Read status: %d Reverse\n", (int)ulInputs[0] );
			else if ( ulInputs[0] == 3 )
				printf( "Read status: %d Stopped\n", (int)ulInputs[0] );
			else
				printf( "Read status: %d Unknown\n", (int)ulInputs[0] );
		}
		printf("\n");

		usleep( 50000 );

		// fault code
		iLen = 1;
		addr = 0x8000;

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read fault code: %d\n", (int)ulInputs[0] );
		}
		printf("\n");

		if ( vsdOperation >= 1 && vsdOperation <= 7 )
		{	// vsd must be set for communications control P0-02 = 2
			usleep( 50000 );

			addr = 0x2000;

			rc = modbus_write_register( ctx, addr, vsdOperation );
			if ( rc == -1 )
			{
				printf( "Error: modbus_write_register() failed: %s\n", modbus_strerror(errno) );
			}
			else
			{
				printf( "VSD operation set to %d\n", vsdOperation );
			}
		}
	}
	else if ( type == 8 )
	{	// Power ELectronics VSD
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		iLen = 9;

		addr = 40162;
		uint16_t ulInputs[iLen];

		// 0 40162: speed reference, 8192 = 100%
		// 1 40163: motor current, A / 10
		// 2 40164: motor torque, 8192 = 100%
		// 3 40165: motor power, kW / 10
		// 4 40166: motor voltage, V
		// 5 40167: motor frequency, Hz
		// 6 40168: motor power factor, pf /10
		// 7 40169: motor speed, rpm
		// 8 40170: motor speed, 8192 = 100%

		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: " );
			for ( int i = 0; i < iLen; i++ )
			{
				printf( "%u ", ulInputs[i] );
			}
			printf ( "\n" );
			printf( "F=%.2fHz, MV=%.1fV, MA=%.1fA, MP=%.2fkW, MT=%.1f%%, RS=%.1frpm\n",
					(double)ulInputs[5],		// Hz
					(double)ulInputs[4], 		// V
					(double)ulInputs[1] / 10,	// 0.1 A
					(double)ulInputs[3] / 10,	// 0.1 kW ??
					100 * (double)ulInputs[2] / 8192, 	// %
					(double)ulInputs[7] );		// rpm
		}
		printf("\n");

		usleep( 50000 );

		if ( vsdOperation >= 1 && vsdOperation <= 7 )
		{	// vsd must be set for communications control
			printf( "VSD operation not supported\n" );
		}
	}
	else if ( type == 9 )
	{	// Toshiba VSD
		printf( "Setting slave addr to %d\n", newAddr );
		if ( modbus_set_slave( ctx, newAddr ) == -1 )
		{
			printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
		}

		iLen = 1;

		uint16_t ulInputs[iLen];

		// 0xfd00: operation frequency, 0.01 Hz
		// 0xfd01: current status, bit 9 fwd/rev, bit 10 stop/run
		// 0xfe03: motor current, 0.01 %
		// 0xfe05: motor voltage, 0.01%
		// 0xfe30: motor power, 0.01 kW
		// 0xfe18: motor torque, 0.01 %
		// 0xfe70: rated current
		// 0xfe71: rated voltage

		addr = 0xfd00;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Frequency %u %.2f Hz", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );

		// status
		addr = 0xfd01;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Status 0x%08x %s %s", ulInputs[0], ((ulInputs[0] & 0x0400) == 0 ? "Stop" : "Run"), ((ulInputs[0] & 0x0200) == 0 ? "Forward" : "Reverse") );
		}
		printf("\n");

		usleep( 50000 );

		// current
		addr = 0xfe03;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Motor Current %u %.02f A", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );

		// voltage
		addr = 0xfe05;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Motor Voltage %u %.02f V", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );

		// power
		addr = 0xfe30;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Motor Power %u %.02f kW", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );

		// torque
		addr = 0xfe18;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Motor Torque %u %.02f %%", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );

		// rated current
		addr = 0xfe70;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Rated Current %u %.02f A", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );

		// rated voltage
		addr = 0xfe71;
		rc = modbus_read_registers( ctx, addr, iLen, ulInputs );
		if ( rc == -1 )
		{
			printf( "Error: modbus_read_registers() failed: %s\n", modbus_strerror(errno) );
		}
		else
		{
			printf( "Read registers: Rated Voltage %u %.02f V", ulInputs[0], (double)ulInputs[0] / 100 );
		}
		printf("\n");

		usleep( 50000 );


		if ( vsdOperation >= 1 && vsdOperation <= 7 )
		{	// vsd must be set for communications control
			printf( "VSD operation not supported\n" );
		}
	}
}



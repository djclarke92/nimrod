// test program to ask a nimrod host how many devices it is connected to
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <modbus.h>
#include "mb_devices.h"
#include "mb_socket.h"



void PrintUsage( char* szProg );


// send msg to nimrod server
int main ( int argc, char** argv )
{
	int rc = 0;
	int iLen;
	int sockfd;

	if ( argc != 3 )
	{
		PrintUsage( argv[0] );
		exit( 1 );
	}


	NIMROD_MSGBUF_TYPE msgBuf;

	bzero( &msgBuf, sizeof(msgBuf) );


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 )
	{
	    perror( "ERROR opening socket\n" );
	    exit( 1 );
	}

	struct sockaddr_in serv_addr;
	struct hostent *server;

	server = gethostbyname(argv[1]);
	if ( server == NULL )
	{
		printf( "ERROR, no such host\n" );
		exit( 1 );
	}

	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons(NIMROD_PORT);

	if ( connect( sockfd, (const sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0 )
	{
		perror( "connect failed\n" );
	}
	else
	{
		msgBuf.msg.eMsgType = E_MT_TOTAL_DEVICES;
		msgBuf.msg.iSegCount = 1;
		msgBuf.msg.iSegTotal = 1;

		iLen = write( sockfd, msgBuf.szBuf, sizeof(msgBuf) );
		if ( iLen != sizeof(msgBuf) )
		{
			printf( "Error: only wrote %d of %lu bytes\n", iLen, (unsigned long)sizeof(msgBuf) );
		}
		else
		{	// read the reply
			iLen = 0;

			bzero( &msgBuf, sizeof(msgBuf) );

			// read all bytes
			while ( iLen != sizeof(msgBuf) )
			{
				rc = recv( sockfd, &msgBuf.szBuf[iLen], sizeof(msgBuf)-iLen, 0 );
				if ( rc == -1 )
				{	// error
					if ( errno != EINTR )
					{	// not interrupt
						printf( "Failed to read socket message (%u of %lu bytes), errno %d\n", iLen, (unsigned long)sizeof(msgBuf), errno );
						break;
					}
				}
				else
				{
					iLen += rc;
				}
			}

			if ( iLen == sizeof(msgBuf) )
			{
				printf( "Got reply msg type %d\n", msgBuf.msg.eMsgType );

				switch ( msgBuf.msg.eMsgType )
				{
				default:
					break;
				case E_MT_TOTAL_DEVICES:
					printf( "Nimrod has a total of %d devices\n", msgBuf.msg.totalDevices.iTotalDevices );
					break;
				}
			}
			else
			{
				printf( "Failed to read reply, got %d of %lu bytes, errno %d\n", iLen, (unsigned long)sizeof(msgBuf),errno );
			}
		}
	}

	close( sockfd );

	return rc;
}

void PrintUsage( char* szProg )
{
	printf( "Usage: %s <server_hostname> <device_address>\n", szProg );
}

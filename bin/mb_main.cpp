/*
 * Nimrod server main
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
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <modbus/modbus.h>
#include "mb_mysql.h"
#include "mb_devices.h"
#include "mb_thread.h"
#include "mb_main.h"


#define NIMROD_RELEASE		1

extern int NIMROD_BUILD_NUMBER;


bool gbDebug = false;
bool gbTerminateNow = false;
bool gbReadConfig = false;
char gszHostname[HOST_NAME_MAX+1] = "";
RS485_CONNECTION_TYPE gRS485 = { 9600, 'N', 8, 1 };
CThreadMsg gThreadMsg;


void InitSSL(void);
void SigIntHandler( int sig );
void SigHupHandler( int sig );
void TestFunc();



void* ThreadStart( void* pp )
{
	((CThread*)pp)->Worker();

	return NULL;
}


#define MAX_THREADS		10

 
int main( int argc, char *argv[] )
{
	//bool bMaster = true;
	bool bRun = false;
	bool bShutdownNimrod = false;
	bool bThreadRunning[MAX_THREADS];
	bool bAllDevicesDead[MAX_THREADS];
	int i;
	int err;
    int status;
    int iThreads = MAX_THREADS;
    int iTotalComPorts;
    time_t tLastUpgradeCheck = 0;
    time_t tConfigTime = 0;
    time_t tUpdated = 0;
    pid_t pid;
	pid_t pidChild;
	pthread_t threadId[MAX_THREADS];
	char szBuf[256];
	char szComPortList[MAX_DEVICES][MAX_COMPORT_LEN+1];
	CDeviceList myDevices;
	CInOutLinks myIOLinks;
	CPlcStates myPlcStates;
	pthread_mutexattr_t attr;
	struct stat statbuf;
	FILE* pStdout = NULL;
	FILE* pStderr = NULL;
	CThread* myThread[MAX_THREADS];
	CMysql myDB;

	for ( i = 0; i < MAX_THREADS; i++ )
	{
		threadId[i] = 0;
		myThread[i] = NULL;
		bThreadRunning[i] = false;
		bAllDevicesDead[i] = false;
	}

	signal( SIGINT, SigIntHandler );
	signal( SIGHUP, SigHupHandler );

	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

	for ( i = 0; i < E_LT_MAX_LOCKS; i++ )
	{
		if ( pthread_mutex_init( &mutexLock[i], &attr ) != 0 )
		{
			printf( "mutex init failed\n" );
			exit( 1 );
		}
    }


	ReadSiteConfig( "NIMROD_LOG_DIR", gszLogDir, sizeof(gszLogDir) );

	// redirect stdout and stderr to a log file
	snprintf( szBuf, sizeof(szBuf), "%s/nimrod-stdout.log", gszLogDir );
	pStdout = freopen( szBuf, "a+", stdout );
	snprintf( szBuf, sizeof(szBuf), "%s/nimrod-stdout.log", gszLogDir );
	pStderr = freopen( szBuf, "a+", stderr );
	if ( pStdout == NULL || pStderr == NULL )
	{
		LogMessage( E_MSG_ERROR, "Error redirecting stdout %p or stderr %p", pStdout, pStderr );
	}

	SetMyHostname();

	snprintf( gszProgName, sizeof(gszProgName), "%s", basename(argv[0]) );



	pidChild = CreateSSHTunnel();

	int iCount = 0;
	long lSleep;
	long lSleepBase = 2000000;
	while ( (bRun = myDB.Connect()) == false && !gbTerminateNow )
	{
		LogMessage( E_MSG_WARN, "Waiting to connect to Mysql..." );

		if ( pidChild != 0 )
		{
			pid = waitpid( pidChild, &status, WNOHANG );
			if ( pid > 0 )
			{	// child has exited !
				pidChild = 0;
			}
		}

		if ( pidChild <= 0 )
		{	// ssh tunnel failed
			LogMessage( E_MSG_ERROR, "SSH tunnel has exited, starting a new one" );
			pidChild = CreateSSHTunnel();
		}

	    if ( tLastUpgradeCheck + 5 < time(NULL) )
	    {
	    	tLastUpgradeCheck = time(NULL);

	    	CheckForUpgrade();
	    	if ( gbTerminateNow )
	    	{
	    		break;
	    	}
	    }

		if ( iCount >  10 )
			lSleep = lSleepBase * 5;	// 10 sec
		else if ( iCount >  50 )
			lSleep = lSleepBase * 30;	// 60 sec
		else
			lSleep  = lSleepBase;	// 2 sec

		LogMessage( E_MSG_INFO, "Sleeping %ld seconds", lSleep / 1000000 );
		usleep( lSleep );

		iCount += 1;
	}

	myDB.LogEvent( 0, 0, E_ET_STARTUP, 0, "Nimrod %d.%d starting", NIMROD_RELEASE, NIMROD_BUILD_NUMBER );

	LogMessage( E_MSG_INFO, "Nimrod build: %d.%d", NIMROD_RELEASE, NIMROD_BUILD_NUMBER );
	LogMessage( E_MSG_INFO, "Compiled against libmodbus v%s", LIBMODBUS_VERSION_STRING );
	LogMessage( E_MSG_INFO, "Linked against libmodbus v%d.%d.%d", libmodbus_version_major, libmodbus_version_minor, libmodbus_version_micro );


	InitSSL();


	tConfigTime = time(NULL);
	myDevices.ReadDeviceConfig( myDB );
	myIOLinks.ReadIOLinks( myDB );

	iTotalComPorts = myDevices.GetTotalComPorts( szComPortList );
	LogMessage( E_MSG_INFO, "Total com ports: %d", iTotalComPorts );

	// check if com ports have been swapped
	myDevices.GetComPortsOnHost( myDB, szComPortList );


	// init libmodbus and connect the com ports
	iCount = 0;
	bRun = false;
	while ( (bRun = myDevices.InitContext()) == false && !gbTerminateNow )
	{	// loop here until we can connect to the com ports
		LogMessage( E_MSG_WARN, "Waiting for COM ports to become available" );

		myDevices.FreeAllContexts();

		// TODO: we may need to reboot the pi if the com ports do not appear

		tUpdated = myDB.ReadConfigUpdateTime();
		if ( tUpdated > tConfigTime )
		{	// config has changed
			pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

			myDevices.ReadDeviceConfig( myDB );
			myIOLinks.ReadIOLinks( myDB );
			tConfigTime = time(NULL);

			pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );
		}

	    if ( tLastUpgradeCheck + 5 < time(NULL) )
	    {
	    	tLastUpgradeCheck = time(NULL);

	    	CheckForUpgrade();
	    	if ( gbTerminateNow )
	    	{
	    		break;
	    	}
	    }

	    if ( iTotalComPorts == 0 )
	    {
	    	bRun = true;
	    	break;
	    }

		if ( iCount < 10 )
			usleep( 2000000 );
		else
			usleep( 30000000 );

		iCount += 1;
	}

	if ( bRun )
	{

		iThreads = iTotalComPorts + 3;

		int iPort = 0;
		for ( i = 0; i < iThreads; i++ )
		{
			if ( i == 0 )
			{
				myThread[i] = new CThread( "", &myDevices, &myIOLinks, &myPlcStates, E_TT_TCPIP, &bThreadRunning[i], &bAllDevicesDead[i] );
			}
			else if ( i == 1 )
			{
				myThread[i] = new CThread( "", &myDevices, &myIOLinks, &myPlcStates, E_TT_TIMER, &bThreadRunning[i], &bAllDevicesDead[i] );
			}
			else if ( i == 2 )
			{
				myThread[i] = new CThread( "", &myDevices, &myIOLinks, &myPlcStates, E_TT_WEBSOCKET, &bThreadRunning[i], &bAllDevicesDead[i] );
			}
			else
			{
				myThread[i] = new CThread( szComPortList[iPort], &myDevices, &myIOLinks, &myPlcStates, E_TT_COMPORT, &bThreadRunning[i], &bAllDevicesDead[i] );
				iPort += 1;
			}


			err = pthread_create( &threadId[i], NULL, ThreadStart, myThread[i] );
			if ( err != 0 )
			{
				LogMessage( E_MSG_ERROR, "pthread_create(%d) failed with errno %d", i, errno );
				gbTerminateNow = true;
				break;
			}
		}




		while ( !gbTerminateNow )
		{
			// check if our ssh tunnel has exited
			if ( pidChild != 0 )
			{
				pid = waitpid( pidChild, &status, WNOHANG );
				if ( pid > 0 )
				{	// child has exited !
					LogMessage( E_MSG_ERROR, "SSH tunnel has exited, starting a new one" );

					pidChild = CreateSSHTunnel();
				}
				else if ( pid == 0 )
				{	// child is still running
					// continue
				}
				else
				{	// error
					LogMessage( E_MSG_WARN, "waitpid() failed for pid %d, errno %d", pidChild, errno );
				}
		    }

			// check if all com ports are dead
			int iDead = 0;
			for ( i = 2; i < iThreads; i++ )
			{
				if ( bAllDevicesDead[i] )
				{
					iDead += 1;
				}
			}
			if ( iDead + 3 == iThreads && iTotalComPorts != 0 )
			{	// all com port devices are dead, restart
				LogMessage( E_MSG_INFO, "All com port devices are dead, restarting" );
				if ( CreateRestartScript( NULL ) )
				{
					gbTerminateNow = true;
					gbRestartNow = true;
					break;
				}
			}

		    if ( tLastUpgradeCheck + 5 < time(NULL) )
		    {
		    	tLastUpgradeCheck = time(NULL);

		    	CheckForUpgrade();
		    	if ( gbTerminateNow )
		    	{
		    		break;
		    	}
		    }

			if ( stat( STOP_NIMROD_FILE, &statbuf ) == 0 )
			{
				bShutdownNimrod = true;
				gbTerminateNow = true;
			}

			// slee 500msec
			usleep( 500000 );
		}
	}

	// wait for threads to exit
	LogMessage( E_MSG_INFO, "Wait for threads to exit" );
	bool bWait = true;
	time_t tStart = time(NULL);
	while ( bWait )
	{
		bWait = false;
		for ( i = 0; i < iThreads; i++ )
		{
			if ( bThreadRunning[i] )
			{
				bWait = true;

				if ( tStart + 3 < time(NULL) )
				{
					LogMessage( E_MSG_FATAL, "Killing thread #%d with SIGTERM", i );

					// SIGTERM will kill the process immediately, systemd will restart us
					pthread_kill( threadId[i], SIGTERM );
				}
			}
		}

		if ( bWait )
		{
			//tStart = time(NULL);
			usleep( 100000 );	// 100msec
		}
		else
		{
			LogMessage( E_MSG_INFO, "All threads have exited" );
			break;
		}
	}

	LogMessage( E_MSG_INFO, "Calling pthread_join" );
	for ( i = 0; i < iThreads; i++ )
	{
		if ( threadId[i] != 0 )
		{
			pthread_join( threadId[i], NULL );
			threadId[i] = 0;
		}

		if ( myThread[i] != NULL )
		{
			delete myThread[i];
			myThread[i] = NULL;
		}
	}
	LogMessage( E_MSG_INFO, "All threads terminated" );

	myDevices.FreeAllContexts();

	myDB.Disconnect();

	if ( pidChild != 0 )
	{
	    kill( pidChild, SIGINT );
	    kill( pidChild, SIGHUP );
	    kill( pidChild, SIGTERM );
	    kill( pidChild, SIGKILL );

	    pid = waitpid( pidChild, &status, 0 );

	    LogMessage( E_MSG_INFO, "waitpid returned %d", pid );
	}

	LogMessage( E_MSG_INFO, "Main terminating" );

	for ( i = 0; i < E_LT_MAX_LOCKS; i++ )
	{
		pthread_mutex_destroy( &mutexLock[i] );
	}

	if ( bShutdownNimrod )
	{
		LogMessage( E_MSG_INFO, "Shutdown file '%s' found, terminating", STOP_NIMROD_FILE );
		unlink( STOP_NIMROD_FILE );
	}
	else if ( gbUpgradeNow )
	{
		snprintf( szBuf, sizeof(szBuf), "bash %s&", UPGRADE_SCRIPT );
		LogMessage( E_MSG_INFO, "Running upgrade script: %s", szBuf );
		if ( system( szBuf ) < 0 )
		{
			LogMessage( E_MSG_ERROR, "system() failed for upgrade script, errno %d", errno );
		}
	}
	else if ( gbRestartNow )
	{
		LogMessage( E_MSG_INFO, "Running restart script" );
		snprintf( szBuf, sizeof(szBuf), "bash %s&", UPGRADE_SCRIPT );
		if ( system( szBuf ) < 0 )
		{
			LogMessage( E_MSG_ERROR, "system() failed for restart script, errno %d", errno );
		}
	}

	fclose( pStdout );
	fclose( pStderr );

	// sleep or the upgrade does not work - something to do with running from systemd
	sleep( 2 );
//	TestFunc();

	return 0;
}

void InitSSL(void)
{
    SSL_load_error_strings();   /* load all error messages */
    ERR_load_crypto_strings();

    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_library_init();
}



void SigIntHandler( int sig )
{
	gbTerminateNow = true;
}

void SigHupHandler( int sig )
{
	gbReadConfig = true;
}

void TestFunc()
{
	int newAddr = 2;
	modbus_t* ctx = NULL;

	printf( "Compiled against libmodbus v%s\n", LIBMODBUS_VERSION_STRING );
	printf( "Linked against libmodbus v%d.%d.%d\n", libmodbus_version_major, libmodbus_version_minor, libmodbus_version_micro );
	printf( "\n" );

	ctx = modbus_new_rtu( "/dev/ttyUSB2", 19200, 'N', 8, 1 );
	if ( ctx == NULL )
	{
		printf( "Error: modbus ctx in is null, aborting: %s\n", modbus_strerror(errno) );
		exit( 1 );
	}

	printf( "Ctx %p\n", ctx );

	modbus_set_debug(ctx, true);

	modbus_rtu_set_serial_mode( ctx, MODBUS_RTU_RS232 );

	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

#if LIBMODBUS_VERSION_CHECK(3,1,0)
	modbus_set_response_timeout( ctx, tv.tv_sec, tv.tv_usec );
#else
	modbus_set_response_timeout( ctx, &tv );
#endif

	usleep( 50000 );

	if ( modbus_connect(ctx) == -1 )
	{
		printf( "Error: modbus_connect() failed, aborting: %s\n", modbus_strerror(errno) );
		exit( 1 );
	}

	usleep( 50000 );

	printf( "Setting slave addr to %d\n", newAddr );
	if ( modbus_set_slave( ctx, newAddr ) == -1 )
	{
		printf( "Error: modbus_set_slave(%d) failed: %s\n", newAddr, modbus_strerror(errno) );
	}

	usleep( 150000 );

	// read input bits to see if the device is talking
	int addr = 0;
	const int iLen = 8;
	uint8_t uInputs[iLen];

	for ( int i = 0; i < iLen; i++ )
	{
		uInputs[i] = 0;
	}

	int rc = modbus_read_input_bits( ctx, addr, iLen, uInputs );
	if ( rc == -1 )
	{
		printf( "Error: modbus_read_input_bits() failed: %s\n", modbus_strerror(errno) );
	}
	else
	{
		printf( "Read input bits: %u %u %u %u %u %u %u %u\n", uInputs[0], uInputs[1], uInputs[2], uInputs[3], uInputs[4], uInputs[5], uInputs[6], uInputs[7] );
	}

	modbus_free( ctx );
}

void SetMyHostname()
{
	if ( gethostname( gszHostname, sizeof(gszHostname) ) == 0 )
	{	// success
		LogMessage( E_MSG_INFO, "Running on host '%s'", gszHostname );
	}
	else
	{	// error
		gszHostname[0] = '\0';

		LogMessage( E_MSG_ERROR, "gethostname() failed with errno %d", errno );
	}
}

bool IsMyHostname( const char* szHost )
{
	bool bRc = false;

	if ( strcasecmp( szHost, gszHostname ) == 0 )
	{
		bRc = true;
	}

	return bRc;
}

const char* GetMyHostname()
{
	return gszHostname;
}



//***********************************************************************************************************************
//
//	CThrteadMsg class
//
//***********************************************************************************************************************
CThreadMsg::CThreadMsg()
{
	Init();
}

CThreadMsg::~CThreadMsg()
{

}

void CThreadMsg::Init()
{
	for ( int i = 0; i < MAX_THREAD_MESSAGES; i++ )
	{
		m_szMsg[i][0] = '\0';
	}
}

void CThreadMsg::PutMessage( const char* szFmt, ... )
{
	bool found = false;
	int i;
	va_list args;
	char szBuf[100];

	pthread_mutex_lock( &mutexLock[E_LT_WEBSOCKET] );

	va_start( args, szFmt );

	vsnprintf( szBuf, sizeof(szBuf), szFmt, args );

	va_end( args );

	for ( i = 0; i < MAX_THREAD_MESSAGES; i++ )
	{
		if ( m_szMsg[i][0] == '\0' )
		{	// found an empty slot
			found = true;
			snprintf( m_szMsg[i], sizeof(m_szMsg[i]), "%s", szBuf );
			break;
		}
	}

	pthread_mutex_unlock( &mutexLock[E_LT_WEBSOCKET] );

	if ( !found )
	{
		LogMessage( E_MSG_ERROR, "Failed to put ws message" );
	}
}

const bool CThreadMsg::GetMessage( char* szMsg, const size_t uLen )
{
	bool found = false;
	int i;

	szMsg[0] = '\0';

	pthread_mutex_lock( &mutexLock[E_LT_WEBSOCKET] );

	for ( i = 0; i < MAX_THREAD_MESSAGES; i++ )
	{
		if ( m_szMsg[i][0] != '\0' )
		{	// found a message
			found = true;
			snprintf( szMsg, uLen, "%s", m_szMsg[i] );
			m_szMsg[i][0] = '\0';
			break;
		}
	}

	pthread_mutex_unlock( &mutexLock[E_LT_WEBSOCKET] );

	return found;
}

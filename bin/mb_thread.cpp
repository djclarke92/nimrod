/*
 * Nimrod server
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <string>
#include <math.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/tls1.h>
#include <openssl/ssl.h>
#include <mariadb/mysql.h>
#include <modbus/modbus.h>
#include "mb_devices.h"
#include "mb_thread.h"
#include "mb_main.h"
#include "mb_mysql.h"
#include "mb_socket.h"


extern bool gbPlcIsActive;
extern CThreadMsg gThreadMsgToWS;
extern CThreadMsg gThreadMsgFromWS;

pthread_mutex_t mutexLock[E_LT_MAX_LOCKS];


CThread::CThread( const char* szPort, CDeviceList* pmyDevices, CInOutLinks* pmyIOLinks, CPlcStates* pmyPlcStates, const enum E_THREAD_TYPE eThreadType, bool* pbThreadRunning, bool* pbAllDevicesDead )
{
	m_eThreadType = eThreadType;
	snprintf( m_szMyComPort, sizeof(m_szMyComPort), "%s", szPort );
	m_pmyDevices = pmyDevices;
	m_pmyIOLinks = pmyIOLinks;
	m_pmyPlcStates = pmyPlcStates;
	m_pbThreadRunning = pbThreadRunning;
	m_pbAllDevicesDead = pbAllDevicesDead;

	m_tThreadStartTime = time(NULL);
	m_tConfigTime = time(NULL);
	m_tPlcStatesTimeAll = 0;
	m_tPlcStatesTimeDelayTime = 0;
	m_tLastConfigCheck = time(NULL);
	m_tLastPlcStatesCheck = 0;
	m_tLastCameraSnapshot = time(NULL);
	m_tLastCertificateCheck = 0;
	m_tCardReaderStart = 0;
	m_iLevelMessage = 0;
	m_bSecureWebSocket = true;
	m_tLastSentEspTime = 0;

	m_sslServerCtx = NULL;
	m_sslClientCtx = NULL;
	m_Sockfd = -1;
	m_szEspResponseMsg[0] = '\0';

	m_WSContext = NULL;

	m_szComBuffer[0] = '\0';

	m_iClientCount = 0;
	for ( int i = 0; i < MAX_TCPIP_SOCKETS; i++ )
	{
		m_iClientFd[i] = -1;
		m_xClientSSL[i] = NULL;
		m_tClientLastMsg[i] = 0;
		bzero( m_szClientEspName[i], sizeof(m_szClientEspName[i]) );
		bzero( (char*)&m_xClientInAddr[i], sizeof( m_xClientInAddr[i]) );
	}
}

CThread::~CThread()
{
	m_iClientCount = 0;
	for ( int i = 0; i < MAX_TCPIP_SOCKETS; i++ )
	{
		if ( m_iClientFd[i] != -1 )
		{
			CloseSocket( m_iClientFd[i], i );
		}
	}

	if ( IsTimerThread() )
	{
		gbPlcIsActive = false;
	}

	LogMessage( E_MSG_INFO, "Thread %s terminating", GetThreadType(m_eThreadType) );
}

// Server PSK callback
const char* psKey = "baadf00d1f2d3e4abaadf00d1f2d3e4abaadf00d1f2d3e4abaadf00d1f2d3e4a";
//const char* psKey = "baadf00d1f2d3e4abaadf00d1f2d3e4a";
//const char* psKey = "baadf00d";
unsigned int psk_server_cb(SSL* ssl, const char* identity,
                           unsigned char* psk, unsigned int max_psk_len) {
    // ... logic to lookup PSK based on identity ...
	unsigned int uLen = strlen(psKey);
    if (strcmp(identity, "esp_client") == 0) {
        memcpy(psk, psKey, uLen);
		psk[uLen] = '\0';
		LogMessage( E_MSG_INFO, "psk_server_cb: %u %u", uLen, max_psk_len );

        return uLen;
    }
		LogMessage( E_MSG_INFO, "psk_server_cb: 0" );
    return 0; // PSK not found
}

SSL_CTX* CThread::InitServerCTX(void)
{
	const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();	//TLS_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
    	LogMessage( E_MSG_ERROR, "InitServerCTX: SSL_CTX_new failed: %s", ERR_reason_error_string(ERR_get_error()) );
    }
    else
    {
    	//SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

		//SSL_CTX_set_cert_verify_callback(ctx, always_true_callback, NULL);
		SSL_CTX_set_psk_server_callback( ctx, psk_server_cb );

		/*
		Shared Ciphers: ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA
		Signature Algorithms: ECDSA+SHA256:ECDSA+SHA224:ECDSA+SHA384:ECDSA+SHA512:ECDSA+SHA1:RSA+SHA256:RSA+SHA224:RSA+SHA384:RSA+SHA512:RSA+SHA1
		Shared Signature Algorithms: ECDSA+SHA256:ECDSA+SHA224:ECDSA+SHA384:ECDSA+SHA512:RSA+SHA256:RSA+SHA224:RSA+SHA384:RSA+SHA512
		CIPHER is ECDHE-RSA-CHACHA20-POLY1305
		*/
		int rc;
		rc = SSL_CTX_set_cipher_list( ctx, "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA" );
		if ( rc == 0 ) {
			LogSSLError();
		}
		//rc = SSL_CTX_set_ciphersuites( ctx, "PSK" );
		//if ( rc == 0 ) {
		//	LogSSLError();
		//}

    	SSL_CTX_set_min_proto_version( ctx, TLS1_1_VERSION );
    	SSL_CTX_set_max_proto_version( ctx, TLS1_2_VERSION );

    	//SSL_CTX_set_max_send_fragment( ctx, 1024 );

    	LogMessage( E_MSG_INFO, "Server SSL ctx initialised %p", ctx );
    }

    return ctx;
}

SSL_CTX* CThread::InitClientCTX(void)
{
	const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_client_method();	//TLS_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
    	LogMessage( E_MSG_ERROR, "InitClientCTX: SSL_CTX_new failed: %s", ERR_reason_error_string(ERR_get_error()) );
    }
    else
    {
    	//SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);

		//SSL_CTX_set_cert_verify_callback(ctx, always_true_callback, NULL);

    	SSL_CTX_set_min_proto_version( ctx, TLS1_2_VERSION );
    	SSL_CTX_set_max_proto_version( ctx, TLS1_2_VERSION );

    	//SSL_CTX_set_max_send_fragment( ctx, 1024 );

    	LogMessage( E_MSG_INFO, "Client SSL ctx initialised %p", ctx );
    }

    return ctx;
}

bool CThread::LoadCertificates( SSL_CTX* ctx, const char* szCertFile, const char* szKeyFile )
{
	bool bRc = true;

    // set the local certificate from CertFile
    if ( SSL_CTX_use_certificate_file(ctx, szCertFile, SSL_FILETYPE_PEM) <= 0 )
    {
    	bRc = false;
    	LogMessage( E_MSG_ERROR, "SSL_CTX_use_certificate_file failed: %s", ERR_reason_error_string(ERR_get_error()) );
    	LogMessage( E_MSG_ERROR, "Check certificate file '%s' exists and is readable", szCertFile );
    }

    // set the private key from KeyFile (may be the same as CertFile)
    if ( SSL_CTX_use_PrivateKey_file(ctx, szKeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
    	bRc = false;
    	LogMessage( E_MSG_ERROR, "SSL_CTX_use_PrivateKey_file failed: %s", ERR_reason_error_string(ERR_get_error()) );
    	LogMessage( E_MSG_ERROR, "Check key file '%s' exists and is readable", szKeyFile );
    }

    // verify private key
    if ( !SSL_CTX_check_private_key(ctx) )
    {
    	bRc = false;
    	LogMessage( E_MSG_ERROR, "SSL_CTX_check_private_key failed: Private key does not match the public certificate" );
    }

	if (SSL_CTX_load_verify_locations(ctx,"/etc/ssl/certs/ca-certificates.crt", NULL) <1) 
	{
		bRc = false;
		LogMessage( E_MSG_ERROR, "SSL_CTX_load_verify_locations failed: %s", ERR_reason_error_string(ERR_get_error()) );
	}

    if ( bRc )
    {
    	LogMessage( E_MSG_INFO, "SSL Certificates loaded" );
    }

	return bRc;
}

void CThread::ShowCerts(SSL* ssl)
{
	X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        LogMessage( E_MSG_INFO, "Cert Info: %s", line );
        free(line);

        X509_free(cert);
    }
    else
    {
        LogMessage( E_MSG_INFO, "No certificates" );
    }
}

void CThread::CertErrorFlagFile( const bool bError )
{
	static bool bLastError = true;
	char szDir[256] = "";
	char szFile[512];
	FILE* pFile = NULL;
	struct stat statbuf;

	if ( bLastError != bError )
	{	// something to do
		bLastError = bError;

		if ( getcwd(szDir, sizeof(szDir)) != NULL )
		{
			snprintf( szFile, sizeof(szFile), "%s/nimrod.certng", szDir );
			if ( bError )
			{	// create flag file
				pFile = fopen( szFile, "wt" );
				if ( pFile != NULL )
				{
					fclose( pFile );
					LogMessage( E_MSG_INFO, "Created '%s'", szFile );
				}
				else
				{
					LogMessage( E_MSG_WARN, "Failed to create '%s', errno %d", szFile, errno );
				}
			}
			else if ( stat( szFile, &statbuf ) == 0 )
			{	// remove flag file
				if ( unlink( szFile ) == 0 )
					LogMessage( E_MSG_INFO, "Removed '%s'", szFile );
				else
					LogMessage( E_MSG_WARN, "Failed to remove '%s', errno %d", szFile, errno );
			}
		}
		else
		{
			LogMessage( E_MSG_ERROR, "Failed to getcwd(), errno %d", errno );
		}
	}
}

void CThread::CertAgingFlagFile( const bool bError )
{
	static bool bLastError = true;
	char szDir[256] = "";
	char szFile[512];
	FILE* pFile = NULL;
	struct stat statbuf;

	if ( bLastError != bError )
	{	// something to do
		bLastError = bError;

		if ( getcwd(szDir, sizeof(szDir)) != NULL )
		{
			snprintf( szFile, sizeof(szFile), "%s/nimrod.certag", szDir );
			if ( bError )
			{	// create flag file
				pFile = fopen( szFile, "wt" );
				if ( pFile != NULL )
				{
					fclose( pFile );
					LogMessage( E_MSG_INFO, "Created '%s'", szFile );
				}
				else
				{
					LogMessage( E_MSG_WARN, "Failed to create '%s', errno %d", szFile, errno );
				}
			}
			else if ( stat( szFile, &statbuf ) == 0 )
			{	// remove flag file
				if ( unlink( szFile ) == 0 )
					LogMessage( E_MSG_INFO, "Removed '%s'", szFile );
				else
					LogMessage( E_MSG_WARN, "Failed to remove '%s', errno %d", szFile, errno );
			}
		}
		else
		{
			LogMessage( E_MSG_ERROR, "Failed to getcwd(), errno %d", errno );
		}
	}
}

void CThread::Worker()
{
	bool bPrintedTO = false;
	int i;
	int idx;
	int iLastAddress = 0;
	int iDayMinutes;
	int iLastDayMinutes = -1;
	int iAllDeadCheckPeriod = 10;
	int iPlcStateReadCount = 0;
	time_t tTimenow;
	time_t tUpdated;
	time_t tAllDeadStart = 0;
	time_t tLastLevelK02Prompt = 0;
	time_t tLastTimerDeviceCheck = 0;
	time_t tLastCurlCheckTime = 0;
	time_t tCertAgingStartTime = 0;
	const char* pszCertFile = "/home/nimrod/nimrod-cert.pem";
	const char* pszKeyFile = "/home/nimrod/nimrod-cert.key";
	struct tm tm;
	struct timeval old_response_to_tv;
	struct timeval response_to_tv;
	CMysql myDB;


	*m_pbThreadRunning = true;

	LogMessage( E_MSG_INFO, "Thread starting: type %s", CThread::GetThreadType(m_eThreadType) );

	myDB.Connect();

	// all threads need ssl clients
	m_sslClientCtx = InitClientCTX();

	if ( IsTcpipThread() )
	{	// create listener socket
		m_sslServerCtx = InitServerCTX();

		// openssl req -x509 -nodes -days 3650 -newkey rsa:2048 -keyout nimrod-cert.key -out nimrod-cert.pem
		if ( !LoadCertificates( m_sslServerCtx, pszCertFile, pszKeyFile ) )
		{
			gbCertificateError = true;
		}

		CreateListenerSocket();

		// mark all ESP devices as being suspect
		for ( idx = 0; idx < MAX_DEVICES; idx++ )
		{
			if ( strncmp( m_pmyDevices->GetComPort(idx), "ESP", 3 ) == 0 )
			{
				LogMessage( E_MSG_INFO, "Marking ESP device '%s' as SUSPECT", m_pmyDevices->GetDeviceName(idx) );
				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_SUSPECT ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				}
			}
			else if ( m_pmyDevices->GetComPort(idx)[0] == '\0' )
			{	// end of list
				break;
			}
		}
	}

	if ( IsTimerThread() )
	{
		ReadCameraRecords( myDB, m_CameraList );
	}

	if ( IsWebsocketThread() )
	{
		char szBuf[100] = "";
		ReadSiteConfig( "SECURE_WEBSOCKET", szBuf, sizeof(szBuf) );
		if ( strlen(szBuf) != 0 )
		{
			m_bSecureWebSocket = (bool)atoi(szBuf);
		}

		websocket_init();
	}


	while ( !gbTerminateNow )
	{
		bool bAllDead = true;
		int iMax = MAX_DEVICES;

		tTimenow = time(NULL);
		localtime_r( &tTimenow, &tm );
		iDayMinutes = tm.tm_hour * 60 + tm.tm_min;

		if ( IsTcpipThread() && m_tLastConfigCheck + 5 < tTimenow )
		{	// load new config settings in tcpip thread only
			tUpdated = myDB.ReadConfigUpdateTime();
			if ( tUpdated > m_tConfigTime )
			{	// config has changed
				LogMessage( E_MSG_INFO, "config time %lu vs %lu vs %lu", tUpdated, m_tConfigTime, time(NULL) );

				m_tConfigTime = ReadDeviceConfig( myDB, m_pmyDevices, m_pmyIOLinks, false );
				m_tLastConfigCheck = time(NULL);
			}
		}

		if ( IsWebsocketThread() )
		{	// the lws_service() function can block for a few seconds
			char szMsg[256] = "";

			if ( gThreadMsgToWS.GetMessage( szMsg, sizeof(szMsg) ) )
			{
				//LogMessage( E_MSG_INFO, "WS msg '%s'", szMsg );
				websocket_addmessage( szMsg );
			}
			websocket_process();

			usleep( 10000 );
		}
		else if ( IsTcpipThread() )
		{
			AcceptTcpipClient();
			ReadTcpipMessage( m_pmyDevices, myDB );
			SendEspMessage();

			usleep( 50000 );
		}
		else if ( IsTimerThread() )
		{
			if ( m_tLastCertificateCheck + 10 < tTimenow )
			{
				if ( gbCertificateError )
				{	// send websocket msg to home page - but websockets wont work if the cert wasn't loaded
					PostToWebSocket( E_ET_CERTIFICATENG, 0, 0, 1, true );
				}
				CertErrorFlagFile( gbCertificateError );

				if ( gbCertificateAging )
				{
					if ( tCertAgingStartTime == 0 )
					{
						tCertAgingStartTime =  time(NULL);
					}
					else if ( time(NULL) - tCertAgingStartTime >= 60*60*4 )
					{	// clear this flag every 4 hours
						gbCertificateAging = false;
					}
					PostToWebSocket( E_ET_CERTIFICATEAG, 0, 0, 1, gbCertificateAging );
				}
				CertAgingFlagFile( gbCertificateAging );

				m_tLastCertificateCheck = time(NULL);
			}

			if ( m_tLastPlcStatesCheck + 2 < tTimenow )
			{	// load new plcstates settings in timer thread only
				if ( iPlcStateReadCount == 0 )
				{	// first time after startup - clear the active state
					LogMessage( E_MSG_INFO, "Clearing PlcActiveState at startup" );
					ClearPlcActiveState( myDB );
				}

				iPlcStateReadCount += 1;
				tUpdated = myDB.ReadPlcStatesUpdateTimeAll();
				if ( tUpdated > m_tPlcStatesTimeAll )
				{	// config has changed
					//LogMessage( E_MSG_INFO, "plcstates time %lu vs %lu vs %lu", tUpdated, m_tPlcStatesTime, time(NULL) );
					m_tPlcStatesTimeAll = time(NULL);
					m_tLastPlcStatesCheck = time(NULL);

					if ( ReadPlcStatesTableAll( myDB, m_pmyPlcStates ) != 0 )
					{	// plc state index is bad, clear everything
						ClearPlcActiveState( myDB );
					}
				}

				tUpdated = myDB.ReadPlcStatesUpdateTimeDelayTime();
				if ( tUpdated > m_tPlcStatesTimeDelayTime )
				{	// config has changed
					//LogMessage( E_MSG_INFO, "plcstates time DT %lu vs %lu vs %lu", tUpdated, m_tPlcStatesTime, time(NULL) );
					m_tPlcStatesTimeDelayTime = time(NULL);
					m_tLastPlcStatesCheck = time(NULL);

					ReadPlcStatesTableDelayTime( myDB, m_pmyPlcStates );
				}
			}

			if ( tLastTimerDeviceCheck != time(NULL) )
			{
				tLastTimerDeviceCheck = time(NULL);

				for ( idx = 0; idx < iMax && !gbTerminateNow; idx++ )
				{
					if ( !IsMyHostname( m_pmyDevices->GetDeviceHostname(idx) ) )
					{	// device is not on this host
						continue;
					}
					else if ( m_pmyDevices->GetAddress(idx) == 0 && iLastDayMinutes != iDayMinutes )
					{	// timer check once every minute
						pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

						LogMessage( E_MSG_INFO, "Checking timer at %d minutes", iDayMinutes );
						iLastDayMinutes = iDayMinutes;

						//char szMsg[100];
						//snprintf( szMsg, sizeof(szMsg), "Checking timer at %d minutes", iDayMinutes );
						//gThreadMsg.PutMessage( szMsg );

						for ( i = 0; i < m_pmyDevices->GetNumInputs(idx); i++ )
						{
							if ( m_pmyDevices->GetStartTime( idx, i ) == iDayMinutes )
							{
								if ( m_pmyDevices->IsTimerEnabledToday( idx, i ) )
								{
									LogMessage( E_MSG_INFO, "Timer start event for '%s' (0x%x->%d,%d)", m_pmyDevices->GetInIOName(idx,i), m_pmyDevices->GetAddress(idx), idx, i+1 );
									myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), i, E_ET_TIMER, 0, "Timer start event for '%s' (0x%x->%d,%d)", m_pmyDevices->GetInIOName(idx,i), m_pmyDevices->GetAddress(idx), idx, i+1 );

									ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), i, true, E_ET_TIMER );
								}
								else
								{
									LogMessage( E_MSG_INFO, "Timer start reached for '%s' (0x%x->%d,%d) but is disabled today", m_pmyDevices->GetInIOName(idx,i), m_pmyDevices->GetAddress(idx), idx, i+1 );
								}
							}
						}

						pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );

						// only run the cleanup on the primary nimrod pi
						char szHostname[50] = "nimrod";
						if ( strlen(m_pmyDevices->GetDeviceHostname(0)) > 0 )
						{	// assume the first device runs on the primary host
							snprintf( szHostname, sizeof(szHostname), "%s", m_pmyDevices->GetDeviceHostname(0) );
						}
						if ( iDayMinutes == 71 && IsMyHostname( szHostname ) )
						{	// delete events table records at 01:11am each day (60 + 11 = 71 minutes)
							myDB.CleanupEventsTable();
						}
					}
					else if ( m_pmyDevices->IsEspDevice(idx) )
					{	// handle timer off event for ESP devices
						pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

						CheckForTimerOffTime( myDB, idx );

						pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );
					}
				}

				std::string sAttachments;
				GetCameraSnapshots( myDB, m_CameraList, NULL, sAttachments );
			}

			if ( tLastCurlCheckTime + 300 <= time(NULL) )
			{	// every 5 minutes
				// ping back to flatcatit.co.nz for those hosts where flatcatit is managing their DNS records
				char szCmd[256];
				char szHostname[50] = "";
				gethostname( szHostname, sizeof(szHostname) );
				snprintf( szCmd, sizeof(szCmd), "curl --silent --connect-timeout 2 \"http://flatcatit.co.nz/helo-nimrod/%s\" > /dev/null &", szHostname );
				//LogMessage( E_MSG_INFO, "Running '%s'", szCmd );
				int rc = system( szCmd );
				if ( rc != 0 )
				{
					LogMessage( E_MSG_INFO, "curl helo-nimrod returned %d", rc );
				}

				tLastCurlCheckTime = time(NULL);
			}


			ProcessPlcStates( myDB, m_pmyPlcStates );

			usleep( 100000 );	// 100 msec sleep
		}
		else
		{	// com port thread (multiple threads !)
			for ( idx = 0; idx < iMax && !gbTerminateNow; idx++ )
			{
				if ( strlen(m_pmyDevices->GetDeviceName(idx)) == 0  )
				{	// end of list
					break;
				}
				else if ( !IsMyHostname( m_pmyDevices->GetDeviceHostname(idx) ) )
				{	// device is not on this host
					//LogMessage( E_MSG_INFO, "Bad host %s", m_pmyDevices->GetDeviceName(idx));
					continue;
				}
				else if ( !IsMyComPort( m_pmyDevices->GetComPort(idx) ) && !IsVirtualComPort( m_pmyDevices->GetComPort(idx) ))
				{	// not this thread's com port
					//LogMessage( E_MSG_INFO, "Bad com port %s", m_pmyDevices->GetDeviceName(idx));
					continue;
				}
				else if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED && (m_pmyDevices->GetTimeoutCount(idx) % 100) != 0 )
				{	// skip this device
					//LogMessage( E_MSG_INFO, "Bad dead %s", m_pmyDevices->GetDeviceName(idx));
					m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD );
					continue;
				}
				else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_LEVEL_K02 )
				{
					bool bSendPrompt = false;
					if ( tLastLevelK02Prompt + LEVEL_CHECK_PERIOD < time(NULL) )
					{	// send one character to prompt a level string to be returned
						tLastLevelK02Prompt = time(NULL);
						bSendPrompt = true;
					}

					HandleLevelDevice( myDB, idx, bSendPrompt, bAllDead );
				}
				else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_CARD_READER )
				{
					HandleCardReaderDevice( myDB, idx, bAllDead );
				}
				else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_SYSTEC_IT1 )
				{
					HandleSystecIT1Device( myDB, idx, bAllDead );
				}
				else if ( m_pmyDevices->GetContext(idx) == NULL && !IsVirtualComPort(m_pmyDevices->GetComPort(idx)) )
				{	// com port for this context does not exist
					//LogMessage( E_MSG_INFO, "Bad ctx %s", m_pmyDevices->GetDeviceName(idx));
					continue;
				}
				else if ( m_pmyDevices->GetAddress(idx) > 0 )
				{
					pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

					modbus_t* ctx = m_pmyDevices->GetContext(idx);
					//modbus_set_debug(ctx, true);

					//LogMessage( E_MSG_INFO, "SetSlave to %d", m_pmyDevices->GetAddress(idx) );
					if ( iLastAddress != m_pmyDevices->GetAddress(idx) )
					{
						if ( ctx != NULL && modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
						{
							LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
						}
						//else
						//{
						//	LogMessage( E_MSG_INFO, "modbus_set_slave(%p) slave is %d", ctx, idx);
						//}
						usleep( 15000 );	// was 20000

						iLastAddress = m_pmyDevices->GetAddress(idx);
					}

					if ( !bPrintedTO )
					{	// Save original timeout
						bPrintedTO = true;
#if LIBMODBUS_VERSION_CHECK(3,1,0)
						response_to_tv.tv_sec = 0;
						response_to_tv.tv_usec = 500000;
						modbus_set_response_timeout(ctx, (uint32_t)response_to_tv.tv_sec, (uint32_t)response_to_tv.tv_usec );

						modbus_get_response_timeout( ctx, (uint32_t*)&old_response_to_tv.tv_sec, (uint32_t*)&old_response_to_tv.tv_usec );
						LogMessage( E_MSG_INFO, "Response timeout ctx %p is %u.%06u usec", ctx, old_response_to_tv.tv_sec, old_response_to_tv.tv_usec );
#else
						response_to_tv.tv_sec = 0;
						response_to_tv.tv_usec = 500000;
						modbus_set_response_timeout(ctx, &response_to_tv);

						modbus_get_response_timeout( ctx, &old_response_to_tv );
						LogMessage( E_MSG_INFO, "Response timeout ctx %p is %u.%06u usec", ctx, old_response_to_tv.tv_sec, old_response_to_tv.tv_usec );
#endif
						//uint32_t uSec, uUsec;
						//modbus_get_response_timeout( ctx, &uSec, &uUsec );
						//LogMessage( E_MSG_INFO, "Response timeout ctx %p is %u.%06u usec", ctx, uSec, uUsec );
					}

					if ( m_pmyDevices->GetDeviceType(idx) == E_DT_TEMPERATURE_DS ||
							m_pmyDevices->GetDeviceType(idx) == E_DT_TEMPERATURE_K1 )
					{	// temperature sensor
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + TEMPERATURE_CHECK_PERIOD <= TimeNowMS() )
						{	// only check temperature devices every 5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleTemperatureDevice( myDB, ctx, idx, bAllDead );
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_VOLTAGE )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + VOLTAGE_CHECK_PERIOD <= TimeNowMS() )
						{	// only check voltage devices every 5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleVoltageDevice( myDB, ctx, idx, bAllDead );
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_LEVEL_HDL )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + LEVEL_CHECK_PERIOD <= TimeNowMS() )
						{	// only check level devices every 5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleHdlLevelDevice( myDB, ctx, idx, bAllDead );
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_ROTARY_ENC_12BIT )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + ROTENC_CHECK_PERIOD <= TimeNowMS() )
						{	// only check rotary encoder devices every 1 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							// to avoid "response not from requested slave" error
							//usleep( 20000 );
							HandleRotaryEncoderDevice( myDB, ctx, idx, bAllDead );
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_VIPF_MON )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + VIPF_CHECK_PERIOD <= TimeNowMS() )
						{	// only check VIPF devices every 0.5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleVIPFDevice( myDB, ctx, idx, bAllDead );

							if ( !m_pmyDevices->GetAlwaysPoweredOn(idx) )
								bAllDead = false;
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_HDHK_CURRENT )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + HDHK_CHECK_PERIOD <= TimeNowMS() )
						{	// check HDHK devices every 0.5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleHDHKDevice( myDB, ctx, idx, bAllDead );

							if ( !m_pmyDevices->GetAlwaysPoweredOn(idx) )
								bAllDead = false;
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_VSD_NFLIXEN )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + VSD_CHECK_PERIOD <= TimeNowMS() )
						{	// only check VSD devices every 0.5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleVSDNFlixenDevice( myDB, ctx, idx, bAllDead );

							if ( !m_pmyDevices->GetAlwaysPoweredOn(idx) )
								bAllDead = false;
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_VSD_PWRELECT )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + VSD_CHECK_PERIOD <= TimeNowMS() )
						{	// only check VSD devices every 0.5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleVSDPwrElectDevice( myDB, ctx, idx, bAllDead );

							if ( !m_pmyDevices->GetAlwaysPoweredOn(idx) )
								bAllDead = false;
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_SHT40_TH )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + SHT_CHECK_PERIOD <= TimeNowMS() )
						{	// only check SHT devices every 1 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandleSHTDevice( myDB, ctx, idx, bAllDead );

							if ( !m_pmyDevices->GetAlwaysPoweredOn(idx) )
								bAllDead = false;
						}
					}
					else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_PT113_LCT )
					{
						if ( m_pmyDevices->GetLastCheckedTimeMS(idx) + PT113_LCT_CHECK_PERIOD <= TimeNowMS() )
						{	// only check PT115 devices every 0.5 seconds
							m_pmyDevices->SetLastCheckedTimeMS( idx, TimeNowMS() );

							HandlePT113Device( myDB, ctx, idx, bAllDead );

							if ( !m_pmyDevices->GetAlwaysPoweredOn(idx) )
								bAllDead = false;
						}
					}
					else
					{
						if ( m_pmyDevices->GetNumInputs(idx) > 0 )
						{	// standard switch input
							HandleSwitchDevice( myDB, ctx, idx, bAllDead );
						}
						if ( m_pmyDevices->GetNumOutputs(idx) > 0 )
						{	// output only device
							// sleep if this device has inputs and outputs otherwise we get a timeout in modbus_read_bits()
							if ( m_pmyDevices->GetNumInputs(idx) > 0 )
							{
								usleep( 10000 );
							}

							HandleOutputDevice( myDB, ctx, idx, bAllDead );

							CheckForTimerOffTime( myDB, idx );
						}
					}

					pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );

					if ( m_pmyDevices->IsSharedWithNext(idx) )
					{
						usleep( 20000 );	// was 30000
					}
				}
			}	// end of for

			if ( bAllDead )
			{
				if ( tAllDeadStart == 0 )
				{
					tAllDeadStart = time(NULL);
				}
				else if ( tAllDeadStart != 0 && tAllDeadStart + iAllDeadCheckPeriod < time(NULL) )
				{	// give up after 10 seconds
					LogMessage( E_MSG_ERROR, "%s: All devices are dead, '%s' com port issue", m_szMyComPort, CThread::GetThreadType(m_eThreadType) );

					iAllDeadCheckPeriod = 60;
					tAllDeadStart = 0;
					*m_pbAllDevicesDead = true;

					// TODO: we may need to reboot the pi if we keep failing
				}
			}
			else
			{
				tAllDeadStart = 0;
			}

			usleep( 5000 );	// 5 milli sec
		}
	}	// end of while

	if ( IsWebsocketThread() )
	{
		LogMessage( E_MSG_INFO, "websocket lws destroy");
		websocket_destroy();
	}

	if ( IsTcpipThread() )
	{
		SSL_CTX_free(m_sslServerCtx);
		m_sslServerCtx = NULL;
	}

	SSL_CTX_free(m_sslClientCtx);
	m_sslClientCtx = NULL;

	myDB.Disconnect();


	LogMessage( E_MSG_INFO, "Thread terminating: type %s", CThread::GetThreadType(m_eThreadType) );

	*m_pbThreadRunning = false;
}

// ultrasonic level device
void CThread::HandleLevelDevice( CMysql& myDB, const int idx, const bool bSendPrompt, bool& bAllDead )
{
	// Gap=xxxmm\n
	int iLen;
	int n;
	int iErr;
	int bytes;

	char szByte[2];

	if ( bSendPrompt )
	{
		if ( (bytes = write( m_pmyDevices->GetComHandle(idx), "x", 1 )) != 1 )
		{	// cannot write to the com port
			iErr = errno;
			bAllDead = true;
			LogMessage( E_MSG_ERROR, "Only wrote %d of %d bytes to K02 com port", bytes, 1 );

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
			{	// device has just failed
				LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
			}
			if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
			{
				LogMessage( E_MSG_WARN, "'%s' (0x%x->%d) %d failed: errno %s", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx,
						m_pmyDevices->GetNumOutputs(idx), strerror(iErr) );
			}
		}
		else
		{
			bAllDead = false;
			m_iLevelMessage += 1;
			if ( m_iLevelMessage > 3 )
			{	// no reply received !
				LogMessage( E_MSG_WARN, "No K02 message received for too long" );
				bAllDead = true;

				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				}
				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "'%s' (0x%x->%d) %d failed: errno %s", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx,
							m_pmyDevices->GetNumOutputs(idx), "timeout" );
				}
			}
		}

	}

	while ( (n = read( m_pmyDevices->GetComHandle(idx), szByte, 1 )) > 0 )
	{	// read 1 byte at a time
		bAllDead = false;
		if ( isalnum(szByte[0]) || ispunct(szByte[0]) )
		{
			iLen = strlen(m_szComBuffer);
			m_szComBuffer[iLen] = szByte[0];
			if ( iLen+1 < (int)sizeof(m_szComBuffer) )
			{
				m_szComBuffer[iLen+1] = '\0';
			}
			else
			{
				LogMessage( E_MSG_ERROR, "ComBuf too small !" );
				m_szComBuffer[0] = '\0';
			}
		}
		else if ( szByte[0] == '\n' && strlen(m_szComBuffer) < 6 )
		{	// invalid message
			m_szComBuffer[0] = '\0';
		}
		else if ( szByte[0] == '\n' )
		{	// end of message
			if ( m_iLevelMessage > 0 )
			{
				m_iLevelMessage -= 1;
			}

			// success
			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
			}

			bool bLogit = false;
			int iChannel = 0;
			int iVal = 0;
			int iDiff = 5;
			double dVal = 0;
			char* cptr;

			cptr = strchr( m_szComBuffer, '=' );
			if ( cptr != NULL )
			{
				iVal = atoi(cptr+1);
			}
			//LogMessage( E_MSG_INFO, "Msg [%s]", m_szComBuffer );
			m_szComBuffer[0] = '\0';


			m_pmyDevices->GetNewData(idx)[iChannel] = (uint16_t)iVal;
			dVal = m_pmyDevices->CalcLevel(idx,iChannel,true);


			bLogit = false;
			if ( abs(m_pmyDevices->GetNewData(idx,iChannel) - m_pmyDevices->GetLastLogData(idx,iChannel)) >= iDiff ||
					m_pmyDevices->GetLastRecorded(idx,iChannel) + MAX_EVENT_PERIOD < time(NULL) )
			{	// 5 mm change
				bLogit = true;

				// save 10x the value so we have one decimal place for graphing
				myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), iChannel, E_ET_LEVEL, dVal, "" );

				m_pmyDevices->GetLastRecorded(idx,iChannel) = time(NULL);
				m_pmyDevices->GetLastLogData(idx,iChannel) = m_pmyDevices->GetNewData(idx,iChannel);
			}

			if ( bLogit )
				LogMessage( E_MSG_INFO, "Level %d '%s' %d %.1f%%", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
						dVal );
			else
				LogMessage( E_MSG_DEBUG, "Level %d '%s' %d %.1f%%", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
						dVal );

			if ( m_pmyDevices->GetInChannelType(idx,iChannel) == E_IO_LEVEL_HIGH || m_pmyDevices->GetInChannelType(idx,iChannel) == E_IO_LEVEL_HIGHLOW )
			{
				if ( m_pmyDevices->GetNewData(idx,iChannel) > m_pmyDevices->GetLastData(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,true) >= m_pmyDevices->GetMonitorValueHi(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,false) < m_pmyDevices->GetMonitorValueHi(idx,iChannel) )
				{	// level increasing and high voltage trigger reached
					if ( !m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
					{	// trigger level reached
						LogMessage( E_MSG_INFO, "Channel %d '%s' High Level %.2f %% on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
							m_pmyDevices->GetMonitorValueHi(idx,iChannel), m_pmyDevices->GetAddress(idx) );

						m_pmyDevices->GetAlarmTriggered(idx,iChannel) = true;

						ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, true, E_ET_LEVEL );
					}
					else
					{
						LogMessage( E_MSG_INFO, "Channel %d '%s' High Level %.2f %% on device %d already reached", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
															m_pmyDevices->GetMonitorValueHi(idx,iChannel), m_pmyDevices->GetAddress(idx) );
					}
				}
				else if ( m_pmyDevices->GetNewData(idx,iChannel) < m_pmyDevices->GetLastData(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,true) <= m_pmyDevices->GetMonitorValueHi(idx,iChannel) - m_pmyDevices->GetHysteresis(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,false) > m_pmyDevices->GetMonitorValueHi(idx,iChannel) - m_pmyDevices->GetHysteresis(idx,iChannel) &&
						m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
				{	// level decreasing and hysteresis reached
					LogMessage( E_MSG_INFO, "Channel %d '%s' High Level Hysteresis %.2f %% on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
							m_pmyDevices->GetMonitorValueHi(idx,iChannel) - m_pmyDevices->GetHysteresis(idx,iChannel), m_pmyDevices->GetAddress(idx) );

					m_pmyDevices->GetAlarmTriggered(idx,iChannel) = false;

					ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, false, E_ET_LEVEL );
				}
			}

			if ( m_pmyDevices->GetInChannelType(idx,iChannel) == E_IO_LEVEL_LOW || m_pmyDevices->GetInChannelType(idx,iChannel) == E_IO_LEVEL_HIGHLOW )
			{
				if ( m_pmyDevices->GetNewData(idx,iChannel) < m_pmyDevices->GetLastData(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,true) <= m_pmyDevices->GetMonitorValueLo(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,false) > m_pmyDevices->GetMonitorValueLo(idx,iChannel) )
				{	// level decreasing and low voltage trigger reached
					if ( !m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
					{	// low level trigger reached
						LogMessage( E_MSG_INFO, "Channel %d '%s' Low Level %.2f %% on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
							m_pmyDevices->GetMonitorValueLo(idx,iChannel), m_pmyDevices->GetAddress(idx) );

						m_pmyDevices->GetAlarmTriggered(idx,iChannel) = true;

						ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, true, E_ET_LEVEL );
					}
					else
					{
						LogMessage( E_MSG_INFO, "Channel %d '%s' Low Level %.2f %% on device %d already reached", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
															m_pmyDevices->GetMonitorValueLo(idx,iChannel), m_pmyDevices->GetAddress(idx) );
					}
				}
				else if ( m_pmyDevices->GetNewData(idx,iChannel) > m_pmyDevices->GetLastData(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,true) >= m_pmyDevices->GetMonitorValueLo(idx,iChannel) + m_pmyDevices->GetHysteresis(idx,iChannel) &&
						m_pmyDevices->CalcLevel(idx,iChannel,false) < m_pmyDevices->GetMonitorValueLo(idx,iChannel) + m_pmyDevices->GetHysteresis(idx,iChannel) &&
						m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
				{	// voltage increasing and hysteresis reached
					LogMessage( E_MSG_INFO, "Channel %d '%s' Low Level Hysteresis %.2f %% on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
							m_pmyDevices->GetMonitorValueLo(idx,iChannel) + m_pmyDevices->GetHysteresis(idx,iChannel), m_pmyDevices->GetAddress(idx) );

					m_pmyDevices->GetAlarmTriggered(idx,iChannel) = false;

					ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, false, E_ET_LEVEL );
				}
			}
		}
		else
		{	// ignore other characters
		}
	}
}

// card reader and PIN pad
// always assumed to be alive
void CThread::HandleCardReaderDevice( CMysql& myDB, const int idx, bool& bAllDead )
{
	bool bCardError = false;
	//int iLen;
	int n;
	int iCount = 0;
	char* cptr;
	//char szPin[7] = "";
	char szPinDB[7] = "";
	unsigned char szByte[2] = "";
	unsigned char szCard[17] = "";


	// success
	bAllDead = false;
	if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
	{
		LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
	}

	if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
	{
		m_pmyDevices->UpdateDeviceStatus( myDB, idx );
	}

	while ( (n = read( m_pmyDevices->GetComHandle(idx), szByte, 1 )) > 0 )
	{	// read 1 byte at a time
		szByte[0] &= 0xff;
		LogMessage( E_MSG_INFO, "Data: 0x%02x 0b%d%d%d%d%d%d%d%d", szByte[0], (szByte[0]&0x80 ? 1:0), (szByte[0]&0x40 ? 1:0), (szByte[0]&0x20 ? 1:0), (szByte[0]&0x10 ? 1:0),
				(szByte[0]&0x08 ? 1:0), (szByte[0]&0x04 ? 1:0), (szByte[0]&0x02 ? 1:0), (szByte[0]&0x01 ? 1:0) );

		// pretend its a card read
		szCard[iCount] = szByte[0];
		szCard[iCount+1] = '\0';
		iCount += 1;
		if ( iCount >= (int)sizeof(szCard) )
		{	// error
			bCardError = true;
			LogMessage( E_MSG_ERROR, "Too many card digits, %d", iCount );
			break;
		}

		// sleep to get all the bytes
		usleep(10000);
	}

	/*if ( iCount == 1 )
	{	// pin digit - always ends with #
		if ( szByte[0] == 0x0a )
			szByte[0] = '*';
		else if ( szByte[0] == 0x0b )
			szByte[0] = '#';
		else
			szByte[0] += '0';

		// add to the com buffer
		iLen = strlen(m_szComBuffer);
		m_szComBuffer[iLen] = szByte[0];
		if ( iLen+1 < (int)sizeof(m_szComBuffer) )
		{
			m_szComBuffer[iLen+1] = '\0';
		}
		else
		{
			LogMessage( E_MSG_ERROR, "ComBuf too small !" );
			m_szComBuffer[0] = '\0';
		}

		LogMessage( E_MSG_INFO, "CardReader: [****] pin hidden" );
	}
	else */
	if ( iCount >= 3 && !bCardError )
	{	// card number 3 or more bytes
		m_szComBuffer[0] = 0;

		unsigned long uVal = 0;
		if ( iCount == 16 ) 
		{
			int iShift = 2;
			for ( int i = 6; i < 9; i++ )
			{
				uVal += (szCard[i] << (8 * iShift));
				iShift -= 1;
			}
		}
		else
		{
			int iShift = iCount-1;
			for ( int i = 0; i < iCount; i++ )
			{
				uVal += (szCard[i] << (8 * iShift));
				iShift -= 1;
			}
		}
		m_tCardReaderStart = time(NULL);
		snprintf( m_szComBuffer, sizeof(m_szComBuffer), "%lu;", uVal );

		LogMessage( E_MSG_INFO, "CardReader: '%s'", m_szComBuffer );
	}

	if ( m_szComBuffer[strlen(m_szComBuffer)-1] == ';' )
	{	// card only
		if ( (cptr = strchr( m_szComBuffer, ';' )) != NULL )
		{	// card + pin
			bool bEnabled = false;
			int iPinFailCount = 0;
			char szCardNumber[11];

			LogMessage( E_MSG_INFO, "Process card '%s'", m_szComBuffer );
			*cptr = '\0';
			strncpy( szCardNumber, m_szComBuffer, sizeof(szCardNumber)-1 );
			szCardNumber[sizeof(szCardNumber)-1] = '\0';

			// get linked devices for card reader (esp display and weigh bridge)
			int iInDeviceNo = m_pmyDevices->GetDeviceNo(idx);
			int iInChannel = 0;
			int iEspIdx = -1;
			if ( (iEspIdx = m_pmyIOLinks->FindLinkedDevice( iInDeviceNo, iInChannel, E_DT_ESP_DISPLAY, m_pmyDevices )) >= 0 )
			{	
				LogMessage( E_MSG_INFO, "Card reader linked device: %s", m_pmyDevices->GetDeviceName(iEspIdx));
			}

			if ( m_pmyDevices->SelectCardNumber( myDB, szCardNumber, szPinDB, sizeof(szPinDB), bEnabled, iPinFailCount ) )
			{
				if ( !bEnabled )
				{	// card is disabled
					bCardError = true;
					LogMessage( E_MSG_WARN, "Card '%s' is disabled", szCardNumber );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDREADER, 2, "Access invalid: CardDisabled '%s'", szCardNumber );

					switch ( m_pmyDevices->GetInChannelType(idx, 0) )
					{
					default:
						LogMessage( E_MSG_WARN, "Unhandled card reader io type" );
						break;
					case E_IO_READER_WEIGHBRIDGE:
						LogMessage( E_MSG_INFO, "Got disabled card for weigh bridge" );
			
						if ( iEspIdx >= 0 )
						{
							char szEspResponseMsg[ESP_MSG_SIZE];
							snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XD%s", szCardNumber );
							SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
						}
						break;
					}
				}
				else
				{	
					LogMessage( E_MSG_INFO, "Card found" );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDREADER, 0, "Access valid: Card '%s'", szCardNumber );

					char szThisRego[7] = "";
					char szTruckRego[7] = "";
					char szTruckTare[11] = "";
					if ( m_pmyDevices->SelectCardNumberRego( myDB, szCardNumber, szTruckRego, sizeof(szTruckRego), szTruckTare, sizeof(szTruckTare) ) )
					{	// rego details found
						LogMessage( E_MSG_INFO, "Card '%s': Truck rego '%s' %s Kg", szCardNumber, szTruckRego, szTruckTare );

						int iWbIdx = -1;
						for ( int i = 0; i < MAX_DEVICES; i++ )
						{
							if ( m_pmyDevices->GetDeviceType(i) == E_DT_PT113_LCT || m_pmyDevices->GetDeviceType(i) == E_DT_SYSTEC_IT1)
							{	// found the weigh bridge
								iWbIdx = i;
								break;
							}
						}

						if ( iWbIdx >= 0 )
						{
							// are we weighting truck or trailer ?
							bool bTruck = true;
							time_t tLastCardSwipe = 0;
							double dLastWeight = 0;
							m_pmyDevices->GetLastCardSwipeTimestamp( myDB, szCardNumber, tLastCardSwipe, dLastWeight );
							if ( time(NULL) - tLastCardSwipe < 60 )
							{	// 2 minutes
								// TODO: move this into config
								bTruck = false;
							}
							else 
							{
								snprintf( szThisRego, sizeof(szThisRego), "%s", szTruckRego );
							}
							LogMessage( E_MSG_INFO, "Last swipe for '%s' was %d minutes ago, weight %.1f kg", szCardNumber, (time(NULL)-tLastCardSwipe)/60, dLastWeight );


							bool bEmpty = false;
							double dTruckWeight = 0.0;
							double dBridgeTare = 0.0;
							double dDBTruckTare = (double)atol( szTruckTare );

							// read the bridge tare weight
							dBridgeTare = m_pmyDevices->GetCalcFactor(iWbIdx,0);

							// read the current weight
							if ( m_pmyDevices->GetDeviceType(iWbIdx) == E_DT_PT113_LCT )
								dTruckWeight = m_pmyDevices->CalcPT113Weight(iWbIdx, 0, true );
							else
								dTruckWeight = m_pmyDevices->CalcSystecIT1Weight(iWbIdx, 0, true );
							//dTruckWeight -= dBridgeTare;
							LogMessage( E_MSG_INFO, "Bridge Tare %.1f, Truck Weight %.1f", dBridgeTare, dTruckWeight );
							if ( dTruckWeight < 0.0 )
							{
								dTruckWeight = 0.0;
							}
							if ( fabs(dTruckWeight) < dBridgeTare*0.01 )
							{	// 1% of bridge tare
								LogMessage( E_MSG_INFO, "Nothing on the weigh bridge: Bridge %.1f, %.1f", dBridgeTare, dTruckWeight );
								bEmpty = true;
							}

							if ( bTruck )
							{
								LogMessage( E_MSG_INFO, "Truck Tare for '%s' %.1f / %.1f / %.1f / %.1f", szThisRego, dTruckWeight, dDBTruckTare, dTruckWeight+dBridgeTare, dBridgeTare );

								if ( dDBTruckTare > 0 && fabs(dTruckWeight-dDBTruckTare) < dDBTruckTare * 0.05 )
								{	// empty truck
									LogMessage( E_MSG_INFO, "Truck '%s' is empty: Tare %.1f, Weight %.1f", szCardNumber, dDBTruckTare, dTruckWeight );
									bEmpty = true;
								}
							}

							if ( iEspIdx >= 0 )
							{
								char szEspResponseMsg[ESP_MSG_SIZE];
								snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XF%s", szCardNumber );
								SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );

								if ( bEmpty )
								{
									snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XR%s", szTruckRego );
									SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );

									if ( m_pmyDevices->GetOffset(iWbIdx,0) < 1000)
										snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XE%06.1f", dTruckWeight );
									else
										snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XE%06d", (int)dTruckWeight );
									SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
								}
								else if ( bTruck )
								{
									snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XR%s", szTruckRego );
									SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );

									snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XT%s", szTruckTare );
									SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );

									if ( m_pmyDevices->GetOffset(iWbIdx,0) < 1000)
										snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XW%06.1f", dTruckWeight );
									else
										snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XW%06d", (int)dTruckWeight );

									SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );

									// record this weight event
									myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDWEIGHT, (long)dTruckWeight, "%s", szCardNumber );

									// save the new tare value
									// do not save the new tare
									//m_pmyDevices->UpdateTruckTare( myDB, szCardNumber, dTruckWeight );

									time_t timenow = time(NULL);
									struct tm* tmptr;
									char szDir[256];
									char szFile[512];
									char szBuf[256];
									char szBillingName[51] = "";
									char szBillingAddr1[51] = "";
									char szBillingAddr2[51] = "";
									char szBillingAddr3[51] = "";
									char szBillingEmail[51] = "";
									FILE* pFile = NULL;
									
									tmptr = localtime( &timenow );
									snprintf( szDir, sizeof(szDir), "/home/nimrod/invoices/%d%02d%02d_%02d%02d%02d_%s", tmptr->tm_year+1900, tmptr->tm_mon+1, tmptr->tm_mday,
										tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec, szCardNumber );
									mkdir( "/home/nimrod/invoices", 0775 );
									
									if ( mkdir( szDir, 0775 ) == 0 )
									{
										LogMessage( E_MSG_INFO, "Created dir '%s'", szDir );

										// take camera snapshots
										std::string sAttachments;
										ReadCameraRecords( myDB, m_CameraList );
										for ( int i = 0; i < m_CameraList.GetNumCameras(); i++ )
										{
											GetCameraSnapshots( myDB, m_CameraList, szDir, sAttachments );

											m_CameraList.NextSnapshotIdx();
										}
								
										// save invoice details
										if ( !m_pmyDevices->SelectCardNumberBilling( myDB, szCardNumber, szBillingName, sizeof(szBillingName), szBillingAddr1, sizeof(szBillingAddr1),
											szBillingAddr2, sizeof(szBillingAddr2), szBillingAddr3, sizeof(szBillingAddr3), szBillingEmail, sizeof(szBillingEmail) ) )
										{
											LogMessage( E_MSG_ERROR, "Failed to read billing details for card %s", szCardNumber );
										}

										snprintf( szFile, sizeof(szFile), "%s/invoice.txt", szDir );
										pFile = fopen( szFile, "wt" );
										if ( pFile != NULL )
										{
											snprintf( szBuf, sizeof(szBuf), "Weigh Bridge Invoice %02d/%02d/%d %02d:%02d:%02d\n\n", tmptr->tm_mday, tmptr->tm_mon+1, tmptr->tm_year+1900,
												tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec );
											fputs( szBuf, pFile );
											snprintf( szBuf, sizeof(szBuf), "Card No: %s\nVehicle Rego: %s\n", szCardNumber, szTruckRego );
											fputs( szBuf, pFile );
											if ( m_pmyDevices->GetOffset(iWbIdx,0) < 1000)
												snprintf( szBuf, sizeof(szBuf), "Weight: %.1f KG\n", dTruckWeight );
											else
												snprintf( szBuf, sizeof(szBuf), "Weight: %d KG\n", (int)dTruckWeight );
											fputs( szBuf, pFile );

											snprintf( szBuf, sizeof(szBuf), "Billing Name: %s\n", szBillingName );
											fputs( szBuf, pFile );
											snprintf( szBuf, sizeof(szBuf), "Billing Address 1: %s\n", szBillingAddr1 );
											fputs( szBuf, pFile );
											snprintf( szBuf, sizeof(szBuf), "Billing Address 2: %s\n", szBillingAddr2 );
											fputs( szBuf, pFile );
											snprintf( szBuf, sizeof(szBuf), "Billing Address 3: %s\n", szBillingAddr3 );
											fputs( szBuf, pFile );
											snprintf( szBuf, sizeof(szBuf), "Billing Email: %s\n", szBillingEmail );
											fputs( szBuf, pFile );

											fputs( "\n", pFile );

											fclose( pFile );
										}
										else
										{
											LogMessage( E_MSG_ERROR, "Failed to open %s for writing, errno %d", szFile, errno );
										}

										// send email
										std::string sEmails;
										m_pmyDevices->SelectAccountEmails( myDB, sEmails );
										if ( sEmails.length() > 0 )
										{
											std::string sCmd = "mail -s \"Weigh Bridge Invoice\" ";
											sCmd += sAttachments;
											sCmd += " ";
											sCmd += sEmails;
											sCmd += " <";
											sCmd += szFile;

											//LogMessage( E_MSG_INFO, "CMD: %s", sCmd.c_str() );
											int rc = system( sCmd.c_str() );
											if ( rc != 0 )
											{
												LogMessage( E_MSG_WARN, "system returned %d", rc );
											}
										}
										else
										{
											LogMessage( E_MSG_WARN, "No account emails exist" );
										}
									}
									else
									{
										LogMessage( E_MSG_ERROR, "Failed to create dir '%s', errno %d", szDir, errno );
									}
								}
								else 
								{	// error
									bCardError = true;
									LogMessage( E_MSG_WARN, "Card '%s' for %s already swiped", szCardNumber, szTruckRego);
									myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDREADER, 4, "Access invalid: AlreadySwiped '%s'", szCardNumber );

									snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XS%s", szCardNumber );
									SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
								}
							}
						}
						else
						{
							LogMessage( E_MSG_WARN, "PT113 LCT or Systec IT1 not found in devices list" );
						}
					}
					else
					{	// error reading rego details
						bCardError = true;
						LogMessage( E_MSG_WARN, "Card '%s' rego not found in the database", m_szComBuffer );
						myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDREADER, 4, "Access invalid: UnknownCardRego '%s'", szCardNumber );

						if ( iEspIdx >= 0 )
						{
							char szEspResponseMsg[ESP_MSG_SIZE];
							snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XN%s", szCardNumber );
							SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
						}
					}

				}
				/*else
				{	// bad pin
					bCardError = true;
					LogMessage( E_MSG_WARN, "Card PIN not matched '%s'", szPin );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDREADER, 1, "Access invalid: BadPin '%s'", m_szComBuffer );

					// disable the card after x pin failures
					iPinFailCount += 1;
					m_pmyDevices->UpdatePinFailCount( myDB, m_szComBuffer, iPinFailCount );

					if ( iPinFailCount >= MAX_PIN_FAILURES )
					{
						LogMessage( E_MSG_WARN, "Card '%s' disabled, too many PIN failures", m_szComBuffer );
					}
				}*/
			}
			else
			{	// card not found
				bCardError = true;
				LogMessage( E_MSG_WARN, "Card '%s' not found in the database", m_szComBuffer );
				myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_CARDREADER, 3, "Access invalid: UnknownCard '%s'", m_szComBuffer );

				if ( iEspIdx >= 0 )
				{
					char szEspResponseMsg[ESP_MSG_SIZE];
					snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "XN%s", szCardNumber );
					SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
				}
			}
		}
	}


	if ( bCardError )
	{	// send beep to reader
		// TODO

		// red led on reader, long beep on reader


	}
	m_szComBuffer[0] = '\0';
}

int giLastWeight = -1;
void CThread::HandleSystecIT1Device( CMysql& myDB, const int idx, bool& bAllDead )
{
	int n;
	int iCount = 0;
	//char* cptr;
	unsigned char szByte[2] = "";
	unsigned char szData[18] = "";


	// success
	bAllDead = false;
	if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
	{
		LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
	}

	if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
	{
		m_pmyDevices->UpdateDeviceStatus( myDB, idx );
	}

	while ( (n = read( m_pmyDevices->GetComHandle(idx), szByte, 1 )) > 0 )
	{	// read 1 byte at a time
		szByte[0] &= 0xff;
		//LogMessage( E_MSG_INFO, "Data: 0x%02x", szByte[0] );

		// pretend its a card read
		if ( szByte[0] == 0x0d )
		{

		} 
		else if ( szByte[0] == 0x0a )
		{	// end of message
			break;
		}
		else 
		{
			szData[iCount] = szByte[0];
			szData[iCount+1] = '\0';
			iCount += 1;
			if ( iCount >= (int)sizeof(szData) )
			{	// error
				LogMessage( E_MSG_ERROR, "Too many bytes, %d", iCount );
				break;
			}
		}

		// sleep to get all the bytes
		usleep(10000);
	}

	if ( iCount >= 15 )
	{	// 3 or more bytes
		m_szComBuffer[0] = 0;

//		unsigned long uVal = 0;
//		int iShift = iCount-1;
//		for ( int i = 0; i < iCount; i++ )
//		{
//			uVal += (szData[i] << (8 * iShift));
//			iShift -= 1;
//		}
		snprintf( m_szComBuffer, sizeof(m_szComBuffer), "%s", szData );

		if ( atoi((const char*)&szData[4]) != giLastWeight )
		{
			LogMessage( E_MSG_INFO, "IT1 data: '%s'", m_szComBuffer );
			giLastWeight = atoi((const char*)&szData[4]);

			m_pmyDevices->GetLastData(idx,0) = m_pmyDevices->GetNewData(idx)[0];
			m_pmyDevices->GetNewData(idx)[0] = giLastWeight;
		}
	}

	if ( strlen(m_szComBuffer) >= 15 )
	{	//
		// always use channel 0
		int iChannel;
		for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
		{
			// 10kg change
			// unloaded bridge weight / 10 ??
			double dDiff = 10;	// m_pmyDevices->GetOffset(idx,iChannel) * m_pmyDevices->GetResolution(idx,iChannel) / 100;

			E_EVENT_TYPE eEventType = E_ET_WEIGHT;
			E_IO_TYPE eIOTypeL = E_IO_WEIGHT_LOW;
			E_IO_TYPE eIOTypeH = E_IO_WEIGHT_HIGH;
			E_IO_TYPE eIOTypeHL = E_IO_WEIGHT_HIGHLOW;
			E_IO_TYPE eIOTypeMon = E_IO_WEIGHT_MONITOR;
			char szUnits[10] = "Kg";
			char szDesc[20] = "Weight";
			char szName[50] = "Weight";
			double dValOld = m_pmyDevices->CalcSystecIT1Weight(idx,iChannel,false);
			double dValNew = m_pmyDevices->CalcSystecIT1Weight(idx,iChannel,true);

			//LogMessage( E_MSG_INFO, "Old %u %u %.1f", m_pmyDevices->GetLastData(idx,iChannel), m_pmyDevices->GetLastData(idx,iChannel+1), dValOld);

			HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );

			if ( fabs(dValNew - dValOld) >= dDiff )
			{	// find the display
				LogMessage(E_MSG_INFO, "WW %.1f %.1f %.1f", dValNew, dValOld, dDiff );
				for ( int i = 0; i < MAX_DEVICES; i++ )
				{
					if ( m_pmyDevices->GetDeviceType(i) == E_DT_CARD_READER )
					{
						int iInDeviceNo = m_pmyDevices->GetDeviceNo(i);
						int iInChannel = 0;
						int iEspIdx = -1;
						if ( (iEspIdx = m_pmyIOLinks->FindLinkedDevice( iInDeviceNo, iInChannel, E_DT_ESP_DISPLAY, m_pmyDevices )) >= 0 )
						{	
							char szEspResponseMsg[ESP_MSG_SIZE];
							if ( m_pmyDevices->GetOffset(idx,iChannel) < 1000)
								snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "WW%06.1f", dValNew );
							else
								snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "WW%06d", (int)round(dValNew) );
							SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
						}
						break;
					}
				}
			}
		}	// end for loop

	}

	m_szComBuffer[0] = '\0';
}

void CThread::CheckForTimerOffTime( CMysql& myDB, const int idx )
{
	bool bError;
	bool bIsEsp;
	int j;

	bIsEsp = m_pmyDevices->IsEspDevice(idx);

	// check for timer off time
	for ( j = 0; j < MAX_IO_PORTS; j++ )
	{
		switch ( m_pmyDevices->GetOutChannelType(idx,j) )
		{
		default:
			break;

		case E_IO_OUTPUT:
			if ( m_pmyDevices->IsOffTime(idx,j) )
			{	// turn off this output
				LogMessage( E_MSG_INFO, "Time to turn '%s' output off, device %d channel %d", m_pmyDevices->GetOutIOName(idx,j), idx, j+1 );
				myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), j, E_ET_TIMER, 0, "Time to turn '%s' output off, device %d channel %d", m_pmyDevices->GetOutIOName(idx,j), idx, j+1 );

				bError = false;
				if ( !bIsEsp )
				{
					if ( !m_pmyDevices->WriteOutputBit( idx, j, false ) )
						bError = true;
				}

				if ( !bError )
				{	// success
					m_pmyDevices->LogOnTime( idx, j );
					m_pmyDevices->SetOutOnStartTime( idx, j, 0 );

					m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, idx, j );
				}
				else
				{
					LogMessage( E_MSG_ERROR, "Turn off timer: WriteOutputBit() failed for device %d, channel %d: %s", idx, j+1, modbus_strerror(errno) );
				}
			}
			break;
		}
	}
}

const char* CThread::GetTcpipMsgType( const enum E_MESSAGE_TYPE eMT )
{
	static char szDummyMT[50];

	switch ( eMT )
	{
	default:
		snprintf( szDummyMT, sizeof(szDummyMT), "E_MT_UNKNOWN(%d)", eMT );
		return szDummyMT;
		break;
	case E_MT_HELLO:
		return "E_MT_HELLO";
		break;
	case E_MT_ACK:
		return "E_MT_ACK";
		break;
	case E_MT_TOTAL_DEVICES:
		return "E_MT_TOTAL_DEVICES";
		break;
	case E_MT_DEVICE_INFO:
		return "E_MT_DEVICE_INFO";
		break;
	case E_MT_CHANGE_OUTPUT:
		return "E_MT_CHANGE_OUTPUT";
		break;
	case E_MT_ESP_MSG:
		return "E_MT_ESP_MSG";
		break;
	}

	return "E_MT_UNKNOWN";
}

const char* CThread::GetThreadType( const enum E_THREAD_TYPE eTT )
{
	static char szDummyTT[50];

	switch ( eTT )
	{
	default:
		snprintf( szDummyTT, sizeof(szDummyTT), "E_TT_UNKNOWN(%d)", eTT );
		return szDummyTT;
		break;
	case E_TT_TCPIP:
		return "E_TT_TCPIP";
		break;
	case E_TT_TIMER:
		return "E_TT_TIMER";
		break;
	case E_TT_WEBSOCKET:
		return "E_TT_WEBSOCKET";
		break;
	case E_TT_COMPORT:
		return "E_TT_COMPORT";
		break;
	}

	return "E_TT_UNKNOWN";
}

const bool CThread::IsComPortThread()
{
	if ( m_eThreadType == E_TT_COMPORT )
		return true;
	else
		return false;
}

const bool CThread::IsTimerThread()
{
	if ( m_eThreadType == E_TT_TIMER )
		return true;
	else
		return false;
}

const bool CThread::IsTcpipThread()
{
	if ( m_eThreadType == E_TT_TCPIP )
		return true;
	else
		return false;
}

const bool CThread::IsWebsocketThread()
{
	if ( m_eThreadType == E_TT_WEBSOCKET )
		return true;
	else
		return false;
}

const bool CThread::IsMyComPort( const char* szPort )
{
	if ( m_szMyComPort[0] == '\0' )
		return false;
	else if ( strcmp( szPort, m_szMyComPort ) == 0 )
		return true;
	else
		return false;
}

const bool CThread::IsVirtualComPort( const char* szPort )
{
	if ( strcasecmp( szPort, "Virtual" ) == 0 )
		return true;

	return false;
}
void CThread::CreateListenerSocket()
{
	struct sockaddr_in serv_addr;

	m_Sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( m_Sockfd < 0 )
	{
		LogMessage( E_MSG_ERROR, "Failed to create listener socket, errno %d", errno );
	}
	else
	{
		// set socket as non blocking
		int flags = fcntl( m_Sockfd, F_GETFL, 0 );
		if ( flags < 0 )
		{
			LogMessage( E_MSG_ERROR, "fcntl F_GETFL on listener socket failed, errno %d", errno );
		}
		else
		{
			if ( fcntl( m_Sockfd, F_SETFL, flags | O_NONBLOCK ) < 0 )
			{
				LogMessage( E_MSG_ERROR, "fcntl(O_NONBLOCK) failed on listener socket, errno %d", errno );
			}
			else
			{
				LogMessage( E_MSG_INFO, "fcntl(O_NONBLOCK) set" );
			}

			int enable = 1;
			if ( setsockopt( m_Sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int) ) < 0 )
			{
				LogMessage( E_MSG_ERROR, "setsockopt(SO_REUSEADDR) failed, errno %d", errno );
			}
			else
			{
				LogMessage( E_MSG_INFO, "setsockopt(SO_REUSEADDR) set" );
			}
		}

		// Initialize socket structure
		bzero((char *) &serv_addr, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(NIMROD_PORT);

		// Now bind the host address using bind() call.
		if ( bind( m_Sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 )
		{
			LogMessage( E_MSG_ERROR, "Failed to bind listener socket, errno %d", errno );

			CloseSocket( m_Sockfd, -1 );
		}
		else
		{
			if ( listen( m_Sockfd, 5 ) < 0 )
			{
				LogMessage( E_MSG_ERROR, "listen() failed, errno %d", errno );

				CloseSocket( m_Sockfd, -1 );
			}
			else
			{
				LogMessage( E_MSG_INFO, "Listening for tcpip connections on port %d", NIMROD_PORT );
			}
		}
	}
}

void CThread::CloseSocket( int& fd, const int fdx )
{
	if ( fd != -1 )
	{
		LogMessage( E_MSG_INFO, "CloseSocket()" );

		if ( fdx >= 0  && fdx < MAX_TCPIP_SOCKETS )
		{
			if ( m_xClientSSL[fdx] != NULL )
			{
				SSL_shutdown( m_xClientSSL[fdx] );
				SSL_free(m_xClientSSL[fdx]);        // release ssl connection state
				m_xClientSSL[fdx] = NULL;
			}
		}

		// socket is shutdown in SSL_shutdown call
		if ( fdx < 0 )
		{
			if ( shutdown( fd, SHUT_RDWR ) < 0 )
			{
				LogMessage( E_MSG_ERROR, "Failed to shutdown socket, errno %d", errno );
			}
		}
		close( fd );
		fd = -1;

		if ( fdx >= 0 && fdx < MAX_TCPIP_SOCKETS )
		{
			m_szClientEspName[fdx][0] = '\0';
			m_tClientLastMsg[fdx] = 0;
			bzero( (char*)&m_xClientInAddr[fdx], sizeof(m_xClientInAddr[fdx]) );
			if ( m_iClientCount > 0 )
			{
				m_iClientCount -= 1;
			}
		}
	}
}

size_t CThread::ReadTcpipMsgBytes( SSL* ssl, const int newfd, NIMROD_MSGBUF_TYPE& msgBuf, const bool bBlock )
{
	int rc;
	size_t uMsgSize = sizeof(msgBuf);
	size_t uLen = 0;

	bzero( &msgBuf, sizeof(msgBuf) );

	// check if there are any bytes to read
	rc = recv( newfd, msgBuf.szBuf, sizeof(msgBuf), MSG_PEEK | MSG_DONTWAIT );
	if ( rc <= 0 && !bBlock )
	{
		return 0;
	}
	else
	{
		//LogMessage( E_MSG_INFO, "calling SSL_read(), peek %d", rc );
		rc = SSL_read( ssl, msgBuf.szBuf, sizeof(msgBuf.szBuf) );
		//LogMessage( E_MSG_INFO, "SSL_read() %d", rc );

		uMsgSize = rc;
		uLen = uMsgSize;

		// handle esp msg
		if ( uMsgSize == sizeof(msgBuf.msg.esp.szBuf) && uLen == sizeof(msgBuf.msg.esp.szBuf) )
		{
			unsigned int i;
			char szEsp[sizeof(msgBuf.msg.esp.szBuf)+1];

			uLen = sizeof(msgBuf);
			bzero( szEsp, sizeof(szEsp) );

			// move the data
			for ( i = 0; i < uMsgSize; i++ )
			{
				szEsp[i] = msgBuf.szBuf[i];
			}

			LogMessage( E_MSG_DEBUG, "Got ESP msg '%s'", szEsp );

			bzero( &msgBuf, sizeof(msgBuf) );
			msgBuf.msg.eMsgType = E_MT_ESP_MSG;
			for ( i = 0; i < uMsgSize; i++ )
			{
				msgBuf.msg.esp.szBuf[i] = szEsp[i];
			}

			// check the msg format
			// 012345678901234
			// ESPxxxCLKx
			// ESPxxxTMPn:xx.x
			// ESPxxxCIDyyyyyyyyyy
			if ( szEsp[0] == 'E' && szEsp[1] == 'S' && szEsp[2] == 'P' )
			{	// valid
				strncpy( msgBuf.msg.esp.szEspName, szEsp, sizeof(msgBuf.msg.esp.szEspName)-1 );
				msgBuf.msg.esp.szEspName[sizeof(msgBuf.msg.esp.szEspName)-1] = '\0';

				// save the msg type
				strncpy( msgBuf.msg.esp.szEvent, &szEsp[6], sizeof(msgBuf.msg.esp.szEvent)-1 );
				msgBuf.msg.esp.szEvent[sizeof(msgBuf.msg.esp.szEvent)-1] = '\0';

				if ( strcmp( msgBuf.msg.esp.szEvent, "CLK" ) == 0 )
				{
					msgBuf.msg.esp.iButton = (int)(szEsp[9] - '0');
				}
				else if ( strcmp( msgBuf.msg.esp.szEvent, "TMP" ) == 0 )
				{
					msgBuf.msg.esp.iChannel = (int)(szEsp[9] - '0');
					msgBuf.msg.esp.dTemperature = atof( &szEsp[11] );
				}
				else if ( strcmp( msgBuf.msg.esp.szEvent, "CID" ) == 0 )
				{	// mac address
					strncpy( msgBuf.msg.esp.szChipMac, &szEsp[9], 12 );
					msgBuf.msg.esp.szChipMac[12] = '\0';
				}
			}
			else
			{	// invalid msg data
				uLen = 0;
			}
		}
	}

	return uLen;
}

void CThread::LogSSLError()
{
    int err;
    while ( (err = ERR_get_error()) )
    {
        char *str = ERR_error_string(err, 0);
        if (!str)
            return;
        LogMessage( E_MSG_ERROR, "SSL error: %s", str );
	}
}

void CThread::AcceptTcpipClient()
{
	int i;
	int newfd;
	unsigned int clilen;
	struct sockaddr_in cli_addr;

	if ( m_Sockfd >= 0 )
	{
		clilen = sizeof(cli_addr);

		// socket is non blocking
		newfd = accept( m_Sockfd, (struct sockaddr *)&cli_addr, &clilen );
		if ( newfd < 0 )
		{
			if ( errno != EWOULDBLOCK )
			{
				LogMessage( E_MSG_ERROR, "accept returned errno %d", errno );
			}
		}
		else
		{	// new client connection
			LogMessage( E_MSG_INFO, "new client connection (%d clients)", m_iClientCount+1 );

			// make socket non blocking - the SSL_accept fails if the socket is non blocking
			// set socket as non blocking
/*			int flags = fcntl( newfd, F_GETFL, 0 );
			if ( flags < 0 )
			{
				LogMessage( E_MSG_ERROR, "fcntl F_GETFL on client socket failed, errno %d", errno );
			}
			else
			{
				if ( fcntl( newfd, F_SETFL, flags | O_NONBLOCK ) < 0 )
				{
					LogMessage( E_MSG_ERROR, "fcntl(O_NONBLOCK) failed on client socket, errno %d", errno );
				}
				else
				{
					LogMessage( E_MSG_INFO, "fcntl(O_NONBLOCK) set on client socket" );
				}
			}*/
			LogMessage( E_MSG_INFO, "socket is blocking" );

			for ( i = 0; i < MAX_TCPIP_SOCKETS; i++ )
			{
				if ( m_iClientFd[i] == -1 )
				{	// found an empty slot
					m_iClientCount += 1;
					m_iClientFd[i] = newfd;
					memcpy( &m_xClientInAddr[i], &cli_addr.sin_addr, sizeof(m_xClientInAddr[i]) );
					newfd = -1;

					m_xClientSSL[i] = SSL_new( m_sslServerCtx );      // create new SSL connection state

			    	//SSL_set_max_send_fragment( m_xClientSSL[i], 1024 );

					SSL_set_fd( m_xClientSSL[i], m_iClientFd[i] );    	// attach the socket descriptor

					int rc = SSL_accept(m_xClientSSL[i]);
					if ( rc <= 0 )   // perform the connection
					{	// error
						int ret = 0;
						int rc1 = SSL_get_error( m_xClientSSL[i], ret );
						LogMessage( E_MSG_ERROR, "AcceptTcpipClient(): SSL_accept failed, %d %d, %d %d", rc, ret, rc1, errno );

						LogSSLError();

						CloseSocket( m_iClientFd[i], i );


						// dummy
						//SendTcpipChangeOutputToHost( "nimrod2", 0, 0, 0, 0, 0, 0, 0, E_IO_TOGGLE, 0 );
					}
					else
					{
						LogMessage( E_MSG_INFO, "SSL_accept returned %d", rc );
						LogMessage( E_MSG_INFO, "SSL connection using %s", SSL_get_cipher(m_xClientSSL[i]) );
						ShowCerts(m_xClientSSL[i]);
						m_tClientLastMsg[i] = time(NULL);
					}

					break;
				}
			}

			if ( newfd != -1 )
			{	// error failed to find an empty slot
				LogMessage( E_MSG_ERROR, "No client socket slots available" );

				CloseSocket( newfd, -1 );
			}
		}
	}
}

void CThread::SendEspMessage()
{
	int fdx;
	char szEspResponseMsg[ESP_MSG_SIZE];
	char szEspName[MAX_DEVICE_NAME_LEN+1];
	NIMROD_MSGBUF_TYPE replyBuf;

	while ( m_pmyDevices->GetEspMessageCount() > 0 )
	{
		pthread_mutex_lock( &mutexLock[E_LT_NODEESP] );

		m_pmyDevices->GetEspResponseMsg( szEspName, sizeof(szEspName), szEspResponseMsg, sizeof(szEspResponseMsg) );

		pthread_mutex_unlock( &mutexLock[E_LT_NODEESP] );

		replyBuf.msg.eMsgType = E_MT_ESP_MSG;
		snprintf( replyBuf.msg.esp.szBuf, sizeof(replyBuf.msg.esp.szBuf), "%s", szEspResponseMsg );

		LogMessage( E_MSG_INFO, "Find ESP with name '%s'", szEspName );

		// find the correct client socket
		for ( fdx = 0; fdx < MAX_TCPIP_SOCKETS; fdx++ )
		{
			LogMessage( E_MSG_INFO, "checking %d %d '%s'", fdx, m_iClientFd[fdx], m_szClientEspName[fdx] );
			if ( m_iClientFd[fdx] != -1 && strcmp( szEspName, m_szClientEspName[fdx] ) == 0 )
			{
				int rc;
				size_t uMsgSize = strlen(replyBuf.msg.esp.szBuf );

				// TODO: handle write being interrupted
				rc = SSL_write( m_xClientSSL[fdx], replyBuf.msg.esp.szBuf, uMsgSize );
				if ( rc != (int)uMsgSize )
				{
					LogMessage( E_MSG_ERROR, "Failed to send reply to client, errno %d", errno );

					CloseSocket( m_iClientFd[fdx], fdx );
				}
				else
				{
					char *ip = inet_ntoa( m_xClientInAddr[fdx] );

					LogMessage( E_MSG_INFO, "Sent msg type %s to %s, %d bytes", CThread::GetTcpipMsgType(replyBuf.msg.eMsgType), (ip == NULL ? "unknown" : ip), rc );
				}


				szEspResponseMsg[0] = '\0';
				break;
			}
		}

		if ( szEspResponseMsg[0] != '\0' )
		{
			LogMessage( E_MSG_ERROR, "Failed to find ESP client socket" );
		}

		szEspResponseMsg[0] = '\0';
		szEspName[0] = '\0';
	}

	// send the time to all ESP devices
	time_t timenow = time(NULL);
	if ( m_tLastSentEspTime + 30 <= timenow )
	{	// send every 30 seconds
		char szTime[20];
		char szWeight[20];

		int iWbIdx = -1;
		double dWeight = 0.0;
		double dBridgeTare = 0.0;
		for ( int i = 0; i < MAX_DEVICES; i++ )
		{
			if ( m_pmyDevices->GetDeviceType(i) == E_DT_PT113_LCT )
			{	// found the weigh bridge
				iWbIdx = i;
				dWeight = m_pmyDevices->CalcPT113Weight(iWbIdx, 0, true);
				dBridgeTare = m_pmyDevices->GetOffset(iWbIdx,0);
				break;
			}
			else if ( m_pmyDevices->GetDeviceType(i) == E_DT_SYSTEC_IT1 )
			{	// found the weigh bridge
				iWbIdx = i;
				dWeight = m_pmyDevices->CalcSystecIT1Weight(iWbIdx, 0, true);
				dBridgeTare = m_pmyDevices->GetOffset(iWbIdx,0);
				break;
			}
		}



		struct tm* tmptr;
		tmptr = localtime(&timenow);
		snprintf( szTime, sizeof(szTime), "TM%02d%02d%02d", tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);
		if ( dBridgeTare < 1000 )
			snprintf( szWeight, sizeof(szWeight), "WW%06.1f", dWeight );
		else
			snprintf( szWeight, sizeof(szWeight), "WW%06d", (int)dWeight );

		for ( fdx = 0; fdx < MAX_TCPIP_SOCKETS; fdx++ )
		{
			if ( m_iClientFd[fdx] != -1 )
			{
				m_tLastSentEspTime = timenow;
				int rc;
				size_t uMsgSize = strlen(szTime);

				// TODO: handle write being interrupted
				rc = SSL_write( m_xClientSSL[fdx], szTime, uMsgSize );
				if ( rc != (int)uMsgSize )
				{
					LogMessage( E_MSG_ERROR, "Failed to send time to client, errno %d", errno );

					CloseSocket( m_iClientFd[fdx], fdx );
				}
				else
				{
//						char *ip = inet_ntoa( m_xClientInAddr[fdx] );
//						LogMessage( E_MSG_INFO, "Sent msg type TM to %s", (ip == NULL ? "unknown" : ip) );
					if ( iWbIdx >= 0 )
					{
						uMsgSize = strlen(szWeight);

						rc = SSL_write( m_xClientSSL[fdx], szWeight, uMsgSize );
						if ( rc != (int)uMsgSize )
						{
							LogMessage( E_MSG_ERROR, "Failed to send weight to client, errno %d", errno );

							CloseSocket( m_iClientFd[fdx], fdx );
						}
					}
				}
			}
		}
	}

}

void CThread::ReadTcpipMessage( CDeviceList* pmyDevices, CMysql& myDB )
{
	int fdx;
	int rc;
	int idx;
	int iAddr;
	size_t uLen;
	NIMROD_MSGBUF_TYPE msgBuf;
	NIMROD_MSGBUF_TYPE replyBuf;

	if ( m_Sockfd >= 0 )
	{
		for ( fdx = 0; fdx < MAX_TCPIP_SOCKETS; fdx++ )
		{
			if ( m_iClientFd[fdx] == -1 )
				continue;

			uLen = ReadTcpipMsgBytes( m_xClientSSL[fdx], m_iClientFd[fdx], msgBuf, false );

			if ( uLen == sizeof(msgBuf) )
			{	// complete message received
				enum E_MESSAGE_TYPE eMsgType = msgBuf.msg.eMsgType;
				size_t uMsgSize = sizeof(replyBuf);

				LogMessage( E_MSG_DEBUG, "Got msg type %s (segment %d of %d)", CThread::GetTcpipMsgType(eMsgType), msgBuf.msg.iSegCount, msgBuf.msg.iSegTotal );

				m_tClientLastMsg[fdx] = time(NULL);

				// TODO: handle multi part messages


				// send ack back to client
				bzero( &replyBuf, sizeof(replyBuf) );
				replyBuf.msg.eMsgType = E_MT_ACK;
				replyBuf.msg.iSegCount = 1;
				replyBuf.msg.iSegTotal = 1;

				// handle message
				switch ( eMsgType )
				{
				default:
					LogMessage( E_MSG_WARN, "Unhandled msg type %s", CThread::GetTcpipMsgType(eMsgType) );
					break;

				case E_MT_ACK:
					break;

				case E_MT_TOTAL_DEVICES:
					replyBuf.msg.eMsgType = E_MT_TOTAL_DEVICES;
					replyBuf.msg.totalDevices.iTotalDevices = pmyDevices->GetTotalDevices();
					break;

				case E_MT_DEVICE_INFO:
					iAddr = msgBuf.msg.iAddress;
					idx = m_pmyDevices->GetIdxForAddr(iAddr);

					replyBuf.msg.eMsgType = E_MT_DEVICE_INFO;
					replyBuf.msg.listDevices.iAddress = iAddr;
					replyBuf.msg.listDevices.iNumInputs = m_pmyDevices->GetNumInputs(idx);
					replyBuf.msg.listDevices.iNumOutputs = m_pmyDevices->GetNumOutputs(idx);
					replyBuf.msg.listDevices.m_eDeviceStatus = m_pmyDevices->GetDeviceStatus(idx);
					replyBuf.msg.listDevices.m_eDeviceType = m_pmyDevices->GetDeviceType(idx);
					snprintf( replyBuf.msg.listDevices.szName, sizeof(replyBuf.msg.listDevices.szName), "%s", m_pmyDevices->GetDeviceName(idx) );
					break;

				case E_MT_CHANGE_OUTPUT:
					iAddr = pmyDevices->GetAddress(msgBuf.msg.changeOutput.iInIdx);
					if ( msgBuf.msg.changeOutput.iInAddress == iAddr )
					{	// check config has not changed
						pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

						LogMessage( E_MSG_INFO, "TCP Output change for '%s' (0x%x->%d,%d), new state %u (%d,%.1f), on %s", m_pmyDevices->GetOutIOName(msgBuf.msg.changeOutput.iOutIdx, msgBuf.msg.changeOutput.iOutChannel),
								msgBuf.msg.changeOutput.iOutAddress, msgBuf.msg.changeOutput.iOutIdx, msgBuf.msg.changeOutput.iOutChannel+1, msgBuf.msg.changeOutput.uState, msgBuf.msg.changeOutput.iOutOnPeriod,
								msgBuf.msg.changeOutput.dVsdFrequency, m_pmyDevices->GetDeviceHostname( msgBuf.msg.changeOutput.iOutIdx ) );

						ChangeOutputState( myDB, msgBuf.msg.changeOutput.iInIdx, msgBuf.msg.changeOutput.iInAddress, msgBuf.msg.changeOutput.iInChannel,
								msgBuf.msg.changeOutput.iOutIdx, msgBuf.msg.changeOutput.iOutAddress, msgBuf.msg.changeOutput.iOutChannel, msgBuf.msg.changeOutput.uState,
								msgBuf.msg.changeOutput.eSwType, msgBuf.msg.changeOutput.iOutOnPeriod, msgBuf.msg.changeOutput.dVsdFrequency );

						pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );
					}
					else
					{
						LogMessage( E_MSG_ERROR, "ChangeOutput event: address not matched for InIdx %d: %d vs %d", msgBuf.msg.changeOutput.iInIdx,
								msgBuf.msg.changeOutput.iInAddress, iAddr );
					}
					break;

				case E_MT_ESP_MSG:
					if ( strcmp( msgBuf.msg.esp.szEvent, "CLK" ) == 0 )
						LogMessage( E_MSG_INFO, "ESP button %d click from '%s'", msgBuf.msg.esp.iButton, msgBuf.msg.esp.szEspName );
					else if ( strcmp( msgBuf.msg.esp.szEvent, "TMP" ) == 0 )
						LogMessage( E_MSG_INFO, "ESP temperature %d %.1f degC from '%s'", msgBuf.msg.esp.iChannel, msgBuf.msg.esp.dTemperature, msgBuf.msg.esp.szEspName );
					else if ( strcmp( msgBuf.msg.esp.szEvent, "CID" ) == 0 )
						LogMessage( E_MSG_INFO, "ESP chip id from '%s' '%s'", msgBuf.msg.esp.szEspName, msgBuf.msg.esp.szChipMac );
					else
						LogMessage( E_MSG_INFO, "ESP '%s' message from '%s'", msgBuf.msg.esp.szEvent, msgBuf.msg.esp.szEspName );

					// dummy do nothing message
					replyBuf.msg.eMsgType = E_MT_ESP_MSG;
					snprintf( replyBuf.msg.esp.szBuf, sizeof(replyBuf.msg.esp.szBuf), "OK000000" );

					// save the esp name
					snprintf( m_szClientEspName[fdx], sizeof(m_szClientEspName[fdx]), "%s", msgBuf.msg.esp.szEspName );
					//LogMessage( E_MSG_INFO, "ESP name set to '%s'", m_szClientEspName[fdx] );

					if ( msgBuf.msg.esp.iButton != 0 )
					{
						pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

						m_szEspResponseMsg[0] = '\0';
						ProcessEspSwitchEvent( myDB, msgBuf.msg.esp.szEspName, msgBuf.msg.esp.iButton - 1 );

						if ( strlen(m_szEspResponseMsg) > 0 )
						{
							snprintf( replyBuf.msg.esp.szBuf, sizeof(replyBuf.msg.esp.szBuf), "%s", m_szEspResponseMsg );
							m_szEspResponseMsg[0] = '\0';
						}

						pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );
					}
					else if ( msgBuf.msg.esp.iChannel != 0 )
					{
						pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

						m_szEspResponseMsg[0] = '\0';
						ProcessEspTemperatureEvent( myDB, msgBuf.msg.esp.szEspName, msgBuf.msg.esp.iChannel-1, msgBuf.msg.esp.dTemperature );

						if ( strlen(m_szEspResponseMsg) > 0 )
						{
							snprintf( replyBuf.msg.esp.szBuf, sizeof(replyBuf.msg.esp.szBuf), "%s", m_szEspResponseMsg );
							m_szEspResponseMsg[0] = '\0';
						}

						pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );
					}
					else if ( strcmp( msgBuf.msg.esp.szEvent, "CID" ) == 0 )
					{	// check the device name
						if ( strcmp( msgBuf.msg.esp.szEspName, "ESP000" ) == 0 )
						{	// default name - we must change it
							int iNum = 1;
							int iNumFields;
							char szData[100] = "";
							char szNewName[7] = "";
							MYSQL_ROW row;

							if ( myDB.RunQuery( "select de_Name from devices where de_ComPort='ESP%s' order by de_Name desc limit 1", msgBuf.msg.esp.szChipMac ) )
							{
								LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
							}
							else if ( (row = myDB.FetchRow( iNumFields )) )
							{	// chip id already exists
								strncpy( szData, (const char*)row[0], sizeof(szData)-1 );
								szData[sizeof(szData)-1] = '\0';

								iNum = atoi( &szData[3] );

								LogMessage( E_MSG_INFO, "Found matching ESP chip id for %s", szData );

								myDB.FreeResult();
							}
							else
							{
								myDB.FreeResult();

								if ( myDB.RunQuery( "select de_Name from devices where de_ComPort='ESP' order by de_Name desc limit 1" ) )
								{
									LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
								}
								else
								{	// get the last ESP device from the database devices table

									if ( (row = myDB.FetchRow( iNumFields )) )
									{
										strncpy( szData, (const char*)row[0], sizeof(szData)-1 );
										szData[sizeof(szData)-1] = '\0';

										iNum = atoi( &szData[3] ) + 1;
									}
									else
									{
										LogMessage( E_MSG_INFO, "ESP device not found in the database" );
									}

									myDB.FreeResult();
								}
							}

							if ( iNum > 0 && iNum <= 999 )
							{
								snprintf( szNewName, sizeof(szNewName), "ESP%03d", iNum );

								LogMessage( E_MSG_INFO, "New ESP device name is '%s'", szNewName );

								// save the esp name
								snprintf( m_szClientEspName[fdx], sizeof(m_szClientEspName[fdx]), "%s", szNewName );
								LogMessage( E_MSG_INFO, "New ESP name set to '%s'", m_szClientEspName[fdx] );

								// tell the device it's new name
								snprintf( replyBuf.msg.esp.szBuf, sizeof(replyBuf.msg.esp.szBuf), "NN%s", szNewName );
							}
							else
							{
								LogMessage( E_MSG_ERROR, "Cannot allocate new ESP device name, num is %d", iNum );
							}

						}

						// update the device status
						int idx2;
						idx2 = m_pmyDevices->GetIdxForName( m_szClientEspName[fdx] );
						if ( idx2 >= 0 )
						{
							if ( m_pmyDevices->SetDeviceStatus( idx2, E_DS_ALIVE ) )
							{
								m_pmyDevices->UpdateDeviceStatus( myDB, idx2 );
							}

							LogMessage( E_MSG_INFO, "ESP device '%s' (0x%x->%d,%d) is now alive !", m_pmyDevices->GetDeviceName(idx2), m_pmyDevices->GetAddress(idx2), idx2, fdx );
							myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx2), 0, E_ET_DEVICE_OK, 0, "ESP device '%s' (0x%x->%d,%d) is now alive !", m_pmyDevices->GetDeviceName(idx2), m_pmyDevices->GetAddress(idx2), idx2, fdx );
						}
						else
						{
							LogMessage( E_MSG_ERROR, "Failed to find idx for ESP device '%s'", m_szClientEspName[fdx] );
						}

						// check if we have another socket for this chip id
						int fdx2;

						for ( fdx2 = 0; fdx2 < MAX_TCPIP_SOCKETS; fdx2++)
						{
							if ( m_iClientFd[fdx2] != -1 && fdx2 != fdx )
							{	// different slot is in use
								if ( strcmp( m_szClientEspName[fdx2], m_szClientEspName[fdx] ) == 0 )
								{	// same esp name
									LogMessage( E_MSG_WARN, "Extra socket %d for esp device %s, closing the extra socket", fdx2, m_szClientEspName[fdx] );

									CloseSocket( m_iClientFd[fdx2], fdx2 );
								}
							}

						}
					}

					uMsgSize = strlen(replyBuf.msg.esp.szBuf);
					break;
				}


				// TODO: handle write being interrupted
				if ( eMsgType == E_MT_ESP_MSG )
				{
					rc = SSL_write( m_xClientSSL[fdx], replyBuf.msg.esp.szBuf, uMsgSize );
				}
				else
				{
					rc = SSL_write( m_xClientSSL[fdx], replyBuf.szBuf, uMsgSize );
				}
				if ( rc != (int)uMsgSize )
				{
					LogMessage( E_MSG_ERROR, "Failed to send reply to client, errno %d", errno );

					CloseSocket( m_iClientFd[fdx], fdx );
				}
				else
				{
					char *ip = inet_ntoa( m_xClientInAddr[fdx] );

					LogMessage( E_MSG_DEBUG, "Sent reply msg type %s to %s, len %d, socket %d", CThread::GetTcpipMsgType(replyBuf.msg.eMsgType), (ip == NULL ? "unknown" : ip), (int)uMsgSize, fdx );
				}

				// close the socket if it is from nimrod pi
				if ( eMsgType != E_MT_ESP_MSG )
				{
					CloseSocket( m_iClientFd[fdx], fdx );
				}
			}
			else
			{
				if ( uLen != 0 )
				{
					LogMessage( E_MSG_ERROR, "ESP Msg data missing, only %u bytes received", uLen );
				}

				if ( m_tClientLastMsg[fdx] + ESP_PING_TIMEOUT < time(NULL) )
				{	// last client message was more than 90 sec ago
					// esp device is no longer connected
					int idx2;

					idx2 = m_pmyDevices->GetIdxForName( m_szClientEspName[fdx] );
					if ( idx2 >= 0 )
					{
						if ( m_pmyDevices->SetDeviceStatus( idx2, E_DS_DEAD ) )
						{
							m_pmyDevices->UpdateDeviceStatus( myDB, idx2 );
						}

						LogMessage( E_MSG_INFO, "ESP device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx2), m_pmyDevices->GetAddress(idx2), idx2 );
						myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx2), 0, E_ET_DEVICE_NG, 0, "ESP device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx2), m_pmyDevices->GetAddress(idx2), idx2 );
					}
					else
					{
						LogMessage( E_MSG_ERROR, "Failed to find idx for ESP device '%s'", m_szClientEspName[fdx] );
					}

					CloseSocket( m_iClientFd[fdx], fdx );

					LogMessage( E_MSG_INFO, "client socket closed, (%d clients)", m_iClientCount );
				}
			}

		}
	}
}

void CThread::ProcessEspSwitchEvent( CMysql& myDB, const char* szName, const int i )
{
	enum E_EVENT_TYPE eType = E_ET_CLICK;
	int idx = -1;

	// find the device index from the name
	idx = m_pmyDevices->GetIdxForName( szName );
	if ( idx >= 0 )
	{
		if ( m_pmyDevices->GetInIOName(idx,i)[0] != '\0' )
		{
			m_pmyDevices->GetNewInput(idx,i) = 1;

			LogMessage( E_MSG_INFO, "Switch '%s' %d = %d on device addr %d (%s) %d", m_pmyDevices->GetInIOName(idx,i), i+1, m_pmyDevices->GetNewInput(idx,i), m_pmyDevices->GetAddress(idx),
				m_pmyDevices->GetEventTypeDesc( eType ), idx );

			myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), i, eType, i, "Switch '%s' %d = %d on device addr %d (%s)", m_pmyDevices->GetInIOName(idx,i), i+1, m_pmyDevices->GetNewInput(idx,i),
				m_pmyDevices->GetAddress(idx), m_pmyDevices->GetEventTypeDesc( eType ) );

			ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), i, m_pmyDevices->GetNewInput(idx,i), eType );
		}
		else
		{
			LogMessage( E_MSG_WARN, "ESP '%s' button click has no deviceinfo association", szName );
		}
	}
}

bool CThread::SendTcpipChangeOutputToHost( const char* szHostname, const int iInIdx, const int iInAddress, const int iInChannel, const int iOutIdx, const int iOutAddress,
		const int iOutChannel, const uint8_t uState, const enum E_IO_TYPE eSwType, const int iOutOnPeriod, const double dVsdFrequency )
{
	bool bRc = false;
	int rc;
	int sockfd;
	size_t uLen;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	NIMROD_MSGBUF_TYPE msgBuf;

	bzero( &msgBuf, sizeof(msgBuf) );

	msgBuf.msg.eMsgType = E_MT_CHANGE_OUTPUT;
	msgBuf.msg.iSegCount = 1;
	msgBuf.msg.iSegTotal = 1;

	msgBuf.msg.changeOutput.iInIdx = iInIdx;
	msgBuf.msg.changeOutput.iInAddress = iInAddress;
	msgBuf.msg.changeOutput.iInChannel = iInChannel;
	msgBuf.msg.changeOutput.iOutIdx = iOutIdx;
	msgBuf.msg.changeOutput.iOutAddress = iOutAddress;
	msgBuf.msg.changeOutput.iOutChannel = iOutChannel;
	msgBuf.msg.changeOutput.uState = uState;
	msgBuf.msg.changeOutput.eSwType = eSwType;
	msgBuf.msg.changeOutput.iOutOnPeriod = iOutOnPeriod;
	msgBuf.msg.changeOutput.dVsdFrequency = dVsdFrequency;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 )
	{
	    LogMessage( E_MSG_ERROR, "SendTcpipChangeOutputToHost: socket() failed, errno %d", errno );
	}
	else
	{
		server = gethostbyname( szHostname );
		if ( server == NULL )
		{
			LogMessage( E_MSG_ERROR, "SendTcpipChangeOutputToHost: gethostbyname() failed, errno %d", errno );
		}
		else
		{
			serv_addr.sin_family = AF_INET;
			bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
			serv_addr.sin_port = htons(NIMROD_PORT);

			if ( connect( sockfd, (const sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0 )
			{
				LogMessage( E_MSG_ERROR, "SendTcpipChangeOutputToHost: connect() failed, errno %d", errno );
			}
			else
			{
				SSL *ssl = NULL;

				ssl = SSL_new( m_sslClientCtx );      // create new SSL connection state

		    	//SSL_set_max_send_fragment( ssl, 1024 );
				LogMessage( E_MSG_INFO, "Calling SSL_connect: %p %p %d", m_sslClientCtx, ssl, sockfd );

				SSL_set_fd( ssl, sockfd );    	// attach the socket descriptor
				if ( SSL_connect(ssl) <= 0 )   // perform the connection
				{
					LogMessage( E_MSG_ERROR, "SSL_connect failed" );
					LogSSLError();
				}
				else
				{
					ShowCerts(ssl);
					rc = SSL_write( ssl, msgBuf.szBuf, sizeof(msgBuf) );
					if ( rc != sizeof(msgBuf) )
					{
						LogMessage( E_MSG_ERROR, "SendTcpipChangeOutputToHost: Failed to send event to host '%s', errno %d", szHostname, errno );
					}
					else
					{
						bRc = true;
						LogMessage( E_MSG_INFO, "Sent event msg to '%s'", szHostname );

						// read ack
						uLen = ReadTcpipMsgBytes( ssl, sockfd, msgBuf, true );

						if ( uLen == sizeof(msgBuf) )
						{	// complete message received
							if ( msgBuf.msg.eMsgType == E_MT_ACK )
							{
								LogMessage( E_MSG_INFO, "Got ACK from '%s'", szHostname );
							}
							else
							{
								LogMessage( E_MSG_ERROR, "Got msg type %d from '%s'", msgBuf.msg.eMsgType, szHostname );
							}
						}
						else
						{
							LogMessage( E_MSG_ERROR, "Got incomplete msg waiting for ACK from '%s', only %lu bytes", szHostname, uLen );
						}
					}
				}

				SSL_free(ssl);        // release connection state
			}
		}

		CloseSocket( sockfd, -1 );
	}

	return bRc;
}

void CThread::HandleVSDOutputDevice( CMysql& myDB, const int idx, const int iChannel, double dVsdFreq )
{
	int rc;
	int addr;
	int iLoop = 2;
	int iVsdOp = 6;	// speed down stop

	if ( dVsdFreq > 0 )
	{
		iVsdOp = 1;	// forward
	}
	else if ( dVsdFreq < 0 )
	{
		iVsdOp = 2;	// backward
	}

	modbus_t* ctx = m_pmyDevices->GetContext(idx);

	if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
	{
		LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
	}

	usleep( 30000 );

	switch ( m_pmyDevices->GetDeviceType(idx) )
	{
	default:
		LogMessage( E_MSG_ERROR, "HandleVSDOutputDevice: unhandled device type %d", m_pmyDevices->GetDeviceType(idx) );
		break;
	case E_DT_VSD_NFLIXEN:

			// status
		int iLen = 1;
		addr = 0x3000;
		uint16_t uiInputs[1];

		rc = modbus_read_registers( ctx, addr, iLen, uiInputs );
		if ( rc == -1 )
		{
			LogMessage( E_MSG_WARN, "Error: modbus_read_registers() failed: %s", modbus_strerror(errno) );
		}
		else
		{
			LogMessage( E_MSG_INFO, "VSD status: %u", uiInputs[0] );
		}

		usleep( 50000 );

		// turn the vsd on or off
		addr = 0x2000;
		while ( iLoop > 0 )
		{
			rc = modbus_write_register( ctx, addr, iVsdOp );
			if ( rc == -1 )
			{
				iLoop -= 1;
				LogMessage( E_MSG_ERROR, "HandleVsdOutputDevice() modbus_write_register() failed: %s", modbus_strerror(errno) );
			}
			else
			{
				iLoop = 0;
				LogMessage( E_MSG_INFO, "HandleVsdOutputDevice() VSD operation set to %d", iVsdOp );
			}
		}

		usleep( 50000 );

		iLoop = 2;
		while ( iLoop > 0 )
		{	// value is % * 100 of max frequency (50Hz)
			addr = 0x1000;

			int val = 10000 * (fabs(dVsdFreq) / 50);
			if ( val > 10000 )
				val = 10000;

			rc = modbus_write_register( ctx, addr, val );
			if ( rc == -1 )
			{
				iLoop -= 1;
				LogMessage( E_MSG_ERROR, "HandleVSDOutputDevice() modbus_write_register() failed: %s", modbus_strerror(errno) );
			}
			else
			{
				iLoop = 0;
				LogMessage( E_MSG_INFO, "HandleVsdOutputDevice() VSD frequency set to %.1f Hz -> %d", dVsdFreq, val );
			}
		}
		break;
	}
}

void CThread::HandleOutputDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int i;
	int iLoop;
	int iRetry = 3;
	unsigned char cData[MAX_IO_PORTS];

	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		if ( modbus_read_bits( ctx, 0, m_pmyDevices->GetNumOutputs(idx), cData ) == -1 )
		{	// error
			int err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}
				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_bits(%p) '%s' (0x%x->%d) %d failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx,
							m_pmyDevices->GetNumOutputs(idx), modbus_strerror(err), iLoop );
				}
			}
			else
			{
				LogMessage( E_MSG_WARN, "modbus_read_bits(%p) '%s' (0x%x->%d) %d failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx,
						m_pmyDevices->GetNumOutputs(idx), modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_bits() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			// check each output bit
			for ( i = 0; i < m_pmyDevices->GetNumOutputs(idx); i++ )
			{
				if ( m_pmyDevices->GetNumInputs(idx) > 0 && m_pmyDevices->GetNumOutputs(idx) > 0 )
				{	// read_input_bits() reads the input state not the output state !!
					break;
				}

				int iState = 0;
				if ( m_pmyDevices->GetOutOnStartTime(idx,i) != 0 )
				{
					iState = 1;
				}
				if ( cData[i] != iState )
				{	// state mismatch !
					if ( !IsMyHostname( m_pmyDevices->GetDeviceHostname(idx) ) )
					{	// not my device - but this should never happen
						static time_t gtLastHostnameErrorTime = 0;
						if ( gtLastHostnameErrorTime + 60 < time(NULL) )
						{
							LogMessage( E_MSG_ERROR, "Configuration error for '%s', hostname '%s' should be '%s'", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetDeviceHostname(idx), GetMyHostname() );
							gtLastHostnameErrorTime = time(NULL);
						}
					}
					else if ( cData[i] != 0 )
					{	// device bit is on but nimrod thinks it is off
						LogMessage( E_MSG_INFO, "DIO device '%s' (idx %d) bit %d '%s' is on, nimrod state is %d (start %ld)", m_pmyDevices->GetDeviceName(idx), idx, i, m_pmyDevices->GetOutIOName(idx,i), iState, m_pmyDevices->GetOutOnStartTime(idx,i) );

						usleep( 10000 );
						if ( m_pmyDevices->WriteOutputBit( idx, i, iState ) )
						{	// success
							LogMessage( E_MSG_INFO, "DIO device '%s' channel '%s' state restored to OFF (%d,%d)", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetOutIOName(idx,i), idx, i );
						}
					}
					else
					{	// device bit is off but nimrod thinks it should be on
						// device has been power cycled
						LogMessage( E_MSG_INFO, "DIO device '%s' (idx %d) bit %d '%s' is off, nimrod state is %d", m_pmyDevices->GetDeviceName(idx), idx, i, m_pmyDevices->GetOutIOName(idx,i), iState );

						usleep( 10000 );
						if ( m_pmyDevices->WriteOutputBit( idx, i, iState ) )
						{	// success
							LogMessage( E_MSG_INFO, "DIO device '%s' channel '%s' state restored to ON (%d,%d)", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetOutIOName(idx,i), idx, i );
						}
					}
				}
			}
			break;
		}
	}
}

void CThread::HandleSwitchDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int i;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	addr = 0;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = 0;
		if ( ctx != NULL )
		{
			rc = modbus_read_input_bits( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewInput(idx) );
		}
		if ( rc == -1 )
		{
			err = errno;
			if ( iLoop+1 < iRetry )
			{
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "DIO device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}
				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_input_bits(%p) '%s' (0x%x->%d) %d failed: %s", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx,
							m_pmyDevices->GetNumInputs(idx), modbus_strerror(err) );
				}
			}
			else
			{
				LogMessage( E_MSG_WARN, "modbus_read_input_bits(%p) '%s' (0x%x->%d) %d failed: %s, loop %d retry ", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx,
						m_pmyDevices->GetNumInputs(idx), modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_input_bits() retry successful, loop %d", iLoop );
			}

			if ( ctx == NULL && m_pmyDevices->GetDeviceStatus(idx) != E_DS_ALIVE )
			{
				LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			
				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
				}
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "DIO device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}
			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( i = 0; i < m_pmyDevices->GetNumInputs(idx); i++ )
			{
				if ( m_pmyDevices->GetInChannelType(idx,i) == E_IO_ON_OFF_INV )
				{	// invert the state
					if ( m_pmyDevices->GetNewInput( idx, i ) )
						m_pmyDevices->GetNewInput( idx, i ) = false;
					else
						m_pmyDevices->GetNewInput( idx, i ) = true;
				}

				// check for click file from web gui
				bool bWebClick = false;
				if ( WebClickExists( myDB, m_pmyDevices->GetDeviceNo(idx), i ) )
				{
					bWebClick = true;
					m_pmyDevices->GetNewInput( idx, i ) = true;
					LogMessage( E_MSG_INFO, "WebClick from %d,%d", idx, i );
				}

				if ( m_pmyDevices->GetNewInput( idx, i ) != m_pmyDevices->GetLastInput( idx, i ) )
				{	// data has changed
					enum E_EVENT_TYPE eType = E_ET_CLICK;
					struct timespec tNow;
					long lDiff;

					LogMessage( E_MSG_INFO, "Last state %d, %d", m_pmyDevices->GetNewInput( idx, i ), m_pmyDevices->GetLastInput( idx, i ) );
					m_pmyDevices->GetLastInput(idx,i) = m_pmyDevices->GetNewInput(idx,i);

					m_pmyDevices->GetEventTimeNow( tNow );
					if ( m_pmyDevices->GetNewInput(idx,i) != 0 )
					{	// button is pressed, check if this is a double click
						lDiff = m_pmyDevices->GetEventTimeDiff( idx, i, tNow );
						if ( lDiff <= DOUBLE_CLICK_MSEC && lDiff > 0 )
						{
							eType = E_ET_DBLCLICK;
						}

						m_pmyDevices->SetEventTime( idx, i, tNow );
					}
					else
					{	// button is released, check if this was a long press
						lDiff = m_pmyDevices->GetEventTimeDiff( idx, i, tNow );
						if ( lDiff >= LONG_PRESS_MSEC )
						{
							eType = E_ET_LONGPRESS;
						}
					}

					LogMessage( E_MSG_INFO, "Switch '%s' %d = %d on device addr %d (%s)", m_pmyDevices->GetInIOName(idx,i), i+1, m_pmyDevices->GetNewInput(idx,i), m_pmyDevices->GetAddress(idx),
							m_pmyDevices->GetEventTypeDesc( eType ) );

					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), i, eType, i, "Switch '%s' %d = %d on device addr %d (%s)", m_pmyDevices->GetInIOName(idx,i), i+1, m_pmyDevices->GetNewInput(idx,i),
							m_pmyDevices->GetAddress(idx), m_pmyDevices->GetEventTypeDesc( eType ) );

					ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), i, m_pmyDevices->GetNewInput(idx,i), eType );

					// check if this event should be passed onto the plc handling
					if ( m_pmyDevices->GetNewInput( idx, i )  == true && m_pmyPlcStates->FindInputDevice( m_pmyDevices->GetDeviceNo(idx), i ) )
					{	// only pass the switch down event not the release
						m_pmyPlcStates->AddInputEvent( m_pmyDevices->GetDeviceNo(idx), i, 1 );
					}

					if ( bWebClick )
					{
						m_pmyDevices->GetNewInput( idx, i ) = false;
//						m_pmyDevices->GetLastInput( idx, i ) = false;
					}
				}
			}

			// break out of retry loop
			break;
		}
	}
}

void CThread::HandleHdlLevelDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	addr = 0x04;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = modbus_read_registers( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewData(idx) );
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "HdlLevel device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "HdlLevel device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{	// retry
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				// level is in %
				double dDiff = 0.1;

				E_EVENT_TYPE eEventType = E_ET_LEVEL;
				E_IO_TYPE eIOTypeL = E_IO_LEVEL_LOW;
				E_IO_TYPE eIOTypeH = E_IO_LEVEL_HIGH;
				E_IO_TYPE eIOTypeHL = E_IO_LEVEL_HIGHLOW;
				E_IO_TYPE eIOTypeMon = E_IO_LEVEL_MONITOR;
				char szUnits[10] = "%";
				char szDesc[20] = "Level";
				char szName[50] = "HDL Level";
				double dValOld = m_pmyDevices->CalcLevel(idx,iChannel,false);
				double dValNew = m_pmyDevices->CalcLevel(idx,iChannel,true);

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

void CThread::HandleRotaryEncoderDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	//modbus_set_debug(ctx, true);

	//printf ("testing %ld\n", time(NULL));

	addr = 0x00;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{	// 1 input channel but we need to read 2 registers for multi turn encoders
		int count = m_pmyDevices->GetNumInputs(idx);
		if ( count == 1 )
			count = 2;

		rc = modbus_read_registers( ctx, addr, count, m_pmyDevices->GetNewData(idx) );

		if ( rc == -1 && errno == EMBMDATA+1 )	// EMBBADSLAVE
		{	// response not from requested slave
			// devices seems to set the hi bit of the slave address on replies some times but all other data is correct
			// ignore this error
			rc = 0;
		}
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "RotEnc device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "RotEnc device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{	// retry
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) %d '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, err, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				// distance is in mm
				double dDiff = 0.2;

				E_EVENT_TYPE eEventType = E_ET_ROTARY_ENC;
				E_IO_TYPE eIOTypeL = E_IO_ROTENC_LOW;
				E_IO_TYPE eIOTypeH = E_IO_ROTENC_HIGH;
				E_IO_TYPE eIOTypeHL = E_IO_ROTENC_HIGHLOW;
				E_IO_TYPE eIOTypeMon = E_IO_ROTENC_MONITOR;
				char szUnits[10] = "mm";
				char szDesc[20] = "Distance";
				char szName[50] = "Rotary Encoder";
				double dValOld = m_pmyDevices->CalcRotaryEncoderDistance(idx,iChannel,false);
				double dValNew = m_pmyDevices->CalcRotaryEncoderDistance(idx,iChannel,true);

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
	//modbus_set_debug(ctx, false);
}

void CThread::HandleVIPFDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;
	int iCheckAgain = 1;


	addr = 0x00;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{	// read 10 bytes
		int count = 10;
		uint16_t uiVal[10];

		// VIPF devices may not respond as they are powered by the 230VAC they are monitoring
		if ( m_pmyDevices->GetDeviceStatus(idx) != E_DS_ALIVE )
		{
			iCheckAgain = 0;
			if ( time(NULL) % 10 == 0 || m_tThreadStartTime + 10 >= time(NULL) )
			{	// check if the device has come alive every 10 seconds
				//LogMessage( E_MSG_INFO, "Check VIPF again %d, address %d", m_pmyDevices->GetDeviceStatus(idx), m_pmyDevices->GetAddress(idx) );
				iCheckAgain = -1;
			}
		}

		if ( iCheckAgain != 0 )
		{
			rc = modbus_read_input_registers( ctx, addr, count, uiVal );
		}
		else
		{
			break;
		}
		if ( rc == -1 )
		{	// failed
			// don't retry VIPF devices
			err = errno;

			//if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
			if ( m_pmyDevices->GetDeviceStatus(idx) != E_DS_BURIED )
			{	// device has just failed
				LogMessage( E_MSG_INFO, "VIPF device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
				// do not record this in the DB - expected failure
				//myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "VIPF device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			//if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_BURIED, true ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
			}

			if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
			{
				LogMessage( E_MSG_WARN, "modbus_read_input_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
			}

			iLoop = iRetry;

			memset( uiVal, 0, 10*sizeof(uint16_t) );
		}

		// always process the data values, use 0 values when the device is powered off
		if ( true )
		{	// success
			// put register data into channel data
			m_pmyDevices->GetNewData(idx)[0] = uiVal[0];	// volts, lsb = 0.1V
			m_pmyDevices->GetNewData(idx)[1] = (uiVal[1] + (uiVal[2] >> 8));	// current, lsb = 0.001A
			m_pmyDevices->GetNewData(idx)[2] = (uiVal[3] + (uiVal[5] >> 8));	// power, lsb = 0.1W
			m_pmyDevices->GetNewData(idx)[3] = (uiVal[5] + (uiVal[6] >> 8));	// energy, lsb = 1Wh
			m_pmyDevices->GetNewData(idx)[4] = uiVal[7];	// frequency, lsb = 0.1Hz
			m_pmyDevices->GetNewData(idx)[5] = uiVal[8];	// power factor, lsb = 0.01


			if ( rc != -1 )
			{
				bAllDead = false;
				if ( iLoop > 0 )
				{
					LogMessage( E_MSG_INFO, "modbus_read_input_registers() retry successful, loop %d", iLoop );
				}

				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
				{
					LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
				}
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				double dDiff;
				double dValNew;
				double dValOld;
				char szUnits[10] = "?";
				char szDesc[20] = "?";
				char szName[50] = "VIPF Monitor";
				E_EVENT_TYPE eEventType = E_ET_VOLTAGE;
				E_IO_TYPE eIOTypeL = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeH = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeHL = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeMon = E_IO_VOLT_MONITOR;

				dValOld = m_pmyDevices->CalcVIPFValue(idx,iChannel,false);
				dValNew = m_pmyDevices->CalcVIPFValue(idx,iChannel,true);
				switch ( iChannel )
				{
				default:
					dDiff = 5;
					dValNew = 0;
					break;
				case 0:		// voltage
					dDiff = 0.5;	// 0.5 volts
					eEventType = E_ET_VOLTAGE;
					strcpy( szUnits, "V" );
					strcpy( szDesc, "Voltage" );
					eIOTypeL = E_IO_VOLT_LOW;
					eIOTypeH = E_IO_VOLT_HIGH;
					eIOTypeHL = E_IO_VOLT_HIGHLOW;
					eIOTypeMon = E_IO_VOLT_MONITOR;
					break;
				case 1:		// current
					dDiff = 0.05;	// 100mA
					eEventType = E_ET_CURRENT;
					strcpy( szUnits, "A" );
					strcpy( szDesc, "Current" );
					eIOTypeL = E_IO_CURRENT_LOW;
					eIOTypeH = E_IO_CURRENT_HIGH;
					eIOTypeHL = E_IO_CURRENT_HIGHLOW;
					eIOTypeMon = E_IO_CURRENT_MONITOR;
					break;
				case 2:		// power
					dDiff = 20;	// 20 Watts
					eEventType = E_ET_POWER;
					strcpy( szUnits, "W" );
					strcpy( szDesc, "Power" );
					eIOTypeL = E_IO_POWER_LOW;
					eIOTypeH = E_IO_POWER_HIGH;
					eIOTypeHL = E_IO_POWER_HIGHLOW;
					eIOTypeMon = E_IO_POWER_MONITOR;
					break;
				case 3:		// energy
					dDiff = 10;	// 10 Wh
					break;
				case 4:		// frequency
					dDiff = 1;	// 1Hz
					eEventType = E_ET_FREQUENCY;
					strcpy( szUnits, "Hz" );
					strcpy( szDesc, "Frequency" );
					eIOTypeL = E_IO_FREQ_LOW;
					eIOTypeH = E_IO_FREQ_HIGH;
					eIOTypeHL = E_IO_FREQ_HIGHLOW;
					eIOTypeMon = E_IO_FREQ_MONITOR;
					break;
				case 5:		// power factor
					dDiff = 0.55;	// 0.05 deg
					eEventType = E_ET_POWERFACTOR;
					strcpy( szUnits, "PF" );
					strcpy( szDesc, "Power Factor" );
					eIOTypeL = E_IO_PWRFACT_MONITOR;
					eIOTypeH = E_IO_PWRFACT_MONITOR;
					eIOTypeHL = E_IO_PWRFACT_MONITOR;
					eIOTypeMon = E_IO_PWRFACT_MONITOR;
					break;
				}

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

void CThread::HandleVSDNFlixenDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;
	int iCheckAgain = 1;


	addr = 0x1001;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{	// read 10 bytes
		int count = 10;
		uint16_t uiVal[10];

		// VSD devices may not respond as they are powered by the 230VAC they are monitoring
		if ( m_pmyDevices->GetDeviceStatus(idx) != E_DS_ALIVE )
		{
			iCheckAgain = 0;
			if ( time(NULL) % 10 == 0 || m_tThreadStartTime + 10 >= time(NULL) )
			{	// check if the device has come alive every 10 seconds
				//LogMessage( E_MSG_INFO, "Check VSD again %d, address %d", m_pmyDevices->GetDeviceStatus(idx), m_pmyDevices->GetAddress(idx) );
				iCheckAgain = -1;
			}
		}

		if ( iCheckAgain != 0 )
		{
			rc = modbus_read_registers( ctx, addr, count, uiVal );
			if ( rc == -1 )
			{
				rc = modbus_read_registers( ctx, addr, count, uiVal );
			}
		}
		else
		{
			break;
		}
		if ( rc == -1 )
		{	// failed
			// don't retry VSD devices
			err = errno;

			//if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
			if ( m_pmyDevices->GetDeviceStatus(idx) != E_DS_BURIED )
			{	// device has just failed
				LogMessage( E_MSG_INFO, "VSD device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
				// do not record this in the DB - expected failure
				//myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "VSD device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			//if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_BURIED, true ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
			}

			if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
			{
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
			}

			iLoop = iRetry;

			memset( uiVal, 0, 10*sizeof(uint16_t) );
		}

		if ( true )
		{	// success
			// put register data into channel data
			// voltage
			m_pmyDevices->GetNewData(idx)[0] = uiVal[2];	// 0x1003: Output Voltage 1V
			// current
			m_pmyDevices->GetNewData(idx)[1] = uiVal[3];	// 0x1004: Output Current 0.1A
			// power
			m_pmyDevices->GetNewData(idx)[2] = uiVal[4];	// 0x1005: Output Power 0.1kW
			// frequency
			m_pmyDevices->GetNewData(idx)[3] = uiVal[0];	// 0x1001: Frequency Hz 0.01Hz
			// torque
			m_pmyDevices->GetNewData(idx)[4] = uiVal[5];	// 0x1006: Torque %


			if ( rc != -1 )
			{
				bAllDead = false;
				if ( iLoop > 0 )
				{
					LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
				}

				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
				{
					LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
				}
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				double dDiff;
				double dValNew;
				double dValOld;
				char szUnits[10] = "?";
				char szDesc[20] = "?";
				char szName[50] = "VSD Monitor";
				E_EVENT_TYPE eEventType = E_ET_VOLTAGE;
				E_IO_TYPE eIOTypeL = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeH = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeHL = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeMon = E_IO_VOLT_MONITOR;

				dValOld = m_pmyDevices->CalcVSDNFlixenValue(idx,iChannel,false);
				dValNew = m_pmyDevices->CalcVSDNFlixenValue(idx,iChannel,true);
				switch ( iChannel )
				{
				default:
					dDiff = 5;
					dValNew = 0;
					break;
				case 0:		// voltage
					dDiff = 5;	// 0.5 volts
					eEventType = E_ET_VOLTAGE;
					strcpy( szUnits, "V" );
					strcpy( szDesc, "Voltage" );
					eIOTypeL = E_IO_VOLT_LOW;
					eIOTypeH = E_IO_VOLT_HIGH;
					eIOTypeHL = E_IO_VOLT_HIGHLOW;
					eIOTypeMon = E_IO_VOLT_MONITOR;
					break;
				case 1:		// current
					dDiff = 0.1;	// 100mA
					eEventType = E_ET_CURRENT;
					strcpy( szUnits, "A" );
					strcpy( szDesc, "Current" );
					eIOTypeL = E_IO_CURRENT_LOW;
					eIOTypeH = E_IO_CURRENT_HIGH;
					eIOTypeHL = E_IO_CURRENT_HIGHLOW;
					eIOTypeMon = E_IO_CURRENT_MONITOR;
					break;
				case 2:		// power
					dDiff = 20;	// 20 Watts
					eEventType = E_ET_POWER;
					strcpy( szUnits, "W" );
					strcpy( szDesc, "Power" );
					eIOTypeL = E_IO_POWER_LOW;
					eIOTypeH = E_IO_POWER_HIGH;
					eIOTypeHL = E_IO_POWER_HIGHLOW;
					eIOTypeMon = E_IO_POWER_MONITOR;
					break;
				case 3:		// frequency
					dDiff = 1;	// 1Hz
					eEventType = E_ET_FREQUENCY;
					strcpy( szUnits, "Hz" );
					strcpy( szDesc, "Frequency" );
					eIOTypeL = E_IO_FREQ_LOW;
					eIOTypeH = E_IO_FREQ_HIGH;
					eIOTypeHL = E_IO_FREQ_HIGHLOW;
					eIOTypeMon = E_IO_FREQ_MONITOR;
					break;
				case 4:		// torque %
					dDiff = 2;	//
					eEventType = E_ET_TORQUE;
					strcpy( szUnits, "T%" );
					strcpy( szDesc, "Torque Percent" );
					eIOTypeL = E_IO_TORQUE_LOW;
					eIOTypeH = E_IO_TORQUE_HIGH;
					eIOTypeHL = E_IO_TORQUE_HIGHLOW;
					eIOTypeMon = E_IO_TORQUE_MONITOR;
					break;
				}

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

void CThread::HandleVSDPwrElectDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;
	int iCheckAgain = 1;


	addr = 40162;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{	// read 10 bytes
		int count = 10;
		uint16_t uiVal[10];

		// VIPF devices may not respond as they are powered by the 230VAC they are monitoring
		if ( m_pmyDevices->GetDeviceStatus(idx) != E_DS_ALIVE )
		{
			iCheckAgain = 0;
			if ( time(NULL) % 10 == 0 || m_tThreadStartTime + 10 >= time(NULL) )
			{	// check if the device has come alive every 10 seconds
				//LogMessage( E_MSG_INFO, "Check VSD again %d, address %d", m_pmyDevices->GetDeviceStatus(idx), m_pmyDevices->GetAddress(idx) );
				iCheckAgain = -1;
			}
		}

		if ( iCheckAgain != 0 )
		{
			rc = modbus_read_registers( ctx, addr, count, uiVal );
		}
		else
		{
			break;
		}
		if ( rc == -1 )
		{	// failed
			// don't retry VSD devices
			err = errno;

			//if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
			if ( m_pmyDevices->GetDeviceStatus(idx) != E_DS_BURIED )
			{	// device has just failed
				LogMessage( E_MSG_INFO, "VSD device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
				// do not record this in the DB - expected failure
				//myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "VSD device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			//if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_BURIED, true ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
			}

			if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
			{
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
			}

			iLoop = iRetry;

			memset( uiVal, 0, 10*sizeof(uint16_t) );
		}

		if ( true )
		{	// success
			// put register data into channel data
			// voltage
			m_pmyDevices->GetNewData(idx)[0] = uiVal[4];	// 40166: Output Voltage 1V
			// current
			m_pmyDevices->GetNewData(idx)[1] = uiVal[1];	// 40163: Output Current 0.1A
			// power
			m_pmyDevices->GetNewData(idx)[2] = uiVal[3];	// 40165: Output Power 0.1kW
			// frequency
			m_pmyDevices->GetNewData(idx)[3] = uiVal[5];	// 40167: Frequency Hz 0.1Hz
			// torque
			m_pmyDevices->GetNewData(idx)[4] = uiVal[2];	// 40164: Torque %


			if ( rc != -1 )
			{
				bAllDead = false;
				if ( iLoop > 0 )
				{
					LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
				}

				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
				{
					LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
				}
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				double dDiff;
				double dValNew;
				double dValOld;
				char szUnits[10] = "?";
				char szDesc[20] = "?";
				char szName[50] = "VSD Monitor";
				E_EVENT_TYPE eEventType = E_ET_VOLTAGE;
				E_IO_TYPE eIOTypeL = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeH = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeHL = E_IO_VOLT_MONITOR;
				E_IO_TYPE eIOTypeMon = E_IO_VOLT_MONITOR;

				dValOld = m_pmyDevices->CalcVSDPwrElectValue(idx,iChannel,false);
				dValNew = m_pmyDevices->CalcVSDPwrElectValue(idx,iChannel,true);
				switch ( iChannel )
				{
				default:
					dDiff = 5;
					dValNew = 0;
					break;
				case 0:		// voltage
					dDiff = 0.5;	// 0.5 volts
					eEventType = E_ET_VOLTAGE;
					strcpy( szUnits, "V" );
					strcpy( szDesc, "Voltage" );
					eIOTypeL = E_IO_VOLT_LOW;
					eIOTypeH = E_IO_VOLT_HIGH;
					eIOTypeHL = E_IO_VOLT_HIGHLOW;
					eIOTypeMon = E_IO_VOLT_MONITOR;
					break;
				case 1:		// current
					dDiff = 0.1;	// 100mA
					eEventType = E_ET_CURRENT;
					strcpy( szUnits, "A" );
					strcpy( szDesc, "Current" );
					eIOTypeL = E_IO_CURRENT_LOW;
					eIOTypeH = E_IO_CURRENT_HIGH;
					eIOTypeHL = E_IO_CURRENT_HIGHLOW;
					eIOTypeMon = E_IO_CURRENT_MONITOR;
					break;
				case 2:		// power
					dDiff = 20;	// 20 Watts
					eEventType = E_ET_POWER;
					strcpy( szUnits, "W" );
					strcpy( szDesc, "Power" );
					eIOTypeL = E_IO_POWER_LOW;
					eIOTypeH = E_IO_POWER_HIGH;
					eIOTypeHL = E_IO_POWER_HIGHLOW;
					eIOTypeMon = E_IO_POWER_MONITOR;
					break;
				case 3:		// frequency
					dDiff = 1;	// 1Hz
					eEventType = E_ET_FREQUENCY;
					strcpy( szUnits, "Hz" );
					strcpy( szDesc, "Frequency" );
					eIOTypeL = E_IO_FREQ_LOW;
					eIOTypeH = E_IO_FREQ_HIGH;
					eIOTypeHL = E_IO_FREQ_HIGHLOW;
					eIOTypeMon = E_IO_FREQ_MONITOR;
					break;
				case 4:		// torque %
					dDiff = 1;	//
					eEventType = E_ET_TORQUE;
					strcpy( szUnits, "T%" );
					strcpy( szDesc, "Torque Percent" );
					eIOTypeL = E_IO_TORQUE_LOW;
					eIOTypeH = E_IO_TORQUE_HIGH;
					eIOTypeHL = E_IO_TORQUE_HIGHLOW;
					eIOTypeMon = E_IO_TORQUE_MONITOR;
					break;
				}

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}


void CThread::HandleVoltageDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	addr = 0x00;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = modbus_read_registers( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewData(idx) );
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "Voltage device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "Voltage device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{	// retry
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				// 50mV change
				double dDiff = 0.05;

				E_EVENT_TYPE eEventType = E_ET_VOLTAGE;
				E_IO_TYPE eIOTypeL = E_IO_VOLT_LOW;
				E_IO_TYPE eIOTypeH = E_IO_VOLT_HIGH;
				E_IO_TYPE eIOTypeHL = E_IO_VOLT_HIGHLOW;
				E_IO_TYPE eIOTypeMon = E_IO_VOLT_MONITOR;
				char szUnits[10] = "V";
				char szDesc[20] = "Voltage";
				char szName[50] = "Voltage";
				double dValOld = m_pmyDevices->CalcVoltage(idx,iChannel,false);
				double dValNew = m_pmyDevices->CalcVoltage(idx,iChannel,true);

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

void CThread::HandleSHTDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	addr = 0x0001;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = modbus_read_input_registers( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewData(idx) );
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "Temp/Humidity device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "Voltage device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_input_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{	// retry
				LogMessage( E_MSG_WARN, "modbus_read_input_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				double dDiff;
				double dValNew;
				double dValOld;
				char szUnits[10] = "degC";
				char szDesc[20] = "?";
				char szName[50] = "Temp/Humidity";
				E_EVENT_TYPE eEventType = E_ET_TEMPERATURE;
				E_IO_TYPE eIOTypeL = E_IO_TEMP_MONITOR;
				E_IO_TYPE eIOTypeH = E_IO_TEMP_MONITOR;
				E_IO_TYPE eIOTypeHL = E_IO_TEMP_MONITOR;
				E_IO_TYPE eIOTypeMon = E_IO_TEMP_MONITOR;

				dValOld = m_pmyDevices->CalcTHValue(idx,iChannel,false);
				dValNew = m_pmyDevices->CalcTHValue(idx,iChannel,true);
				switch ( iChannel )
				{
				default:
					dDiff = 1;
					dValNew = 0;
					break;
				case 0:		// temperature
					dDiff = 0.5;	// 0.5 degC
					eEventType = E_ET_TEMPERATURE;
					strcpy( szUnits, "degC" );
					strcpy( szDesc, "Temperature" );
					eIOTypeL = E_IO_TEMP_LOW;
					eIOTypeH = E_IO_TEMP_HIGH;
					eIOTypeHL = E_IO_TEMP_HIGHLOW;
					eIOTypeMon = E_IO_TEMP_MONITOR;
					break;
				case 1:		// humidity
					dDiff = 0.5;	// percent
					eEventType = E_ET_HUMIDITY;
					strcpy( szUnits, "%" );
					strcpy( szDesc, "Humidity" );
					eIOTypeL = E_IO_HUMIDITY_LOW;
					eIOTypeH = E_IO_HUMIDITY_HIGH;
					eIOTypeHL = E_IO_HUMIDITY_HIGHLOW;
					eIOTypeMon = E_IO_HUMIDITY_MONITOR;
					break;
				}

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

// HDHK 8Ch current meter
void CThread::HandleHDHKDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	addr = 0x0008;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = modbus_read_registers( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewData(idx) );
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "Current device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "Current device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{	// retry
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( iChannel = 0; iChannel < m_pmyDevices->GetNumInputs(idx); iChannel++ )
			{
				// 0.5A change
				double dDiff = 0.5;

				E_EVENT_TYPE eEventType = E_ET_CURRENT;
				E_IO_TYPE eIOTypeL = E_IO_CURRENT_LOW;
				E_IO_TYPE eIOTypeH = E_IO_CURRENT_HIGH;
				E_IO_TYPE eIOTypeHL = E_IO_CURRENT_HIGHLOW;
				E_IO_TYPE eIOTypeMon = E_IO_CURRENT_MONITOR;
				char szUnits[10] = "A";
				char szDesc[20] = "Current";
				char szName[50] = "Current";
				double dValOld = m_pmyDevices->CalcCurrent(idx,iChannel,false);
				double dValNew = m_pmyDevices->CalcCurrent(idx,iChannel,true);

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

// PT113MB load cell transmitter
void CThread::HandlePT113Device( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int iChannel;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	// count=0&1, status=2 
	addr = 0x0000;
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = modbus_read_registers( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewData(idx) );
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{	// give up
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "Weight device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "Weight device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{	// retry
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			// always use channel 0
			for ( iChannel = 0; iChannel < 1 /*m_pmyDevices->GetNumInputs(idx)*/; iChannel++ )
			{
				// 10kg change
				// unloaded bridge weight / 10 ??
				double dDiff = m_pmyDevices->GetOffset(idx,iChannel) * m_pmyDevices->GetResolution(idx,iChannel) / 100;

				E_EVENT_TYPE eEventType = E_ET_WEIGHT;
				E_IO_TYPE eIOTypeL = E_IO_WEIGHT_LOW;
				E_IO_TYPE eIOTypeH = E_IO_WEIGHT_HIGH;
				E_IO_TYPE eIOTypeHL = E_IO_WEIGHT_HIGHLOW;
				E_IO_TYPE eIOTypeMon = E_IO_WEIGHT_MONITOR;
				char szUnits[10] = "Kg";
				char szDesc[20] = "Weight";
				char szName[50] = "Weight";
				double dValOld = m_pmyDevices->CalcPT113Weight(idx,iChannel,false);
				double dValNew = m_pmyDevices->CalcPT113Weight(idx,iChannel,true);

				//LogMessage( E_MSG_INFO, "Old %u %u %.1f", m_pmyDevices->GetLastData(idx,iChannel), m_pmyDevices->GetLastData(idx,iChannel+1), dValOld);

				HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );

				if ( fabs(dValNew - dValOld) >= dDiff )
				{	// find the display
					LogMessage(E_MSG_INFO, "WW %.1f %.1f %.1f", dValNew, dValOld, dDiff );
					for ( int i = 0; i < MAX_DEVICES; i++ )
					{
						if ( m_pmyDevices->GetDeviceType(i) == E_DT_CARD_READER )
						{
							int iInDeviceNo = m_pmyDevices->GetDeviceNo(i);
							int iInChannel = 0;
							int iEspIdx = -1;
							if ( (iEspIdx = m_pmyIOLinks->FindLinkedDevice( iInDeviceNo, iInChannel, E_DT_ESP_DISPLAY, m_pmyDevices )) >= 0 )
							{	
								char szEspResponseMsg[ESP_MSG_SIZE];
								if ( m_pmyDevices->GetOffset(idx,iChannel) < 1000)
									snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "WW%06.1f", dValNew );
								else
									snprintf( szEspResponseMsg, sizeof(szEspResponseMsg), "WW%06d", (int)round(dValNew) );
								SetEspResponse( m_pmyDevices->GetDeviceName(iEspIdx), szEspResponseMsg );
							}
							break;
						}
					}
				}
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

void CThread::HandleTemperatureDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead )
{
	int i;
	int rc;
	int err;
	int addr;
	int iLoop;
	int iRetry = 3;

	addr = 0;
	if ( m_pmyDevices->GetDeviceType(idx) == E_DT_TEMPERATURE_K1 )
	{
		addr = 0x20;
	}
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		rc = modbus_read_registers( ctx, addr, m_pmyDevices->GetNumInputs(idx), m_pmyDevices->GetNewData(idx) );
		if ( rc == -1 )
		{	// failed
			err = errno;
			if ( iLoop+1 >= iRetry )
			{
				if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_SUSPECT )
				{	// device has just failed
					LogMessage( E_MSG_INFO, "TEMP device '%s' (0x%x->%d) no longer connected, loop %d", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, iLoop );
					myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), 0, E_ET_DEVICE_NG, 0, "TEMP device '%s' (0x%x->%d) no longer connected", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
				}

				if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_DEAD ) )
				{
					m_pmyDevices->UpdateDeviceStatus( myDB, idx );
					PostToWebSocket( E_ET_DEVICE_NG, idx, 0, 0, true );
				}

				if ( m_pmyDevices->GetDeviceStatus( idx ) != E_DS_BURIED )
				{
					LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );
				}
			}
			else
			{
				LogMessage( E_MSG_WARN, "modbus_read_registers(%p) '%s' (0x%x->%d) failed: %s, loop %d retry", ctx, m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx, modbus_strerror(err), iLoop );

				usleep( 10000 + (10000*iLoop) );

				if ( modbus_set_slave( ctx, m_pmyDevices->GetAddress(idx) ) == -1 )
				{
					LogMessage( E_MSG_ERROR, "modbus_set_slave(%p) %d failed: %s", ctx, idx, modbus_strerror(errno) );
				}

				usleep( 10000 + (10000*iLoop) );
			}
		}
		else
		{	// success
			bAllDead = false;
			if ( iLoop > 0 )
			{
				LogMessage( E_MSG_INFO, "modbus_read_registers() retry successful, loop %d", iLoop );
			}

			if ( m_pmyDevices->GetDeviceStatus(idx) == E_DS_DEAD || m_pmyDevices->GetDeviceStatus(idx) == E_DS_BURIED )
			{
				LogMessage( E_MSG_INFO, "Device '%s' (0x%x->%d) is now alive !", m_pmyDevices->GetDeviceName(idx), m_pmyDevices->GetAddress(idx), idx );
			}

			if ( m_pmyDevices->SetDeviceStatus( idx, E_DS_ALIVE ) )
			{
				m_pmyDevices->UpdateDeviceStatus( myDB, idx );
				PostToWebSocket( E_ET_DEVICE_OK, idx, 0, 1, true );
			}

			for ( i = 0; i < m_pmyDevices->GetNumInputs(idx); i++ )
			{
				ProcessTemperatureData( myDB, idx, i );
			}	// end for loop

			// break out of retry loop
			break;
		}
	}
}

void CThread::ProcessEspTemperatureEvent( CMysql& myDB, const char* szEspName, const int iChannel, const double dValue )
{
	int idx;

	idx = m_pmyDevices->GetIdxForName( szEspName );
	if ( idx >= 0 )
	{	// ESP temperature channels are 11-14
		uint16_t uValue;
		int i = iChannel + 10;

		if ( dValue > 0 )
			uValue = dValue * 10;
		else
			uValue = (dValue * 10) + 10000;

		m_pmyDevices->GetNewData( idx, i ) = uValue;

		ProcessTemperatureData( myDB, idx, i );
	}
	else
	{
		LogMessage( E_MSG_ERROR, "Failed to find idx for '%s'", szEspName );
	}
}

void CThread::ProcessTemperatureData( CMysql& myDB, const int idx, const int iChannel )
{
	//LogMessage( E_MSG_INFO, "ProcessTemperatureData() %d %d", idx, i );

	// check for bad data
	if ( m_pmyDevices->GetNewData( idx, iChannel ) != m_pmyDevices->GetLastData( idx, iChannel ) && m_pmyDevices->IsSensorConnected(idx,iChannel) )
	{	// data has changed
		m_pmyDevices->SaveDataValue( idx, iChannel );

		bool bDeviceTypeOk = false;
		if ( m_pmyDevices->GetDeviceType(idx) == E_DT_TEMPERATURE_DS )
			bDeviceTypeOk = true;
		else if ( m_pmyDevices->GetDeviceType(idx) == E_DT_DIGITAL_IO && strncmp( m_pmyDevices->GetComPort(idx), "ESP", 3 ) == 0 )
			bDeviceTypeOk = true;

		if ( bDeviceTypeOk &&
			(m_pmyDevices->CalcTemperature(idx,iChannel,true) < -24.0 || m_pmyDevices->CalcTemperature(idx,iChannel,true) > 110.0 ||
			(fabs(m_pmyDevices->CalcTemperature(idx,iChannel,true) - m_pmyDevices->CalcTemperature(idx,iChannel,false)) >= MAX_TEMPERATURE_DIFF &&
			m_pmyDevices->GetLastData( idx, iChannel ) != (uint16_t)-1)) )
		{	// out of range - DS18B20 devices are susceptible to noise
			if ( m_pmyDevices->DataBufferIsStable(idx,iChannel) )
			{
				LogMessage( E_MSG_WARN, "Unstable temperature channel %d '%s' %u %.1f degC, %.1f degC, %u", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), m_pmyDevices->GetNewData( idx, iChannel ),
						m_pmyDevices->CalcTemperature(idx,iChannel,true), m_pmyDevices->CalcTemperature(idx,iChannel,false), m_pmyDevices->GetLastData( idx, iChannel ) );
			}
			else
			{
				if ( m_pmyDevices->GetNewData(idx,iChannel) != (uint16_t)-1 )
				{
					LogMessage( E_MSG_WARN, "Invalid temperature channel %d '%s' %u %.1f degC, %.1f degC, %u", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), m_pmyDevices->GetNewData( idx, iChannel ),
							m_pmyDevices->CalcTemperature(idx,iChannel,true), m_pmyDevices->CalcTemperature(idx,iChannel,false), m_pmyDevices->GetLastData( idx, iChannel ) );
				}

				// leave the data unchanged
				m_pmyDevices->GetNewData( idx, iChannel ) = m_pmyDevices->GetLastData( idx, iChannel );
			}
		}
	}


	//0.2 degC change = 2 units
	double dDiff = 0.2;

	E_EVENT_TYPE eEventType = E_ET_TEMPERATURE;
	E_IO_TYPE eIOTypeL = E_IO_TEMP_LOW;
	E_IO_TYPE eIOTypeH = E_IO_TEMP_HIGH;
	E_IO_TYPE eIOTypeHL = E_IO_TEMP_HIGHLOW;
	E_IO_TYPE eIOTypeMon = E_IO_TEMP_MONITOR;
	char szUnits[10] = "degC";
	char szDesc[20] = "Temperature";
	char szName[50] = "Temperature";
	double dValOld = m_pmyDevices->CalcTemperature(idx,iChannel,false);
	double dValNew = m_pmyDevices->CalcTemperature(idx,iChannel,true);

	HandleChannelThresholds( myDB, idx, iChannel, dDiff, eEventType, eIOTypeL, eIOTypeH, eIOTypeHL, eIOTypeMon, szName, szDesc, szUnits, dValNew, dValOld );
}

void CThread::HandleChannelThresholds( CMysql& myDB, const int idx, const int iChannel, const double dDiff, const E_EVENT_TYPE eEventType, const E_IO_TYPE eIOTypeL, const E_IO_TYPE eIOTypeH,
			const E_IO_TYPE eIOTypeHL, const E_IO_TYPE eIOTypeMon, const char* szName, const char* szDesc, const char* szUnits, const double dValNew, const double dValOld )
{
	bool bLogit;

	if ( eEventType == E_ET_WEIGHT )
	{
//		LogMessage( E_MSG_INFO, "val %.1f %.1f %.1f", dValNew, dValOld, dDiff );
	}
	if ( strlen(m_pmyDevices->GetInIOName(idx,iChannel)) == 0 )
	{	// no name, assume this channel is unused
		if ( eEventType == E_ET_TEMPERATURE &&  m_pmyDevices->WasSensorConnected(idx,iChannel) )
		{
			LogMessage( E_MSG_INFO, "%s sensor %d '%s' no longer connected", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel) );
			myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), iChannel, E_ET_DEVICE_NG, 0, "%s sensor %d no longer connected", szName, iChannel+1 );
		}
	}
	else if ( dValNew != dValOld )
	{	// data has changed
		if ( eEventType == E_ET_TEMPERATURE && !m_pmyDevices->WasSensorConnected(idx,iChannel) )
		{
			LogMessage( E_MSG_INFO, "%s sensor %d '%s' is connected", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel) );
			myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), iChannel, E_ET_DEVICE_OK, 0, "%s sensor %d is connected", szName, iChannel+1 );
		}

		bool bWeighBridgeStable = false;
		if (eEventType == E_ET_WEIGHT && 
			dValNew == dValOld && dValNew != m_pmyDevices->GetLastLogData(idx,iChannel) )
		{	// weight is stable
			bWeighBridgeStable = true;
		}

		bLogit = false;
		if ( fabs(dValNew - dValOld) >= dDiff ||
			m_pmyDevices->GetLastRecorded(idx,iChannel) + MAX_EVENT_PERIOD < time(NULL) ||
			bWeighBridgeStable )
		{	// interesting amount of change
			bLogit = true;

			myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), iChannel, eEventType, dValNew, "" );

			m_pmyDevices->GetLastRecorded(idx,iChannel) = time(NULL);
			m_pmyDevices->GetLastLogData(idx,iChannel) = m_pmyDevices->GetNewData(idx,iChannel);
			if ( eEventType == E_ET_WEIGHT )
			{
				m_pmyDevices->GetLastRecorded(idx,iChannel+1) = time(NULL);
				m_pmyDevices->GetLastRecorded(idx,iChannel+2) = time(NULL);
				m_pmyDevices->GetLastLogData(idx,iChannel+1) = m_pmyDevices->GetNewData(idx,iChannel+1);
				m_pmyDevices->GetLastLogData(idx,iChannel+2) = m_pmyDevices->GetNewData(idx,iChannel+2);
			}

			// post to the websocket
			PostToWebSocket( eEventType, idx, iChannel, dValNew, true );
		}

		if ( bLogit )
		{
			if ( eEventType == E_ET_ROTARY_ENC )
				LogMessage( E_MSG_INFO, "%s %d '%s' %d %d %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
					(int16_t)(m_pmyDevices->GetNewData( idx, iChannel+1 )), dValNew, szUnits );
			else if ( eEventType == E_ET_LEVEL )
				LogMessage( E_MSG_INFO, "%s %d '%s' %d %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int)m_pmyDevices->GetOffset(idx,iChannel) * (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
					dValNew, szUnits );
			else if ( eEventType == E_ET_WEIGHT )
				LogMessage( E_MSG_INFO, "%s %d '%s' %u %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (uint32_t)m_pmyDevices->GetNewData( idx, iChannel )*65536+(uint32_t)m_pmyDevices->GetNewData( idx, iChannel+1 ),
					dValNew, szUnits );
			else
				LogMessage( E_MSG_INFO, "%s %d '%s' %d %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
					dValNew, szUnits );
		}
		else
		{
			if ( eEventType == E_ET_ROTARY_ENC )
				LogMessage( E_MSG_DEBUG, "%s %d '%s' %d %d %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
					(int16_t)(m_pmyDevices->GetNewData( idx, iChannel+1 )), dValNew, szUnits );
			else if ( eEventType == E_ET_LEVEL )
				LogMessage( E_MSG_DEBUG, "%s %d '%s' %d %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int)m_pmyDevices->GetOffset(idx,iChannel) * (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
					dValNew, szUnits );
			else if ( eEventType == E_ET_WEIGHT )
				LogMessage( E_MSG_DEBUG, "%s %d '%s' %u %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (uint32_t)m_pmyDevices->GetNewData( idx, iChannel )*65536+(uint32_t)m_pmyDevices->GetNewData( idx, iChannel+1 ),
					dValNew, szUnits );
			else
				LogMessage( E_MSG_DEBUG, "%s %d '%s' %d %.1f %s", szName, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
					dValNew, szUnits );
		}

		if ( m_pmyDevices->GetInChannelType(idx,iChannel) == eIOTypeH || m_pmyDevices->GetInChannelType(idx,iChannel) == eIOTypeHL )
		{
			if ( dValNew > dValOld &&
					dValNew >= m_pmyDevices->GetMonitorValueHi(idx,iChannel) &&
					dValOld < m_pmyDevices->GetMonitorValueHi(idx,iChannel) )
			{	// increasing and high trigger reached
				if ( !m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
				{	// trigger level reached
					LogMessage( E_MSG_INFO, "Channel %d '%s' High Alarm %s %s %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
						szName, szDesc, m_pmyDevices->GetMonitorValueHi(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );

					m_pmyDevices->GetAlarmTriggered(idx,iChannel) = true;

					ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, true, eEventType );
				}
				else
				{
					LogMessage( E_MSG_INFO, "Channel %d '%s' %s %s %.1f %s on device %d alarm already reached", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
														szName, szDesc, m_pmyDevices->GetMonitorValueHi(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );
				}
			}
			else if ( dValNew < dValOld &&
					dValNew < m_pmyDevices->GetMonitorValueHi(idx,iChannel) - m_pmyDevices->GetHysteresis(idx,iChannel) &&
					dValOld >= m_pmyDevices->GetMonitorValueHi(idx,iChannel) - m_pmyDevices->GetHysteresis(idx,iChannel) &&
					m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
			{	// decreasing and hysteresis reached
				LogMessage( E_MSG_INFO, "Channel %d '%s' High Alarm %s %s Hysteresis %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
						szName, szDesc, m_pmyDevices->GetMonitorValueHi(idx,iChannel) - m_pmyDevices->GetHysteresis(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );

				m_pmyDevices->GetAlarmTriggered(idx,iChannel) = false;

				ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, false, eEventType );
			}
		}

		if ( m_pmyDevices->GetInChannelType(idx,iChannel) == eIOTypeL || m_pmyDevices->GetInChannelType(idx,iChannel) == eIOTypeHL )
		{
			if ( dValNew < dValOld &&
					dValNew <= m_pmyDevices->GetMonitorValueLo(idx,iChannel) &&
					dValOld > m_pmyDevices->GetMonitorValueLo(idx,iChannel) )
			{	// decreasing and low trigger reached
				if ( !m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
				{	// low distance trigger reached
					LogMessage( E_MSG_INFO, "Channel %d '%s' Low Alarm %s %s %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
						szName, szDesc, m_pmyDevices->GetMonitorValueLo(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );

					m_pmyDevices->GetAlarmTriggered(idx,iChannel) = true;

					ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, true, eEventType );
				}
				else
				{
					LogMessage( E_MSG_INFO, "Channel %d '%s' Low %s %s %.1f %s on device %d alarm already reached", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
														szName, szDesc, m_pmyDevices->GetMonitorValueLo(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );
				}
			}
			else if ( dValNew > dValOld &&
					dValNew > m_pmyDevices->GetMonitorValueLo(idx,iChannel) + m_pmyDevices->GetHysteresis(idx,iChannel) &&
					dValOld <= m_pmyDevices->GetMonitorValueLo(idx,iChannel) + m_pmyDevices->GetHysteresis(idx,iChannel) &&
					m_pmyDevices->GetAlarmTriggered(idx,iChannel) )
			{	// increasing and hysteresis reached
				LogMessage( E_MSG_INFO, "Channel %d '%s' Low Alarm %s %s Hysteresis %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
						szName, szDesc, m_pmyDevices->GetMonitorValueLo(idx,iChannel) + m_pmyDevices->GetHysteresis(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );

				m_pmyDevices->GetAlarmTriggered(idx,iChannel) = false;

				ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, false, eEventType );
			}
		}

		// Monitor devices are used in io link conditions
		if ( m_pmyDevices->GetInChannelType(idx,iChannel) == eIOTypeMon || m_pmyDevices->GetInChannelType(idx,iChannel) == eIOTypeMon )
		{
			bool bTrigger;
			double dValue = 0.0;
			int iFoundIdx = 0;
			int iIolIdx = 0;
			char szLinkTest[6];

			// TODO: handle TimeOfDay
			
			while ( (iFoundIdx = GetConditionValue( iIolIdx, idx, iChannel, dValue, szLinkTest, sizeof(szLinkTest) )) >= 0 )
			{
				bTrigger = false;
				if ( dValNew > dValOld )
				{	// increasing value
					if ( strcmp( szLinkTest, "GE" ) == 0 && dValNew >= dValue && dValOld < dValue )
					{	// increasing and high trigger reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "GT" ) == 0 && dValNew > dValue && dValOld <= dValue )
					{	// increasing and high trigger reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "EQ" ) == 0 && dValNew == dValue && dValOld < dValue )
					{	// increasing and high trigger reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "LE" ) == 0 && dValNew < dValue && dValOld >= dValue )
					{	// increasing and low hysteresis reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "LT" ) == 0 && dValNew <= dValue && dValOld > dValue )
					{	// increasing and low hysteresis reached
						bTrigger = true;
					}

					if ( bTrigger )
					{
						if ( !m_pmyDevices->GetConditionTriggered(idx,iChannel) && (strcmp( szLinkTest, "GE" ) == 0 || strcmp( szLinkTest, "GT" ) == 0 || strcmp( szLinkTest, "EQ" ) == 0) )
						{	// trigger level reached
							LogMessage( E_MSG_INFO, "Channel %d '%s' High Condition %s %s %s %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
								szLinkTest, szName, szDesc, dValue, szUnits, m_pmyDevices->GetAddress(idx) );

							if ( ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, true, eEventType ) )
							{
								m_pmyDevices->GetConditionTriggered(idx,iChannel) = true;
							}
						}
						else if ( !m_pmyDevices->GetConditionTriggered(idx,iChannel) && (strcmp( szLinkTest, "LE" ) == 0 || strcmp( szLinkTest, "LT" ) == 0))
						{	// condition now finished
							LogMessage( E_MSG_INFO, "Channel %d '%s' Low Condition %s %s %s Hysteresis %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
									szLinkTest, szName, szDesc, dValue - m_pmyDevices->GetHysteresis(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );

							m_pmyDevices->GetConditionTriggered(idx,iChannel) = false;

							//ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, false, eEventType );
						}
					}
				}
				else if ( dValNew < dValOld )
				{	// decreasing value
					if ( strcmp( szLinkTest, "GE" ) == 0 && dValNew < dValue && dValOld >= dValue )
					{	// decreasing and hysteresis reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "GT" ) == 0 && dValNew <= dValue && dValOld > dValue )
					{	// decreasing and hysteresis reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "EQ" ) == 0 && dValNew < dValue && dValOld == dValue )
					{	// decreasing and hysteresis reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "LE" ) == 0 && dValNew <= dValue && dValOld > dValue )
					{	// decreasing and low trigger reached
						bTrigger = true;
					}
					else if ( strcmp( szLinkTest, "LT" ) == 0 && dValNew < dValue && dValOld >= dValue )
					{	// decreasing and low trigger reached
						bTrigger = true;
					}

					if ( bTrigger )
					{
						if ( m_pmyDevices->GetConditionTriggered(idx,iChannel) && (strcmp( szLinkTest, "GE" ) == 0 || strcmp( szLinkTest, "GT" ) == 0 || strcmp( szLinkTest, "EQ" ) == 0))
						{	// condition now finished
							LogMessage( E_MSG_INFO, "Channel %d '%s' High Condition %s %s %s Hysteresis %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
									szLinkTest, szName, szDesc, dValue - m_pmyDevices->GetHysteresis(idx,iChannel), szUnits, m_pmyDevices->GetAddress(idx) );

							m_pmyDevices->GetConditionTriggered(idx,iChannel) = false;

							//ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, false, eEventType );
						}
						else if ( !m_pmyDevices->GetConditionTriggered(idx,iChannel) && (strcmp( szLinkTest, "LE" ) == 0 || strcmp( szLinkTest, "LT" ) == 0))
						{	// trigger level reached
							LogMessage( E_MSG_INFO, "Channel %d '%s' Low Condition %s %s %s %.1f %s on device %d", iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel),
								szLinkTest, szName, szDesc, dValue, szUnits, m_pmyDevices->GetAddress(idx) );

							if ( ChangeOutput( myDB, m_pmyDevices->GetAddress(idx), iChannel, true, eEventType ) )
							{
								m_pmyDevices->GetConditionTriggered(idx,iChannel) = true;
							}
						}
					}
				}

				iIolIdx = iFoundIdx+1;
			}
		}

		// check if this event should be passed onto the plc handling
		if ( bLogit && m_pmyPlcStates->FindInputDevice( m_pmyDevices->GetDeviceNo(idx), iChannel ) )
		{	// only pass the switch down event not the release
			m_pmyPlcStates->AddInputEvent( m_pmyDevices->GetDeviceNo(idx), iChannel, (int)(dValNew*10) );
		}
	}
	else if ( m_pmyDevices->GetLastRecorded(idx,iChannel) + MAX_EVENT_PERIOD < time(NULL) )
	{	// record data every 5 minutes regardless of change
		myDB.LogEvent( m_pmyDevices->GetDeviceNo(idx), iChannel, eEventType, dValNew, "" );

		m_pmyDevices->GetLastRecorded(idx,iChannel) = time(NULL);
		m_pmyDevices->GetLastLogData(idx,iChannel) = m_pmyDevices->GetNewData(idx,iChannel);
		if ( eEventType == E_ET_WEIGHT )
		{
			m_pmyDevices->GetLastRecorded(idx,iChannel+1) = time(NULL);
			m_pmyDevices->GetLastRecorded(idx,iChannel+2) = time(NULL);
			m_pmyDevices->GetLastLogData(idx,iChannel+1) = m_pmyDevices->GetNewData(idx,iChannel+1);
			m_pmyDevices->GetLastLogData(idx,iChannel+2) = m_pmyDevices->GetNewData(idx,iChannel+2);
		}

		if ( eEventType == E_ET_ROTARY_ENC )
			LogMessage( E_MSG_INFO, "%s %s %d '%s' %d %d %.1f %s", szName, szDesc, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
				(int16_t)(m_pmyDevices->GetNewData( idx, iChannel+1 )), dValNew, szUnits );
		else if ( eEventType == E_ET_LEVEL )
			LogMessage( E_MSG_INFO, "%s %s %d '%s' %d %.1f %s", szName, szDesc, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int)m_pmyDevices->GetOffset(idx,iChannel) * (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
				dValNew, szUnits );
		else if ( eEventType == E_ET_WEIGHT )
			LogMessage( E_MSG_INFO, "%s %s %d '%s' %d %.1f %s", szName, szDesc, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )*65536+m_pmyDevices->GetNewData( idx, iChannel+1 )),
				dValNew, szUnits );
		else
			LogMessage( E_MSG_INFO, "%s %s %d '%s' %d %.1f %s", szName, szDesc, iChannel+1, m_pmyDevices->GetInIOName(idx,iChannel), (int16_t)(m_pmyDevices->GetNewData( idx, iChannel )),
				dValNew, szUnits );

		PostToWebSocket( eEventType, idx, iChannel, dValNew, true );
	}

	//LogMessage( E_MSG_INFO, "SetLastData %d %d %.1f", idx, iChannel, m_pmyDevices->GetNewData(idx,iChannel) );
	m_pmyDevices->GetLastData(idx,iChannel) = m_pmyDevices->GetNewData(idx,iChannel);
	if ( eEventType == E_ET_WEIGHT )
	{
		m_pmyDevices->GetLastData(idx,iChannel+1) = m_pmyDevices->GetNewData(idx,iChannel+1);
		m_pmyDevices->GetLastData(idx,iChannel+2) = m_pmyDevices->GetNewData(idx,iChannel+2);
	}
}

// device can be used more than once
const int CThread::GetConditionValue( const int iIolIdx, const int idx, const int iInChannel, double& dValue, char* szLinkTest, size_t uLinkTestLen )
{
	int iFoundIdx = -1;
	int i;
	int j;
	int iInDeviceNo;
	
	dValue = 0.0;
	szLinkTest[0] = '\0';

	iInDeviceNo = m_pmyDevices->GetDeviceNo( idx );

	// check if this device/channel is used in an iolink condition
	for ( i = iIolIdx; i < MAX_IO_LINKS; i++ )
	{
		if ( m_pmyIOLinks->GetInDeviceNo(i) == iInDeviceNo && m_pmyIOLinks->GetInChannel(i) == iInChannel )
		{
			for ( j = 0; j < MAX_CONDITIONS; j++ )
			{
				if ( m_pmyIOLinks->GetLinkDeviceNo(i,j) == iInDeviceNo && m_pmyIOLinks->GetLinkChannel(i,j) == iInChannel )
				{
					iFoundIdx = i;
					dValue = m_pmyIOLinks->GetLinkValue(i,j);
					snprintf( szLinkTest, uLinkTestLen, "%s", m_pmyIOLinks->GetLinkTest(i,j));
					//LogMessage( E_MSG_INFO, "GetConditionValue() for %s %d,%d %.1f", m_pmyDevices->GetInIOName(idx,j), iInDeviceNo, iInChannel+1, dValue );
					break;
				}
				else if ( m_pmyIOLinks->GetLinkDeviceNo(i,j) == 0 )
				{	// end of list
					break;
				}
			}
			break;
		}
		else if ( m_pmyIOLinks->GetInDeviceNo(i) == 0 )
		{	// end of list
			break;
		}
	}
	
	return iFoundIdx;
}

void CThread::PostToWebSocket( const enum E_EVENT_TYPE& eEventType, const int idx, const int iChannel, const double dValNew, const bool bInput )
{
	int iPrec = 1;
	int iDeviceNo;
	char szMsg[100];
	char szType[4] = "";

	iDeviceNo = m_pmyDevices->GetDeviceNo(idx);
	switch ( eEventType )
	{
	default:
		break;
	case E_ET_VOLTAGE:
		snprintf( szType, sizeof(szType), "VV%s", (bInput ? "I" : "O"));
		if ( dValNew > 99 )
			iPrec = 0;
		break;
	case E_ET_CURRENT:
		iPrec = 2;
		snprintf( szType, sizeof(szType), "VV%s", (bInput ? "I" : "O"));
		break;
	case E_ET_FREQUENCY:
		iPrec = 2;
		snprintf( szType, sizeof(szType), "VV%s", (bInput ? "I" : "O"));
		break;
	case E_ET_POWER:
		snprintf( szType, sizeof(szType), "VV%s", (bInput ? "I" : "O"));
		break;
	case E_ET_TORQUE:
		snprintf( szType, sizeof(szType), "VV%s", (bInput ? "I" : "O"));
		break;
	case E_ET_TEMPERATURE:
		snprintf( szType, sizeof(szType), "TT%s", (bInput ? "I" : "O"));
		break;
	case E_ET_HUMIDITY:
		snprintf( szType, sizeof(szType), "TT%s", (bInput ? "I" : "O"));
		break;
	case E_ET_LEVEL:
	case E_ET_WEIGHT:
		snprintf( szType, sizeof(szType), "LL%s", (bInput ? "I" : "O"));
		break;
	case E_ET_DEVICE_OK:
		iPrec = 0;
		snprintf( szType, sizeof(szType), "STT" );
		break;
	case E_ET_DEVICE_NG:
		iPrec = 0;
		snprintf( szType, sizeof(szType), "STT" );
		break;
	case E_ET_CERTIFICATENG:
		iPrec = 0;
		iDeviceNo = 0;
		snprintf( szType, sizeof(szType), "CNG" );
		break;
	case E_ET_CERTIFICATEAG:
		iPrec = 0;
		iDeviceNo = 0;
		snprintf( szType, sizeof(szType), "CAG" );
		break;
	}
	if ( strlen(szType) > 0 )
	{
		snprintf( szMsg, sizeof(szMsg), "%s_%02d_%02d:%.*f", szType, iDeviceNo, iChannel, iPrec, dValNew );
		gThreadMsgToWS.PutMessage( szMsg );
	}
	else
	{
		LogMessage( E_MSG_INFO, "Unhandled websocket msg for E_ET type %d", eEventType);
	}
}

bool CThread::ChangeOutput( CMysql& myDB, const int iInAddress, const int iInChannel, const uint8_t uState, const enum E_EVENT_TYPE eEvent )
{
	bool bRc = false;
	bool bInvertState = false;
	bool bLinkTestPassed = false;
	int idx;
	int count = 0;
	int iOutAddress = 0;
	int iOutChannel = 0;
	int iOutIdx;
	int iOutOnPeriod = 0;
	int iInIdx;
	int iInDeviceNo;
	int iOutDeviceNo;
	double dOutVsdFrequency = 0.0;
	uint8_t uLinkState;
	char szOutDeviceName[MAX_DEVICE_NAME_LEN+1];
	char szOutHostname[HOST_NAME_MAX+1] = "";
	enum E_IO_TYPE eSwType = E_IO_UNUSED;

	iInIdx = m_pmyDevices->GetIdxForAddr(iInAddress);
	eSwType = m_pmyDevices->GetInChannelType( iInIdx, iInChannel );

	iInDeviceNo = m_pmyDevices->GetDeviceNoForAddr(iInAddress);

	//LogMessage( E_MSG_INFO, "ChangeOutput %d %d %d %d", iInIdx, eSwType, iInAddress, iInDeviceNo );

	idx = 0;
	while ( m_pmyIOLinks->Find( iInDeviceNo, iInChannel, idx, iOutDeviceNo, iOutChannel, iOutOnPeriod, dOutVsdFrequency, bLinkTestPassed, bInvertState, m_pmyDevices ) )
	{	// loop through all linked devices/channels
		count += 1;

		bInvertState = false;
		uLinkState = uState;
		iOutAddress = m_pmyDevices->GetAddressForDeviceNo( iOutDeviceNo );
		iOutIdx = m_pmyDevices->GetIdxForAddr(iOutAddress);

		strcpy( szOutHostname, m_pmyDevices->GetDeviceHostname( iOutIdx ) );
		strcpy( szOutDeviceName, m_pmyDevices->GetDeviceName( iOutIdx ) );

		if ( bInvertState )
			uLinkState = (uState ? false : true);

		if ( !bLinkTestPassed )
		{
			LogMessage( E_MSG_INFO, "Skipped output change for '%s' (0x%x->%d,%d), new state %u (%d,%.1f)", m_pmyDevices->GetOutIOName(iOutIdx, iOutChannel),
					iOutAddress, iOutIdx, iOutChannel+1, uLinkState, iOutOnPeriod, dOutVsdFrequency );

			continue;
		}

		LogMessage( E_MSG_INFO, "Output change for '%s' (0x%x->%d,%d), new state %u (%d,%.1f), on %s", m_pmyDevices->GetOutIOName(iOutIdx, iOutChannel),
				iOutAddress, iOutIdx, iOutChannel+1, uLinkState, iOutOnPeriod, dOutVsdFrequency, szOutHostname );

		if ( iOutIdx >= 0 )
		{
			bRc = true;
			if ( !IsMyHostname( szOutHostname ) )
			{	// output device is on another host
				SendTcpipChangeOutputToHost( szOutHostname, iInIdx, iInAddress, iInChannel, iOutIdx, iOutAddress, iOutChannel, uLinkState, eSwType, iOutOnPeriod, dOutVsdFrequency);
			}
			else
			{
				ChangeOutputState( myDB, iInIdx, iInAddress, iInChannel, iOutIdx, iOutAddress, iOutChannel, uLinkState, eSwType, iOutOnPeriod, dOutVsdFrequency );

				if ( m_szEspResponseMsg[0] != '\0' && !IsTcpipThread() )
				{	// send msg to esp device via the tcpip thread
					pthread_mutex_lock( &mutexLock[E_LT_NODEESP] );

					m_pmyDevices->SetEspResponseMsg( szOutDeviceName, m_szEspResponseMsg );

					pthread_mutex_unlock( &mutexLock[E_LT_NODEESP] );

					m_szEspResponseMsg[0] = '\0';
				}
			}
		}
		else
		{
			LogMessage( E_MSG_ERROR, "GetIdxForAddr() failed for out address %d, no such device", iOutAddress );
		}

		usleep( 10000 );	// was 20000
	}

	return bRc;
}

void CThread::SetEspResponse( const char* szOutDeviceName, const char* szEspResponseMsg )
{
	LogMessage( E_MSG_INFO, "set ESP msg '%s'", szEspResponseMsg );

	pthread_mutex_lock( &mutexLock[E_LT_NODEESP] );

	m_pmyDevices->SetEspResponseMsg( szOutDeviceName, szEspResponseMsg );

	pthread_mutex_unlock( &mutexLock[E_LT_NODEESP] );
}

void CThread::ChangeOutputState( CMysql& myDB, const int iInIdx, const int iInAddress, const int iInChannel, const int iOutIdx, const int iOutAddress, const int iOutChannel,
		const uint8_t uState, const enum E_IO_TYPE eSwType, int iOutOnPeriod, const double dOutVsdFrequency )
{
	bool bIsEsp = false;
	bool bState;
	bool bError = false;
	int iLoop;
	int iRetry = 3;
	int iInOnPeriod;
	time_t tStart;

	bIsEsp = m_pmyDevices->IsEspDevice(iOutIdx);

	LogMessage( E_MSG_INFO, "ChangeOutputState: esp=%d", bIsEsp );
	for ( iLoop = 0; iLoop < iRetry; iLoop++ )
	{
		bError = false;
		if ( bIsEsp )
		{
		}
		else if ( modbus_set_slave( m_pmyDevices->GetContext(iOutIdx), iOutAddress ) == -1 )
		{
			bError = true;
			LogMessage( E_MSG_ERROR, "modbus_set_slave() failed, %d %d, %s", iOutIdx, iOutAddress, modbus_strerror(errno) );
		}

		if ( !bError )
		{	// change the output state
			usleep( 10000 );
			if ( uState )
			{	// new state is ON
				switch ( eSwType )
				{
				default:
				case E_IO_UNUSED:
					LogMessage( E_MSG_WARN, "Switch type is UNUSED (%d) for addr %d and switch %d, state %u", eSwType, iInAddress, iInChannel+1, uState );
					break;

				case E_IO_ON_OFF:
				case E_IO_ON_OFF_INV:
				case E_IO_TEMP_HIGH:
				case E_IO_TEMP_LOW:
				case E_IO_TEMP_HIGHLOW:
				case E_IO_TEMP_MONITOR:
				case E_IO_VOLT_HIGH:
				case E_IO_VOLT_LOW:
				case E_IO_VOLT_HIGHLOW:
				case E_IO_VOLT_MONITOR:
				case E_IO_LEVEL_HIGH:
				case E_IO_LEVEL_LOW:
				case E_IO_LEVEL_HIGHLOW:
				case E_IO_LEVEL_MONITOR:
				case E_IO_ROTENC_HIGH:
				case E_IO_ROTENC_LOW:
				case E_IO_ROTENC_HIGHLOW:
				case E_IO_ROTENC_MONITOR:
					if ( !bIsEsp )
					{
						switch ( m_pmyDevices->GetDeviceType(iOutIdx) )
						{
						case E_DT_VSD_NFLIXEN:
						case E_DT_VSD_PWRELECT:
							HandleVSDOutputDevice( myDB, iOutIdx, iOutChannel, dOutVsdFrequency );
							break;
						default:
							if ( !m_pmyDevices->WriteOutputBit( iOutIdx, iOutChannel, uState ) )
								bError = true;
							break;
						}
					}
					else
					{	// format the response message to the esp device
						snprintf( m_szEspResponseMsg, sizeof(m_szEspResponseMsg), "OK%d%05d", iOutChannel+1, (uState ? iOutOnPeriod : 0) );
					}

					if ( !bError )
					{	// success
						LogMessage( E_MSG_INFO, "Output state set to %d, period %d sec, %.1f Hz", uState, iOutOnPeriod, dOutVsdFrequency );

						if ( m_pmyDevices->GetOutOnStartTime(iOutIdx,iOutChannel) == 0 )
						{
							m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, time(NULL) );
						}

						m_pmyDevices->SetOutOnPeriod( iOutIdx, iOutChannel, iOutOnPeriod );

						m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, iOutIdx, iOutChannel );
					}
					break;

				case E_IO_ON_TIMER:
					iInOnPeriod = m_pmyDevices->GetInOnPeriod( iInIdx, iInChannel );
					if ( iOutOnPeriod == 0 )
					{
						iOutOnPeriod = iInOnPeriod;
					}

					if ( m_pmyDevices->GetOutOnStartTime( iOutIdx, iOutChannel ) == 0 )
					{	// output is currently off, so turn it on
						tStart = time(NULL);
					}
					else
					{	// output is currently on, restart the timer
						tStart = time(NULL);
					}

					if ( m_pmyDevices->GetOutOnPeriod( iOutIdx, iOutChannel ) > iOutOnPeriod )
					{	// output was already on for longer than this trigger, keep the longer time
						LogMessage( E_MSG_INFO, "Output on period adjusted from %d to %d seconds", iOutOnPeriod, m_pmyDevices->GetOutOnPeriod( iOutIdx, iOutChannel ) );
						iOutOnPeriod = m_pmyDevices->GetOutOnPeriod( iOutIdx, iOutChannel );
					}

					if ( iOutOnPeriod == 0 )
					{
						LogMessage( E_MSG_WARN, "OutOnPeriod not set for device %d output %d (%d,%d)", iOutIdx, iOutChannel, iInOnPeriod, iOutOnPeriod );

						m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, iOutIdx, iOutChannel );
					}
					else
					{
						if ( !bIsEsp )
						{
							switch ( m_pmyDevices->GetDeviceType(iOutIdx) )
							{
							case E_DT_VSD_NFLIXEN:
							case E_DT_VSD_PWRELECT:
								HandleVSDOutputDevice( myDB, iOutIdx, iOutChannel, dOutVsdFrequency );
								break;
							default:
								if ( !m_pmyDevices->WriteOutputBit( iOutIdx, iOutChannel, true ) )
									bError = true;
								break;
							}
						}
						else
						{	// format the response message to the esp device
							snprintf( m_szEspResponseMsg, sizeof(m_szEspResponseMsg), "OK%d%05d", iOutChannel+1, (uState ? iOutOnPeriod : 0) );
						}

						if ( !bError )
						{	// success
							LogMessage( E_MSG_INFO, "Output state set to %d, period %d sec, %.1f Hz (%d,%d)", true, iOutOnPeriod, dOutVsdFrequency, iOutIdx, iOutChannel );

							m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, tStart );
							m_pmyDevices->SetOutOnPeriod( iOutIdx, iOutChannel, iOutOnPeriod );

							m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, iOutIdx, iOutChannel );
						}
					}
					break;

				case E_IO_TOGGLE:
					// TODO: handle the case where a nimrod host has missed the previous toggle message to prevent the nodes getting out of sync

					if ( m_pmyDevices->GetOutOnStartTime( iOutIdx, iOutChannel ) == 0 )
					{	// output is currently off, so turn it on
						bState = true;
					}
					else
					{	// output is currently on, so turn if off
						bState = false;
					}

					if ( !bIsEsp )
					{
						switch ( m_pmyDevices->GetDeviceType(iOutIdx) )
						{
						case E_DT_VSD_NFLIXEN:
						case E_DT_VSD_PWRELECT:
							HandleVSDOutputDevice( myDB, iOutIdx, iOutChannel, (bState ? dOutVsdFrequency : 0.0) );
							break;
						default:
							if ( !m_pmyDevices->WriteOutputBit( iOutIdx, iOutChannel, bState ) )
								bError = true;
							break;
						}
					}
					else
					{	// format the response message to the esp device
						int iPeriod = (bState ? iOutOnPeriod : 0);
						if ( bState && iOutOnPeriod == 0 )
							iPeriod = -1;
						snprintf( m_szEspResponseMsg, sizeof(m_szEspResponseMsg), "OK%d%05d", iOutChannel+1, iPeriod );
					}

					if ( !bError )
					{	// success
						LogMessage( E_MSG_INFO, "Output state set to %d, period %d sec, %.1f Hz", bState, iOutOnPeriod, dOutVsdFrequency );

						if ( bState )
						{
							m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, time(NULL) );
						}
						else
						{
							m_pmyDevices->LogOnTime( iOutIdx, iOutChannel );
							m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, 0 );
						}

						m_pmyDevices->SetOutOnPeriod( iOutIdx, iOutChannel, iOutOnPeriod );

						m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, iOutIdx, iOutChannel );
					}
					break;

				case E_IO_ON_OFF_TIMER:
					iInOnPeriod = m_pmyDevices->GetInOnPeriod( iInIdx, iInChannel );
					if ( iOutOnPeriod == 0 )
					{
						iOutOnPeriod = iInOnPeriod;
					}
					if ( m_pmyDevices->GetOutOnStartTime( iOutIdx, iOutChannel ) == 0 )
					{	// output is currently off, so turn it on
						bState = true;
					}
					else
					{	// output is currently on, turn it off
						bState = false;
					}

					if ( !bIsEsp )
					{
						switch ( m_pmyDevices->GetDeviceType(iOutIdx) )
						{
						case E_DT_VSD_NFLIXEN:
						case E_DT_VSD_PWRELECT:
							HandleVSDOutputDevice( myDB, iOutIdx, iOutChannel, (bState ? dOutVsdFrequency : 0.0) );
							break;
						default:
							if ( !m_pmyDevices->WriteOutputBit( iOutIdx, iOutChannel, bState ) )
								bError = true;
							break;
						}
					}
					else
					{	// format the response message to the esp device
						snprintf( m_szEspResponseMsg, sizeof(m_szEspResponseMsg), "OK%d%05d", iOutChannel+1, (bState ? iOutOnPeriod : 0) );
					}

					if ( !bError )
					{
						LogMessage( E_MSG_INFO, "Output set to %d, period %d sec, %.1f Hz", bState, iOutOnPeriod, dOutVsdFrequency );

						if ( bState )
						{	// turn on
							m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, time(NULL) );
							m_pmyDevices->SetOutOnPeriod( iOutIdx, iOutChannel, iOutOnPeriod );
						}
						else
						{	// turn off
							m_pmyDevices->LogOnTime( iOutIdx, iOutChannel );
							m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, 0 );
							m_pmyDevices->SetOutOnPeriod( iOutIdx, iOutChannel, 0 );
						}

						m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, iOutIdx, iOutChannel );
					}
					break;
				}
			}
			else
			{	// new state is OFF
				switch ( eSwType )
				{
				default:
				case E_IO_UNUSED:
					LogMessage( E_MSG_WARN, "Switch type is UNUSED (%d) for addr %d and switch %d, state %u", eSwType, iInAddress, iInChannel+1, uState );
					break;

				case E_IO_ON_OFF:
				case E_IO_ON_OFF_INV:
				case E_IO_TEMP_HIGH:
				case E_IO_TEMP_LOW:
				case E_IO_TEMP_HIGHLOW:
				case E_IO_TEMP_MONITOR:
				case E_IO_VOLT_HIGH:
				case E_IO_VOLT_LOW:
				case E_IO_VOLT_HIGHLOW:
				case E_IO_VOLT_MONITOR:
				case E_IO_LEVEL_HIGH:
				case E_IO_LEVEL_LOW:
				case E_IO_LEVEL_HIGHLOW:
				case E_IO_LEVEL_MONITOR:
				case E_IO_ROTENC_HIGH:
				case E_IO_ROTENC_LOW:
				case E_IO_ROTENC_HIGHLOW:
				case E_IO_ROTENC_MONITOR:
					if ( !bIsEsp )
					{
						switch ( m_pmyDevices->GetDeviceType(iOutIdx) )
						{
						case E_DT_VSD_NFLIXEN:
						case E_DT_VSD_PWRELECT:
							HandleVSDOutputDevice( myDB, iOutIdx, iOutChannel, 0.0 );
							break;
						default:
							if ( !m_pmyDevices->WriteOutputBit( iOutIdx, iOutChannel, uState ) )
								bError = true;
							break;
						}
					}
					else
					{	// format the response message to the esp device
						snprintf( m_szEspResponseMsg, sizeof(m_szEspResponseMsg), "OK%d%05d", iOutChannel+1, (uState ? iOutOnPeriod : 0) );
					}

					if ( !bError )
					{	// success
						LogMessage( E_MSG_INFO, "Output set to %d", uState );

						m_pmyDevices->LogOnTime( iOutIdx, iOutChannel );
						m_pmyDevices->SetOutOnStartTime( iOutIdx, iOutChannel, 0 );

						m_pmyDevices->UpdateDeviceOutOnStartTime( myDB, iOutIdx, iOutChannel );
					}
					break;

				case E_IO_ON_TIMER:
					// nothing to do here, state is only changed on button press not release
					LogMessage( E_MSG_INFO, "E_IO_ON_TIMER off event ignored" );
					break;

				case E_IO_TOGGLE:
					// nothing to do here, state is only changed on button press not release
					LogMessage( E_MSG_INFO, "E_IO_TOGGLE off event ignored" );
					break;

				case E_IO_ON_OFF_TIMER:
					// turn off the output
					LogMessage( E_MSG_INFO, "E_IO_ON_OFF_TIMER off event ignored" );
					break;
				}
			}
		}

		if ( bError )
		{	// error
			LogMessage( E_MSG_WARN, "Retrying ChangeOutputState(), loop %d", iLoop );
			usleep( 10000 + (10000*iLoop) );
		}
		else
		{	// success, all done
			break;
		}
	}

}

const bool CThread::WebClickExists( CMysql& myDB, const int iDeviceNo, const int iChannel )
{
	bool bRc = false;

	if ( myDB.WebClickEvent(iDeviceNo, iChannel ) )
	{
		bRc = true;
	}

	return bRc;
}

void CThread::GetCameraSnapshots( CMysql& myDB, CCameraList& CameraList, const char* szSaveDir, std::string& sAttachment )
{
	if ( m_tLastCameraSnapshot + CAMERA_SNAPSHOT_PERIOD < time(NULL) || szSaveDir != NULL )
	{
		if ( CameraList.GetNumCameras() > 0 )
		{
			int rc;
			char szParms[256];
			char szCmd[1024];
			char szOutput[256];
			char szOutput2[256];
			char szOutParms[256+20];
			char szPwd[100];
			
			if ( szSaveDir != NULL )
				snprintf( szOutput, sizeof(szOutput), "%s/%s.jpg", szSaveDir, CameraList.GetSnapshotCamera().GetName() );
			else
				snprintf( szOutput, sizeof(szOutput), "%s/latest_snapshot.jpg", CameraList.GetSnapshotCamera().GetDirectory() );
			snprintf( szOutput2, sizeof(szOutput), "%s/file_count.txt", CameraList.GetSnapshotCamera().GetDirectory() );

			sAttachment += " -A ";
			sAttachment +=szOutput;

			CameraList.GetSnapshotCamera().GetPassword( szPwd, sizeof(szPwd));
			snprintf( szParms, sizeof(szParms), "cmd=snapPicture2&usr=%s&pwd=%s", CameraList.GetSnapshotCamera().GetUserId(), szPwd );

			urlEncode( szParms, szOutParms );
			//LogMessage( E_MSG_INFO, "Params [%s]", szOutParms );
			snprintf( szCmd, sizeof(szCmd), "curl --silent --connect-timeout 2 --max-time 2 \"http://%s:88/cgi-bin/CGIProxy.fcgi?%s\" -o %s",
					CameraList.GetSnapshotCamera().GetIPAddress(), szOutParms, szOutput );

			//LogMessage( E_MSG_INFO, "cmd [%s]", szCmd );
			char szFile[100];

			snprintf( szFile, sizeof(szFile), "/tmp/cam%d.sh", CameraList.GetSnapshotCamera().GetCameraNo() );
			FILE* fp = fopen( szFile, "wt" );
			if ( fp != NULL )
			{
				fputs( "#!/bin/bash\n", fp );
				fputs( szCmd, fp );
				fputs( "\n", fp );
				fputs( "chmod 0666 ", fp );
				fputs( szOutput, fp );
				fputs( "\n", fp );
				if ( szSaveDir == NULL )
				{
					snprintf( szCmd, sizeof(szCmd), "find %s -daystart -mtime 0 -name \"MDalarm*.mkv\" | wc -l > %s\n", CameraList.GetSnapshotCamera().GetDirectory(), szOutput2 );
					fputs( szCmd, fp );
				}

				fclose( fp );
			}
			else
			{
				LogMessage( E_MSG_ERROR, "Failed to write cam file '%s', errno %d", szFile, errno );
			}

			snprintf( szCmd, sizeof(szCmd), "bash %s", szFile );
			//LogMessage( E_MSG_DEBUG, "%s", szCmd );
			rc = system( szCmd );
			if ( rc != 0 )
			{
				LogMessage( E_MSG_WARN, "system returned %d", rc );
			}

			CameraList.NextSnapshotIdx();
			if ( CameraList.GetSnapshotIdx() == 0 )
			{	// all cameras done, take a break
				m_tLastCameraSnapshot = time(NULL);

				ReadCameraRecords( myDB, CameraList );
			}
		}
		else
		{
			m_tLastCameraSnapshot = time(NULL);
			ReadCameraRecords( myDB, CameraList );
		}
	}
}

void CThread::ReadCameraRecords( CMysql& myDB, CCameraList& CameraList )
{
	int i;
	int iNumFields;
	MYSQL_ROW row;

	CameraList.Init();

	LogMessage( E_MSG_DEBUG, "Reading camera list" );

	// read from mysql
	//                          0           1       2            3      4           5            6         7           8        9
	if ( myDB.RunQuery( "SELECT ca_CameraNo,ca_Name,ca_IPAddress,ca_PTZ,ca_Encoding,ca_Directory,ca_UserId,ca_Password,ca_Model,ca_MJpeg "
			"FROM cameras order by ca_Name") != 0 )
	{
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		i = 0;
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			CameraList.GetCamera(i).SetCameraNo( atoi( (const char*)row[0] ) );
			CameraList.GetCamera(i).SetName( (const char*)row[1] );
			CameraList.GetCamera(i).SetIPAddress( (const char*)row[2] );
			CameraList.GetCamera(i).SetPTZ( (const char*)row[3] );
			CameraList.GetCamera(i).SetEncoding( (const char*)row[4] );
			CameraList.GetCamera(i).SetDirectory( (const char*)row[5] );
			CameraList.GetCamera(i).SetUserId( (const char*)row[6] );
			CameraList.GetCamera(i).SetPassword( (const char*)row[7] );
			CameraList.GetCamera(i).SetModel( (const char*)row[8] );
			CameraList.GetCamera(i).SetMJpeg( (const char*)row[9] );
			CameraList.AddCamera();

			LogMessage( E_MSG_DEBUG, "CameraNo:%d, Name:%s, IPAddress:%s", CameraList.GetCamera(i).GetCameraNo(),
					CameraList.GetCamera(i).GetName(), CameraList.GetCamera(i).GetIPAddress() );

			i += 1;
		}
	}

	myDB.FreeResult();

	LogMessage( E_MSG_INFO, "Read %d cameras", CameraList.GetNumCameras() );
}

void CThread::ClearPlcActiveState( CMysql& myDB )
{
	if ( myDB.RunQuery( "update plcstates set pl_StateIsActive='N'") )
	{
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		LogMessage( E_MSG_INFO, "PLC active state cleared" );
	}
}

int CThread::ReadPlcStatesTableAll( CMysql& myDB, CPlcStates* pPlcStates )
{
	int i;
	int iRet = 1;	// plc state error
	int iOps = 0;
	int iNumFields;
	char szOperation[100] = "";
	MYSQL_ROW row;

	LogMessage( E_MSG_INFO, "Reading plcstates list, active idx %d", pPlcStates->GetActiveStateIdx() );

	pPlcStates->Init();

	// find out which operation is active
	if ( myDB.RunQuery( "SELECT pl_Operation from plcstates where pl_StateIsActive='Y'" ) )
	{
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			iOps += 1;
			snprintf( szOperation, sizeof(szOperation), "%s", (const char*)row[0] );
		}
	}

	if ( iOps == 1 )
	{
		LogMessage( E_MSG_INFO, "PLC Operation '%s' is active", szOperation );
		
		// read from mysql
		//                          0          1            2            3                               4                  5
		if ( myDB.RunQuery( "SELECT pl_StateNo,pl_Operation,pl_StateName,pl_StateIsActive,unix_timestamp(pl_StateTimestamp),pl_RuleType,"
			//   6           7            8        9       10               11       12           13             14
				"pl_DeviceNo,pl_IOChannel,pl_Value,pl_Test,pl_NextStateName,pl_Order,pl_DelayTime,pl_TimerValues,pl_RuntimeValue "
				"FROM plcstates where pl_Operation='%s' order by pl_Operation,pl_StateName,pl_RuleType,pl_Order,pl_StateNo", szOperation) != 0 )
		{
			LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
		}
		else
		{
			i = 0;
			while ( (row = myDB.FetchRow( iNumFields )) )
			{
				int iStateNo = atoi( (const char*)row[0] );
				pPlcStates->GetState(i).SetStateNo( iStateNo );
				pPlcStates->GetState(i).SetOperation( (const char*)row[1] );
				pPlcStates->GetState(i).SetStateName( (const char*)row[2] );
				pPlcStates->GetState(i).SetStateIsActive( (const char*)row[3] );
				pPlcStates->GetState(i).SetStateTimestampMS( (const double)atof((const char*)row[4]) );
				pPlcStates->GetState(i).SetRuleType( (const char*)row[5] );
				pPlcStates->GetState(i).SetDeviceNo( (const int)atoi((const char*)row[6]) );
				pPlcStates->GetState(i).SetIOChannel( (const int)atoi((const char*)row[7]) );
				pPlcStates->GetState(i).SetValue( (const double)atof((const char*)row[8]) );
				pPlcStates->GetState(i).SetTest( (const char*)row[9] );
				pPlcStates->GetState(i).SetNextStateName( (const char*)row[10] );
				pPlcStates->GetState(i).SetOrder( (const int)atoi((const char*)row[11]) );
				pPlcStates->GetState(i).SetDelayTime( (const double)atof((const char*)row[12]) );
				pPlcStates->GetState(i).SetTimerValues( (const char*)row[13] );
				pPlcStates->GetState(i).SetRuntimeValue( (const double)atof((const char*)row[14]) );
				pPlcStates->AddState();

				if ( pPlcStates->GetState(i).GetStateIsActive() )
				{
					iRet = 0;	// all ok
					pPlcStates->SetActiveStateIdx( i );
				}

				LogMessage( E_MSG_INFO, "%d: StateNo:%d, Op:%s, State:%s, Active:%d, RuleType:%s, DeviceNo:%d, IOChannel:%d, Value:%.1f, Test:'%s', NextState:%s, Order:%d, Delay:%.1f, TValues:%s, RTVal:%.1f",
						i, pPlcStates->GetState(i).GetStateNo(), pPlcStates->GetState(i).GetOperation(), pPlcStates->GetState(i).GetStateName(),
						pPlcStates->GetState(i).GetStateIsActive(), pPlcStates->GetState(i).GetRuleType(), pPlcStates->GetState(i).GetDeviceNo(), pPlcStates->GetState(i).GetIOChannel(),
						pPlcStates->GetState(i).GetValue(), pPlcStates->GetState(i).GetTest(), pPlcStates->GetState(i).GetNextStateName(), pPlcStates->GetState(i).GetOrder(),
						pPlcStates->GetState(i).GetDelayTime(), pPlcStates->GetState(i).GetTimerValues(), pPlcStates->GetState(i).GetRuntimeValue() );

				if ( i+1 < MAX_PLC_STATES)
				{
					i += 1;
				}
				else
				{
					LogMessage( E_MSG_ERROR, "Too many plcstates in operation '%s', max is %d", szOperation, MAX_PLC_STATES );
					break;
				}
			}
		}
	}
	else if ( iOps == 0 )
	{
		LogMessage( E_MSG_INFO, "There are no Operations with an active state" );
	}
	else
	{
		LogMessage( E_MSG_ERROR, "There are %d Operations with an active state", iOps );
	}

	myDB.FreeResult();

	LogMessage( E_MSG_INFO, "Read %d plcstates, active idx %d, iRet %d", pPlcStates->GetStateCount(), pPlcStates->GetActiveStateIdx(), iRet );

	return iRet;
}

void CThread::ReadPlcStatesTableDelayTime( CMysql& myDB, CPlcStates* pPlcStates )
{
	int i;
	int iOps = 0;
	int iNumFields;
	char szOperation[100] = "";
	MYSQL_ROW row;

	LogMessage( E_MSG_INFO, "Reading plcstates DelayTime" );

	// find out which operation is active
	if ( myDB.RunQuery( "SELECT pl_Operation from plcstates where pl_StateIsActive='Y'" ) )
	{
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			iOps += 1;
			snprintf( szOperation, sizeof(szOperation), "%s", (const char*)row[0] );
		}
	}

	if ( iOps == 1 )
	{
		LogMessage( E_MSG_INFO, "PLC Operation '%s' is active", szOperation );

		// read from mysql
		//                          0          1
		if ( myDB.RunQuery( "SELECT pl_StateNo,pl_DelayTime "
				"FROM plcstates where pl_Operation='%s' order by pl_Operation,pl_StateName,pl_RuleType,pl_Order,pl_StateNo", szOperation) != 0 )
		{
			LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
		}
		else
		{
			while ( (row = myDB.FetchRow( iNumFields )) )
			{
				int iStateNo = atoi( (const char*)row[0] );
				i = pPlcStates->FindStateNo(iStateNo);
				pPlcStates->GetState(i).SetDelayTime( (const double)atof((const char*)row[1]) );

				LogMessage( E_MSG_INFO, "idx %d: StateNo:%d, Delay:%.1f",
						i, pPlcStates->GetState(i).GetStateNo(), pPlcStates->GetState(i).GetDelayTime() );
			}
		}
	}
	else if ( iOps != 0 )
	{
		LogMessage( E_MSG_ERROR, "There are %d Operations with an active state", iOps );
	}

	myDB.FreeResult();

	LogMessage( E_MSG_INFO, "Read plcstates DelayTime end" );
}

void CThread::ProcessPlcStates( CMysql& myDB, CPlcStates* pPlcStates )
{
	static double dLastStateWSTime = 0.0;

	if ( pPlcStates->IsActive() )
	{
		int iStateNo = 0;
		int iDeviceNo = 0;
		int iIOChannel = 0;
		double dValue = 0;
		char szEventName[50] = "?";
		char szWsMsg[50];

		if ( !gbPlcIsActive )
			LogMessage( E_MSG_INFO, "PLC is active (state idx %d)", pPlcStates->GetActiveStateIdx() );
		gbPlcIsActive = true;

		if ( gThreadMsgFromWS.GetMessage( szWsMsg, sizeof(szWsMsg)) )
		{
			LogMessage( E_MSG_INFO, "PLC WS event: %s", szWsMsg );

			// change to VSD frequency
			if ( strncmp( szWsMsg, "VSDF", 4 ) == 0 )
			{	// 012345678901234
				// VSDF_xx_xx_nn
				double dVal = atof( &szWsMsg[11] );
				iDeviceNo = atoi( &szWsMsg[5] );
				iIOChannel = atoi( &szWsMsg[8] );

				int iOutAddress = m_pmyDevices->GetAddressForDeviceNo( iDeviceNo );
				int iOutIdx = m_pmyDevices->GetIdxForAddr(iOutAddress);

				int iVsdStateIdx = pPlcStates->GetVsdStateIdx(pPlcStates->GetActiveStateIdx(), iDeviceNo, iIOChannel);

				double dNewVal = pPlcStates->GetState(iVsdStateIdx).GetRuntimeValue();
				if ( dNewVal >= 0 )
				{	// forward
					dNewVal += dVal;
				}
				else if ( dVal < 0 )
				{	// backward, go slower
					dNewVal += fabs(dVal);
				}
				else
				{	// backward, go faster
					dNewVal -= fabs(dVal);
				}
				if ( dNewVal < -50 )
					dNewVal = -50;
				else if ( dNewVal > 50 )
					dNewVal = 50;

				pPlcStates->GetState(iVsdStateIdx).SetRuntimeValue( dNewVal );
				LogMessage( E_MSG_INFO, "Setting VSD frequency for %d,%d to %.1f (%.1f) %.1f", iDeviceNo, iIOChannel, dNewVal, pPlcStates->GetState(iVsdStateIdx).GetRuntimeValue(), dVal );

				// send value change to VSD
				pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );
				HandleVSDOutputDevice( myDB, iOutIdx, iIOChannel, pPlcStates->GetState(iVsdStateIdx).GetRuntimeValue() );
				pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );

				// update database with new value
				myDB.UpdatePlcStateRuntimeValue( pPlcStates->GetState(iVsdStateIdx).GetOperation(), pPlcStates->GetState(iVsdStateIdx).GetStateName(), iDeviceNo, iIOChannel, dNewVal );

				// post new value back to web page
				PostToWebSocket( E_ET_FREQUENCY, iOutIdx, iIOChannel, dNewVal, false );
			}
			else
			{
				LogMessage( E_MSG_WARN, "unhandled ws message '%s'", szWsMsg );
			}
		}

		iStateNo = myDB.ReadPlcStatesScreenButton();
		if ( iStateNo == 0 )
		{
			pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );
			// lock required as the input event comes from the com port threads
			iStateNo = pPlcStates->ReadInputEvent( iDeviceNo, iIOChannel, dValue );
			pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );

			if ( iStateNo != 0 )
			{
				snprintf( szEventName, sizeof(szEventName), "PLC Input Event" );
				LogMessage( E_MSG_INFO, "PLC Input event %d,%d,%.1f StateNo %d", iDeviceNo, iIOChannel, dValue, iStateNo );
			}
			else if ( iDeviceNo != 0 )
			{
				LogMessage( E_MSG_WARN, "PLC invalid input event %d,%d,%.1f for state %s", iDeviceNo, iIOChannel, dValue, pPlcStates->GetState(pPlcStates->GetActiveStateIdx()).GetStateName() );
				gThreadMsgToWS.PutMessage( "PLC invalid input event %d,%d,%.1f for state %s", iDeviceNo, iIOChannel, dValue, pPlcStates->GetState(pPlcStates->GetActiveStateIdx()).GetStateName() );
				myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC invalid input event %d,%d,%.1f for state %s", iDeviceNo, iIOChannel, dValue, pPlcStates->GetState(pPlcStates->GetActiveStateIdx()).GetStateName() );
			}
		}
		else
		{
			snprintf( szEventName, sizeof(szEventName), "PLC Screen Button Event" );
			LogMessage( E_MSG_INFO, "PLC screen button event, StateNo %d", iStateNo );
		}


		double dTimenowMS = TimeNowMS();
		if ( iStateNo == 0 )
		{	// check for timer event and changed event values
			int idx;
			double dDelay;
			int iStartIdx = pPlcStates->GetActiveStateIdx();
			while ( (idx = pPlcStates->GetEvent(iStartIdx)) >= 0 )
			{
				dDelay = pPlcStates->GetState(idx).GetDelayTime();
				if ( pPlcStates->GetState(idx).GetRuleType()[0] == 'E' && pPlcStates->GetState(idx).GetDeviceNo() == 0 && dDelay >= 0 )
				{
					if ( pPlcStates->GetActiveState().GetStateTimestampMS() + dDelay > dTimenowMS )
					{
						if ( dTimenowMS - pPlcStates->GetActiveState().GetStateTimestampMS() <= 0 )
						{	// only log once
							LogMessage( E_MSG_INFO, "PLC: timer event disabled for another %.1f seconds", pPlcStates->GetActiveState().GetStateTimestampMS() + dDelay - dTimenowMS );
						}
					}
					else
					{	// event time - change states
						snprintf( szEventName, sizeof(szEventName), "PLC Timer Event" );

						iStateNo = pPlcStates->GetState(idx).GetStateNo();
						LogMessage( E_MSG_INFO, "PLC: timer event due, StateNo %d", iStateNo );
						break;
					}
				}

				if ( pPlcStates->GetState(idx).GetRuleType()[0] == 'E' && strlen(pPlcStates->GetState(idx).GetTest()) != 0 && strlen(pPlcStates->GetState(idx).GetNextStateName()) != 0 )
				{	// check button/switch condition value
					int iInAddress = m_pmyDevices->GetAddressForDeviceNo( pPlcStates->GetState(idx).GetDeviceNo() );
					int iInIdx = m_pmyDevices->GetIdxForAddr(iInAddress);
					int iInChannel = pPlcStates->GetState(idx).GetIOChannel();

					dValue = 0.0;
					switch ( m_pmyDevices->GetInChannelType(iInIdx,iInChannel) )
					{
					default:	// switch 0/1 off/on
						dValue = (double)m_pmyDevices->GetNewInput( iInIdx, iInChannel );
						break;
					case E_IO_TEMP_HIGH:
					case E_IO_TEMP_LOW:
					case E_IO_TEMP_HIGHLOW:
					case E_IO_TEMP_MONITOR:
						dValue = m_pmyDevices->CalcTemperature(iInIdx,iInChannel,true);
						break;
					case E_IO_VOLT_HIGH:
					case E_IO_VOLT_LOW:
					case E_IO_VOLT_HIGHLOW:
					case E_IO_VOLT_MONITOR:
						if ( m_pmyDevices->GetDeviceType(iInIdx) == E_DT_VIPF_MON )
							dValue = m_pmyDevices->CalcVIPFValue(iInIdx,iInChannel,true);
						else
							dValue = m_pmyDevices->CalcVoltage(iInIdx,iInChannel,true);
						break;
					case E_IO_CURRENT_HIGH:
					case E_IO_CURRENT_LOW:
					case E_IO_CURRENT_HIGHLOW:
					case E_IO_CURRENT_MONITOR:
						dValue = m_pmyDevices->CalcVIPFValue(iInIdx,iInChannel,true);
						break;
					case E_IO_ROTENC_HIGH:
					case E_IO_ROTENC_LOW:
					case E_IO_ROTENC_HIGHLOW:
					case E_IO_ROTENC_MONITOR:
						dValue = m_pmyDevices->CalcRotaryEncoderDistance(iInIdx,iInChannel,true);
						break;
					}
					//LogMessage( E_MSG_INFO, "Checking condition event StateNo %d %.1f == %d ?", pPlcStates->GetState(idx).GetStateNo(), pPlcStates->GetState(idx).GetValue(), (int)uVal);
					if ( strcmp( pPlcStates->GetState(idx).GetTest(), "EQ" ) == 0 )
					{
						if ( dValue == pPlcStates->GetState(idx).GetValue() )
						{
							snprintf( szEventName, sizeof(szEventName), "PLC State Change Event" );
							iStateNo = pPlcStates->GetState(idx).GetStateNo();
							LogMessage( E_MSG_INFO, "PLC: condition event EQ, StateNo %d, %.1f == %.1f", iStateNo, dValue, pPlcStates->GetState(idx).GetValue() );
							break;
						}
					}
					else if ( strcmp( pPlcStates->GetState(idx).GetTest(), "NE" ) == 0 )
					{
						if ( dValue != pPlcStates->GetState(idx).GetValue() )
						{
							snprintf( szEventName, sizeof(szEventName), "PLC State Change Event" );
							iStateNo = pPlcStates->GetState(idx).GetStateNo();
							LogMessage( E_MSG_INFO, "PLC: condition event NE, StateNo %d, %.1f != %.1f", iStateNo, dValue, pPlcStates->GetState(idx).GetValue() );
							break;
						}
					}
					else if ( strcmp( pPlcStates->GetState(idx).GetTest(), "GE" ) == 0 )
					{
						if ( dValue >= pPlcStates->GetState(idx).GetValue() )
						{
							snprintf( szEventName, sizeof(szEventName), "PLC State Change Event" );
							iStateNo = pPlcStates->GetState(idx).GetStateNo();
							LogMessage( E_MSG_INFO, "PLC: condition event GE, StateNo %d, %.1f >= %.1f", iStateNo, dValue, pPlcStates->GetState(idx).GetValue() );
							break;
						}
					}
					else if ( strcmp( pPlcStates->GetState(idx).GetTest(), "GT" ) == 0 )
					{
						if ( dValue > pPlcStates->GetState(idx).GetValue() )
						{
							snprintf( szEventName, sizeof(szEventName), "PLC State Change Event" );
							iStateNo = pPlcStates->GetState(idx).GetStateNo();
							LogMessage( E_MSG_INFO, "PLC: condition event GT, StateNo %d, %.1f > %.1f", iStateNo, dValue, pPlcStates->GetState(idx).GetValue() );
							break;
						}
					}
					else if ( strcmp( pPlcStates->GetState(idx).GetTest(), "LE" ) == 0 )
					{
						if ( dValue <= pPlcStates->GetState(idx).GetValue() )
						{
							snprintf( szEventName, sizeof(szEventName), "PLC State Change Event" );
							iStateNo = pPlcStates->GetState(idx).GetStateNo();
							LogMessage( E_MSG_INFO, "PLC: condition event LE, StateNo %d, %.1f <= %.1f", iStateNo, dValue, pPlcStates->GetState(idx).GetValue() );
							break;
						}
					}
					else if ( strcmp( pPlcStates->GetState(idx).GetTest(), "LT" ) == 0 )
					{
						if ( dValue < pPlcStates->GetState(idx).GetValue() )
						{
							snprintf( szEventName, sizeof(szEventName), "PLC State Change Event" );
							iStateNo = pPlcStates->GetState(idx).GetStateNo();
							LogMessage( E_MSG_INFO, "PLC: condition event LT, StateNo %d, %.1f < %.1f", iStateNo, dValue, pPlcStates->GetState(idx).GetValue() );
							break;
						}
					}
					else
					{
						LogMessage( E_MSG_WARN, "PLC: condition event, unhandled Operator '%s'", pPlcStates->GetState(idx).GetTest() );
					}
				}

				iStartIdx = idx;
			}
		}

		if ( iStateNo != 0 )
		{
			bool bProcess = true;
			int rc;
			int idx;
			int idx2;

			idx = pPlcStates->FindStateNo( iStateNo );

			double dDelay = pPlcStates->GetState(idx).GetDelayTime();

			LogMessage( E_MSG_INFO, "%s, StateNo %d, idx %d, goto state '%s', delay %.1f sec", szEventName, iStateNo, idx, pPlcStates->GetState(idx).GetNextStateName(), dDelay );
			gThreadMsgToWS.PutMessage( "%s, StateNo %d, idx %d, goto state '%s', delay %.1f sec", szEventName, iStateNo, idx, pPlcStates->GetState(idx).GetNextStateName(), dDelay );
			myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "%s, StateNo %d, idx %d, goto state '%s', delay %.1f sec", szEventName, iStateNo, idx, pPlcStates->GetState(idx).GetNextStateName(), dDelay );

			// check if this event should be delayed

			if ( dDelay > 0 )
			{
				if ( pPlcStates->GetActiveState().GetStateTimestampMS() + dDelay > dTimenowMS )
				{
					bProcess = false;
					LogMessage( E_MSG_INFO, "PLC: Event is disabled for another %.1f seconds", pPlcStates->GetActiveState().GetStateTimestampMS() + dDelay - dTimenowMS );
					gThreadMsgToWS.PutMessage( "PLC: Event is disabled for another %.1f seconds", pPlcStates->GetActiveState().GetStateTimestampMS() + dDelay - dTimenowMS );
					myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: Event is disabled for another %.1f seconds", pPlcStates->GetActiveState().GetStateTimestampMS() + dDelay - dTimenowMS );
				}
			}
			else
			{
				CPlcState& myState = pPlcStates->GetState(idx);

				if ( strlen(myState.GetTest()) != 0 )
				{
					bProcess = false;

					if ( strcmp( myState.GetTest(), "GE" ) == 0 )
					{
						if ( dValue >= myState.GetValue() )
						{
							bProcess = true;
						}
					}
					else if ( strcmp( myState.GetTest(), "GT" ) == 0 )
					{
						if ( dValue > myState.GetValue() )
						{
							bProcess = true;
						}
					}
					else if ( strcmp( myState.GetTest(), "EQ" ) == 0 )
					{
						if ( dValue == myState.GetValue() )
						{
							bProcess = true;
						}
					}
					else if ( strcmp( myState.GetTest(), "NE" ) == 0 )
					{
						if ( dValue != myState.GetValue() )
						{
							bProcess = true;
						}
					}
					else if ( strcmp( myState.GetTest(), "LE" ) == 0 )
					{
						if ( dValue <= myState.GetValue() )
						{
							bProcess = true;
						}
					}
					else if ( strcmp( myState.GetTest(), "LT" ) == 0 )
					{
						if ( dValue < myState.GetValue() )
						{
							bProcess = true;
						}
					}
					else
					{
						LogMessage( E_MSG_WARN, "Plc unhandled Test type '%s'", myState.GetTest() );
					}

					LogMessage( E_MSG_INFO, "PLC State %d, Test '%s' %.1f vs %.1f: %s", iStateNo, myState.GetTest(), dValue, myState.GetValue(), (bProcess ? "action" : "skip") );
					gThreadMsgToWS.PutMessage( "PLC State %d, Test '%s' %.1f vs %.1f: %s", iStateNo, myState.GetTest(), dValue, myState.GetValue(), (bProcess ? "action" : "skip") );
					myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC State %d, Test '%s' %.1f vs %.1f: %s", iStateNo, myState.GetTest(), dValue, myState.GetValue(), (bProcess ? "action" : "skip") );
				}
				else
				{
					LogMessage( E_MSG_INFO, "PLC State %d, No Test: %s", iStateNo, (bProcess ? "action" : "skip") );
					gThreadMsgToWS.PutMessage( "PLC State %d, No Test: %s", iStateNo, (bProcess ? "action" : "skip") );
					myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC State %d, No Test: %s", iStateNo, (bProcess ? "action" : "skip") );
				}
			}

			if ( bProcess )
			{	// check for pre condition failure - checked before moving to the next state
				// pre conditions have NO next state name
				int iStartIdx = pPlcStates->GetNextStateIdx( pPlcStates->GetState(idx).GetOperation(), pPlcStates->GetState(idx).GetNextStateName() );
				while ( (idx2 = pPlcStates->GetEvent(iStartIdx)) >= 0 )
				{
					if ( pPlcStates->GetState(idx2).GetRuleType()[0] == 'E' && strlen( pPlcStates->GetState(idx2).GetTest()) != 0 && strlen(pPlcStates->GetState(idx2).GetNextStateName()) == 0 )
					{	// check button/switch condition value
						LogMessage( E_MSG_INFO, "checking idx2 %d '%s'", idx2, pPlcStates->GetState(idx2).GetStateName() );
						int iInAddress = m_pmyDevices->GetAddressForDeviceNo( pPlcStates->GetState(idx2).GetDeviceNo() );
						int iInIdx = m_pmyDevices->GetIdxForAddr(iInAddress);
						int iInChannel = pPlcStates->GetState(idx2).GetIOChannel();
						LogMessage( E_MSG_INFO, "checking idx2 %d '%s' (%d,%d)", idx2, pPlcStates->GetState(idx2).GetStateName(), iInIdx, iInChannel );

						double dVal = 0.0;
						switch ( m_pmyDevices->GetInChannelType(iInIdx,iInChannel) )
						{
						default:	// switch 0/1 off/on
							dVal = (double)m_pmyDevices->GetNewInput( iInIdx, iInChannel );
							break;
						case E_IO_TEMP_HIGH:
						case E_IO_TEMP_LOW:
						case E_IO_TEMP_HIGHLOW:
						case E_IO_TEMP_MONITOR:
							dVal = m_pmyDevices->CalcTemperature(iInIdx,iInChannel,true);
							break;
						case E_IO_VOLT_HIGH:
						case E_IO_VOLT_LOW:
						case E_IO_VOLT_HIGHLOW:
						case E_IO_VOLT_MONITOR:
							if ( m_pmyDevices->GetDeviceType(iInIdx) == E_DT_VIPF_MON )
								dVal = m_pmyDevices->CalcVIPFValue(iInIdx,iInChannel,true);
							else
								dVal = m_pmyDevices->CalcVoltage(iInIdx,iInChannel,true);
							break;
						case E_IO_CURRENT_HIGH:
						case E_IO_CURRENT_LOW:
						case E_IO_CURRENT_HIGHLOW:
						case E_IO_CURRENT_MONITOR:
							dVal = m_pmyDevices->CalcVIPFValue(iInIdx,iInChannel,true);
							break;
						case E_IO_ROTENC_HIGH:
						case E_IO_ROTENC_LOW:
						case E_IO_ROTENC_HIGHLOW:
						case E_IO_ROTENC_MONITOR:
							dVal = m_pmyDevices->CalcRotaryEncoderDistance(iInIdx,iInChannel,true);
							break;
						}
						if ( strcmp( pPlcStates->GetState(idx2).GetTest(), "EQ" ) == 0 )
						{
							if ( dVal == pPlcStates->GetState(idx2).GetValue() )
							{
								bProcess = false;

								LogMessage( E_MSG_INFO, "PLC: initial condition EQ exception (%.1f == %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition EQ exception (%.1f == %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition EQ exception (%.1f == %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								break;
							}
							else
							{
								LogMessage( E_MSG_INFO, "PLC: initial condition EQ passed (%.1f == %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition EQ passed (%.1f == %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition EQ passed (%.1f == %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							}
						}
						else if ( strcmp( pPlcStates->GetState(idx2).GetTest(), "NE" ) == 0 )
						{
							if ( dVal != pPlcStates->GetState(idx2).GetValue() )
							{
								bProcess = false;

								LogMessage( E_MSG_INFO, "PLC: initial condition NE exception (%.1f != %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition NE exception (%.1f != %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition NE exception (%.1f != %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								break;
							}
							else
							{
								LogMessage( E_MSG_INFO, "PLC: initial condition NE passed (%.1f != %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition NE passed (%.1f != %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition NE passed (%.1f != %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							}
						}
						else if ( strcmp( pPlcStates->GetState(idx2).GetTest(), "GE" ) == 0 )
						{
							if ( dVal >= pPlcStates->GetState(idx2).GetValue() )
							{
								bProcess = false;

								LogMessage( E_MSG_INFO, "PLC: initial condition GE exception (%.1f >= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition GE exception (%.1f >= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition GE exception (%.1f >= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								break;
							}
							else
							{
								LogMessage( E_MSG_INFO, "PLC: initial condition GE passed (%.1f >= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition GE passed (%.1f >= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition GE passed (%.1f >= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							}
						}
						else if ( strcmp( pPlcStates->GetState(idx2).GetTest(), "GT" ) == 0 )
						{
							if ( dVal > pPlcStates->GetState(idx2).GetValue() )
							{
								bProcess = false;

								LogMessage( E_MSG_INFO, "PLC: initial condition GT exception (%.1f > %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition GT exception (%.1f > %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition GT exception (%.1f > %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								break;
							}
							else
							{
								LogMessage( E_MSG_INFO, "PLC: initial condition GT passed (%.1f > %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition GT passed (%.1f > %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition GT passed (%.1f > %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							}
						}
						else if ( strcmp( pPlcStates->GetState(idx2).GetTest(), "LT" ) == 0 )
						{
							if ( dVal < pPlcStates->GetState(idx2).GetValue() )
							{
								bProcess = false;

								LogMessage( E_MSG_INFO, "PLC: initial condition LT exception (%.1f < %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition LT exception (%.1f < %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition LT exception (%.1f < %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								break;
							}
							else
							{
								LogMessage( E_MSG_INFO, "PLC: initial condition LT passed (%.1f < %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition LT passed (%.1f < %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition LT passed (%.1f < %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							}
						}
						else if ( strcmp( pPlcStates->GetState(idx2).GetTest(), "LE" ) == 0 )
						{
							if ( dVal <= pPlcStates->GetState(idx2).GetValue() )
							{
								bProcess = false;

								LogMessage( E_MSG_INFO, "PLC: initial condition LE exception (%.1f <= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition LE exception (%.1f <= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition LE exception (%.1f <= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								break;
							}
							else
							{
								LogMessage( E_MSG_INFO, "PLC: initial condition LE passed (%.1f <= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								gThreadMsgToWS.PutMessage( "PLC: initial condition LE passed (%.1f <= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
								myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition LE passed (%.1f <= %.1f), StateNo %d '%s'", pPlcStates->GetState(idx2).GetValue(), dVal, iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							}
						}
						else
						{
							bProcess = false;

							LogMessage( E_MSG_WARN, "PLC: initial condition unhandled '%s' exception, StateNo %d '%s'", pPlcStates->GetState(idx2).GetTest(), iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							gThreadMsgToWS.PutMessage( "PLC: initial condition unhandled '%s' exception, StateNo %d '%s'", pPlcStates->GetState(idx2).GetTest(), iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "PLC: initial condition unhandled '%s' exception, StateNo %d '%s'", pPlcStates->GetState(idx2).GetTest(), iStateNo, m_pmyDevices->GetInIOName( iInIdx, iInChannel ) );
							break;
						}
					}

					iStartIdx = idx2;
				}
			}

			if ( bProcess )
			{
				// set the new active state
				rc = myDB.SetNextPlcState( pPlcStates->GetState(idx).GetOperation(), pPlcStates->GetState(idx).GetNextStateName(), pPlcStates->GetState(idx).GetStateName(), dTimenowMS );
				if ( rc == 0 )
				{
					LogMessage( E_MSG_INFO, "Operation %s: Changed active state from '%s' to '%s'", pPlcStates->GetState(idx).GetOperation(), pPlcStates->GetState(idx).GetStateName(),
							pPlcStates->GetState(idx).GetNextStateName() );
					gThreadMsgToWS.PutMessage( "Operation %s: Changed active state from '%s' to '%s'", pPlcStates->GetState(idx).GetOperation(), pPlcStates->GetState(idx).GetStateName(),
							pPlcStates->GetState(idx).GetNextStateName() );
					myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "Operation %s: Changed active state from '%s' to '%s'", pPlcStates->GetState(idx).GetOperation(), pPlcStates->GetState(idx).GetStateName(),
							pPlcStates->GetState(idx).GetNextStateName() );
				}
				else
				{	// error
					LogMessage( E_MSG_ERROR, "Operation %s: Failed to change active state from '%s' to '%s', rc %d", pPlcStates->GetState(idx).GetOperation(),
							pPlcStates->GetState(idx).GetStateName(), pPlcStates->GetState(idx).GetNextStateName(), rc );
					gThreadMsgToWS.PutMessage( "Operation %s: Failed to change active state from '%s' to '%s', rc %d", pPlcStates->GetState(idx).GetOperation(),
							pPlcStates->GetState(idx).GetStateName(), pPlcStates->GetState(idx).GetNextStateName(), rc );
					myDB.LogEvent( -7, 0, E_ET_PLCEVENT, 0, "Operation %s: Failed to change active state from '%s' to '%s', rc %d", pPlcStates->GetState(idx).GetOperation(),
							pPlcStates->GetState(idx).GetStateName(), pPlcStates->GetState(idx).GetNextStateName(), rc );
				}

				pPlcStates->SetNextStateActive( idx, dTimenowMS );

				// tell the browser to refresh
				LogMessage( E_MSG_INFO, "ws Refresh" );
				gThreadMsgToWS.PutMessage( "Refresh" );

				int iInAddress = m_pmyDevices->GetAddressForDeviceNo( pPlcStates->GetState(idx).GetDeviceNo() );
				int iInIdx = m_pmyDevices->GetIdxForAddr(iInAddress);
				int iInChannel = pPlcStates->GetState(idx).GetIOChannel();
				//LogMessage( E_MSG_INFO, "Operation op change de_no %d: %d,%d,%d", pPlcStates->GetState(idx).GetDeviceNo(), iInIdx, iInAddress, iInChannel );

				// check for condition
				bool bConditionsPassed = true;


				if ( bConditionsPassed )
				{
					// set intital outputs for the new state
					int iCount = 0;
					int iStartIdx = pPlcStates->GetActiveStateIdx();
					while ( (idx = pPlcStates->GetInitialAction(iStartIdx)) >= 0 )
					{
						iCount += 1;


						LogMessage( E_MSG_INFO, "Operation %s: State %s: '%s' initial state %d,%d = %.1f", pPlcStates->GetState(idx).GetOperation(), pPlcStates->GetState(idx).GetStateName(),
								pPlcStates->GetState(idx).GetRuleType(), pPlcStates->GetState(idx).GetDeviceNo(), pPlcStates->GetState(idx).GetIOChannel(), pPlcStates->GetState(idx).GetValue() );

						uint8_t uLinkState = pPlcStates->GetState(idx).GetValue();
						char szOutDeviceName[MAX_DEVICE_NAME_LEN+1];
						char szOutHostname[HOST_NAME_MAX+1] = "";

						int iOutAddress = m_pmyDevices->GetAddressForDeviceNo( pPlcStates->GetState(idx).GetDeviceNo() );
						int iOutIdx = m_pmyDevices->GetIdxForAddr(iOutAddress);
						int iOutChannel = pPlcStates->GetState(idx).GetIOChannel();
						int iOutOnPeriod = 0;
						double dVsdFrequency = 0.0;

						enum E_IO_TYPE eSwType = m_pmyDevices->GetInChannelType( iInIdx, iInChannel );
						if ( eSwType == E_IO_UNUSED )
						{	// timer event
							eSwType = E_IO_ON_OFF;
						}

						if ( uLinkState > 1 )
						{
							uLinkState = 1;
						}

						strcpy( szOutHostname, m_pmyDevices->GetDeviceHostname( iOutIdx ) );
						strcpy( szOutDeviceName, m_pmyDevices->GetDeviceName( iOutIdx ) );

						LogMessage( E_MSG_INFO, "Operation Output change for '%s' (0x%x->%d,%d), new state %u (%d,%.1f), on %s", m_pmyDevices->GetOutIOName(iOutIdx, iOutChannel),
								iOutAddress, iOutIdx, iOutChannel+1, uLinkState, iOutOnPeriod, dVsdFrequency, szOutHostname );

						pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

						switch ( m_pmyDevices->GetDeviceType(iOutIdx) )
						{
						default:
							// TODO: handle ESP devices
							// TODO: handle sending to another pi
							ChangeOutputState( myDB, iInIdx, iInAddress, iInChannel, iOutIdx, iOutAddress, iOutChannel, uLinkState, eSwType, iOutOnPeriod, dVsdFrequency );
							break;

						case E_DT_VSD_NFLIXEN:
						case E_DT_VSD_PWRELECT:
							HandleVSDOutputDevice( myDB, iOutIdx, iOutChannel, pPlcStates->GetState(idx).GetValue() );
							break;
						}

						pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );


						iStartIdx = idx;
					}

					if ( iCount == 0 )
					{
						LogMessage( E_MSG_INFO, "Operation %s: State %s: No initial actions to process", pPlcStates->GetState(iStartIdx).GetOperation(), pPlcStates->GetState(iStartIdx).GetStateName() );
					}
				}
				else
				{
					LogMessage( E_MSG_INFO, "Initial states skipped due to condition failure" );
				}
			}
		}
		else if ( pPlcStates->IsActive() )
		{
			if ( dLastStateWSTime + 0.2 < TimeNowMS() )
			{
				int idx = pPlcStates->GetActiveStateIdx();
				gThreadMsgToWS.PutMessage( "State:%s", pPlcStates->GetState(idx).GetStateName() );

				dLastStateWSTime = TimeNowMS();
			}
		}
	}
	else
	{
		if ( gbPlcIsActive )
			LogMessage( E_MSG_INFO, "PLC is no longer active" );
		gbPlcIsActive = false;
	}
}



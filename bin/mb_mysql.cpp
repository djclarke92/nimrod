#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>

#include <modbus/modbus.h>
#include <errmsg.h>

#include "mb_mysql.h"
#include "mb_utils.h"


extern bool gbTerminateNow;


CMysql::CMysql()
{
	m_pMySQLCon = NULL;
	m_pResult = NULL;
	m_szQuery[0] = '\0';

	ReadSiteConfig( "DB_HOST_NAME", m_szHostName, sizeof(m_szHostName) );
	ReadSiteConfig( "DB_USER_NAME", m_szUserId, sizeof(m_szUserId) );
	ReadSiteConfig( "DB_PASSWORD", m_szPassword, sizeof(m_szPassword) );
	ReadSiteConfig( "DB_DATABASE_NAME", m_szDBName, sizeof(m_szDBName) );

	Init();
}

void CMysql::Init()
{
	if ( m_pMySQLCon == NULL )
	{
		m_pMySQLCon = mysql_init( NULL );
	}
}

CMysql::~CMysql()
{
	Disconnect();

	mysql_thread_end();
}

bool CMysql::Connect()
{
	bool bRc = true;
	MYSQL *MySQLConRet;

	Init();

	MySQLConRet = mysql_real_connect( m_pMySQLCon, m_szHostName, m_szUserId, m_szPassword,
	                                          m_szDBName, 0, NULL, 0 );
    if ( MySQLConRet == NULL )
    {	// error
	    bRc = false;
	    LogMessage( E_MSG_ERROR, "MySQL connect error: %s", mysql_error(m_pMySQLCon) );
    }
    else
    {	// success
    	LogMessage( E_MSG_INFO, "MySQL Connection Info: %s", mysql_get_host_info(m_pMySQLCon) );
    	LogMessage( E_MSG_INFO, "MySQL Client Info: %s", mysql_get_client_info() );
    	LogMessage( E_MSG_INFO, "MySQL Server Info: %s", mysql_get_server_info(m_pMySQLCon) );
    }

	return bRc;
}

void CMysql::Disconnect()
{
	if ( m_pMySQLCon != NULL )
	{
		mysql_close(m_pMySQLCon);
		m_pMySQLCon = NULL;
	}
}


int CMysql::RunQuery( const char* szQuery, ... )
{
	int iRetry = 2;
	int rc = -1;
	int myerr = 0;
	va_list args;

	va_start( args, szQuery );

	vsnprintf( m_szQuery, sizeof(m_szQuery), szQuery, args );

	va_end( args );

	while ( iRetry > 0 && !gbTerminateNow )
	{
		iRetry -= 1;

		rc = mysql_query( m_pMySQLCon, m_szQuery );
		if ( rc != 0 )
		{	// error
			myerr = mysql_errno( m_pMySQLCon );
			LogMessage( E_MSG_ERROR, "mysql_query() error %d, %d (%s)", rc, myerr, mysql_error(m_pMySQLCon) );

			if ( myerr == CR_SERVER_GONE_ERROR || myerr == CR_SERVER_LOST )
			{	// try to reconnect
				LogMessage( E_MSG_INFO, "Mysql has gone away, try to reconnect..." );

				Disconnect();
				usleep( 100000 );
				Connect();

				LogMessage( E_MSG_INFO, "Retry the query" );
			}
		}
		else
		{	// success
			iRetry = 0;
		}
		m_pResult = mysql_store_result( m_pMySQLCon );
	}

	return rc;
}

MYSQL_ROW CMysql::FetchRow( int& iNumFields )
{
	iNumFields = mysql_num_fields( m_pResult );

	return mysql_fetch_row( m_pResult );
}

void CMysql::FreeResult()
{
	mysql_free_result(m_pResult);
	m_pResult = NULL;
}

const char* CMysql::GetError()
{
	return mysql_error(m_pMySQLCon);
}

const char* CMysql::GetQuery()
{
	return m_szQuery;
}

void CMysql::AddSlashes( const char* szIn, char* szOut )
{
	mysql_real_escape_string( m_pMySQLCon, szOut, szIn, strlen(szIn) );
}

void CMysql::CleanupEventsTable()
{
	int rc1, rc2;
	int iCount1 = 0;
	int iCount2 = 0;
	int iNumFields;
	MYSQL_ROW row;

	LogMessage( E_MSG_INFO, "CleanupEventsTable() starting" );

	if ( RunQuery( "select count(*) from events" ) == 0 )
	{
		if ( (row = FetchRow( iNumFields )) )
		{
			iCount1 = atoi( row[0] );
		}
	}
	FreeResult();

	rc1 = RunQuery( "DELETE FROM events WHERE ev_Timestamp < DATE_SUB(NOW(), INTERVAL 13 MONTH)" );
	FreeResult();

	rc2 = RunQuery( "optimize table events" );
	FreeResult();

	if ( RunQuery( "select count(*) from events" ) == 0 )
	{
		if ( (row = FetchRow( iNumFields )) )
		{
			iCount2 = atoi( row[0] );
		}
	}
	FreeResult();

	LogMessage( E_MSG_INFO, "CleanupEventsTable() %s: %d records deleted (%d,%d)", (rc1 == 0 && rc2 == 0 ? "success" : "failed"), iCount2 - iCount1, rc1, rc2 );
}

void CMysql::LogEvent( const int iDeviceNo, const int iIOChannel, enum E_EVENT_TYPE eEventType, const int iValue, const char* fmt, ... )
{
	char szBuf[256];
	char szOut[512];
	va_list args;

	va_start( args, fmt );

	vsnprintf( szBuf, sizeof(szBuf), fmt, args );

	va_end( args );

	AddSlashes( szBuf, szOut );

	if ( RunQuery( "insert into events (ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description) "
			"values(now(),%d,%d,%d,%d,\"%s\")", iDeviceNo, iIOChannel, eEventType, iValue, szOut ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}

	FreeResult();

}

time_t CMysql::ReadConfigUpdateTime()
{
	time_t tUpdated = 0;
	int iNumFields;
	MYSQL_ROW row;

	if ( RunQuery( "select unix_timestamp(ev_Timestamp) from events where ev_DeviceNo=-1" ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}
	else if ( (row = FetchRow( iNumFields )) )
	{
		tUpdated = atol( row[0] );
	}

	FreeResult();

	return tUpdated;
}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>

#include <modbus/modbus.h>
#include <mariadb/errmsg.h>

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

	rc1 = RunQuery( "DELETE FROM events WHERE ev_Timestamp < DATE_SUB(NOW(), INTERVAL 13 MONTH) and ev_Timestamp!='0000-00-00'" );
	FreeResult();

	rc2 = RunQuery( "optimize table events" );
	FreeResult();

	if ( RunQuery( "select count(*) from events" ) == 0 )
	{
		if ( (row = FetchRow( iNumFields )) )
		{
			iCount2 = atoi( (const char*)row[0] );
		}
	}
	FreeResult();

	// some records can be added to the table while these queries are running
	LogMessage( E_MSG_INFO, "CleanupEventsTable() %s: %d -> %d, %d records deleted (%d,%d)", (rc1 == 0 && rc2 == 0 ? "success" : "failed"), iCount1, iCount2, iCount1 - iCount2, rc1, rc2 );
}

void CMysql::LogEvent( const int iDeviceNo, const int iIOChannel, enum E_EVENT_TYPE eEventType, const double dValue, const char* fmt, ... )
{
	char szBuf[256];
	char szOut[512];
	va_list args;

	va_start( args, fmt );

	vsnprintf( szBuf, sizeof(szBuf), fmt, args );

	va_end( args );

	AddSlashes( szBuf, szOut );

	if ( RunQuery( "insert into events (ev_Timestamp,ev_DeviceNo,ev_IOChannel,ev_EventType,ev_Value,ev_Description) "
			"values(now(),%d,%d,%d,%.3f,\"%s\")", iDeviceNo, iIOChannel, eEventType, dValue, szOut ) != 0 )
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
		tUpdated = atol( (const char*)row[0] );
	}

	FreeResult();

	return tUpdated;
}

time_t CMysql::ReadPlcStatesUpdateTimeAll()
{
	time_t tUpdated = 0;
	int iNumFields;
	MYSQL_ROW row;

	if ( RunQuery( "select unix_timestamp(ev_Timestamp) from events where ev_DeviceNo=-4" ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}
	else if ( (row = FetchRow( iNumFields )) )
	{
		tUpdated = atol( (const char*)row[0] );
	}

	FreeResult();

	return tUpdated;
}

time_t CMysql::ReadPlcStatesUpdateTimeDelayTime()
{
	time_t tUpdated = 0;
	int iNumFields;
	MYSQL_ROW row;

	if ( RunQuery( "select unix_timestamp(ev_Timestamp) from events where ev_DeviceNo=-6" ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}
	else if ( (row = FetchRow( iNumFields )) )
	{
		tUpdated = atol( (const char*)row[0] );
	}

	FreeResult();

	return tUpdated;
}

int CMysql::ReadPlcStatesScreenButton()
{
	int iStateNo = 0;
	int iNumFields;
	MYSQL_ROW row;

	if ( RunQuery( "select ev_Value from events where ev_DeviceNo=-5" ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}
	else if ( (row = FetchRow( iNumFields )) )
	{
		iStateNo = atoi( (const char*)row[0] );
	}

	FreeResult();

	if ( RunQuery( "delete from events where ev_DeviceNo=-5" ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}

	return iStateNo;
}

int CMysql::SetNextPlcState( const char* szOperation, const char* szNextStateName, const char* szStateName, const time_t tTimenow )
{
	int rc = 0;

	if ( RunQuery( "update plcstates set pl_StateIsActive='Y',pl_StateTimestamp=from_unixtime(%ld) where pl_Operation='%s' and pl_StateName='%s' and pl_RuleType=''",
			tTimenow, szOperation, szNextStateName ) )
	{	// error
		rc = 1;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}
	else if ( RunQuery( "update plcstates set pl_StateIsActive='N' where pl_Operation='%s' and pl_StateName='%s' and pl_RuleType=''", szOperation, szStateName ) )
	{	// error
		rc = 2;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}

	return rc;
}

bool CMysql::UpdatePlcStateRuntimeValue( const char* szOperation, const char* szStateName, const int iDeviceNo, const int iIOChannel, const double dNewVal )
{
	int rc = 0;

	LogMessage( E_MSG_INFO, "UpdatePlcStateRuntimeValue() '%s' '%s' %d,%d", szOperation, szStateName, iDeviceNo, iIOChannel );

	if ( RunQuery( "update plcstates set pl_RuntimeValue=%.1f where pl_Operation='%s' and pl_StateName='%s' and pl_DeviceNo=%d and pl_IOChannel=%d", 
			dNewVal, szOperation, szStateName, iDeviceNo, iIOChannel ) )
	{	// error
		rc = 1;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}

	return rc;
}

bool CMysql::WebClickEvent( const int iDeviceNo, const int iIOChannel )
{
	bool bRet = false;
	int iNumFields;
	MYSQL_ROW row;
	char szBuf[100];

	snprintf( szBuf, sizeof(szBuf), "select unix_timestamp(ev_Timestamp) from events where ev_DeviceNo=%d and ev_IOChannel=%d", -(1000 + iDeviceNo), iIOChannel );
	if ( RunQuery( szBuf ) != 0 )
	{	// error
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
	}
	else if ( (row = FetchRow( iNumFields )) )
	{
		bRet = true;
	}

	FreeResult();

	if ( bRet )
	{
		snprintf( szBuf, sizeof(szBuf), "delete from events where ev_DeviceNo=%d and ev_IOChannel=%d", -(1000 + iDeviceNo), iIOChannel );
		if ( RunQuery( szBuf ) != 0 )
		{	// error
			LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", GetQuery(), GetError() );
		}
	}

	return bRet;
}


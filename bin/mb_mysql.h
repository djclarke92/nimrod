#ifndef _INC_MB_MYSQL_H
#define _INC_MB_MYSQL_H

#include <mysql.h>
#include "mb_devices.h"



class CMysql {
private:
	MYSQL *m_pMySQLCon;
	MYSQL_RES *m_pResult;
	char m_szHostName[100];
	char m_szUserId[100];
	char m_szPassword[100];
	char m_szDBName[100];
	char m_szQuery[4096];

public:
	CMysql();
	~CMysql();

	void LogEvent( const int iDeviceNo, const int iChannel, enum E_EVENT_TYPE eEventType, const int iValue, const char* fmt, ... );

	void Init();
	bool Connect();
	void Disconnect();
	void FreeResult();
	int RunQuery( const char* szQuery, ... );
	MYSQL_ROW FetchRow( int& iNumFields );
	const char* GetError();
	const char* GetQuery();
	void AddSlashes( const char* szIn, char* szOut );
	time_t ReadConfigUpdateTime();
	void CleanupEventsTable();
};



#endif

#ifndef _INC_MB_MAIN_H
#define _INC_MB_MAIN_H

#include "mb_utils.h"
#include "mb_devices.h"


#define STOP_NIMROD_FILE		"./stop-nimrod"

typedef struct {
	int iBaud;
	char cParity;
	int iDataBits;
	int iStopBits;
} RS485_CONNECTION_TYPE;

extern RS485_CONNECTION_TYPE gRS485;

extern bool gbTerminateNow;
extern bool gbCertificateError;
extern bool gbCertificateAging;
extern char gszHostname[HOST_NAME_MAX+1];



void SetMyHostname();
bool IsMyHostname( const char* szHost );
const char* GetMyHostname();


#define MAX_THREAD_MESSAGES		20
class CThreadMsg {
private:
	char m_szMsg[MAX_THREAD_MESSAGES][256];

public:
	CThreadMsg();
	~CThreadMsg();

	void Init();
	void PutMessage( const char* szFmt, ... );
	const bool GetMessage( char* szMsg, const size_t uLen );
};

#endif

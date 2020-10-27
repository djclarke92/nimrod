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
extern char gszHostname[HOST_NAME_MAX+1];



void SetMyHostname();
bool IsMyHostname( const char* szHost );
const char* GetMyHostname();



#endif

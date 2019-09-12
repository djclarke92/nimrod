#ifndef _INC_MB_UTILS_H
#define _INC_MB_UTILS_H


#define MAX_LOG_FILE_SIZE			5000000
#define MAX_LOG_BACKUPS				4
#define UPGRADE_SCRIPT				"./nimrod-upgrade.sh"


enum E_MSG_CLASS {
	E_MSG_DEBUG = 0,
	E_MSG_INFO,
	E_MSG_WARN,
	E_MSG_ERROR,
	E_MSG_FATAL
};


extern bool gbRestartNow;
extern bool gbUpgradeNow;
extern bool gbDebug;
extern time_t gtTarFileTimestamp;
extern char gszLogDir[256];
extern char gszProgName[256];
extern char gszTarFileName[256];


void LogMessage( enum E_MSG_CLASS msgClass, const char* fmt, ... );
bool ReadSiteConfig( const char* szName, char* szValue, size_t uLen );
pid_t CreateSSHTunnel();
void CheckForUpgrade();
bool CreateRestartScript( const char* szTar );



#endif

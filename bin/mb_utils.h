#ifndef _INC_MB_UTILS_H
#define _INC_MB_UTILS_H

#include <string>


#define MAX_LOG_FILE_SIZE			5000000
#define MAX_LOG_BACKUPS				4
#define UPGRADE_SCRIPT				"./nimrod-upgrade.sh"
#define PACKAGE_VER_FILE			"./package.txt"


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
extern char gszTarFileName[512];

const double TimeNowMS();
void LogMessage( enum E_MSG_CLASS msgClass, const char* fmt, ... );
bool ReadSiteConfig( const char* szName, char* szValue, size_t uLen );
time_t ReadDeviceConfig( CMysql& myDB, CDeviceList* pDeviceList, CInOutLinks* pIOLinks, bool bInit );
pid_t CreateSSHTunnel();
bool CheckForUpgrade();
bool CreateRestartScript( const char* szTar );
bool CreatePackageVerFile( const char* szTar );

int SetComInterfaceAttribs( int iHandle, int iSpeed, int iNumBits, int iParity );
void SetComBlocking( int iHandle, bool bShouldBlock );

std::string base64_decode(std::string const& encoded_string);
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string urlEncode(std::string str);


#endif

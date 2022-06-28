#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>

#include <modbus/modbus.h>
#include "mb_devices.h"
#include "mb_thread.h"
#include "mb_main.h"
#include "mb_utils.h"


bool gbUpgradeNow = false;
bool gbRestartNow = false;
time_t gtTarFileTimestamp = 0;
char gszLogDir[256] = {""};
char gszProgName[256] = {""};
char gszTarFileName[256] = {""};


void LogMessage( enum E_MSG_CLASS msgClass, const char* fmt, ... )
{
	int i;
	char szClass[10];
	char szBuf[4096];
	char szLogFile[256];
	char szLogFile1[256];
	char szLogFile2[256];
	time_t timenow;
	struct tm* tmptr;
	struct stat statbuf;
	va_list args;
	FILE* pFile = NULL;

	pthread_mutex_lock( &mutexLock[E_LT_LOGGING] );

	switch ( msgClass )
	{
	default:
	case E_MSG_DEBUG:
		strcpy( szClass, "DEBUG" );
		if ( !gbDebug )
		{	// skip debug messages if debug is not turned on
			pthread_mutex_unlock( &mutexLock[E_LT_LOGGING] );
			return;
		}
		break;
	case E_MSG_INFO:
		strcpy( szClass, "INFO" );
		break;
	case E_MSG_WARN:
		strcpy( szClass, "WARN" );
		break;
	case E_MSG_ERROR:
		strcpy( szClass, "ERROR" );
		break;
	case E_MSG_FATAL:
		strcpy( szClass, "FATAL" );
		break;
	}

	timenow = time(NULL);
	tmptr = localtime( &timenow );
	snprintf( szBuf, sizeof(szBuf), "%d%02d%02d-%02d%02d%02d %-5s: ", tmptr->tm_year+1900, tmptr->tm_mon+1, tmptr->tm_mday,
			tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec, szClass );

	va_start( args, fmt );

	vsnprintf( &szBuf[strlen(szBuf)], sizeof(szBuf), fmt, args );

	va_end( args );

	// log to stdout as well
	//fprintf( stdout, szBuf );
	//fprintf( stdout, "\n" );

	// write to our log file
	snprintf( szLogFile, sizeof(szLogFile), "%s/nimrod.log", gszLogDir );
	pFile = fopen( szLogFile, "at" );
	if ( pFile != NULL )
	{
		fputs( szBuf, pFile );
		fputs( "\n", pFile );

		fclose( pFile );
	}

	// check max file size
	if ( stat( szLogFile, &statbuf ) == 0 )
	{
		if ( statbuf.st_size > MAX_LOG_FILE_SIZE )
		{
			snprintf( szLogFile2, sizeof(szLogFile2), "%s.%d", szLogFile, MAX_LOG_BACKUPS );
			unlink( szLogFile2 );

			for ( i = MAX_LOG_BACKUPS; i > 0; i-- )
			{
				snprintf( szLogFile2, sizeof(szLogFile2), "%s.%d", szLogFile, i );

				if ( i-1 == 0 )
					snprintf( szLogFile1, sizeof(szLogFile1), "%s", szLogFile );
				else
					snprintf( szLogFile1, sizeof(szLogFile1), "%s.%d", szLogFile, i-1 );

				rename( szLogFile1, szLogFile2 );
			}
		}
	}

	pthread_mutex_unlock( &mutexLock[E_LT_LOGGING] );
}

bool ReadSiteConfig( const char* szName, char* szValue, size_t uLen )
{
	bool bRc = false;
	char* cptr;
	char* cptr2;
	char szBuf[256];
	char szFile[256];
	FILE* pFile = NULL;

	szValue[0] = '\0';

	snprintf( szFile, sizeof(szFile), "./files/site_config.php" );
	pFile = fopen( szFile, "rt" );
	if ( pFile != NULL )
	{
		while ( (cptr = fgets( szBuf, sizeof(szBuf), pFile )) != NULL )
		{
			cptr = strrchr( szBuf, '\n' );
			if ( cptr != NULL )
				*cptr = '\0';

			if ( szBuf[0] != '/' )
			{
				cptr = strstr( szBuf, szName );
				if ( cptr != NULL )
				{
					// strip leading crap
					cptr += strlen( szName ) + 1;
					while ( *cptr != '\0' && (*cptr == ' ' || *cptr == '\'' || *cptr == ',') )
						cptr += 1;

					// strip trailing crap
					cptr2 = cptr;
					while ( *cptr2 != '\0' && *cptr2 != '\'' )
						cptr2 += 1;
					*cptr2 = '\0';

					snprintf( szValue, uLen, "%s", cptr );

					bRc = true;
				}
			}
		}

		fclose( pFile );
	}
	else
	{
		LogMessage( E_MSG_ERROR, "Failed to open file '%s' for reading, errno %d", szFile, errno );
	}

	return bRc;
}

time_t ReadDeviceConfig( CMysql& myDB, CDeviceList* pDeviceList, CInOutLinks* pIOLinks, bool bInit )
{
	LogMessage( E_MSG_INFO, "ReadDeviceConfig(%d)", bInit );

	pthread_mutex_lock( &mutexLock[E_LT_MODBUS] );

	pDeviceList->ReadDeviceConfig( myDB );
	pIOLinks->ReadIOLinks( myDB );

	int iMax = MAX_DEVICES;

	// reset the last recorded time so we take a new reading
	for ( int idx = 0; idx < iMax; idx++ )
	{
		for ( int i = 0; i < pDeviceList->GetNumInputs(idx); i++ )
		{
			pDeviceList->GetLastRecorded(idx,i) = 0;
		}
	}


	pthread_mutex_unlock( &mutexLock[E_LT_MODBUS] );

	return time(NULL);
}

pid_t CreateSSHTunnel()
{
	int iSleep = 2;
	pid_t pidChild = 0;
	char szHost[50];
	char szBuf[256];
	struct stat statbuf;

	if ( ReadSiteConfig( "REMOTE_MYSQL_HOST", szHost, sizeof(szHost) ) )
	{
		if ( stat( "./no.tunnel", &statbuf ) == 0 || strcasecmp( szHost, "localhost" ) == 0 || strcasecmp( szHost, "127.0.0.1" ) == 0 )
		{
			LogMessage( E_MSG_INFO, "Skipping ssh tunnel for host %s", szHost );
		}
		else
		{	// create our mysql tunnel
			pidChild = fork();

			if ( pidChild == -1 )
			{	// error, failed to fork()
				LogMessage( E_MSG_ERROR, "CreateSSHTunnel() fork() failed with errno %d", errno );
			}
			else if ( pidChild > 0 )
			{	// this is the parent
				LogMessage( E_MSG_INFO, "CreateSSHTunnel() child pid is %d", pidChild );

				LogMessage( E_MSG_INFO, "Sleeping %d sec to allow tunnel to setup", iSleep );
				sleep( iSleep );
			}
			else
			{   // this is the child
				snprintf( szBuf, sizeof(szBuf), "ssh -N -L 3306:127.0.0.1:3306 nimrod@%s", szHost );
	//			snprintf( szBuf, sizeof(szBuf), "ssh -N -L 3306:%s:3306 nimrod@%s", szHost, szHost );

				execlp( "/bin/bash", "/bin/bash", "-c", szBuf, NULL );

				_exit(EXIT_FAILURE);   // exec never returns
			}
		}
	}

	return pidChild;
}

#define MAX_DIR_LIST	4
void CheckForUpgrade()
{
	int i;
	int idx;
	DIR *d;
	struct stat statbuf;
	struct dirent *dir;
	char szDirList[MAX_DIR_LIST][100];

	if ( stat( "./no.upgrade", &statbuf ) == 0 || gbTerminateNow )
	{
		return;
	}

	// check for file in /var/www/html/uploads
	d = opendir( "/var/www/html/uploads" );
	if ( d != NULL )
	{
		while ( (dir = readdir(d) ) != NULL)
	    {
			if ( dir->d_type == DT_REG && strstr( dir->d_name, ".tgz" ) != NULL && strncmp( dir->d_name, "nimrod", 6 ) == 0 )
			{
				char szSrc[256];
				char szDest[256];
				snprintf( szSrc, sizeof(szSrc), "/var/www/html/uploads/%s", dir->d_name );
				snprintf( szDest, sizeof(szDest), "/var/www/html/%s", dir->d_name );

				LogMessage( E_MSG_INFO, "Found uploaded tgz file '%s'", szSrc );
				if ( rename( szSrc, szDest ) != 0 )
				{	// failed
					LogMessage( E_MSG_ERROR, "Failed to rename uploaded tgz file, errno %d", errno );

					unlink( szSrc );
				}
				break;
			}
	    }

	    closedir(d);
	}


	// check /var/www/html dir first
	d = opendir( "." );
	if ( d != NULL )
	{
		while ( (dir = readdir(d) ) != NULL)
	    {
			if ( dir->d_type == DT_REG && strstr( dir->d_name, ".tgz" ) != NULL && strncmp( dir->d_name, "nimrod", 6 ) == 0 )
			{
				snprintf( gszTarFileName, sizeof(gszTarFileName), "%s", dir->d_name );
				break;
			}
	    }

	    closedir(d);
	}

	if ( strlen( gszTarFileName ) == 0 )
	{	// check for usb stick at /media/nimrod/*
		for ( i = 0; i < MAX_DIR_LIST; i++ )
		{
			szDirList[i][0] = '\0';
		}
		d = opendir( "/media/nimrod" );
		if ( d != NULL )
		{
			idx = 0;
			while ( (dir = readdir(d) ) != NULL)
		    {
				if ( dir->d_type == DT_DIR && dir->d_name[0] != '.' )
				{
					snprintf( szDirList[idx], sizeof(szDirList[idx]), "/media/nimrod/%s", dir->d_name );
					LogMessage( E_MSG_INFO, "Found usb dir '%s'", szDirList[idx] );
					if ( idx+1 < MAX_DIR_LIST )
					{
						idx += 1;
					}
					else
					{
						LogMessage( E_MSG_WARN, "Too many usb dirs" );
						break;
					}
				}
		    }

		    closedir(d);

		    // now check each dir for a tgz file
		    for ( idx = 0; idx < MAX_DIR_LIST; idx++ )
			{
				if ( szDirList[idx][0] != '\0' )
				{
					d = opendir( szDirList[idx] );
					if ( d != NULL )
					{
						while ( (dir = readdir(d) ) != NULL)
					    {
							if ( dir->d_type == DT_REG && strstr( dir->d_name, ".tgz" ) != NULL && strncmp( dir->d_name, "nimrod", 6 ) == 0 )
							{
								snprintf( gszTarFileName, sizeof(gszTarFileName), "%s", dir->d_name );
								break;
							}
					    }

					    closedir(d);
					}

					if ( strlen( gszTarFileName ) != 0 )
					{	// found a tar file
						// move the tar file to our local directory
						int rc;
						char szCmd[512];

						snprintf( szCmd, sizeof(szCmd), "cp %s/%s .; rm %s/%s; sync; umount %s", szDirList[idx], gszTarFileName, szDirList[idx], gszTarFileName, szDirList[idx] );
						rc = system( szCmd );

						LogMessage( E_MSG_INFO, "system(%s) returned %d", szCmd, rc );
						gszTarFileName[0] = '\0';
						break;
					}
				}
			}
		}
	}

	if ( strlen( gszTarFileName ) != 0 )
	{
		LogMessage( E_MSG_INFO, "Found upgrade tar file '%s'", gszTarFileName );

		if ( stat( gszTarFileName, &statbuf ) == 0 )
		{
			if ( statbuf.st_mtime + 5 < time(NULL) )
			{
				if ( CreateRestartScript( gszTarFileName ) )
				{
					CreatePackageVerFile( gszTarFileName );
					gbUpgradeNow = true;
					gbTerminateNow = true;
				}
			}
			else
			{	// too soon to use the tar file, it might still be being copied
				gszTarFileName[0] = '\0';
			}
		}
		else
		{
			LogMessage( E_MSG_ERROR, "stat(%s) failed with errno %d", gszTarFileName, errno );
		}
	}
}

bool CreateRestartScript( const char* szTar )
{
	bool bRc = true;
	char szBuf[256];
	FILE* pFile = NULL;

	// create upgrade script
	pFile = fopen( UPGRADE_SCRIPT, "wt" );
	if ( pFile != NULL )
	{
		fputs( "#!/bin/bash\n", pFile );
		fputs( "\n", pFile );
		fputs( "#sleep 2\n", pFile );

		if ( szTar != NULL )
		{
			snprintf( szBuf, sizeof(szBuf), "tar xvf %s >> %s/nimrod.log 2>&1\n", szTar, gszLogDir );
			fputs( szBuf, pFile );
		}

		// we will be restarted by inittab
		//snprintf( szBuf, sizeof(szBuf), "nohup ./scripts/%s >/dev/null 2>&1 &\n", gszProgName );
		//fputs( szBuf, pFile );

		if ( szTar != NULL )
		{
			snprintf( szBuf, sizeof(szBuf), "rm -rf %s >> %s/nimrod.log 2>&1\n", szTar, gszLogDir );
			fputs( szBuf, pFile );
		}

		snprintf( szBuf, sizeof(szBuf), "rm -rf nohup.out >> %s/nimrod.log 2>&1\n", gszLogDir );
		fputs( szBuf, pFile );
		fputs( "\n", pFile );

		fclose( pFile );

		// make sure the script is executable
		snprintf( szBuf, sizeof(szBuf), "chmod 0755 %s", UPGRADE_SCRIPT );
		if ( system( szBuf ) < 0 )
		{
			LogMessage( E_MSG_ERROR, "system(chmod) failed with errno %d", errno );
		}
	}
	else
	{
		bRc = false;
		LogMessage( E_MSG_ERROR, "fopen(%s) failed with errno %d", UPGRADE_SCRIPT, errno );
	}

	return bRc;
}

bool CreatePackageVerFile( const char* szTar )
{
	bool bRc = true;
	char szBuf[256];
	FILE* pFile = NULL;

	LogMessage( E_MSG_INFO, "Creating package file for %s", szTar );

	// create package.txt file
	pFile = fopen( PACKAGE_VER_FILE, "wt" );
	if ( pFile != NULL )
	{
		snprintf( szBuf, sizeof(szBuf), "%s", szTar );
		fputs( basename(szBuf), pFile );
		fputs( "\n", pFile );

		fclose( pFile );
	}
	else
	{
		bRc = false;
		LogMessage( E_MSG_ERROR, "fopen(%s) failed with errno %d", PACKAGE_VER_FILE, errno );
	}

	return bRc;
}

int SetComInterfaceAttribs( int iHandle, int iSpeed, int iNumBits, int iParity )
{
	struct termios tty;

	if ( tcgetattr(iHandle, &tty) != 0)
	{
		LogMessage( E_MSG_ERROR, "error %d from tcgetattr", errno );
		return -1;
	}

	cfsetospeed(&tty, iSpeed);
	cfsetispeed(&tty, iSpeed);

	if ( iNumBits == 5 )
		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;     // 5-bit chars
	else if ( iNumBits == 6 )
		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;     // 6-bit chars
	else if ( iNumBits == 7 )
		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;     // 7-bit chars
	else
		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
									// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays

	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
									// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= iParity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if ( tcsetattr(iHandle, TCSANOW, &tty) != 0)
	{
		LogMessage( E_MSG_ERROR, "error %d from tcsetattr", errno );
		return -1;
	}

	LogMessage( E_MSG_INFO, "Com attributes set to 0x%x,%d,%d,1", iSpeed, iNumBits, iParity );

	return 0;
}

void SetComBlocking( int iHandle, bool bShouldBlock )
{
	struct termios tty;

	memset(&tty, 0, sizeof tty);

	if ( tcgetattr(iHandle, &tty) != 0 )
	{
		LogMessage( E_MSG_ERROR, "SetComBlocking: error %d from tggetattr", errno );
		return;
	}

	tty.c_cc[VMIN]  = bShouldBlock ? 1 : 0;
	tty.c_cc[VTIME] = 1;            // 0.5 seconds read timeout

	if ( tcsetattr(iHandle, TCSANOW, &tty) != 0 )
	{
		LogMessage( E_MSG_ERROR, "SetComBlocking: error %d setting term attributes", errno );
	}
	else
	{
		LogMessage( E_MSG_INFO, "Com blocking set to %s", (bShouldBlock ? "ON" : "OFF") );
	}
}

/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string)
{
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

std::string urlEncode(std::string str)
{
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++){
        c = chars[i];
        ic = c;
        // uncomment this if you want to encode spaces with +
        /*if (c==' ') new_str += '+';
        else */if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
        else {
            sprintf(bufHex,"%X",c);
            if(ic < 16)
                new_str += "%0";
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
}

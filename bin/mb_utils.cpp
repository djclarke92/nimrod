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
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

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
	fprintf( stdout, szBuf );
	fprintf( stdout, "\n" );

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
			snprintf( szBuf, sizeof(szBuf), "tar xvf %s 2>&1 >> ~/nimrod.log\n", szTar );
			fputs( szBuf, pFile );
		}

		// we will be restarted by inittab
		//snprintf( szBuf, sizeof(szBuf), "nohup ./scripts/%s 2>&1 >/dev/null &\n", gszProgName );
		//fputs( szBuf, pFile );

		if ( szTar != NULL )
		{
			snprintf( szBuf, sizeof(szBuf), "rm -rf %s 2>&1 >> ~/nimrod.log\n", szTar );
			fputs( szBuf, pFile );
		}

		snprintf( szBuf, sizeof(szBuf), "rm -rf nohup.out 2>&1 >> ~/nimrod.log\n" );
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

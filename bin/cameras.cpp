#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>
//#include <arpa/inet.h>
//#include <openssl/ssl.h>
//#include <openssl/err.h>
//#include <openssl/tls1.h>
//#include <openssl/ssl.h>

#include <modbus/modbus.h>
#include "mb_devices.h"
#include "mb_thread.h"
#include "mb_main.h"
#include "mb_mysql.h"
#include "mb_socket.h"






//**************************************************************************************************************
//
//	CCamera classes
//
//**************************************************************************************************************
CCameraList::CCameraList()
{
	Init();
}

CCameraList::~CCameraList()
{

}

void CCameraList::Init()
{
	m_iNumCameras = 0;
	m_iSnapshotIdx = 0;

	for ( int i = 0; i < MAX_CAMERAS; i++ )
	{
		m_List[i].Init();
	}
}

void CCameraList::AddCamera()
{
	if ( m_iNumCameras + 1 < MAX_CAMERAS )
	{
		m_iNumCameras += 1;
	}
	else
	{
		LogMessage( E_MSG_ERROR, "Too many cameras added,, %d", m_iNumCameras );
	}
}

void CCameraList::NextSnapshotIdx()
{
	if ( m_iSnapshotIdx + 1 < m_iNumCameras )
	{
		m_iSnapshotIdx += 1;
	}
	else
	{
		m_iSnapshotIdx = 0;
	}
}

const int CCameraList::GetSnapshotIdx()
{
	return m_iSnapshotIdx;
}

const int CCameraList::GetNumCameras()
{
	return m_iNumCameras;
}

CCamera& CCameraList::GetSnapshotCamera()
{
	return GetCamera( m_iSnapshotIdx );
}

CCamera& CCameraList::GetCamera( const int idx )
{
	if ( idx >= 0 && idx < MAX_CAMERAS )
	{
		return m_List[idx];
	}

	LogMessage( E_MSG_ERROR, "Camera index %d out of range", idx );

	return m_List[0];
}

CCamera::CCamera()
{
	Init();
}

CCamera::~CCamera()
{

}

void CCamera::Init()
{
	m_iCameraNo = 0;
	m_szName[0] = '\0';
	m_szIPAddress[0] = '\0';
	m_szPTZ[0] = '\0';
	m_szEncoding[0] = '\0';
	m_szDirectory[0] = '\0';
	m_szUserId[0] = '\0';
	m_szPassword[0] = '\0';
	m_szModel[0] = '\0';
	m_szMJpeg[0] = '\0';
}

void CCamera::SetCameraNo( const int iCameraNo )
{
	m_iCameraNo = iCameraNo;
}

void CCamera::SetName( const char* szName )
{
	snprintf( m_szName, sizeof(m_szName), "%s", szName );
}

void CCamera::SetIPAddress( const char* szIPAddress )
{
	snprintf( m_szIPAddress, sizeof(m_szIPAddress), "%s", szIPAddress );
}

void CCamera::SetPTZ( const char* szPTZ )
{
	snprintf( m_szPTZ, sizeof(m_szPTZ), "%s", szPTZ );
}

void CCamera::SetEncoding( const char* szEncoding )
{
	snprintf( m_szEncoding, sizeof(m_szEncoding), "%s", szEncoding );
}

void CCamera::SetDirectory( const char* szDirectory )
{
	snprintf( m_szDirectory, sizeof(m_szDirectory), "%s", szDirectory );
}

void CCamera::SetUserId( const char* szUserId )
{
	snprintf( m_szUserId, sizeof(m_szUserId), "%s", szUserId );
}

void CCamera::SetPassword( const char* szPassword )
{
	snprintf( m_szPassword, sizeof(m_szPassword), "%s", szPassword );
}

void CCamera::SetModel( const char* szModel )
{
	snprintf( m_szModel, sizeof(m_szModel), "%s", szModel );
}

void CCamera::SetMJpeg( const char* szMJpeg )
{
	snprintf( m_szMJpeg, sizeof(m_szMJpeg), "%s", szMJpeg );
}

const int CCamera::GetCameraNo()
{
	return m_iCameraNo;
}

const char* CCamera::GetName()
{
	return m_szName;
}

const char* CCamera::GetIPAddress()
{
	return m_szIPAddress;
}

const char* CCamera::GetPTZ()
{
	return m_szPTZ;
}

const char* CCamera::GetEncoding()
{
	return m_szEncoding;
}

const char* CCamera::GetDirectory()
{
	return m_szDirectory;
}

const char* CCamera::GetUserId()
{
	return m_szUserId;
}

// decode the pwd
void CCamera::GetPassword( char* szPwd, size_t uLen )
{
	char* pszPwd = base64_decode( m_szPassword );
	snprintf( szPwd, uLen, "%s", pszPwd );
	free( pszPwd );
} 

const char* CCamera::GetModel()
{
	return m_szModel;
}

const char* CCamera::GetMJpeg()
{
	return m_szMJpeg;
}



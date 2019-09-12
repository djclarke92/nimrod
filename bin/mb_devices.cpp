#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <modbus/modbus.h>
#include "mb_mysql.h"
#include "mb_main.h"
#include "mb_devices.h"




//*****************************************************************************************
//
//	CMyDevice
//
//*****************************************************************************************
CMyDevice::CMyDevice()
{
	Init();
}

CMyDevice::~CMyDevice()
{
	if ( m_pCtx != NULL )
	{
		modbus_free( m_pCtx );
		m_pCtx = NULL;
	}
}

void CMyDevice::Init()
{
	int j;
	struct timespec tspec;

	tspec.tv_nsec = 0;
	tspec.tv_sec = 0;

	SetDeviceNo( 0 );
	SetComPort( "" );
	SetDeviceName( "" );
	SetDeviceHostname( "" );
	SetContext( NULL );
	SetAddress( -1 );
	SetTimeoutCount( 0 );
	SetDeviceType( E_DT_UNUSED );
	SetDeviceStatus( E_DS_DEAD );
	SetNumInputs( 8 );
	SetNumOutputs( 8 );
	for ( j = 0; j < MAX_IO_PORTS; j++ )
	{
		SetInIOName( j, "" );
		SetOutIOName( j, "" );
		SetInWeekdays( j, "NNNNNNN" );
		SetOutWeekdays( j, "NNNNNNN" );
		GetInChannelType( j ) = E_IO_UNUSED;
		GetOutChannelType( j ) = E_IO_UNUSED;
		SetInOnPeriod( j, 0 );
		SetOutOnPeriod( j, 0 );
		SetEventTime( j, tspec );

		GetLastInput( j ) = 0;
		GetNewInput( j ) = 0;
		GetLastData( j ) = -1;
		GetLastLogData( j ) = -1;
		GetNewData( j ) = -1;
		GetOutput( j ) = 0;
		SetOutOnStartTime( j, 0 );
		SetStartTime( j, -1 );
		SetAnalogType( j, 'V' );
		SetCalcFactor( j, 0.0 );
		SetOffset( j, 0.0 );
		GetAlarmTriggered( j ) = 0;
		GetHysteresis( j ) = 0;
		GetTriggerTemperature( j ) = 0.0;
		GetLastRecorded( j ) = 0;
		GetTriggerVoltage( j ) = 0.0;
	}

}

void CMyDevice::SetInIOName( const int i, const char* pszName )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		snprintf( m_szInIOName[i], sizeof(m_szInIOName[i]), "%s", pszName );
	}
}

void CMyDevice::SetOutIOName( const int i, const char* pszName )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		snprintf( m_szOutIOName[i], sizeof(m_szOutIOName[i]), "%s", pszName );
	}
}

void CMyDevice::SetInWeekdays( const int i, const char* pszDays )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		snprintf( m_szInWeekdays[i], sizeof(m_szInWeekdays[i]), "%s", pszDays );
	}
}

void CMyDevice::SetOutWeekdays( const int i, const char* pszDays )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		snprintf( m_szOutWeekdays[i], sizeof(m_szOutWeekdays[i]), "%s", pszDays );
	}
}

void CMyDevice::SetOutOnStartTime( const int i, const time_t tt )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_tOutOnStartTime[i] = tt;
	}
}

void CMyDevice::SetInOnPeriod( const int i, const int iSec )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_iInputOnPeriod[i] = iSec;
	}
}

void CMyDevice::SetOutOnPeriod( const int i, const int iSec )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_iOutputOnPeriod[i] = iSec;
	}
}

void CMyDevice::SetStartTime( const int i, const int iMin )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_iStartTime[i] = iMin;
	}
}

void CMyDevice::SetAnalogType( const int i, const char cType )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		if ( cType == '\0' )
			m_cAnalogType[i] = 'V';
		else
			m_cAnalogType[i] = cType;
	}
}

void CMyDevice::SetCalcFactor( const int i, const double dFactor )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_dCalcFactor[i] = dFactor;
	}
}

void CMyDevice::SetOffset( const int i, const double dOffset )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_dOffset[i] = dOffset;
	}
}

void CMyDevice::SetEventTime( const int i, const struct timespec ts )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_tEventTime[i].tv_nsec = ts.tv_nsec;
		m_tEventTime[i].tv_sec = ts.tv_sec;
	}
}

void CMyDevice::SetComPort( const char* szPort )
{
	snprintf( m_szComPort, sizeof(m_szComPort), "%s", szPort );
}

void CMyDevice::SetDeviceName( const char* szName )
{
	snprintf( m_szDeviceName, sizeof(m_szDeviceName), "%s", szName );
}

void CMyDevice::SetDeviceHostname( const char* szHostname )
{
	snprintf( m_szDeviceHostname, sizeof(m_szDeviceHostname), "%s", szHostname );
}

void CMyDevice::SetDeviceStatus( const enum E_DEVICE_STATUS eStatus )
{
	if ( eStatus == E_DS_ALIVE )
	{
		m_iTimeoutCount = 0;
	}
	else
	{
		m_iTimeoutCount += 1;
	}

	//LogMessage( E_MSG_INFO, "SetDeviceSTatus: %d %d", m_eDeviceStatus, m_iTimeoutCount );
	if ( (m_eDeviceStatus == E_DS_DEAD || m_eDeviceStatus == E_DS_BURIED) && m_iTimeoutCount >= 10 )
	{
		m_eDeviceStatus = E_DS_BURIED;
	}
	else if ( eStatus == E_DS_DEAD && m_eDeviceStatus == E_DS_ALIVE )
	{	// first failure
		m_eDeviceStatus = E_DS_SUSPECT;
	}
	else
	{
		m_eDeviceStatus = eStatus;
	}
}

const bool CMyDevice::IsOffTime( const int j )
{
	bool bRc = false;
	time_t tNow;

	tNow = time(NULL);

	if ( j >= 0 && j < m_iNumOutputs )
	{
		if ( m_tOutOnStartTime[j] > 0 && m_iOutputOnPeriod[j] > 0 && m_tOutOnStartTime[j] + m_iOutputOnPeriod[j] <= tNow )
		{
			bRc = true;
		}
	}

	return bRc;
}

const double CMyDevice::CalcTemperature( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	uint16_t uVal;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( bNew )
			uVal = m_uNewData[iChannel];
		else
			uVal = m_uLastData[iChannel];

		if ( uVal == -1 )
		{	// sensor is not connected

		}
		else if ( uVal < 10000 )
		{	// temperature is > 0
			dVal = (double)uVal / 10;
		}
		else
		{	// temperature is < 0
			dVal = -(double)(uVal - 10000) / 10;
		}
	}

	return dVal;
}

const double CMyDevice::CalcVoltage( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dFactor;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		dFactor = m_dCalcFactor[iChannel];

		if ( bNew )
			dUnit = (double)(m_uNewData[iChannel]);
		else
			dUnit = (double)(m_uLastData[iChannel]);

		if ( m_cAnalogType[iChannel] == 'A' )
		{	// WCS1800 current sensor: 1A = 60mV, 0A = 2.65V, 5V max output
			// The voltage at 0A is different for each sensor
			//

			// convert units into mV
			dUnit = (dUnit * 10) / 4.095;	// mV


			dUnit -= m_dOffset[iChannel];	//2.650;	// mV
			dUnit *= dFactor;
		}
		else
		{	// voltage
			dUnit = (dUnit * 10) / 4.095;	// mV
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 )
		{	// sensor is not connected

		}
		else
		{
			dVal = ((double)dUnit / 1000) * dFactor;
		}
	}

	return dVal;
}

const bool CMyDevice::IsSensorConnected( const int iChannel )
{
	bool bRc = false;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( m_uNewData[iChannel] != (uint16_t)-1 )
		{
			bRc = true;
		}
	}

	return bRc;
}

const bool CMyDevice::WasSensorConnected( const int iChannel )
{
	bool bRc = false;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( m_uLastData[iChannel] != (uint16_t)-1 )
		{
			bRc = true;
		}
	}

	return bRc;
}

const bool CMyDevice::IsTimerEnabledToday( const int iChannel )
{
	bool bRc = false;
	time_t timenow;
	struct tm tt;

	timenow = time(NULL);

	if ( localtime_r( &timenow, &tt ) != NULL )
	{
		if ( tt.tm_wday < 0 || tt.tm_wday >= (int)sizeof(m_szInWeekdays[iChannel]) )
		{	// error
			LogMessage( E_MSG_ERROR, "IsTimerEnabledToday(): tt.tm_wday %d out of range", tt.tm_wday );
		}
		else if ( m_szInWeekdays[iChannel][tt.tm_wday] == 'Y' )
		{
			bRc = true;
		}
	}
	else
	{
		LogMessage( E_MSG_ERROR, "IsTimerEnabledToday(): localtime_r() failed with errno %d", errno );
	}

	return bRc;
}

const bool CMyDevice::TestValue( const char* szLinkTest, const double dLinkValue, const double dVal, bool& bInvertState )
{
	bool bRc = false;

	bInvertState = false;
	if ( szLinkTest[2] == '/' )
		bInvertState = true;

	LogMessage( E_MSG_INFO, "TestValue: %.1f '%s' %.1f, %d", dVal, szLinkTest, dLinkValue, bInvertState );
	if ( strcmp( szLinkTest, "EQ" ) == 0 || strcmp( szLinkTest, "EQ/" ) == 0  )
	{
		if ( dVal == dLinkValue )
			bRc = true;
	}
	else if ( strcmp( szLinkTest, "NE" ) == 0 || strcmp( szLinkTest, "NE/" ) == 0 )
	{
		if ( dVal != dLinkValue )
			bRc = true;
	}
	else if ( strcmp( szLinkTest, "LT" ) == 0 )
	{
		if ( dVal < dLinkValue )
			bRc = true;
	}
	else if ( strcmp( szLinkTest, "LE" ) == 0 )
	{
		if ( dVal <= dLinkValue )
			bRc = true;
	}
	else if ( strcmp( szLinkTest, "GT" ) == 0 )
	{
		if ( dVal > dLinkValue )
			bRc = true;
	}
	else if ( strcmp( szLinkTest, "GE" ) == 0 )
	{
		if ( dVal >= dLinkValue )
			bRc = true;
	}
	else
	{
		LogMessage( E_MSG_WARN, "TestValue: unhandled test type '%s', szLinkTest" );
	}

	return bRc;
}

const bool CMyDevice::LinkTestPassed( const int iLinkChannel, const char* szLinkTest, const double dLinkValue, bool& bInvertState )
{
	bool bRc = false;
	double dVal;

	if ( iLinkChannel >= 0 && iLinkChannel < MAX_IO_PORTS )
	{
		switch ( m_eDeviceType )
		{
		default:
			break;
		case E_DT_TEMPERATURE:
			dVal = CalcTemperature( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_VOLTAGE:
			dVal = CalcVoltage( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_DIGITAL_IO:
			dVal = m_uNewInput[iLinkChannel];
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_TIMER:
			// TODO
			LogMessage( E_MSG_WARN, "LinkTestPassed: E_DT_TIMER not implemeneted" );
			break;
		}
	}

	return bRc;
}


//*****************************************************************************************
//
//	CMyDevices
//
//*****************************************************************************************
CDeviceList::CDeviceList()
{
	m_szDummy[0] = '\0';

	Init();
}

CDeviceList::~CDeviceList()
{
	//m_DB.Disconnect();
}

void CDeviceList::Init()
{
	int i;

	//m_DB.Connect();

	m_iMcuMessageCount = 0;
	m_eDayNightState = E_DN_UNKNOWN;
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		m_szHostComPort[i][0] = '\0';
		m_pHostCtx[i] = NULL;

		m_Device[i].Init();
	}
}

bool CDeviceList::IsDeviceAlive( modbus_t* ctx, const char* szHostComPort, const int iAddr )
{
	bool bRc = false;
	uint16_t uiReg[2];
	uint8_t uiData[20];

	if ( modbus_read_registers( ctx, 0, 1, uiReg ) != -1 )
	{	// success
		LogMessage( E_MSG_INFO, "Device address %d exists on '%s' (modbus_read_registers)", iAddr, szHostComPort );
		bRc = true;
	}
	else if ( modbus_read_bits( ctx, 0, 2, uiData ) != -1 )
	{	// success
		LogMessage( E_MSG_INFO, "Device address %d exists on '%s' (modbus_read_bits)", iAddr, szHostComPort );
		bRc = true;
	}
	else if ( modbus_read_input_bits( ctx, 0, 2, uiData ) != -1 )
	{	// success
		LogMessage( E_MSG_INFO, "Device address %d exists on '%s' (modbus_read_input_bits)", iAddr, szHostComPort );
		bRc = true;
	}

	return bRc;
}

void CDeviceList::UpdateDeviceComPort( CMysql& myDB, const char* szNewComPort, const char* szOldComPort, char szPortList[MAX_DEVICES][MAX_COMPORT_LEN+1] )
{
	int j;

	// update the thread com port list
	for ( j = 0; j < MAX_DEVICES; j++ )
	{
		if ( szPortList[j][0] == '\0' )
		{
			break;
		}
		else if ( strcmp( szPortList[j], szOldComPort ) == 0 )
		{
			snprintf( szPortList[j], sizeof(szPortList[j]), "%s", szNewComPort );
			break;
		}
	}

	// change all other shared com ports to match
	for ( j = 0; j < MAX_DEVICES; j++ )
	{
		if ( GetDeviceNo(j) <= 0 )
		{	// end of list
			break;
		}
		else if ( strcmp( szOldComPort, GetComPort(j) ) == 0 && strcmp( GetDeviceHostname(j), gszHostname ) == 0 )
		{	// port is for this host
			LogMessage( E_MSG_INFO, "Device address %d com port now '%s', was '%s'", GetAddress(j), szNewComPort, GetComPort(j) );
			SetComPort( j, szNewComPort );

			UpdateDBDeviceComPort( myDB, j );
		}
	}

}

void CDeviceList::GetComPortsOnHost( CMysql& myDB, char szPortList[MAX_DEVICES][MAX_COMPORT_LEN+1] )
{
	bool bFound;
	bool bFirstTime;
	int i;
	int j;
	int idx;
	int iPos = 0;
	struct stat statbuf;
	char szPort[PATH_MAX];

	// get a list of actual com ports
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		snprintf( szPort, sizeof(szPort), "/dev/ttyUSB%d", i );
		if ( stat( szPort, &statbuf ) == 0 )
		{	// port does exist
			snprintf( m_szHostComPort[iPos], sizeof(m_szHostComPort[iPos]), "/dev/ttyUSB%d", i );
			iPos += 1;
		}
	}

	LogMessage( E_MSG_INFO, "Host '%s' has %d com ports", gszHostname, iPos );

	// connect a context to each real port
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_szHostComPort[i][0] != '\0' )
		{	// found a real com port
			m_pHostCtx[i] = modbus_new_rtu( m_szHostComPort[i], gRS485.iBaud, gRS485.cParity, gRS485.iDataBits, gRS485.iStopBits );
			if ( m_pHostCtx[i] == NULL )
			{	// failed
				LogMessage( E_MSG_ERROR, "modbus ctx in is null for '%s': %s", m_szHostComPort[i], modbus_strerror(errno) );
			}
			else if ( modbus_connect( m_pHostCtx[i] ) == -1 )
			{	// failed
				LogMessage( E_MSG_ERROR, "modbus_connect() failed for '%s': %s", m_szHostComPort[i], modbus_strerror(errno) );

				modbus_free( m_pHostCtx[i] );
				m_pHostCtx[i] = NULL;
			}
			else
			{	// connected
				LogMessage( E_MSG_INFO, "Modbus serial connection on '%s'", m_szHostComPort[i] );
			}
		}
		else
		{	// end of list
			break;
		}
	}

	// check which device is on each real com port
	for ( idx = 0; idx < MAX_DEVICES; idx++ )
	{
		if ( !IsMyHostname( GetDeviceHostname(idx) ) )
		{	// device is not on this host
			continue;
		}
		else if ( strncmp( GetComPort(idx), "MCU", 3 ) == 0 )
		{	// nodemcu device
			continue;
		}
		else if ( GetAddress(idx) > 0 )
		{
			bFirstTime = true;
			bFound = false;
			i = 0;
			while ( i < MAX_DEVICES )
			{
				// try the specified port first
				if ( bFirstTime )
				{
					for ( j = 0; j < MAX_DEVICES; j++ )
					{
						if ( strcmp( m_szHostComPort[j], GetComPort(idx) ) == 0 )
						{
							i = j;
							break;
						}
					}
				}

				if ( m_pHostCtx[i] != NULL )
				{	// this com port is connected
					if ( modbus_set_slave( m_pHostCtx[i], GetAddress(idx) ) == -1 )
					{	// failed
						LogMessage( E_MSG_ERROR, "modbus_set_slave() for '%s' on addr %d failed: %s", m_szHostComPort[i], GetAddress(idx), modbus_strerror(errno) );
					}
					else
					{
						usleep( 10000 );

						//LogMessage( E_MSG_INFO, "modbus slave connected for '%s' on address %d", m_szHostComPort[i], GetAddress(idx) );

						if ( (bFound = IsDeviceAlive( m_pHostCtx[i], m_szHostComPort[i], GetAddress(idx) )) )
						{
							if ( strcmp( m_szHostComPort[i], GetComPort(idx) ) != 0 )
							{	// com port has changed
								UpdateDeviceComPort( myDB, m_szHostComPort[i], GetComPort(idx), szPortList );
							}
							break;
						}
						else
						{	// failed - expected
							//LogMessage( E_MSG_WARN, "modbus_read_*() failed for address %d: %s", GetAddress(idx), modbus_strerror(errno) );
						}
					}
				}
				else
				{	// end of list
					break;
				}

				if ( bFirstTime )
				{
					bFirstTime = false;
					i = 0;
				}
				else
				{
					i += 1;
				}
			}

			if ( !bFound )
			{
				LogMessage( E_MSG_ERROR, "Device address %d did not respond", GetAddress(idx) );
			}
		}
		else if ( GetDeviceNo(idx) == 0 )
		{	// end of list
			break;
		}
	}

	// cleanup
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_szHostComPort[i][0] != '\0' )
		{	// found a real com port
			if ( m_pHostCtx[i] != NULL )
			{	// this com port is connected
				modbus_close( m_pHostCtx[i] );

				modbus_free( m_pHostCtx[i] );
				m_pHostCtx[i] = NULL;
			}
		}
		else
		{	// end of list
			break;
		}
	}

	LogMessage( E_MSG_INFO, "com port reconfiguration complete" );
}

bool CDeviceList::InitContext()
{
	bool bRc = true;
	bool bFound;
	int i;
	int idx;

	for ( idx = 0; idx < MAX_DEVICES; idx++ )
	{
		if ( m_Device[idx].GetDeviceNo() == 0 )
		{	// end of list
			break;
		}
		else if ( m_Device[idx].GetAddress() > 0 && !IsMyHostname(m_Device[idx].GetDeviceHostname()) )
		{	// this com port is not on this host
			continue;
		}

		bFound = false;
		for ( i = 0; i < idx; i++ )
		{
			if ( m_Device[i].GetAddress() > 0 && strcmp( m_Device[i].GetComPort(), m_Device[idx].GetComPort() ) == 0 &&
					IsMyHostname(m_Device[i].GetDeviceHostname()) )
			{	// already setup the context for this port
				bFound = true;
				break;
			}
		}

		if ( m_Device[i].GetAddress() == 0 )
		{	// timer
			LogMessage( E_MSG_INFO, "skip ctx for timer DeviceNo %d on '%s'", m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );
		}
		else if ( strncmp( m_Device[i].GetComPort(), "MCU", 3 ) == 0 )
		{	// nodemcu
			LogMessage( E_MSG_INFO, "skip ctx for MCU DeviceNo %d on '%s'", m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );
		}
		else if ( !bFound )
		{
			LogMessage( E_MSG_INFO, "Create ctx for serial connection on %s for DeviceNo %d", m_Device[idx].GetComPort(), m_Device[idx].GetDeviceNo() );

			m_Device[idx].SetContext( modbus_new_rtu( m_Device[idx].GetComPort(), gRS485.iBaud, gRS485.cParity, gRS485.iDataBits, gRS485.iStopBits ) );
			if ( m_Device[idx].GetContext() == NULL )
			{
				LogMessage( E_MSG_FATAL, "modbus ctx in is null for DeviceNo %d, aborting: %s", m_Device[idx].GetDeviceNo(), modbus_strerror(errno) );
				FreeAllContexts();
				bRc = false;
				break;
			}

			if ( modbus_connect(m_Device[idx].GetContext()) == -1 )
			{
				FreeAllContexts();
				LogMessage( E_MSG_FATAL, "modbus_connect() failed for DeviceNo %d, aborting: %s", m_Device[idx].GetDeviceNo(), modbus_strerror(errno) );
				bRc = false;
				break;
			}

			LogMessage( E_MSG_INFO, "Serial connection started on %s, ctx %p", m_Device[idx].GetComPort(), m_Device[idx].GetContext() );
		}
		else
		{
			LogMessage( E_MSG_INFO, "DeviceNo %d shares a COM port", m_Device[idx].GetDeviceNo() );
		}
	}

	return bRc;
}

void CDeviceList::FreeAllContexts()
{
	int i;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetContext() != NULL )
		{
			modbus_close( m_Device[i].GetContext() );

			modbus_free( m_Device[i].GetContext() );
			m_Device[i].SetContext( NULL );
		}
	}
}

const int CDeviceList::GetDeviceNo( const int idx )
{
	int iDeviceNo = 0;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		iDeviceNo = m_Device[idx].GetDeviceNo();
	}

	return iDeviceNo;
}

const int CDeviceList::GetAddress( const int idx )
{
	int iAddr = -1;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		iAddr = m_Device[idx].GetAddress();
	}

	return iAddr;
}

const enum E_DEVICE_TYPE CDeviceList::GetDeviceType( const int idx )
{
	enum E_DEVICE_TYPE eType = E_DT_UNUSED;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		eType = m_Device[idx].GetDeviceType();
	}

	return eType;
}

const enum E_DEVICE_STATUS CDeviceList::GetDeviceStatus( const int idx )
{
	enum E_DEVICE_STATUS eStatus = E_DS_DEAD;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		eStatus = m_Device[idx].GetDeviceStatus();
	}

	return eStatus;
}

const int CDeviceList::GetTimeoutCount( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetTimeoutCount();
	}

	return E_DS_DEAD;
}

modbus_t* CDeviceList::GetContext( const int idx )
{
	int i;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		for ( i = 0; i < MAX_DEVICES; i++ )
		{
			if ( strcmp( m_Device[idx].GetComPort(), m_Device[i].GetComPort() ) == 0 && IsMyHostname(m_Device[i].GetDeviceHostname()) )
			{
				return m_Device[i].GetContext();
			}
		}
	}

	return NULL;
}

const char* CDeviceList::GetComPort( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetComPort();
	}

	return m_szDummy;
}

const char* CDeviceList::GetDeviceName( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetDeviceName();
	}

	return m_szDummy;
}

const char* CDeviceList::GetDeviceHostname( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetDeviceHostname();
	}

	return m_szDummy;
}

// calling function must get E_LT_NODEMCU lock first
void CDeviceList::GetMcuResponseMsg( char* szMcuName, size_t uNameLen, char* szMcuResponseMsg, size_t uMsgLen )
{
	int i;

	szMcuName[0] = '\0';
	szMcuResponseMsg[0] = '\0';

	if ( m_iMcuMessageCount > 0 )
	{
		snprintf( szMcuName, uNameLen, m_szMcuName[0] );
		snprintf( szMcuResponseMsg, uMsgLen, m_szMcuResponseMsg[0] );

		m_iMcuMessageCount -= 1;
		m_szMcuName[0][0] = '\0';
		m_szMcuResponseMsg[0][0] = '\0';

		// remove the hole in the list
		for ( i = 1; i < MAX_MCU_QUEUE && m_iMcuMessageCount > 0; i++ )
		{
			strcpy( m_szMcuName[i-1], m_szMcuName[i] );
			strcpy( m_szMcuResponseMsg[i-1], m_szMcuResponseMsg[i] );
			m_szMcuName[i][0] = '\0';
			m_szMcuResponseMsg[i][0] = '\0';
		}
	}
}

const int CDeviceList::GetNumInputs( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetNumInputs();
	}

	return 0;
}

const int CDeviceList::GetNumOutputs( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetNumOutputs();
	}

	return 0;
}

uint8_t* CDeviceList::GetNewInput( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetNewInput();
	}

	return m_Device[0].GetNewInput();
}

uint16_t* CDeviceList::GetNewData( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetNewData();
	}

	return m_Device[0].GetNewData();
}

bool& CDeviceList::GetAlarmTriggered( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetAlarmTriggered(j);
	}

	return m_Device[0].GetAlarmTriggered(j);
}

int& CDeviceList::GetHysteresis( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetHysteresis(j);
	}

	return m_Device[0].GetHysteresis(j);
}

double& CDeviceList::GetTriggerTemperature( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetTriggerTemperature(j);
	}

	return m_Device[0].GetTriggerTemperature(j);
}

time_t& CDeviceList::GetLastRecorded( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetLastRecorded(j);
	}

	return m_Device[0].GetLastRecorded(j);
}

uint8_t& CDeviceList::GetLastInput( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetLastInput(j);
	}

	return m_Device[0].GetLastInput(j);
}

uint8_t& CDeviceList::GetNewInput( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetNewInput(j);
	}

	return m_Device[0].GetNewInput(j);
}

uint16_t& CDeviceList::GetLastData( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetLastData(j);
	}

	return m_Device[0].GetLastData(j);
}

uint16_t& CDeviceList::GetLastLogData( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetLastLogData(j);
	}

	return m_Device[0].GetLastLogData(j);
}

uint16_t& CDeviceList::GetNewData( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetNewData(j);
	}

	return m_Device[0].GetNewData(j);
}

uint8_t& CDeviceList::GetOutput( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOutput(j);
	}

	return m_Device[0].GetOutput(j);
}

enum E_IO_TYPE& CDeviceList::GetInChannelType( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetInChannelType(j);
	}

	return m_Device[0].GetInChannelType(j);
}

enum E_IO_TYPE& CDeviceList::GetOutChannelType( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOutChannelType(j);
	}

	return m_Device[0].GetOutChannelType(j);
}

const char* CDeviceList::GetInIOName( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetInIOName(j);
	}

	return m_Device[0].GetInIOName(j);
}

const char* CDeviceList::GetOutIOName( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOutIOName(j);
	}

	return m_Device[0].GetOutIOName(j);
}

const char* CDeviceList::GetInWeekdays( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetInWeekdays(j);
	}

	return m_Device[0].GetInWeekdays(j);
}

const char* CDeviceList::GetOutWeekdays( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOutWeekdays(j);
	}

	return m_Device[0].GetOutWeekdays(j);
}

const time_t CDeviceList::GetOutOnStartTime( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOutOnStartTime(j);
	}

	return 0;
}

const int CDeviceList::GetInOnPeriod( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetInOnPeriod(j);
	}

	return 0;
}

const int CDeviceList::GetOutOnPeriod( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOutOnPeriod(j);
	}

	return 0;
}

const int CDeviceList::GetStartTime( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetStartTime(j);
	}

	return 0;
}

const char CDeviceList::GetAnalogType( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetAnalogType(j);
	}

	return 'V';
}

const double CDeviceList::GetCalcFactor( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetCalcFactor(j);
	}

	return 0.0;
}

const double CDeviceList::GetOffset( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetOffset(j);
	}

	return 0.0;
}

void CDeviceList::GetEventTime( const int idx, const int j, struct timespec& tspec )
{
	tspec.tv_nsec = 0;
	tspec.tv_sec = 0;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].GetEventTime(j, tspec);
	}
}

// set a new day/night state based on the current voltage level
void CDeviceList::SetDayNightState( const double dVoltage )
{
	if ( dVoltage <= GetDayNightVoltage(E_DN_NIGHT) )
	{
		m_eDayNightState = E_DN_NIGHT;
	}
	else if ( dVoltage <= GetDayNightVoltage(E_DN_DAWNDUSK) )
	{
		m_eDayNightState = E_DN_DAWNDUSK;
	}
	else if ( dVoltage <= GetDayNightVoltage(E_DN_OVERCAST) )
	{
		m_eDayNightState = E_DN_OVERCAST;
	}
	else
	{
		m_eDayNightState = E_DN_DAY;
	}
}

const char* CDeviceList::GetDayNightStateName()
{
	static char sz_DN_Unknown[] = "Unknown";
	static char sz_DN_Night[] = "Night";
	static char sz_DN_DawnDusk[] = "Dawn/Dusk";
	static char sz_DN_Overcast[] = "Overcast";
	static char sz_DN_Day[] = "Day";

	switch ( m_eDayNightState )
	{
	default:
	case E_DN_UNKNOWN:
		return sz_DN_Unknown;
		break;
	case E_DN_NIGHT:
		return sz_DN_Night;
		break;
	case E_DN_DAWNDUSK:
		return sz_DN_DawnDusk;
		break;
	case E_DN_OVERCAST:
		return sz_DN_Overcast;
		break;
	case E_DN_DAY:
		return sz_DN_Day;
		break;
	}
}

// return the voltage level for the current day/night state
const double CDeviceList::GetDayNightVoltage( const enum E_DAY_NIGHT_STATE eState )
{
	double dVal = 0.0;
	enum E_DAY_NIGHT_STATE eDNState = eState;

	if ( eDNState == E_DN_UNKNOWN )
	{	// use the current state
		eDNState = m_eDayNightState;
	}

	switch ( eDNState )
	{
	default:
	case E_DN_UNKNOWN:
		dVal = 0.0;
		break;
	case E_DN_NIGHT:
		dVal = 10.0;
		break;
	case E_DN_DAWNDUSK:
		dVal = 20.0;
		break;
	case E_DN_OVERCAST:
		dVal = 30.0;
		break;
	case E_DN_DAY:
		dVal = 100.0;
		break;
	}
	//LogMessage( E_MSG_INFO, "GetDayNightVoltage() %d %.1fV", eDNState, dVal );

	return dVal;
}

double& CDeviceList::GetTriggerVoltage( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetTriggerVoltage(j);
	}

	return m_Device[0].GetTriggerVoltage(j);
}

const double CDeviceList::CalcTemperature( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcTemperature( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcVoltage( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcVoltage( iChannel, bNew );
	}

	return 0.0;
}

const bool CDeviceList::IsSensorConnected( const int idx, const int iChannel )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].IsSensorConnected(iChannel);
	}

	return false;
}

const bool CDeviceList::WasSensorConnected( const int idx, const int iChannel )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].WasSensorConnected(iChannel);
	}

	return false;
}

const bool CDeviceList::IsTimerEnabledToday( const int idx, const int iChannel )
{
	bool bRc = false;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		bRc = m_Device[idx].IsTimerEnabledToday( iChannel );
	}

	return bRc;
}

void CDeviceList::SetContext( const int idx, modbus_t* pCtx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetContext( pCtx );
	}
}

void CDeviceList::SetDeviceNo( const int idx, const int iDeviceNo )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetDeviceNo( iDeviceNo );
	}
}

void CDeviceList::SetComPort( const int idx, const char* szPort )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetComPort( szPort );
	}
}

void CDeviceList::SetDeviceName( const int idx, const char* szName )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetDeviceName( szName );
	}
}

void CDeviceList::SetDeviceHostname( const int idx, const char* szHostname )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetDeviceHostname( szHostname );
	}
}

// calling function must get E_LT_NODEMCU lock first
void CDeviceList::SetMcuResponseMsg( const char* szMcuName, const char* szMcuResponseMsg )
{
	if ( m_iMcuMessageCount <= 0 && m_iMcuMessageCount < MAX_MCU_QUEUE )
	{	// found an empty slot
		snprintf( m_szMcuResponseMsg[m_iMcuMessageCount], sizeof(m_szMcuResponseMsg[m_iMcuMessageCount]), "%s", szMcuResponseMsg );
		snprintf( m_szMcuName[m_iMcuMessageCount], sizeof(m_szMcuName[m_iMcuMessageCount]), "%s", szMcuName );
		m_iMcuMessageCount += 1;
	}
	else
	{
		LogMessage( E_MSG_ERROR, "No slot for Mcu response message" );
	}
}

void CDeviceList::SetAddress( const int idx, const int iAddr )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetAddress( iAddr );
	}
}

void CDeviceList::SetDeviceType( const int idx, const enum E_DEVICE_TYPE eType )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetDeviceType( eType );
	}
}

bool CDeviceList::SetDeviceStatus( const int idx, const enum E_DEVICE_STATUS eStatus )
{
	bool bChanged = false;
	enum E_DEVICE_STATUS eLastStatus;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		eLastStatus = m_Device[idx].GetDeviceStatus();

		m_Device[idx].SetDeviceStatus( eStatus );

		if ( eLastStatus != m_Device[idx].GetDeviceStatus() )
		{	// update the database
			//UpdateDeviceStatus( idx );
			bChanged = true;
		}
	}

	return bChanged;
}

void CDeviceList::SetNumInputs( const int idx, const int iNumInputs )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetNumInputs( iNumInputs );
	}
}

void CDeviceList::SetNumOutputs( const int idx, const int iNumOutputs )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetNumOutputs( iNumOutputs );
	}
}

void CDeviceList::SetInIOName( const int idx, const int j, const char* pszName )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetInIOName( j, pszName );
	}
}

void CDeviceList::SetOutIOName( const int idx, const int j, const char* pszName )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetOutIOName( j, pszName );
	}
}

void CDeviceList::SetInWeekdays( const int idx, const int j, const char* pszDays )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetInWeekdays( j, pszDays );
	}
}

void CDeviceList::SetOutWeekdays( const int idx, const int j, const char* pszDays )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetOutWeekdays( j, pszDays );
	}
}

void CDeviceList::SetOutOnStartTime( const int idx, const int j, const time_t tt )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetOutOnStartTime( j, tt );
	}
}

void CDeviceList::SetInOnPeriod( const int idx, const int j, const int iSec )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetInOnPeriod( j, iSec );
	}
}

void CDeviceList::SetOutOnPeriod( const int idx, const int j, const int iSec )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetOutOnPeriod( j, iSec );
	}
}

void CDeviceList::SetStartTime( const int idx, const int j, const int iMin )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetStartTime( j, iMin );
	}
}

void CDeviceList::SetAnalogType( const int idx, const int j, const char cType )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetAnalogType( j, cType );
	}
}

void CDeviceList::SetCalcFactor( const int idx, const int j, const double dFactor )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetCalcFactor( j, dFactor );
	}
}

void CDeviceList::SetOffset( const int idx, const int j, const double dOffset )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetOffset( j, dOffset );
	}
}

void CDeviceList::SetEventTime( const int idx, const int j, const struct timespec ts )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetEventTime( j, ts );
	}
}

void CDeviceList::GetEventTimeNow( struct timespec& tNow )
{
	int rc;

	tNow.tv_nsec = 0;
	tNow.tv_sec = 0;

	rc = clock_gettime( CLOCK_REALTIME, &tNow );
	if ( rc != 0 )
	{
		LogMessage( E_MSG_ERROR, "clock_gettime() failed with errno %d", errno );
	}
}

// return time difference in milliseconds
const long CDeviceList::GetEventTimeDiff( const int idx, const int j, const struct timespec tNow )
{
	long lDiff = 0;
	struct timespec tLast;

	if ( idx >= 0 && idx < MAX_IO_PORTS )
	{
		m_Device[idx].GetEventTime( j, tLast );

		if ( tNow.tv_sec > tLast.tv_sec )
		{
			lDiff = (tNow.tv_sec - tLast.tv_sec) * 1000;
			lDiff += (long)((tNow.tv_nsec - tLast.tv_nsec) / 1000000);
		}
		else if ( tNow.tv_sec < tLast.tv_sec )
		{	// this should not occur unless the system time has been altered
			lDiff = 0;
		}
		else
		{
			lDiff = (long)((tNow.tv_nsec - tLast.tv_nsec) / 1000000);
		}
	}
	LogMessage( E_MSG_INFO, "GetEventTimeDiff() %ld", lDiff );

	return lDiff;
}

const int CDeviceList::GetMcuMessageCount()
{
	return m_iMcuMessageCount;
}

const bool CDeviceList::IsShared( const int idx )
{
	bool bShared = false;
	int i;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		for ( i = 0; i < MAX_DEVICES; i++ )
		{
			if ( strcmp( m_Device[idx].GetComPort(), m_Device[i].GetComPort() ) == 0 && idx != i && IsMyHostname(m_Device[i].GetDeviceHostname()) )
			{
				bShared = true;
				break;
			}
		}
	}

	return bShared;
}

void CDeviceList::LogOnTime( const int idx, const int iChannel )
{
	time_t tOnTime;

	tOnTime = m_Device[idx].GetOutOnStartTime(iChannel);

	if ( tOnTime != 0 )
	{
		LogMessage( E_MSG_INFO, "LogOnTime(): Device %d, channel %d on for %d sec", idx, iChannel+1, time(NULL)-tOnTime );
	}
	else
	{
		LogMessage( E_MSG_WARN, "LogOnTime(): Device %d, channel %d was not on ??", idx, iChannel+1 );
	}
}

const int CDeviceList::GetTotalComPorts( char szPort[MAX_DEVICES][MAX_COMPORT_LEN+1] )
{
	int i, j;
	int iTotal = 0;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		szPort[i][0] = '\0';
	}

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetAddress() > 0 && strncmp( m_Device[i].GetComPort(), "MCU", 3 ) != 0 &&
				m_Device[i].GetDeviceNo() > 0 && m_Device[i].GetComPort()[0] != '\0' &&
				IsMyHostname(m_Device[i].GetDeviceHostname())  )
		{	// this com port is one of ours
			for ( j = 0; j < MAX_DEVICES; j++ )
			{
				if ( szPort[j][0] == '\0' )
				{	// end of list
					iTotal += 1;
					snprintf( szPort[j], sizeof(szPort[j]), "%s", m_Device[i].GetComPort() );
					break;
				}
				else if ( strcmp( szPort[j], m_Device[i].GetComPort() ) == 0 )
				{	// we already have this one
					break;
				}
			}
		}
	}

	return iTotal;
}

const bool CDeviceList::IsSharedWithNext( const int idx )
{
	bool bShared = false;
	int iNext;
	int iLoop = MAX_DEVICES;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		iNext = idx;
		while ( iLoop > 0 )
		{
			iLoop -= 1;

			if ( iNext+1 >= MAX_DEVICES )
			{	// wrap round the list
				iNext = 0;
			}
			else
			{
				iNext += 1;
			}

			if ( m_Device[iNext].GetAddress() > 0 && m_Device[iNext].GetDeviceNo() > 0 && IsMyHostname(m_Device[iNext].GetDeviceHostname()) )
			{
				break;
			}
		}

		if ( strcmp( m_Device[idx].GetComPort(), m_Device[iNext].GetComPort() ) == 0 && idx != iNext )
		{
			bShared = true;
		}
	}

	return bShared;
}

const bool CDeviceList::WriteOutputBit( const int idx, const int iOutChannel, const uint8_t uState )
{
	bool bRc = true;
	modbus_t* ctx;

	ctx = GetContext(idx);

	LogMessage( E_MSG_DEBUG, "modbus_write_bit(%p) %d %d", ctx, idx, iOutChannel+1 );
	if ( modbus_write_bit( ctx, iOutChannel, (bool)uState ) == -1 )
	{	// failed
		bRc = false;

		LogMessage( E_MSG_ERROR, "WriteOutputBit() failed for device %d, channel %d, state %u: %s", idx, iOutChannel+1, uState, modbus_strerror(errno) );
	}

	return bRc;
}

bool CDeviceList::UpdateDBDeviceComPort( CMysql& myDB, const int idx )
{
	bool bRet = true;

	if ( myDB.RunQuery( "update devices set de_ComPort='%s' where de_DeviceNo=%d", m_Device[idx].GetComPort(), m_Device[idx].GetDeviceNo() ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::UpdateDeviceStatus( CMysql& myDB, const int idx )
{
	bool bRet = true;

	if ( myDB.RunQuery( "update devices set de_Status=%d where de_DeviceNo=%d", m_Device[idx].GetDeviceStatus(), m_Device[idx].GetDeviceNo() ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::UpdateDeviceOutOnStartTime( CMysql& myDB, const int iOutIdx, const int iOutChannel )
{
	bool bRet = true;

	if ( myDB.RunQuery( "update deviceinfo set di_OutOnStartTime=%d,di_OutOnPeriod=%d where di_DeviceNo=%d and di_IOChannel=%d", m_Device[iOutIdx].GetOutOnStartTime(iOutChannel),
			m_Device[iOutIdx].GetOutOnPeriod(iOutChannel), m_Device[iOutIdx].GetDeviceNo(), iOutChannel ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::ReadDeviceConfig( CMysql& myDB )
{
	bool bRet = true;
	int i, j;
	enum E_IO_TYPE eIOType;
	int iNumFields;
	MYSQL_ROW row;

	// read from mysql
	if ( myDB.RunQuery( "SELECT de_DeviceNo,de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Hostname FROM devices order by de_DeviceNo") != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		i = 0;
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			SetDeviceNo( i, atoi(row[0]) );
			SetComPort( i, row[1] );
			SetAddress( i, atoi( row[2] ) );
			SetNumInputs( i, atoi( row[3] ) );
			SetNumOutputs( i, atoi( row[4] ) );
			SetDeviceType( i, (enum E_DEVICE_TYPE)atoi( row[5] ) );
			SetDeviceName( i, row[6] );
			SetDeviceHostname( i, row[7] );

			LogMessage( E_MSG_INFO, "Device(%d): DeNo:%d, Port:'%s', Addr:%d, DType:%d, Name:'%s', Host:'%s'", i, GetDeviceNo(i), GetComPort(i), GetAddress(i), GetDeviceType(i),
					GetDeviceName(i), GetDeviceHostname(i) );

			i += 1;
			if ( i >= MAX_DEVICES )
			{
				LogMessage( E_MSG_ERROR, "Too many devices, max is %d", MAX_DEVICES );
				break;
			}
		}
	}

	myDB.FreeResult();

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetAddress() >= 0 )
		{
			if ( myDB.RunQuery( "SELECT di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,di_Temperature,"
					"di_OutOnStartTime,di_OutOnPeriod,di_Weekdays,di_AnalogType,di_CalcFactor,di_Voltage,di_Offset FROM deviceinfo WHERE di_DeviceNo=%d order by di_IOChannel",
					m_Device[i].GetDeviceNo() ) != 0 )
			{
				bRet = false;
				LogMessage( E_MSG_ERROR, "RunQuery(%s): %s", myDB.GetQuery(), myDB.GetError() );
			}
			else
			{
				while ( (row = myDB.FetchRow( iNumFields )) )
				{
					j = atoi( row[0] );

					eIOType = (enum E_IO_TYPE)atoi( row[2] );

					if ( eIOType == E_IO_OUTPUT )
					{
						SetOutIOName( i, j, row[1] );
						GetOutChannelType( i, j ) = (enum E_IO_TYPE)atoi( row[2] );
						// row[3]
						// row[4]
						// row[5]
						// row[6]
						SetOutOnStartTime( i, j, atoi( row[7] ) );
						SetOutOnPeriod( i, j, atoi( row[8] ) );
						SetOutWeekdays( i, j, row[9] );
						// row[10]
						// row[11]
						// row[12]
						// row[13]

						LogMessage( E_MSG_INFO, "DeviceInfo OUT(%d,%d): Name:'%s', Type:%d, OnPeriod:%d, STime:%d, OnPeriod:%d, Days:%s", i, j,
								GetOutIOName(i,j), GetOutChannelType(i,j), GetOutOnPeriod(i,j),
								GetOutOnStartTime(i,j), GetOutOnPeriod(i,j), GetOutWeekdays(i,j) );
					}
					else
					{
						SetInIOName( i, j, row[1] );
						GetInChannelType( i, j ) = (enum E_IO_TYPE)atoi( row[2] );
						SetInOnPeriod( i, j, atoi( row[3] ) );
						SetStartTime( i, j, atoi( row[4] ) );
						GetHysteresis( i, j ) = atoi( row[5] );
						GetTriggerTemperature( i, j ) = atof( row[6] );
						// row[7]
						// row[8]
						SetInWeekdays( i, j, row[9] );
						SetAnalogType( i, j, row[10][0] );
						SetCalcFactor( i, j, atof( row[11] ) );
						GetTriggerVoltage( i, j ) = atoi( row[12] );
						SetOffset( i, j, atof( row[13] ) );

						LogMessage( E_MSG_INFO, "DeviceInfo  IN(%d,%d): Name:'%s', Type:%d, OnPeriod:%d, STime:%d, Hyst:%d, Temp:%.1f, Days:%s AType:'%c', CFactor:%.3f, Volt:%.1f, Offset:%.2f",
								i, j, GetInIOName(i,j), GetInChannelType(i,j), GetInOnPeriod(i,j),
								GetStartTime(i,j), GetHysteresis(i,j), GetTriggerTemperature(i,j), GetInWeekdays(i,j), GetAnalogType(i,j), GetCalcFactor(i,j), GetTriggerVoltage(i,j),
								GetOffset(i,j) );
					}
				}

				myDB.FreeResult();
			}
		}
	}

	bRet = true;

	return bRet;
}

const bool CDeviceList::IsMcuDevice( const int idx )
{
	bool bRet = false;

	if ( idx >=0 && idx < MAX_DEVICES )
	{
		if ( strncmp( m_Device[idx].GetComPort(), "MCU", 3 ) == 0 )
		{
			bRet = true;
		}
	}

	return bRet;
}

const int CDeviceList::GetIdxForName( const char* szName )
{
	int i;
	int idx = -1;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
//		LogMessage( E_MSG_INFO, "%d check '%s' vs '%s'", i, szName, m_Device[i].GetDeviceName() );
		if ( strcmp( szName, m_Device[i].GetDeviceName() ) == 0 )
		{
			idx = i;
			break;
		}
	}

	return idx;
}

const int CDeviceList::GetIdxForAddr( const int iAddr )
{
	int i;
	int idx = -1;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetAddress() == iAddr )
		{
			idx = i;
			break;
		}
	}

	return idx;
}

const int CDeviceList::GetDeviceNoForAddr( const int iAddr )
{
	int i;
	int iDeviceNo = 0;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetAddress() == iAddr )
		{
			iDeviceNo = m_Device[i].GetDeviceNo();
			break;
		}
		else if ( m_Device[i].GetDeviceName()[0] == '\0' )
		{	// end of the list
			break;
		}
	}

	return iDeviceNo;
}

const int CDeviceList::GetAddressForDeviceNo( const int iDeviceNo )
{
	int i;
	int iAddr = 0;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetDeviceNo() == iDeviceNo )
		{
			iAddr = m_Device[i].GetAddress();
			break;
		}
		else if ( m_Device[i].GetDeviceNo() == 0 )
		{	// end of the list
			break;
		}
	}
	//printf( "find %d %d\n", iDeviceNo, iAddr );

	return iAddr;
}

const bool CDeviceList::IsOffTime( const int idx, const int iChannel )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].IsOffTime(iChannel);
	}

	return false;
}

const char* CDeviceList::GetEventTypeDesc( const enum E_EVENT_TYPE eType )
{
	static char szBuf[100] = {""};

	switch ( eType )
	{
	default:
		snprintf( szBuf, sizeof(szBuf), "** Unhandled **" );
		break;
	case E_ET_CLICK:			// 0:	single press
		snprintf( szBuf, sizeof(szBuf), "Click" );
		break;
	case E_ET_DBLCLICK:			// 1:	double click
		snprintf( szBuf, sizeof(szBuf), "Double Click" );
		break;
	case E_ET_LONGPRESS:		// 2:	long press
		snprintf( szBuf, sizeof(szBuf), "Long Press" );
		break;
	case E_ET_TIMER:			// 3:	timer event
		snprintf( szBuf, sizeof(szBuf), "Timer" );
		break;
	case E_ET_TEMPERATURE:		// 4:	temperature event
		snprintf( szBuf, sizeof(szBuf), "Temperature" );
		break;
	case E_ET_DEVICE_NG:		// 5:	device status is bad
		snprintf( szBuf, sizeof(szBuf), "Device NG" );
		break;
	case E_ET_DEVICE_OK:		// 6:	device status is good
		snprintf( szBuf, sizeof(szBuf), "Device OK" );
		break;
	case E_ET_VOLTAGE:			// 7:	voltage event
		snprintf( szBuf, sizeof(szBuf), "Voltage" );
		break;
	case E_ET_STARTUP:			// 8:	program startup
		snprintf( szBuf, sizeof(szBuf), "Startup" );
		break;
	}

	return szBuf;
}

const bool CDeviceList::LinkTestPassed( const int iLinkIdx, const int iLinkChannel, const char* szLinkTest, const double dLinkValue, bool& bInvertState )
{
	bool bRc = false;

	if ( iLinkIdx >= 0 && iLinkIdx < MAX_DEVICES )
	{
		bRc = m_Device[iLinkIdx].LinkTestPassed( iLinkChannel, szLinkTest, dLinkValue, bInvertState );
	}

	return bRc;
}

int CDeviceList::GetTotalDevices()
{
	int i;
	int count = 0;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( strlen( m_Device[i].GetDeviceName() ) == 0 )
		{	// end of list
			break;
		}
		else
		{
			count += 1;
		}
	}

	return count;
}

//*****************************************************************************************
//
//	CInOutLinks
//
//*****************************************************************************************
CInOutLinks::CInOutLinks()
{
	Init();
}

CInOutLinks::~CInOutLinks()
{
	//m_DB.Disconnect();
}

void CInOutLinks::Init()
{
	int i;

	//m_DB.Connect();
	for ( i = 0; i < MAX_IO_LINKS; i++ )
	{
		SetLinkNo( i, 0 );
		SetInDeviceNo( i, 0 );
		SetInChannel( i, 0 );
		SetOutDeviceNo( i, 0 );
		SetOutChannel( i, 0 );
		SetEventType( i, E_ET_CLICK );
		SetOnPeriod( i, 0 );
		SetLinkDeviceNo( i, 0 );
		SetLinkChannel( i, 0 );
		SetLinkTest( i, "" );
		SetLinkValue( i, 0.0 );
	}
}

void CInOutLinks::SetLinkNo( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < 0xfe )
		{
			m_iLinkNo[idx] = iVal;
		}
	}
}

void CInOutLinks::SetInDeviceNo( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < 0xfe )
		{
			m_iInDeviceNo[idx] = iVal;
		}
	}
}

void CInOutLinks::SetInChannel( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < MAX_IO_PORTS )
		{
			m_iInChannel[idx] = iVal;
		}
	}
}

void CInOutLinks::SetOutDeviceNo( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < 0xfe )
		{
			m_iOutDeviceNo[idx] = iVal;
		}
	}
}

void CInOutLinks::SetOutChannel( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < MAX_IO_PORTS )
		{
			m_iOutChannel[idx] = iVal;
		}
	}
}

void CInOutLinks::SetEventType( const int idx, const enum E_EVENT_TYPE eVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		m_eEventType[idx] = eVal;
	}
}

void CInOutLinks::SetOnPeriod( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		m_iOnPeriod[idx] = iVal;
	}
}

void CInOutLinks::SetLinkDeviceNo( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < 0xfe )
		{
			m_iLinkDeviceNo[idx] = iVal;
		}
	}
}

void CInOutLinks::SetLinkChannel( const int idx, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		if ( iVal >= 0 && iVal < MAX_IO_PORTS )
		{
			m_iLinkChannel[idx] = iVal;
		}
	}
}

void CInOutLinks::SetLinkTest( const int idx, const char* szTest )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		snprintf( m_szLinkTest[idx], sizeof(m_szLinkTest[idx]), "%s", szTest );
	}
}

void CInOutLinks::SetLinkValue( const int idx, const double dVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		m_dLinkValue[idx] = dVal;
	}
}

bool CInOutLinks::Find( const int iInDeviceNo, const int iInChannel, int &idx, int& iOutDeviceNo, int& iOutChannel, int& iOnPeriod,
		int& iLinkDeviceNo, int& iLinkChannel, char* szLinkTest, double& dLinkValue )
{
	bool bRet = false;

	iOutDeviceNo = 0;
	iOutChannel = 0;
	iOnPeriod = 0;
	iLinkDeviceNo = 0;
	iLinkChannel = 0;
	szLinkTest[0] = '\0';
	dLinkValue = 0.0;

	//LogMessage( E_MSG_INFO, "Find() %d %d", iInDeviceNo, iInChannel );

	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		while ( idx < MAX_IO_LINKS )
		{
			if ( m_iInDeviceNo[idx] == iInDeviceNo && m_iInChannel[idx] == iInChannel )
			{
				bRet = true;
				iOutDeviceNo = m_iOutDeviceNo[idx];
				iOutChannel = m_iOutChannel[idx];
				iOnPeriod = m_iOnPeriod[idx];
				iLinkDeviceNo = m_iLinkDeviceNo[idx];
				iLinkChannel = m_iLinkChannel[idx];
				snprintf( szLinkTest, sizeof(m_szLinkTest[0]), "%s", m_szLinkTest[idx] );
				dLinkValue = m_dLinkValue[idx];

				idx += 1;
				break;
			}

			idx += 1;
		}
	}

	return bRet;
}


bool CInOutLinks::ReadIOLinks( CMysql& myDB )
{
	bool bRet = true;
	int i;


	int iNumFields;
	MYSQL_ROW row;

	// read from mysql
	if ( myDB.RunQuery( "SELECT il_LinkNo,il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,"
			"il_LinkDeviceNo,il_LinkChannel,il_LinkTest,il_LinkValue FROM iolinks order by il_InDeviceNo,il_InChannel") != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		i = 0;
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			SetLinkNo( i, atoi( row[0] ) );
			SetInDeviceNo( i, atoi( row[1] ) );
			SetInChannel( i, atoi( row[2] ) );
			SetOutDeviceNo( i, atoi( row[3] ) );
			SetOutChannel( i, atoi( row[4] ) );
			SetEventType( i, (E_EVENT_TYPE)atoi( row[5] ) );
			SetOnPeriod( i, atoi( row[6] ) );
			SetLinkDeviceNo( i, atoi(row[7]) );
			SetLinkChannel( i, atoi( row[8] ) );
			SetLinkTest( i, row[9] );
			SetLinkValue( i, atof( row[10] ) );

			LogMessage( E_MSG_INFO, "IOLink: InDeviceNo:%d, InChannel:%d, OutDeviceNo:%d OutChannel:%d, EventType:%d, OnPeriod:%d LinkDeNo:%d, LinkCh:%d, LinkTest:%s, LinkVal:%.1f",
					GetInDeviceNo(i), GetInChannel(i), GetOutDeviceNo(i), GetOutChannel(i), GetEventType(i), GetOnPeriod(i),
					GetLinkDeviceNo(i), GetLinkChannel(i), GetLinkTest(i), GetLinkValue(i) );

			i += 1;
		}
	}

	myDB.FreeResult();

	return bRet;
}

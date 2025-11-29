#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
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

	SetLastCheckedTimeMS( 0.0 );
	SetDeviceNo( 0 );
	SetComPort( "" );
	SetDeviceName( "" );
	SetDeviceHostname( "" );
	SetContext( NULL );
	SetComHandle( 0 );
	SetAddress( -1 );
	SetTimeoutCount( 0 );
	SetDeviceType( E_DT_UNUSED );
	SetDeviceStatus( E_DS_DEAD, true );
	SetNumInputs( 8 );
	SetNumOutputs( 8 );
	SetAlwaysPoweredOn("Y");
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
		SetResolution( j, 0.0 );
		SetMonitorPos( j, "   " );
		GetConditionTriggered( j ) = 0;
		GetAlarmTriggered( j ) = 0;
		GetHysteresis( j ) = 0;
		GetMonitorValueLo( j ) = 0.0;
		GetMonitorValueHi( j ) = 0.0;
		GetLastRecorded( j ) = 0;
		ClearDataBuffer( j );
	}

}

// keep the last 10 data values in order, 0 = newest
void CMyDevice::SaveDataValue( const int idx )
{
	for ( int i = MAX_DATA_BUFFER-1; i > 0; i-- )
	{
		m_uDataBuffer[idx][i] = m_uDataBuffer[idx][i-1];
	}
	m_uDataBuffer[idx][0] = m_uNewData[idx];
}

// currently only used for temperature values
bool CMyDevice::DataBufferIsStable( const int idx )
{
	bool bRet = true;
	int iMaxCheck = 4;

	for ( int i = 0; i < iMaxCheck; i++ )
	{
		if ( m_eDeviceType == E_DT_TEMPERATURE_DS )
		{
			if ( m_uDataBuffer[idx][i] == (uint16_t)-1 )
			{	// no data
				break;
			}
			else if ( fabs(CalculateTemperature(m_uDataBuffer[idx][i])) - fabs(CalculateTemperature(m_uDataBuffer[idx][i+1])) >= MAX_TEMPERATURE_DIFF )
			{	// last 4 readings are not within 5.0 deg C
				bRet = false;
				break;
			}
		}
		else
		{
			break;
		}
	}

	return bRet;
}

void CMyDevice::ClearDataBuffer( const int idx )
{
	for ( int i = 0; i < MAX_DATA_BUFFER; i++ )
	{
		m_uDataBuffer[idx][i] = (uint16_t)-1;
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

void CMyDevice::SetResolution( const int i, const double dResolution )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		m_dResolution[i] = dResolution;
	}
}

void CMyDevice::SetMonitorPos( const int i, const char* szMonPos )
{
	if ( i >= 0 && i < MAX_IO_PORTS )
	{
		snprintf( m_szMonitorPos[i], sizeof(m_szMonitorPos[i]), "%s", szMonPos );;
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

void CMyDevice::SetDeviceStatus( const enum E_DEVICE_STATUS eStatus, const bool bNoTimeout )
{
	if ( eStatus == E_DS_ALIVE )
	{
		m_iTimeoutCount = 0;
	}
	else if ( bNoTimeout && m_iTimeoutCount < 10 )
	{
		m_iTimeoutCount = 11;
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

		dVal = CalculateTemperature( uVal );
	}

	return dVal;
}

const double CMyDevice::CalculateTemperature( const uint16_t uVal )
{
	double dVal = 0.0;

	if ( m_eDeviceType == E_DT_TEMPERATURE_DS && uVal == -1 )
	{	// sensor is not connected

	}
	else if ( m_eDeviceType == E_DT_TEMPERATURE_K1 && uVal == 64536 )
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

// HDHK current meter
const double CMyDevice::CalcCurrent( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( bNew )
			dUnit = (double)(m_uNewData[iChannel]);
		else
			dUnit = (double)(m_uLastData[iChannel]);

		if ( strlen(m_szInIOName[iChannel]) == 0 )
		{	// sensor is not connected

		}
		else
		{
			dVal = ((double)dUnit / 100);
		}
	}

	return dVal;
}

// data from K02 is in mm
// water height = <max_water_height> - (<measured_value> - <offset_above_max_level>)
// CalcFactor is the max water level height in mm
// Offset if the sensor height above the max level in mm
const double CMyDevice::CalcLevel( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dMaxHeight;
	double dUnit;
	double dOffset = 1.0;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		dMaxHeight = m_dCalcFactor[iChannel];
		if ( m_dOffset[iChannel] != 0 )
		{
			dOffset = m_dOffset[iChannel];
		}

		if ( bNew )
			dUnit = (double)(m_uNewData[iChannel]);
		else
			dUnit = (double)(m_uLastData[iChannel]);

		if ( strlen(m_szInIOName[iChannel]) == 0 || dMaxHeight == 0 )
		{	// sensor is not connected

		}
		else
		{
			if ( GetDeviceType() == E_DT_LEVEL_K02 )
			{
				// K02 - subtract the sensor height above the max level
				dUnit -= m_dOffset[iChannel];	// in mm

				dVal = 100 * ((dMaxHeight - dUnit) / dMaxHeight);
			}
			else
			{
				dVal = 100 * ((dUnit * dOffset) / dMaxHeight);
			}
		}
	}

	return dVal;
}

// return distance in mm
// channel 0 is the current angle,
// channel 1 is the number of turns
const double CMyDevice::CalcRotaryEncoderDistance( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dMMperRotation;
	double dUnit;
	double dTurns;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		dMMperRotation = m_dCalcFactor[iChannel];

		if ( bNew )
		{
			dUnit = (double)(m_uNewData[iChannel]);
			dTurns = (double)(m_uNewData[iChannel+1]);
		}
		else
		{
			dUnit = (double)(m_uLastData[iChannel]);
			dTurns = (double)(m_uLastData[iChannel+1]);
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 || dMMperRotation == 0 )
		{	// sensor is not connected

		}
		else
		{	// assume 12 bit encoder, 4096 = 360 degrees
			dVal = (dUnit / 4096) * dMMperRotation + dTurns * dMMperRotation;
		}
	}

	return dVal;
}

const double CMyDevice::CalcTHValue( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		//dMMperRotation = m_dCalcFactor[iChannel];

		if ( bNew )
		{
			dUnit = (double)(m_uNewData[iChannel]);
		}
		else
		{
			dUnit = (double)(m_uLastData[iChannel]);
		}

		switch ( iChannel )
		{
		default:
			break;
		case 0:	// temperature
			dVal = (double)dUnit / 10;
			break;
		case 1:	// humidity
			dVal = (double)dUnit / 10;
			break;
		}
	}

	return dVal;
}

const double CMyDevice::CalcVIPFValue( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		//dMMperRotation = m_dCalcFactor[iChannel];

		if ( bNew )
		{
			dUnit = (double)(m_uNewData[iChannel]);
		}
		else
		{
			dUnit = (double)(m_uLastData[iChannel]);
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 )
		{	// sensor is not connected

		}
		else
		{	// assume 12 bit encoder, 4096 = 360 degrees
			switch ( iChannel )
			{
			default:
				break;
			case 0:		// voltage, volts
				dVal = (double)dUnit / 10;
				break;
			case 1:		// current, amps
				dVal = (double)dUnit / 1000;
				break;
			case 2:		// power, watts
				dVal = (double)dUnit / 10;
				break;
			case 3:		// energy, Watt Hours
				dVal = (double)dUnit;
				break;
			case 4:		// frequency, Hz
				dVal = (double)dUnit / 10;
				break;
			case 5:		// power factor
				dVal = (double)dUnit / 100;
				break;
			}
		}
	}

	return dVal;
}

const double CMyDevice::CalcVSDNFlixenValue( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( bNew )
		{
			dUnit = (double)(m_uNewData[iChannel]);
		}
		else
		{
			dUnit = (double)(m_uLastData[iChannel]);
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 )
		{	// sensor is not connected

		}
		else
		{
			switch ( iChannel )
			{
			default:
				break;
			case 0:		// voltage, volts
				dVal = (double)dUnit;
				break;
			case 1:		// current, 0.1 amps
				dVal = (double)dUnit / 10;
				break;
			case 2:		// power, 0.1kW
				dVal = (double)dUnit / 10;
				break;
			case 3:		// frequency, 0.01Hz
				dVal = (double)dUnit / 100;
				break;
			case 4:		// torque %
				dVal = (double)dUnit;
				break;
			case 5:		// running speed, rpm
				dVal = (double)dUnit;
				break;
			}
		}
	}

	return dVal;
}

const double CMyDevice::CalcVSDPwrElectValue( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( bNew )
		{
			dUnit = (double)(m_uNewData[iChannel]);
		}
		else
		{
			dUnit = (double)(m_uLastData[iChannel]);
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 )
		{	// sensor is not connected

		}
		else
		{
			switch ( iChannel )
			{
			default:
				break;
			case 0:		// voltage, volts
				dVal = (double)dUnit;
				break;
			case 1:		// current, 0.1 amps
				dVal = (double)dUnit / 10;
				break;
			case 2:		// power, 0.1kW
				dVal = (double)dUnit / 10;
				break;
			case 3:		// frequency, 0.01Hz
				dVal = (double)dUnit / 10;
				break;
			case 4:		// torque %
				dVal = 100 * (double)dUnit / 8192;
				break;
			case 5:		// running speed, rpm
				dVal = (double)dUnit;
				break;
			}
		}
	}

	return dVal;
}

// count is in channels 0 and 1, status is in channel 2
// should only be called for channel 0
const double CMyDevice::CalcPT113Weight( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		bool bDataOk = false;
		if ( bNew )
		{
			dUnit = (double)(m_uNewData[0])*65536 + (double)(m_uNewData[1]);
			if ((m_uNewData[2] & 0x0002) != 0 && (m_uNewData[2] & 0xe000) == 0 )
			{
				bDataOk = true;
			}
		}
		else
		{
			//LogMessage( E_MSG_INFO, "oldval %u %u", m_uLastData[0], m_uLastData[1] );
			dUnit = (double)(m_uLastData[0]*65536 + (double)(m_uLastData[1]));
			if ((m_uLastData[2] & 0x0002) != 0 && (m_uLastData[2] & 0xe000) == 0 )
			{
				bDataOk = true;
			}
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 || !bDataOk )
		{	// sensor is not connected

		}
		else
		{
			switch ( iChannel )
			{
			default:
				break;
			case 0:		// weight, kg
				// Offset: load cell rated weight, 25,000kg
				dVal = (double)dUnit / LOAD_CELL_MAX_COUNT * m_dOffset[0];
				dVal -= m_dCalcFactor[0];
				if ( dVal > (m_dOffset[0]*1.5) || dVal < 0.0 )
				{	// data error out of range
					dVal = 0.0;
				}
				break;
			}
		}
	}

	return dVal;
}

const double CMyDevice::CalcSystecIT1Weight( const int iChannel, const bool bNew )
{
	double dVal = 0.0;
	double dUnit;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		bool bDataOk = false;
		if ( bNew )
		{
			dUnit = (double)(m_uNewData[0]);
//			if ((m_uNewData[2] & 0x0002) != 0 && (m_uNewData[2] & 0xe000) == 0 )
			{
				bDataOk = true;
			}
		}
		else
		{
			//LogMessage( E_MSG_INFO, "oldval %u %u", m_uLastData[0], m_uLastData[1] );
			dUnit = (double)(m_uLastData[0]);
//			if ((m_uLastData[2] & 0x0002) != 0 && (m_uLastData[2] & 0xe000) == 0 )
			{
				bDataOk = true;
			}
		}

		if ( strlen(m_szInIOName[iChannel]) == 0 || !bDataOk )
		{	// sensor is not connected

		}
		else
		{
			switch ( iChannel )
			{
			default:
				break;
			case 0:		// weight, kg
				// Offset: load cell rated weight, 25,000kg
				dVal = (double)dUnit;	// / LOAD_CELL_MAX_COUNT * m_dOffset[0];
				//dVal -= m_dCalcFactor[0];
				if ( dVal > (m_dOffset[0]*1.5) || dVal < 0.0 )
				{	// data error out of range
					dVal = 0.0;
				}
				break;
			}
		}
	}

	return dVal;
}

const bool CMyDevice::IsSensorConnected( const int iChannel )
{
	bool bRc = false;

	if ( iChannel >= 0 && iChannel < MAX_IO_PORTS )
	{
		if ( m_eDeviceType == E_DT_TEMPERATURE_DS && m_uNewData[iChannel] != (uint16_t)-1 )
		{
			bRc = true;
		}
		else if ( m_eDeviceType == E_DT_TEMPERATURE_K1 && m_uNewData[iChannel] != 64536 )
		{
			bRc = true;
		}
		else if ( m_eDeviceType == E_DT_DIGITAL_IO && strncmp( GetComPort(), "ESP", 3 ) == 0 )
		{
			bRc = true;
		}
		else if ( m_eDeviceType == E_DT_SHT40_TH && m_uNewData[iChannel] != (uint16_t)-1 )
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
		if ( m_eDeviceType == E_DT_TEMPERATURE_DS && m_uLastData[iChannel] != (uint16_t)-1 )
		{
			bRc = true;
		}
		else if ( m_eDeviceType == E_DT_TEMPERATURE_K1 && m_uLastData[iChannel] != 64536 )
		{
			bRc = true;
		}
		else if ( m_eDeviceType == E_DT_SHT40_TH && m_uLastData[iChannel] != (uint16_t)-1 )
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
	time_t timenow;
	struct tm tmVal;

	if ( iLinkChannel >= 0 && iLinkChannel < MAX_IO_PORTS )
	{
		switch ( m_eDeviceType )
		{
		default:
			LogMessage( E_MSG_ERROR, "LinkTestPassed: unsupported m_eDeviceType" );
			break;

		case E_DT_TIMER:
			timenow = time(NULL);
			localtime_r( &timenow, &tmVal );
			dVal = (double)(tmVal.tm_hour * 60 + tmVal.tm_min);	// time in minutes
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_TEMPERATURE_DS:
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

		case E_DT_TEMPERATURE_K1:
			dVal = CalcTemperature( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_LEVEL_HDL:
			dVal = CalcLevel( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_ROTARY_ENC_12BIT:
			dVal = CalcRotaryEncoderDistance( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_VIPF_MON:
			dVal = CalcVIPFValue( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_HDHK_CURRENT:
			dVal = CalcCurrent( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
			break;

		case E_DT_SHT40_TH:
			dVal = CalcTHValue( iLinkChannel, true );
			bRc = TestValue( szLinkTest, dLinkValue, dVal, bInvertState );
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
	int i;

	//m_DB.Disconnect();
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_Device[i].GetComHandle() != 0 )
		{
			close( m_Device[i].GetComHandle() );
			m_Device[i].SetComHandle( 0 );
		}
	}
}

void CDeviceList::Init()
{
	int i;

	//m_DB.Connect();

	m_iEspMessageCount = 0;
	m_eDayNightState = E_DN_UNKNOWN;
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		m_bHostComPortModbus[i] = false;
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

// should only be called for modbus devices
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

const int CDeviceList::GetBaudRateForPort( const char* szComPort )
{
	int iBaudRate = 9600;

	for ( int i = 0; i < MAX_DEVICES; i++ )
	{
		if ( strcmp( szComPort, GetComPort(i) ) == 0 )
		{
			iBaudRate = GetBaudRate(i);
			break;
		}
	}

	return iBaudRate;
}

void CDeviceList::ReadSimpleComPort( const char* szPort, char* szSimplePort, size_t uLen )
{
	if ( strncmp( szPort, "/dev/ttyUSB", 11 ) == 0 )
	{	// already a simple port
		snprintf( szSimplePort, uLen, "%s", szPort );
	}
	else if ( strncmp( szPort, "/dev/serial/by-path", 19 ) == 0 )
	{	// find the simple port name
		DIR *d;
		struct dirent *dir;
		d = opendir("/dev/serial/by-path");
		if ( d )
		{
		    while ((dir = readdir(d)) != NULL)
		    {
		    	if ( dir->d_type == DT_LNK )
		    	{
		    		char szBuf[512];
		    		char szFile[512];
		    		ssize_t len;

		    		memset( szBuf, 0, sizeof(szBuf));

		    		snprintf( szFile, sizeof(szFile), "/dev/serial/by-path/%s", dir->d_name );
		    		len = readlink( szFile, szBuf, sizeof(szBuf) );
		    		if ( len != -1 && strcmp( szPort, szFile ) == 0 )
		    		{
		    			LogMessage( E_MSG_INFO, "Port '/dev/serial/by-path/%s'->'%s'", dir->d_name, szBuf );

		    			// strip the "../../"
		    			snprintf( szSimplePort, uLen, "/dev/%s", &szBuf[6] );
		    		}
		    	}
		    }

		    closedir(d);
		}

	}
	else
	{
		LogMessage( E_MSG_WARN, "ReadSimpleComPort: unhandled port type '%s'", szPort );
		snprintf( szSimplePort, uLen, "%s", szPort );
	}
}

const bool CDeviceList::GetComPortsOnHost( CMysql& myDB, char szPortList[MAX_DEVICES][MAX_COMPORT_LEN+1], bool bSwapBaud )
{
	bool bSomeoneIsAlive = true;
	int iCount = 0;
	int i;
	int iPos = 0;
	int iBaudRate;
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

	// get by-path name
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_szHostComPort[i][0] == '\0' )
		{	// end of list
			break;
		}
		else
		{	// search /dev/serial/by-path directory
			DIR *d;
			struct dirent *dir;
			d = opendir("/dev/serial/by-path");
			if ( d )
			{
			    while ((dir = readdir(d)) != NULL)
			    {
			    	if ( dir->d_type == DT_LNK )
			    	{
			    		char szBuf[512];
			    		char szFile[512];
			    		ssize_t len;

			    		memset( szBuf, 0, sizeof(szBuf));

			    		snprintf( szFile, sizeof(szFile), "/dev/serial/by-path/%s", dir->d_name );
			    		len = readlink( szFile, szBuf, sizeof(szBuf) );
			    		if ( len != -1 && strcmp( &m_szHostComPort[i][5], &szBuf[6] ) == 0 )
			    		{	// strip leading "/dev/" and "../../" from device names
			    			LogMessage( E_MSG_INFO, "Port '%s': '/dev/serial/by-path/%s'->'%s'", m_szHostComPort[i], dir->d_name, szBuf );

			    			// update the devices table
			    			snprintf( szBuf, sizeof(szBuf), "/dev/serial/by-path/%s", dir->d_name );
			    			UpdateDeviceComPort( myDB, szBuf, m_szHostComPort[i], szPortList );
			    		}
			    	}
			    }

			    closedir(d);
			}
		}
	}

	// connect a context to each real port
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_szHostComPort[i][0] != '\0' )
		{	// found a real com port
			iBaudRate = GetBaudRateForPort( m_szHostComPort[i] );
			if ( bSwapBaud )
			{	// TODO: handle more baud rates
				iBaudRate = (iBaudRate == 9600 ? 19200 : 9600 );
			}
			m_pHostCtx[i] = modbus_new_rtu( m_szHostComPort[i], iBaudRate, gRS485.cParity, gRS485.iDataBits, gRS485.iStopBits );
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
				LogMessage( E_MSG_INFO, "Checking Modbus serial connection on '%s' (%d)", m_szHostComPort[i], iBaudRate );
			}
		}
		else
		{	// end of list
			break;
		}
	}


	// nothing more to do


	// cleanup
	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		if ( m_szHostComPort[i][0] != '\0' )
		{	// found a real com port
			iCount += 1;
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

	LogMessage( E_MSG_INFO, "Com port reconfiguration complete, %d ports", iCount );

	return bSomeoneIsAlive;
}

bool CDeviceList::InitContext()
{
	bool bRc = true;
	bool bFound;
	int i;
	int idx;
	int iSuccess = 0;
	int iFailure = 0;

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

		if ( m_Device[idx].GetAddress() == 0 )
		{	// timer
			iSuccess += 1;
			LogMessage( E_MSG_INFO, "skip ctx for timer DeviceNo %d on '%s'", m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );
		}
		else if ( strncmp( m_Device[idx].GetComPort(), "ESP", 3 ) == 0 )
		{	// esp32
			iSuccess += 1;
			LogMessage( E_MSG_INFO, "skip ctx for ESP DeviceNo %d on '%s'", m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );
		}
		else if ( strncmp( m_Device[idx].GetComPort(), "Virtual", 7 ) == 0 )
		{	// virtual input
			iSuccess += 1;
			LogMessage( E_MSG_INFO, "skip ctx for Virtual Input DeviceNo %d on '%s'", m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );
		}
		else if ( m_Device[idx].GetDeviceType() == E_DT_LEVEL_K02 )
		{	// level K02 device
			iSuccess += 1;
			LogMessage( E_MSG_INFO, "Open COM port %s for Level K02 DeviceNo %d on '%s'", m_Device[idx].GetComPort(), m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );

			int iHandle;

			iHandle = open( m_Device[idx].GetComPort(), O_RDWR | O_NOCTTY | O_SYNC );
			if ( iHandle > 0 )
			{
				m_Device[idx].SetComHandle( iHandle );

				SetComInterfaceAttribs( iHandle, B9600, 8, 0 );  // set speed to 9600 bps, 8n1 (no parity)
				SetComBlocking( iHandle, 0 );                // set no blocking

			}
			else
			{
				LogMessage( E_MSG_ERROR, "Failed to open COM port %s, errno %d", m_Device[idx].GetComPort(), errno );
			}
		}
		else if ( m_Device[idx].GetDeviceType() == E_DT_CARD_READER )
		{	// card reader device
			iSuccess += 1;
			LogMessage( E_MSG_INFO, "Open COM port %s for CardReader DeviceNo %d on '%s'", m_Device[idx].GetComPort(), m_Device[idx].GetDeviceNo(), m_Device[idx].GetDeviceHostname() );

			int iHandle;

			iHandle = open( m_Device[idx].GetComPort(), O_RDWR | O_NOCTTY | O_SYNC );
			if ( iHandle > 0 )
			{
				m_Device[idx].SetComHandle( iHandle );

				SetComInterfaceAttribs( iHandle, B9600, 8, 0 );  // set speed to 9600 bps, 8n1 (no parity)
				SetComBlocking( iHandle, 0 );                // set no blocking

			}
			else
			{
				LogMessage( E_MSG_ERROR, "Failed to open COM port %s, errno %d", m_Device[idx].GetComPort(), errno );
			}
		}
		else if ( !bFound )
		{
			LogMessage( E_MSG_INFO, "Create ctx for serial connection on %s %d baud for DeviceNo %d", m_Device[idx].GetComPort(), m_Device[idx].GetBaudRate(), m_Device[idx].GetDeviceNo() );

			char szSimplePort[512] = "";
			ReadSimpleComPort( m_Device[idx].GetComPort(), szSimplePort, sizeof(szSimplePort) );
			LogMessage( E_MSG_INFO, "SimpleComPort '%s'", szSimplePort );

			m_Device[idx].SetContext( modbus_new_rtu( szSimplePort, m_Device[idx].GetBaudRate(), gRS485.cParity, gRS485.iDataBits, gRS485.iStopBits ) );
			if ( m_Device[idx].GetContext() == NULL )
			{
				LogMessage( E_MSG_FATAL, "modbus ctx in is null for DeviceNo %d, aborting: %s", m_Device[idx].GetDeviceNo(), modbus_strerror(errno) );
				FreeAllContexts();
				bRc = false;
				break;
			}
			else
			{
				if ( modbus_rtu_set_serial_mode(m_Device[i].GetContext(), MODBUS_RTU_RS232) == -1)
				{
					LogMessage( E_MSG_ERROR, "modbus_rtu_set_serial_mode() failed for DeviceNo %d, %s", m_Device[idx].GetDeviceNo(), modbus_strerror(errno) );
				}

				if ( modbus_connect(m_Device[idx].GetContext()) == -1 )
				{	// com port does not exist...
					iFailure += 1;
	//				FreeAllContexts();

					// try to continue with other devices
					modbus_close( m_Device[i].GetContext() );

					modbus_free( m_Device[i].GetContext() );
					m_Device[i].SetContext( NULL );

					LogMessage( E_MSG_ERROR, "modbus_connect() failed for DeviceNo %d, aborting: %s", m_Device[idx].GetDeviceNo(), modbus_strerror(errno) );
					//bRc = false;
					//break;
				}
				else
				{
					iSuccess += 1;
					LogMessage( E_MSG_INFO, "Serial connection started on %s, ctx %p", m_Device[idx].GetComPort(), m_Device[idx].GetContext() );
				}
			}
		}
		else
		{
			LogMessage( E_MSG_INFO, "DeviceNo %d shares a COM port", m_Device[idx].GetDeviceNo() );
		}
	}

	if ( bRc && iSuccess == 0 )
	{	// no successful devices
		LogMessage( E_MSG_FATAL, "No devices successfully initialised" );
		bRc = false;
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

const double CDeviceList::GetLastCheckedTimeMS( const int idx )
{
	double dTimeMS = 0.0;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		dTimeMS = m_Device[idx].GetLastCheckedTimeMS();
	}

	return dTimeMS;
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

int CDeviceList::GetComHandle( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetComHandle();
	}

	return 0;
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

const int CDeviceList::GetBaudRate( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetBaudRate();
	}

	return 9600;
}

const bool CDeviceList::GetAlwaysPoweredOn( const int idx )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetAlwaysPoweredOn();
	}

	return true;
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

// calling function must get E_LT_NODEESP lock first
void CDeviceList::GetEspResponseMsg( char* szEspName, size_t uNameLen, char* szEspResponseMsg, size_t uMsgLen )
{
	int i;

	szEspName[0] = '\0';
	szEspResponseMsg[0] = '\0';

	if ( m_iEspMessageCount > 0 )
	{
		snprintf( szEspName, uNameLen, m_szEspName[0] );
		snprintf( szEspResponseMsg, uMsgLen, m_szEspResponseMsg[0] );

		m_iEspMessageCount -= 1;
		m_szEspName[0][0] = '\0';
		m_szEspResponseMsg[0][0] = '\0';

		// remove the hole in the list
		for ( i = 1; i < MAX_ESP_QUEUE && m_iEspMessageCount > 0; i++ )
		{
			strcpy( m_szEspName[i-1], m_szEspName[i] );
			strcpy( m_szEspResponseMsg[i-1], m_szEspResponseMsg[i] );
			m_szEspName[i][0] = '\0';
			m_szEspResponseMsg[i][0] = '\0';
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

bool& CDeviceList::GetConditionTriggered( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetConditionTriggered(j);
	}

	return m_Device[0].GetConditionTriggered(j);
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

double& CDeviceList::GetMonitorValueLo( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetMonitorValueLo(j);
	}

	return m_Device[0].GetMonitorValueLo(j);
}

double& CDeviceList::GetMonitorValueHi( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetMonitorValueHi(j);
	}

	return m_Device[0].GetMonitorValueLo(j);
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

bool CDeviceList::DataBufferIsStable( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].DataBufferIsStable(j);
	}

	return m_Device[idx].DataBufferIsStable(0);
}

void CDeviceList::SaveDataValue( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].SaveDataValue(j);
	}
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

const double CDeviceList::GetResolution( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetResolution(j);
	}

	return 0.0;
}

const char* CDeviceList::GetMonitorPos( const int idx, const int j )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].GetMonitorPos(j);
	}

	return "   ";
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

const double CDeviceList::CalcCurrent( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcCurrent( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcTHValue( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcTHValue( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcLevel( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcLevel( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcRotaryEncoderDistance( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcRotaryEncoderDistance( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcVIPFValue( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcVIPFValue( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcVSDNFlixenValue( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcVSDNFlixenValue( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcVSDPwrElectValue( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcVSDPwrElectValue( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcPT113Weight( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcPT113Weight( iChannel, bNew );
	}

	return 0.0;
}

const double CDeviceList::CalcSystecIT1Weight( const int idx, const int iChannel, const bool bNew )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		return m_Device[idx].CalcSystecIT1Weight( iChannel, bNew );
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

void CDeviceList::SetLastCheckedTimeMS( const int idx, const double dTimeMS )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetLastCheckedTimeMS( dTimeMS );
	}
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

void CDeviceList::SetBaudRate( const int idx, const int iBaudRate )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetBaudRate( iBaudRate );
	}
}

void CDeviceList::SetAlwaysPoweredOn( const int idx, const char* szAlwaysPoweredOn )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetAlwaysPoweredOn( szAlwaysPoweredOn );
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

// calling function must get E_LT_NODEESP lock first
void CDeviceList::SetEspResponseMsg( const char* szEspName, const char* szEspResponseMsg )
{
	if ( m_iEspMessageCount >= 0 && m_iEspMessageCount < MAX_ESP_QUEUE )
	{	// found an empty slot
		snprintf( m_szEspResponseMsg[m_iEspMessageCount], sizeof(m_szEspResponseMsg[m_iEspMessageCount]), "%s", szEspResponseMsg );
		snprintf( m_szEspName[m_iEspMessageCount], sizeof(m_szEspName[m_iEspMessageCount]), "%s", szEspName );
		m_iEspMessageCount += 1;
	}
	else
	{
		LogMessage( E_MSG_ERROR, "No slot for ESP response message, %d vs %d", m_iEspMessageCount, MAX_ESP_QUEUE );
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

bool CDeviceList::SetDeviceStatus( const int idx, const enum E_DEVICE_STATUS eStatus, const bool bNoTimeout )
{
	bool bChanged = false;
	enum E_DEVICE_STATUS eLastStatus;

	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		eLastStatus = m_Device[idx].GetDeviceStatus();

		m_Device[idx].SetDeviceStatus( eStatus, bNoTimeout );

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

void CDeviceList::SetResolution( const int idx, const int j, const double dResolution )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetResolution( j, dResolution );
	}
}

void CDeviceList::SetMonitorPos( const int idx, const int j, const char* szMonPos )
{
	if ( idx >= 0 && idx < MAX_DEVICES )
	{
		m_Device[idx].SetMonitorPos( j, szMonPos );
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

const int CDeviceList::GetEspMessageCount()
{
	return m_iEspMessageCount;
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
		LogMessage( E_MSG_DEBUG, "LogOnTime(): Device %d, channel %d was not on ??", idx, iChannel+1 );
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
		if ( m_Device[i].GetAddress() > 0 && 
				strncmp( m_Device[i].GetComPort(), "ESP", 3 ) != 0 &&
				strncmp( m_Device[i].GetComPort(), "Virtual", 7 ) != 0 &&
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

	LogMessage( E_MSG_INFO, "modbus_write_bit(%p) %d %d %u", ctx, idx, iOutChannel+1, uState );
	if ( modbus_write_bit( ctx, iOutChannel, (bool)uState ) == -1 )
	{	// failed
		bRc = false;

		LogMessage( E_MSG_ERROR, "WriteOutputBit() failed for device %d, channel %d, state %u: %s", idx, iOutChannel+1, uState, modbus_strerror(errno) );
	}

	return bRc;
}


bool CDeviceList::UpdatePinFailCount( CMysql& myDB, const char* szCardNumber, const int iPinFailCount )
{
	bool bRet = false;
	char szCardEnabled[2] = "Y";

	if ( iPinFailCount >= MAX_PIN_FAILURES )
	{
		szCardEnabled[0] = 'N';
	}

	if ( myDB.RunQuery( "update users set us_PinFailCount=%d,us_CardEnabled='%s' where us_CardNumber='%s'", iPinFailCount, szCardEnabled, szCardNumber ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::SelectCardNumber( CMysql& myDB, const char* szCardNumber, char* szCardPin, const int iLen, bool& bEnabled, int& iPinFailCount )
{
	bool bRet = false;
	int iNumFields;
	MYSQL_ROW row;

	szCardPin[0] = '\0';
	bEnabled = false;
	iPinFailCount = 0;

	if ( myDB.RunQuery( "select us_CardPin,us_CardEnabled,us_PinFailCount from users where us_CardNumber='%s'", szCardNumber ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else if ( (row = myDB.FetchRow( iNumFields )) )
	{
		bRet = true;
		snprintf( szCardPin, iLen, "%s", (const char*)row[0] );
		bEnabled = ( strcmp((const char*)row[1], "Y" ) == 0 ? true : false);
		iPinFailCount = atoi((const char*)row[2]);

		LogMessage( E_MSG_INFO, "Found card details for '%s'", szCardNumber );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::GetLastCardSwipeTimestamp( CMysql& myDB, const char* szCardNumber, time_t& tLastCardSwipe, double& dLastWeight )
{
	bool bRet = true;
	int iNumFields;
	MYSQL_ROW row;

	tLastCardSwipe = 0;
	dLastWeight = 0;
	if ( myDB.RunQuery( "select unix_timestamp(ev_Timestamp),ev_Value from events where ev_EventType=%d and ev_Description='%s' order by ev_Timestamp desc limit 1", E_ET_CARDWEIGHT, szCardNumber ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else if ( (row = myDB.FetchRow( iNumFields )) )
	{
		bRet = true;
		tLastCardSwipe = atol((const char*)row[0]);
		dLastWeight = atof( (const char*)row[1] );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::UpdateTruckTare( CMysql& myDB, const char* szCardNumber, const double dTruckWeight )
{
	bool bRet = true;

	if ( myDB.RunQuery( "update users set us_TruckTare=%ld where us_CardNumber='%s'", (long)dTruckWeight, szCardNumber ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::SelectAccountEmails(CMysql& myDB, std::string& sEmails )
{
	bool bRet = false;
	int iNumFields;
	MYSQL_ROW row;

	sEmails = "";

	if ( myDB.RunQuery( "select us_BillingEmail from users where us_Features like '___Y%%'" ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else 
	{
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			bRet = true;
			if ( sEmails.length() > 0 )
			{	// comma separated list
				sEmails += ",";
			}
			sEmails += (const char*)row[0];
		}
		LogMessage( E_MSG_INFO, "Found accounts email details" );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::SelectCardNumberBilling( CMysql& myDB, const char* szCardNumber, char* szBillingName, const size_t uName, char* szBillingAddr1, const size_t uAddr1,
	char* szBillingAddr2, const size_t uAddr2, char* szBillingAddr3, const size_t uAddr3, char* szBillingEmail, const size_t uEmail )
{
	bool bRet = false;
	int iNumFields;
	MYSQL_ROW row;

	szBillingName[0] = '\0';
	szBillingAddr1[0] = '\0';
	szBillingAddr2[0] = '\0';
	szBillingAddr3[0] = '\0';
	szBillingEmail[0] = '\0';

	if ( myDB.RunQuery( "select us_BillingName,us_BillingAddr1,us_BillingAddr2,us_BillingAddr3,us_BillingEmail from users where us_CardNumber='%s'", szCardNumber ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else if ( (row = myDB.FetchRow( iNumFields )) )
	{
		bRet = true;
		snprintf( szBillingName, uName, "%s", (const char*)row[0] );
		snprintf( szBillingAddr1, uAddr1, "%s", (const char*)row[1] );
		snprintf( szBillingAddr2, uAddr2, "%s", (const char*)row[2] );
		snprintf( szBillingAddr3, uAddr3, "%s", (const char*)row[3] );
		snprintf( szBillingEmail, uEmail, "%s", (const char*)row[4] );

		LogMessage( E_MSG_INFO, "Found card billing details for '%s'", szCardNumber );
	}
	myDB.FreeResult();

	return bRet;
}

bool CDeviceList::SelectCardNumberRego( CMysql& myDB, const char* szCardNumber, char* szTruckRego, const size_t uLen1, char* szTruckTare, const size_t uLen2 )
{
	bool bRet = false;
	int iNumFields;
	MYSQL_ROW row;

	szTruckRego[0] = '\0';
	szTruckTare[0] = '\0';

	if ( myDB.RunQuery( "select us_TruckRego,us_TruckTare from users where us_CardNumber='%s'", szCardNumber ) != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else if ( (row = myDB.FetchRow( iNumFields )) )
	{
		bRet = true;
		snprintf( szTruckRego, uLen1, "%s", (const char*)row[0] );
		snprintf( szTruckTare, uLen2, "%06ld", atol((const char*)row[1]) );

		LogMessage( E_MSG_INFO, "Found card rego details for '%s'", szCardNumber );
	}
	myDB.FreeResult();

	return bRet;
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
	enum E_DEVICE_STATUS eStatus;
	modbus_t* pCtx;
	int iHandle;
	int iNumFields;
	MYSQL_ROW row;

	for ( i = 0; i < MAX_DEVICES; i++ )
	{
		eStatus = m_Device[i].GetDeviceStatus();
		pCtx = m_Device[i].GetContext();
		iHandle = m_Device[i].GetComHandle();

		m_Device[i].Init();

		m_Device[i].SetComHandle( iHandle );
		m_Device[i].SetContext( pCtx );
		m_Device[i].SetDeviceStatus( eStatus, false );
	}

	// read from mysql
	//                          0           1          2          3            4             5       6       7           8           9
	if ( myDB.RunQuery( "SELECT de_DeviceNo,de_ComPort,de_Address,de_NumInputs,de_NumOutputs,de_Type,de_Name,de_Hostname,de_BaudRate,de_AlwaysPoweredOn FROM devices order by de_DeviceNo") != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		i = 0;
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			SetDeviceNo( i, atoi((const char*)row[0]) );
			SetComPort( i, (const char*)row[1] );
			SetAddress( i, atoi( (const char*)row[2] ) );
			SetNumInputs( i, atoi( (const char*)row[3] ) );
			SetNumOutputs( i, atoi( (const char*)row[4] ) );
			SetDeviceType( i, (enum E_DEVICE_TYPE)atoi( (const char*)row[5] ) );
			SetDeviceName( i, (const char*)row[6] );
			SetDeviceHostname( i, (const char*)row[7] );
			SetBaudRate( i, atoi( (const char*)row[8] ) );
			SetAlwaysPoweredOn( i, (const char*)row[9] );

			LogMessage( E_MSG_INFO, "Device(%d): DeNo:%d, Port:'%s', Addr:%d, DType:%d, Name:'%s', Host:'%s', Baud %d, APO %d", i, GetDeviceNo(i), GetComPort(i), GetAddress(i), GetDeviceType(i),
					GetDeviceName(i), GetDeviceHostname(i), GetBaudRate(i), GetAlwaysPoweredOn(i) );

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
		{	//                          0            1         2         3           4            5
			if ( myDB.RunQuery( "SELECT di_IOChannel,di_IOName,di_IOType,di_OnPeriod,di_StartTime,di_Hysteresis,"
				//   6                 7              8           9             10            11        12
					"di_OutOnStartTime,di_OutOnPeriod,di_Weekdays,di_AnalogType,di_CalcFactor,di_Offset,di_MonitorPos,"
				//   13           14		   15
					"di_MonitorHi,di_MonitorLo,di_Resolution FROM deviceinfo WHERE di_DeviceNo=%d order by di_IOChannel",
					m_Device[i].GetDeviceNo() ) != 0 )
			{
				bRet = false;
				LogMessage( E_MSG_ERROR, "RunQuery(%s): %s", myDB.GetQuery(), myDB.GetError() );
			}
			else
			{
				while ( (row = myDB.FetchRow( iNumFields )) )
				{
					j = atoi( (const char*)row[0] );

					eIOType = (enum E_IO_TYPE)atoi( (const char*)row[2] );

					if ( eIOType == E_IO_OUTPUT )
					{
						SetOutIOName( i, j, (const char*)row[1] );
						GetOutChannelType( i, j ) = eIOType;
						// row[3]
						// row[4]
						// row[5]
						SetOutOnStartTime( i, j, atoi( (const char*)row[6] ) );
						SetOutOnPeriod( i, j, atoi( (const char*)row[7] ) );
						SetOutWeekdays( i, j, (const char*)row[8] );
						// row[9]
						// row[10]
						// row[11]
						SetMonitorPos( i, j, (const char*)row[12] );
						// row[13]
						// row[14]
						// row[15]

						LogMessage( E_MSG_INFO, "DeviceInfo OUT(%d,%d): Name:'%s', Type:%d, OnPeriod:%d, STime:%d, OnPeriod:%d, Days:%s, MonPos:'%s'", i, j,
								GetOutIOName(i,j), GetOutChannelType(i,j), GetOutOnPeriod(i,j),
								GetOutOnStartTime(i,j), GetOutOnPeriod(i,j), GetOutWeekdays(i,j),
								GetMonitorPos(i,j) );
					}
					else
					{
						SetInIOName( i, j, (const char*)row[1] );
						GetInChannelType( i, j ) = eIOType;
						SetInOnPeriod( i, j, atoi( (const char*)row[3] ) );
						SetStartTime( i, j, atoi( (const char*)row[4] ) );
						GetHysteresis( i, j ) = atoi( (const char*)row[5] );
						// row[6]
						// row[7]
						SetInWeekdays( i, j, (const char*)row[8] );
						SetAnalogType( i, j, (const char)row[9][0] );
						SetCalcFactor( i, j, atof( (const char*)row[10] ) );
						SetOffset( i, j, atof( (const char*)row[11] ) );
						SetMonitorPos( i, j, (const char*)row[12] );
						GetMonitorValueHi( i, j ) = atof( (const char*)row[13] );
						GetMonitorValueLo( i, j ) = atof( (const char*)row[14] );
						SetResolution( i, j, atof( (const char*)row[15] ) );

						if ( GetInChannelType(i,j) == E_IO_ON_OFF_INV )
						{	// invert the state
							GetLastInput(i,j) = true;
						}

						LogMessage( E_MSG_INFO, "DeviceInfo  IN(%d,%d): Name:'%s', Type:%d, OnPeriod:%d, STime:%d, Hyst:%d, Temp:%.1f, Days:%s AType:'%c', CFactor:%.3f, Offset:%.2f, MonPos:'%s', MonHi:%.1f, MonLo:%.1f, Res:%.3f",
								i, j, GetInIOName(i,j), GetInChannelType(i,j), GetInOnPeriod(i,j),
								GetStartTime(i,j), GetHysteresis(i,j), GetMonitorValueLo(i,j), GetInWeekdays(i,j), GetAnalogType(i,j), GetCalcFactor(i,j),
								GetOffset(i,j), GetMonitorPos(i,j), GetMonitorValueHi(i,j), GetMonitorValueLo(i,j), GetResolution(i,j) );
					}
				}

				myDB.FreeResult();
			}
		}
	}

	bRet = true;

	return bRet;
}

const bool CDeviceList::IsEspDevice( const int idx )
{
	bool bRet = false;

	if ( idx >=0 && idx < MAX_DEVICES )
	{
		if ( strncmp( m_Device[idx].GetComPort(), "ESP", 3 ) == 0 )
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
	case E_ET_LEVEL:			// 9:	level
		snprintf( szBuf, sizeof(szBuf), "Level" );
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
	int cn;

	for ( i = 0; i < MAX_IO_LINKS; i++ )
	{
		SetLinkNo( i, 0 );
		SetInDeviceNo( i, 0 );
		SetInChannel( i, 0 );
		SetOutDeviceNo( i, 0 );
		SetOutChannel( i, 0 );
		SetEventType( i, E_ET_CLICK );
		SetOnPeriod( i, 0 );
		SetVsdFrequency( i, 0.0 );

		for ( cn = 0; cn < MAX_CONDITIONS; cn++ )
		{
			SetLinkDeviceNo( i, cn, 0 );
			SetLinkChannel( i, cn, 0 );
			SetLinkTest( i, cn, "" );
			SetLinkValue( i, cn, 0.0 );
		}
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

void CInOutLinks::SetVsdFrequency( const int idx, const double dVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		m_dVsdFrequency[idx] = dVal;
	}
}

void CInOutLinks::SetLinkDeviceNo( const int idx, const int cn, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		if ( iVal >= 0 && iVal < 0xfe )
		{
			m_iLinkDeviceNo[idx][cn] = iVal;
		}
	}
}

void CInOutLinks::SetLinkChannel( const int idx, const int cn, const int iVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		if ( iVal >= 0 && iVal < MAX_IO_PORTS )
		{
			m_iLinkChannel[idx][cn] = iVal;
		}
	}
}

void CInOutLinks::SetLinkTest( const int idx, const int cn, const char* szTest )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		snprintf( m_szLinkTest[idx][cn], sizeof(m_szLinkTest[idx][cn]), "%s", szTest );
	}
}

void CInOutLinks::SetLinkValue( const int idx, const int cn, const double dVal )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		m_dLinkValue[idx][cn] = dVal;
	}
}

const int CInOutLinks::GetLinkDeviceNo( const int idx, const int cn )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		return m_iLinkDeviceNo[idx][cn];
	}

	return 0;
}

const int CInOutLinks::GetLinkChannel( const int idx, const int cn )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		return m_iLinkChannel[idx][cn];
	}

	return 0;
}

const char* CInOutLinks::GetLinkTest( const int idx, const int cn )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		return m_szLinkTest[idx][cn];
	}

	return 0;
}

const double CInOutLinks::GetLinkValue( const int idx, const int cn )
{
	if ( idx >= 0 && idx < MAX_IO_LINKS && cn >= 0 && cn < MAX_CONDITIONS )
	{
		return m_dLinkValue[idx][cn];
	}

	return 0.0;
}

// returns the idx of the linked device or -1 if not found
const int CInOutLinks::FindLinkedDevice( const int iInDeviceNo, const int iInChannel, const enum E_DEVICE_TYPE eDeviceType, CDeviceList* m_pmyDevices )
{
	int idx = -1;
	int lnk = 0;

	while ( lnk < MAX_IO_LINKS )
	{
		if ( m_iInDeviceNo[lnk] == iInDeviceNo && m_iInChannel[lnk] == iInChannel )
		{
			LogMessage( E_MSG_INFO, "Found link for %d,%d", iInDeviceNo, iInChannel );
			int iAddr = m_pmyDevices->GetAddressForDeviceNo(m_iOutDeviceNo[lnk]);
			int i = m_pmyDevices->GetIdxForAddr(iAddr);
			if ( m_pmyDevices->GetDeviceType(i) == eDeviceType )
			{
				LogMessage( E_MSG_INFO, "Found linked device type %d for %d,%d: %s", eDeviceType, iInDeviceNo, iInChannel, m_pmyDevices->GetDeviceName(i) );
				idx = i;
				break;
			}
		}

		lnk += 1;
	}

	return idx;
}

bool CInOutLinks::Find( const int iInDeviceNo, const int iInChannel, int &idx, int& iOutDeviceNo, int& iOutChannel, int& iOnPeriod, double& dVsdFrequency, bool& bLinkTestPassed, bool& bInvertState, CDeviceList* m_pmyDevices )
{
	bool bRet = false;
	int cn;

	iOutDeviceNo = 0;
	iOutChannel = 0;
	iOnPeriod = 0;
	dVsdFrequency = 0.0;
	bLinkTestPassed = true;
	bInvertState = false;

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
				dVsdFrequency = m_dVsdFrequency[idx];

				// check if the link test passes all conditions
				for ( cn = 0; cn < MAX_CONDITIONS; cn++ )
				{
					if ( m_iLinkDeviceNo[idx][cn] == 0 )
					{	// end of list
						break;
					}
					else
					{
						int iLinkAddress;
						int iLinkIdx;

						iLinkAddress = m_pmyDevices->GetAddressForDeviceNo( m_iLinkDeviceNo[idx][cn] );
						iLinkIdx = m_pmyDevices->GetIdxForAddr(iLinkAddress);

						if ( m_pmyDevices->LinkTestPassed( iLinkIdx, m_iLinkChannel[idx][cn], m_szLinkTest[idx][cn], m_dLinkValue[idx][cn], bInvertState ) )
						{
							LogMessage( E_MSG_INFO, "Checking link '%s' (0x%x->%d,%d), '%s' %.1f (%d), passed", m_pmyDevices->GetInIOName(iLinkIdx,m_iLinkChannel[idx][cn]),
									iLinkAddress, iLinkIdx, m_iLinkChannel[idx][cn], m_szLinkTest[idx][cn], m_dLinkValue[idx][cn], bInvertState );
						}
						else
						{
							LogMessage( E_MSG_INFO, "Checking link '%s' (0x%x->%d,%d), '%s' %.1f (%d), failed", m_pmyDevices->GetInIOName(iLinkIdx,m_iLinkChannel[idx][cn]),
									iLinkAddress, iLinkIdx, m_iLinkChannel[idx][cn], m_szLinkTest[idx][cn], m_dLinkValue[idx][cn], bInvertState );

							bLinkTestPassed = false;
							break;
						}

					}
				}
				//LogMessage( E_MSG_INFO, "Find() found %d %d %d %d", iOutDeviceNo, iOutChannel, iOnPeriod, bLinkTestPassed );

				idx += 1;
				break;
			}

			idx += 1;
		}
	}

	return bRet;
}

const int CInOutLinks::FindEmptyConditionSlot( const int idx )
{
	int cn = -1;

	if ( idx >= 0 && idx < MAX_IO_LINKS )
	{
		for ( cn = 0; cn < MAX_CONDITIONS; cn++ )
		{
			if ( m_iLinkDeviceNo[idx][cn] == 0 )
			{	// empty slot
				break;
			}
		}
	}

	return cn;
}

bool CInOutLinks::ReadIOLinks( CMysql& myDB )
{
	bool bRet = true;
	int i;
	int cn;
	int iNumFields;
	MYSQL_ROW row;

	Init();

	// read from mysql
	//                          0         1             2            3              4             5            6           7
	if ( myDB.RunQuery( "SELECT il_LinkNo,il_InDeviceNo,il_InChannel,il_OutDeviceNo,il_OutChannel,il_EventType,il_OnPeriod,il_VsdFrequency "
			"FROM iolinks order by il_InDeviceNo,il_InChannel") != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		i = 0;
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			SetLinkNo( i, atoi( (const char*)row[0] ) );
			SetInDeviceNo( i, atoi( (const char*)row[1] ) );
			SetInChannel( i, atoi( (const char*)row[2] ) );
			SetOutDeviceNo( i, atoi( (const char*)row[3] ) );
			SetOutChannel( i, atoi( (const char*)row[4] ) );
			SetEventType( i, (E_EVENT_TYPE)atoi( (const char*)row[5] ) );
			SetOnPeriod( i, atoi( (const char*)row[6] ) );
			SetVsdFrequency( i, atof( (const char*)row[7] ) );

			LogMessage( E_MSG_INFO, "IOLinkNo:%d, InDeviceNo:%d, InChannel:%d, OutDeviceNo:%d, OutChannel:%d, EventType:%d, OnPeriod:%d, VsdFreq:%.1f", GetLinkNo(i),
					GetInDeviceNo(i), GetInChannel(i), GetOutDeviceNo(i), GetOutChannel(i), GetEventType(i), GetOnPeriod(i), GetVsdFrequency(i) );

			i += 1;
		}
	}

	myDB.FreeResult();

	// read the conditions
	//                          0              1         2               3              4           5
	if ( myDB.RunQuery( "SELECT co_ConditionNo,co_LinkNo,co_LinkDeviceNo,co_LinkChannel,co_LinkTest,co_LinkValue "
			"FROM conditions order by co_LinkNo") != 0 )
	{
		bRet = false;
		LogMessage( E_MSG_ERROR, "RunQuery(%s) error: %s", myDB.GetQuery(), myDB.GetError() );
	}
	else
	{
		while ( (row = myDB.FetchRow( iNumFields )) )
		{
			// find the position
			for ( i = 0; i < MAX_IO_LINKS; i++ )
			{
				if ( GetLinkNo(i) == atoi((const char*)row[1]) )
				{
					break;
				}
			}

			if ( i >= 0 && i < MAX_IO_LINKS )
			{
				cn = FindEmptyConditionSlot( i );

				SetLinkDeviceNo( i, cn, atoi( (const char*)row[2] ) );
				SetLinkChannel( i, cn, atoi( (const char*)row[3] ) );
				SetLinkTest( i, cn, (const char*)row[4] );
				SetLinkValue( i, cn, atof( (const char*)row[5] ) );

				LogMessage( E_MSG_INFO, "Conditions: IOLinkNo:%d, LinkDeNo:%d, LinkCh:%d, LinkTest:%s, LinkValue:%.1f",
						GetLinkNo(i), GetLinkDeviceNo(i,cn), GetLinkChannel(i,cn), GetLinkTest(i,cn), GetLinkValue(i,cn) );
			}
			else
			{
				LogMessage( E_MSG_ERROR, "Condition %d, cannot find LinkNo %d", atoi((const char*)row[0]), atoi((const char*)row[1]) );
			}
		}
	}

	return bRet;
}

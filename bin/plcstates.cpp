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
//	plcstates classes
//
//**************************************************************************************************************
CPlcState::CPlcState()
{
	Init();
}

CPlcState::~CPlcState()
{

}

void CPlcState::Init()
{
	m_iStateNo = 0;
	m_szOperation[0] = '\0';
	m_szStateName[0] = '\0';
	m_szStateIsActive[0] = '\0';
	m_dStateTimestampMS = 0;
	m_szRuleType[0] = '\0';
	m_iDeviceNo = 0;
	m_iIOChannel = 0;
	m_dValue = 0.0;
	m_dRuntimeValue = 0.0;
	m_szTest[0] = '\0';
	m_szNextStateName[0] = '\0';
	m_iOrder = 0;
	m_dDelayTime = 0;
	m_szTimerValues[0] = '\0';
}

void CPlcState::SetStateNo( const int iStateNo )
{
	m_iStateNo = iStateNo;
}

void CPlcState::SetOperation( const char* szOperation )
{
	snprintf( m_szOperation, sizeof(m_szOperation), "%s", szOperation );
}

void CPlcState::SetStateName( const char* szStateName )
{
	snprintf( m_szStateName, sizeof(m_szStateName), "%s", szStateName );
}

void CPlcState::SetStateIsActive( const char* szActive )
{
	snprintf( m_szStateIsActive, sizeof(m_szStateIsActive), "%s", szActive );
}

void CPlcState::SetStateTimestampMS( const double dTimenowMS )
{
	m_dStateTimestampMS = dTimenowMS;
}

void CPlcState::SetRuleType( const char* szRuleType )
{
	snprintf( m_szRuleType, sizeof(m_szRuleType), "%s", szRuleType );
}

void CPlcState::SetDeviceNo( const int iDeviceNo )
{
	m_iDeviceNo = iDeviceNo;
}

void CPlcState::SetIOChannel( const int iIOChannel )
{
	m_iIOChannel = iIOChannel;
}

void CPlcState::SetValue( const double iValue )
{
	m_dValue = iValue;
}

void CPlcState::SetRuntimeValue( const double iValue )
{
	m_dRuntimeValue = iValue;
}

void CPlcState::SetTest( const char* szTest )
{
	snprintf( m_szTest, sizeof(m_szTest), "%s", szTest );
}

void CPlcState::SetNextStateName( const char* szNextStateName )
{
	snprintf( m_szNextStateName, sizeof(m_szNextStateName), "%s", szNextStateName );
}

void CPlcState::SetOrder( const int iOrder )
{
	m_iOrder = iOrder;
}

void CPlcState::SetDelayTime( const double dDelayTime )
{
	m_dDelayTime = dDelayTime;
}

void CPlcState::SetTimerValues( const char* szTimerValues )
{
	snprintf( m_szTimerValues, sizeof(m_szTimerValues), "%s", szTimerValues );
}


//*************************************************
CPlcStates::CPlcStates()
{
	Init();
}

CPlcStates::~CPlcStates()
{

}

void CPlcStates::Init()
{
	int i;

	m_iActiveStateIdx = -1;
	m_iStateCount = 0;
	for ( i = 0; i < MAX_PLC_STATES; i++ )
	{
		m_State[i].Init();
	}
	for ( i = 0; i < MAX_PLC_INPUT_EVENTS; i++ )
	{
		m_iInputDeviceNo[i] = 0;
		m_iInputIOChannel[i] = 0;
		m_dInputValue[i] = 0.0;
	}
}

CPlcState& CPlcStates::GetState( const int idx )
{
	if ( idx >= 0 && idx < MAX_PLC_STATES )
	{
		return m_State[idx];
	}

	LogMessage( E_MSG_ERROR, "PlcState idx %d out of range", idx );

	return m_State[0];
}

const int CPlcStates::GetVsdStateIdx( const int iStartIdx, const int iDeviceNo, const int iIOChannel )
{
	int idx = -1;

	if ( iStartIdx >= 0 && iStartIdx < MAX_PLC_STATES )
	{
		for ( int i = iStartIdx; idx < MAX_PLC_STATES; i++ )
		{
			if ( strcmp( m_State[iStartIdx].GetOperation(), m_State[i].GetOperation() ) == 0 && strcmp( m_State[iStartIdx].GetStateName(), m_State[i].GetStateName() ) == 0 )
			{
				if ( m_State[i].GetDeviceNo() == iDeviceNo && m_State[i].GetIOChannel() == iIOChannel )
				{
					idx = i;
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		LogMessage( E_MSG_ERROR, "GetVsdStateIdx(): PlcState idx %d out of range", idx );
	}

	if ( idx < 0 )
	{
		LogMessage( E_MSG_ERROR, "GetVsdStateIdx(): failed to find device %d,%d", iDeviceNo, iIOChannel );
	}

	return idx;
}

const int CPlcStates::GetStateCount()
{
	return m_iStateCount;
}

void CPlcStates::AddState()
{
	m_iStateCount += 1;
}

const bool CPlcStates::IsActive()
{
	return (m_iActiveStateIdx == -1 ? false : true );
}

void CPlcStates::SetActiveStateIdx( const int idx )
{
	m_iActiveStateIdx = idx;
}

const int CPlcStates::FindStateNo( const int iStateNo )
{
	int idx = -1;

	for ( int i = 0; i < m_iStateCount; i++ )
	{
		if ( m_State[i].GetStateNo() == iStateNo )
		{
			idx = i;
			break;
		}
	}

	return idx;
}

const int CPlcStates::GetActiveStateIdx()
{
	return m_iActiveStateIdx;
}

const int CPlcStates::GetInitialAction( const int iStartIdx )
{
	int idx = -1;

	for ( int i = iStartIdx+1; i < m_iStateCount; i++ )
	{
		if ( strcmp( m_State[iStartIdx].GetOperation(), m_State[i].GetOperation() ) == 0 &&
			 strcmp( m_State[iStartIdx].GetStateName(), m_State[i].GetStateName() ) == 0 )
		{	// same operation and state
			if ( m_State[i].GetRuleType()[0] == 'I' )
			{	// found one
				idx = i;
				break;
			}
		}
		else
		{	// end of list
			break;
		}
	}

	return idx;
}

const int CPlcStates::GetEvent( const int iStartIdx )
{
	int idx = -1;

	for ( int i = iStartIdx+1; i < m_iStateCount; i++ )
	{
		if ( strcmp( m_State[iStartIdx].GetOperation(), m_State[i].GetOperation() ) == 0 &&
			 strcmp( m_State[iStartIdx].GetStateName(), m_State[i].GetStateName() ) == 0 )
		{	// same operation and state
			if ( m_State[i].GetRuleType()[0] == 'E' )
			{	// found one
				idx = i;
				break;
			}
		}
		else
		{	// end of list
			break;
		}
	}

	return idx;
}

const bool CPlcStates::FindInputDevice( const int iDeviceNo, const int iIOChannel )
{
	bool rc = false;

	for ( int i = 0; i < m_iStateCount; i++ )
	{
		if ( strcmp( m_State[i].GetOperation(), m_State[m_iActiveStateIdx].GetOperation() ) == 0 &&
			strcmp( m_State[i].GetStateName(), m_State[m_iActiveStateIdx].GetStateName() ) == 0 &&
			m_State[i].GetDeviceNo() == iDeviceNo && m_State[i].GetIOChannel() == iIOChannel && m_State[i].GetRuleType()[0] == 'E' )
		{
			rc = true;
			break;
		}
	}

	return rc;
}

void CPlcStates::AddInputEvent( const int iDeviceNo, const int iIOChannel, const double dValue )
{
	bool bFound = false;
	int i;

	for ( i = 0; i < MAX_PLC_INPUT_EVENTS; i++ )
	{
		if ( m_iInputDeviceNo[i] == 0 )
		{	// empty slot
			bFound = true;
			m_iInputDeviceNo[i] = iDeviceNo;
			m_iInputIOChannel[i] = iIOChannel;
			m_dInputValue[i] = dValue;

			LogMessage( E_MSG_INFO, "AddInputEvent %d %d %.1f", iDeviceNo, iIOChannel, dValue );
			break;
		}
	}

	if ( !bFound )
	{
		LogMessage( E_MSG_ERROR, "PLC no empty slot for input event %d,%d,%.1f", iDeviceNo, iIOChannel, dValue );
	}
}

const int CPlcStates::ReadInputEvent( int& iDeviceNo, int& iIOChannel, double& dValue )
{
	int iStateNo = 0;
	int i;

	iDeviceNo = 0;
	iIOChannel = 0;

	for ( i = 0; i < MAX_PLC_INPUT_EVENTS; i++ )
	{
		if ( m_iInputDeviceNo[i] != 0 )
		{	// found a new event
			iDeviceNo = m_iInputDeviceNo[i];
			iIOChannel = m_iInputIOChannel[i];
			dValue = m_dInputValue[i];

			m_iInputDeviceNo[i] = 0;
			m_iInputIOChannel[i] = 0;
			m_dInputValue[i] = 0;
			break;
		}
	}

	if ( iDeviceNo != 0 )
	{	// now remove the hole in the list
		for  ( ; i+1 < MAX_PLC_INPUT_EVENTS; i++ )
		{
			m_iInputDeviceNo[i] = m_iInputDeviceNo[i+1];
			m_iInputIOChannel[i] = m_iInputIOChannel[i+1];
			m_dInputValue[i] = m_dInputValue[i+1];
		}
		m_iInputDeviceNo[i] = 0;
		m_iInputIOChannel[i] = 0;
		m_dInputValue[i] = 0;

		// find the matching StateNo if this device.channel is valid for this state
		for ( i = m_iActiveStateIdx; i < m_iStateCount; i++ )
		{
			if ( strcmp( m_State[m_iActiveStateIdx].GetOperation(), m_State[i].GetOperation() ) == 0 &&
				 strcmp( m_State[m_iActiveStateIdx].GetStateName(), m_State[i].GetStateName() ) == 0 )
			{	// same operation and state
				if ( m_State[i].GetRuleType()[0] == 'E' && strlen(m_State[i].GetNextStateName()) != 0 && 
					iDeviceNo == m_State[i].GetDeviceNo() && iIOChannel == m_State[i].GetIOChannel() )
				{	// valid input event
					iStateNo = m_State[i].GetStateNo();
					break;
				}
			}
		}
	}

	//LogMessage( E_MSG_INFO, "ReadInputState() %d", iStateNo );

	return iStateNo;
}

void CPlcStates::SetNextStateActive( const int idx, const double dTimenowMS )
{
	int i;

	if ( idx >= 0 && idx < m_iStateCount )
	{
		m_State[m_iActiveStateIdx].SetStateIsActive( "N" );

		for ( i = 0; i < m_iStateCount; i++ )
		{
			if ( strcmp( m_State[i].GetOperation(), m_State[idx].GetOperation() ) == 0 && strcmp( m_State[i].GetStateName(), m_State[idx].GetNextStateName() ) == 0 &&
					m_State[i].GetRuleType()[0] == '\0' )
			{
				m_State[i].SetStateIsActive("Y");
				m_State[i].SetStateTimestampMS( dTimenowMS );
				m_iActiveStateIdx = i;
				break;
			}
		}
	}
}

const int CPlcStates::GetNextStateIdx( const char* szOperation, const char* szNextStateName )
{
	int idx = -1;

	for ( int i = 0; i < m_iStateCount; i++ )
	{
		if ( strcmp( szOperation, m_State[i].GetOperation() ) == 0 &&
			 strcmp( szNextStateName, m_State[i].GetStateName() ) == 0 )
		{	// same operation and state
			idx = i;
			break;
		}
	}

	return idx;
}


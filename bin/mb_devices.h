#ifndef _INC_MB_DEVICES_H
#define _INC_MB_DEVICES_H



#define MAX_EVENT_PERIOD			300		// 5 minutes
#define DOUBLE_CLICK_MSEC			500		// 700 milliseconds
#define LONG_PRESS_MSEC				1400	// 2 seconds
#define MAX_DEVICE_NAME_LEN			100
#define MAX_COMPORT_LEN				100
#define MAX_HOSTNAME_LEN			20
#define MAX_DEVICES					32		// across all pi's
#define MAX_IO_PORTS				16		// max channels per modbus io device
#define MAX_IO_LINKS				100		// across all pi's
#define MCU_MSG_SIZE				20
#define MAX_MCU_QUEUE				4



enum E_DEVICE_TYPE {
	E_DT_UNUSED = 0,
	E_DT_DIGITAL_IO,		// 1: digital input and/or output
	E_DT_TEMPERATURE,		// 2: temperature
	E_DT_TIMER,				// 3: timer
	E_DT_VOLTAGE			// 4: analogue voltage
};

enum E_IO_TYPE {
	E_IO_UNUSED = 0,
	E_IO_ON_OFF,			// 1: 	manual on off switch
	E_IO_ON_TIMER,			// 2: 	manual on, auto off by timer
	E_IO_TOGGLE,			// 3:	press on then press off
	E_IO_ON_OFF_TIMER,		// 4: 	manual on, auto off by timer or button
	E_IO_OUTPUT,			// 5:	output
	E_IO_TEMP_HIGH,			// 6:	temperature too high
	E_IO_TEMP_LOW,			// 7:	temperature too low
	E_IO_VOLT_HIGH,			// 8:	voltage too high
	E_IO_VOLT_LOW,			// 9:	voltage too low
	E_IO_TEMP_MONITOR,		// 10:	temperature monitor only
	E_IO_VOLT_MONITOR,		// 11:	voltage monitor only
	E_IO_VOLT_DAYNIGHT,		// 12:	voltage monitor day/night state
};

enum E_EVENT_TYPE {
	E_ET_CLICK,				// 0:	single press
	E_ET_DBLCLICK,			// 1:	double click
	E_ET_LONGPRESS,			// 2:	long press
	E_ET_TIMER,				// 3:	timer event
	E_ET_TEMPERATURE,		// 4:	temperature event
	E_ET_DEVICE_NG,			// 5:	device status is bad
	E_ET_DEVICE_OK,			// 6:	device status is good
	E_ET_VOLTAGE,			// 7:	voltage event
	E_ET_STARTUP,			// 8:	program startup
};

enum E_DEVICE_STATUS {
	E_DS_ALIVE = 0,			// 0: 	alive
	E_DS_SUSPECT,			// 1:	suspect
	E_DS_DEAD,				// 2:	dead, but only just
	E_DS_BURIED,			// 3:	dead for a long time
};

enum E_DAY_NIGHT_STATE {
	E_DN_UNKNOWN = 0,		// 0: unknown
	E_DN_NIGHT,				// 1: night time
	E_DN_DAWNDUSK,			// 2: dawn or dusk
	E_DN_OVERCAST,			// 3: overcast day
	E_DN_DAY,				// 4: day time
};



class CMysql;
class CInOutLinks {
private:
	int m_iLinkNo[MAX_IO_LINKS];
	int m_iInDeviceNo[MAX_IO_LINKS];
	int m_iInChannel[MAX_IO_LINKS];
	int m_iOutDeviceNo[MAX_IO_LINKS];
	int m_iOutChannel[MAX_IO_LINKS];
	enum E_EVENT_TYPE m_eEventType[MAX_IO_LINKS];
	int m_iOnPeriod[MAX_IO_LINKS];
	int m_iLinkDeviceNo[MAX_IO_LINKS];
	int m_iLinkChannel[MAX_IO_LINKS];
	char m_szLinkTest[MAX_IO_LINKS][6];
	double m_dLinkValue[MAX_IO_LINKS];

public:
	CInOutLinks();
	~CInOutLinks();

	void Init();

	const int GetLinkNo( const int idx ){ return m_iLinkNo[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetInDeviceNo( const int idx ){ return m_iInDeviceNo[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetInChannel( const int idx ){ return m_iInChannel[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetOutDeviceNo( const int idx ){ return m_iOutDeviceNo[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetOutChannel( const int idx ){ return m_iOutChannel[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const enum E_EVENT_TYPE GetEventType( const int idx ){ return m_eEventType[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetOnPeriod( const int idx ){ return m_iOnPeriod[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetLinkDeviceNo( const int idx ){ return m_iLinkDeviceNo[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const int GetLinkChannel( const int idx ){ return m_iLinkChannel[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const char* GetLinkTest( const int idx ){ return m_szLinkTest[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };
	const double GetLinkValue( const int idx ){ return m_dLinkValue[(idx >= 0 && idx < MAX_IO_LINKS ? idx : 0)]; };

	void SetLinkNo( const int idx, const int iVal );
	void SetInDeviceNo( const int idx, const int iVal );
	void SetInChannel( const int idx, const int iVal );
	void SetOutDeviceNo( const int idx, const int iVal );
	void SetOutChannel( const int idx, const int iVal );
	void SetEventType( const int idx, const enum E_EVENT_TYPE eVal );
	void SetOnPeriod( const int idx, const int iVal );
	void SetLinkDeviceNo( const int idx, const int iVal );
	void SetLinkChannel( const int idx, const int iVal );
	void SetLinkTest( const int idx, const char* szTest );
	void SetLinkValue( const int idx, const double dVal );

	bool ReadIOLinks( CMysql& myDB );

	bool Find( const int iInAddress, const int iInSwitch, int &idx, int& iOutADdress, int& iOutChannel, int& iOnPeriod, int& iLinkDeviceNo, int& iLinkChannel, char* szLinkTest, double& dLinkValue );
};



class CMyDevice {
private:
	char m_szComPort[MAX_COMPORT_LEN+1];
	char m_szDeviceName[MAX_DEVICE_NAME_LEN+1];
	char m_szDeviceHostname[MAX_HOSTNAME_LEN+1];
	modbus_t* m_pCtx;
	int m_iDeviceNo;
	int m_iAddress;
	int m_iNumInputs;
	int m_iTimeoutCount;
	enum E_DEVICE_STATUS m_eDeviceStatus;
	enum E_DEVICE_TYPE m_eDeviceType;
	bool m_bAlarmTriggered[MAX_IO_PORTS];
	int m_iHysteresis[MAX_IO_PORTS];
	double m_dTemperature[MAX_IO_PORTS];
	time_t m_tLastRecorded[MAX_IO_PORTS];
	uint8_t m_uLastInput[MAX_IO_PORTS];
	uint8_t m_uNewInput[MAX_IO_PORTS];
	uint16_t m_uLastData[MAX_IO_PORTS];
	uint16_t m_uLastLogData[MAX_IO_PORTS];
	uint16_t m_uNewData[MAX_IO_PORTS];
	int m_iNumOutputs;
	uint8_t m_uOutput[MAX_IO_PORTS];
	time_t m_tOutOnStartTime[MAX_IO_PORTS];
	int m_iInputOnPeriod[MAX_IO_PORTS];		// seconds
	int m_iOutputOnPeriod[MAX_IO_PORTS];		// seconds
	int m_iStartTime[MAX_IO_PORTS];
	enum E_IO_TYPE m_eInChannelType[MAX_IO_PORTS];
	enum E_IO_TYPE m_eOutChannelType[MAX_IO_PORTS];
	char m_szInIOName[MAX_IO_PORTS][MAX_DEVICE_NAME_LEN+1];
	char m_szOutIOName[MAX_IO_PORTS][MAX_DEVICE_NAME_LEN+1];
	char m_szInWeekdays[MAX_IO_PORTS][8];
	char m_szOutWeekdays[MAX_IO_PORTS][8];
	char m_cAnalogType[MAX_IO_PORTS];
	double m_dCalcFactor[MAX_IO_PORTS];
	double m_dVoltage[MAX_IO_PORTS];
	double m_dOffset[MAX_IO_PORTS];
	struct timespec m_tEventTime[MAX_IO_PORTS];

public:
	CMyDevice();
	~CMyDevice();

	void Init();
	const bool IsOffTime( const int j );
	const double CalcTemperature( const int iChannel, const bool bNew );
	const double CalcVoltage( const int iChannel, const bool bNew );
	const bool IsSensorConnected( const int iChannel );
	const bool WasSensorConnected( const int iChannel );
	const bool IsTimerEnabledToday( const int iChannel );
	const bool LinkTestPassed( const int iLinkChannel, const char* szLinkTest, const double dLinkValue, bool& bInvertState );
	const bool TestValue( const char* szLinkTest, const double dLinkValue, const double dVal, bool& bInvertState );

	modbus_t* GetContext(){ return m_pCtx; };
	const int GetDeviceNo(){ return m_iDeviceNo; };
	const char* GetComPort(){ return m_szComPort; };
	const char* GetDeviceName(){ return m_szDeviceName; };
	const char* GetDeviceHostname(){ return m_szDeviceHostname; };
	const int GetAddress(){ return m_iAddress; };
	const enum E_DEVICE_TYPE GetDeviceType(){ return m_eDeviceType; };
	const enum E_DEVICE_STATUS GetDeviceStatus(){ return m_eDeviceStatus; };
	const int GetTimeoutCount(){ return m_iTimeoutCount; };
	const int GetNumInputs(){ return m_iNumInputs; };
	const int GetNumOutputs(){ return m_iNumOutputs; };
	uint8_t* GetNewInput(){ return m_uNewInput; };
	uint16_t* GetNewData(){ return m_uNewData; };
	bool& GetAlarmTriggered( const int i ){ return m_bAlarmTriggered[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	int& GetHysteresis( const int i ){ return m_iHysteresis[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	double& GetTriggerTemperature( const int i ){ return m_dTemperature[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	time_t& GetLastRecorded( const int i ){ return m_tLastRecorded[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint8_t& GetLastInput( const int i ){ return m_uLastInput[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint8_t& GetNewInput( const int i ){ return m_uNewInput[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint16_t& GetLastData( const int i ){ return m_uLastData[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint16_t& GetLastLogData( const int i ){ return m_uLastLogData[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint16_t& GetNewData( const int i ){ return m_uNewData[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint8_t& GetOutput( const int i ){ return m_uOutput[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	enum E_IO_TYPE& GetInChannelType( const int i ){ return m_eInChannelType[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	enum E_IO_TYPE& GetOutChannelType( const int i ){ return m_eOutChannelType[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const char* GetInIOName( const int i ){ return m_szInIOName[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const char* GetOutIOName( const int i ){ return m_szOutIOName[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const char* GetInWeekdays( const int i ){ return m_szInWeekdays[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const char* GetOutWeekdays( const int i ){ return m_szOutWeekdays[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const time_t GetOutOnStartTime( const int i ){ return m_tOutOnStartTime[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const int GetInOnPeriod( const int i ){ return m_iInputOnPeriod[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const int GetOutOnPeriod( const int i ){ return m_iOutputOnPeriod[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const int GetStartTime( const int i ){ return m_iStartTime[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const char GetAnalogType( const int i){ return m_cAnalogType[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const double GetCalcFactor( const int i){ return m_dCalcFactor[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	const double GetOffset( const int i ){ return m_dOffset[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	double& GetTriggerVoltage( const int i ){ return m_dVoltage[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	void GetEventTime( const int i, struct timespec& ts ){ ts = m_tEventTime[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };

	void SetContext( modbus_t* pCtx ){ m_pCtx = pCtx; };
	void SetDeviceNo( const int iDeviceNo ){ m_iDeviceNo = iDeviceNo; };
	void SetComPort( const char* szPort );
	void SetDeviceName( const char* szName );
	void SetDeviceHostname( const char* szDeviceHostname );
	void SetAddress( const int iAddr ){ m_iAddress = iAddr; };
	void SetDeviceType( const enum E_DEVICE_TYPE eType ){ m_eDeviceType = eType; };
	void SetDeviceStatus( const enum E_DEVICE_STATUS eStatus );
	void SetTimeoutCount( const int iCount ){ m_iTimeoutCount = iCount; };
	void SetNumInputs( const int iNumInputs ){ m_iNumInputs = iNumInputs; };
	void SetNumOutputs( const int iNumOutputs ){ m_iNumOutputs = iNumOutputs; };
	void SetInIOName( const int i, const char* pszName );
	void SetOutIOName( const int i, const char* pszName );
	void SetInWeekdays( const int i, const char* pszDays );
	void SetOutWeekdays( const int i, const char* pszDays );
	void SetOutOnStartTime( const int i, const time_t tt );
	void SetInOnPeriod( const int i, const int iSec );
	void SetOutOnPeriod( const int i, const int iSec );
	void SetStartTime( const int i, const int iMin );
	void SetAnalogType( const int i, const char cType );
	void SetCalcFactor( const int i, const double dFactor );
	void SetOffset( const int i, const double dOffset );
	void SetEventTime( const int i, const struct timespec ts );
};

class CDeviceList {
private:
	int m_iMcuMessageCount;
	enum E_DAY_NIGHT_STATE m_eDayNightState;
	CMyDevice m_Device[MAX_DEVICES];
	char m_szDummy[2];
	char m_szHostComPort[MAX_DEVICES][MAX_COMPORT_LEN+1];
	char m_szMcuResponseMsg[MAX_MCU_QUEUE][MCU_MSG_SIZE+1];	// TODO: handle multiple messages
	char m_szMcuName[MAX_MCU_QUEUE][MAX_DEVICE_NAME_LEN+1];
	modbus_t* m_pHostCtx[MAX_DEVICES];

public:
	CDeviceList();
	~CDeviceList();

	void Init();
	bool InitContext();
	const int GetMcuMessageCount();
	const bool IsShared( const int idx );
	const bool IsSharedWithNext( const int idx );
	void FreeAllContexts();
	void GetComPortsOnHost( CMysql& myDB, char szPort[MAX_DEVICES][MAX_COMPORT_LEN+1] );
	const int GetTotalComPorts( char szPort[MAX_DEVICES][MAX_COMPORT_LEN+1] );
	const int GetIdxForAddr( const int iAddr );
	const int GetIdxForName( const char* szName );
	const int GetDeviceNoForAddr( const int iAddr );
	const int GetAddressForDeviceNo( const int iDeviceNo );
	const bool WriteOutputBit( const int iDevice, const int iOutCoil, const uint8_t uState );
	void LogOnTime( const int iDevice, const int iOutCoil );
	void CheckForTimerOff();
	const bool IsOffTime( const int idx, const int iOutCoil );
	const double CalcTemperature( const int idx, const int iChannel, const bool bNew );
	const double CalcVoltage( const int idx, const int iChannel, const bool bNew );
	const bool IsSensorConnected( const int idx, const int iChannel );
	const bool WasSensorConnected( const int idx, const int iChannel );
	const bool IsTimerEnabledToday( const int idx, const int iChannel );
	const char* GetEventTypeDesc( const enum E_EVENT_TYPE eType );
	const bool LinkTestPassed( const int iLinkIdx, const int iLinkChannel, const char* szLinkTest, const double dLinkValue, bool& bInvertState );

	modbus_t* GetContext( const int idx );
	const int GetDeviceNo( const int idx );
	const char* GetComPort( const int idx );
	const char* GetDeviceName( const int idx );
	const char* GetDeviceHostname( const int idx );
	void GetMcuResponseMsg( char* szMcuName, size_t uNameLen, char* szMcuResponseMsg, size_t uMsgLen );
	const int GetAddress( const int idx );
	const int GetTimeoutCount( const int idx );
	const enum E_DEVICE_TYPE GetDeviceType( const int idx );
	const enum E_DEVICE_STATUS GetDeviceStatus( const int idx );
	const int GetNumInputs( const int idx );
	const int GetNumOutputs( const int idx );
	uint8_t* GetNewInput( const int idx );
	uint16_t* GetNewData( const int idx );
	bool& GetAlarmTriggered( const int idx, const int j );
	int& GetHysteresis( const int idx, const int j );
	double& GetTriggerTemperature( const int idx, const int j );
	time_t& GetLastRecorded( const int idx, const int j );
	uint8_t& GetLastInput( const int idx, const int j );
	uint8_t& GetNewInput( const int idx, const int j );
	uint16_t& GetLastData( const int idx, const int j );
	uint16_t& GetLastLogData( const int idx, const int j );
	uint16_t& GetNewData( const int idx, const int j );
	uint8_t& GetOutput( const int idx, const int j );
	enum E_IO_TYPE& GetInChannelType( const int idx, const int j );
	enum E_IO_TYPE& GetOutChannelType( const int idx, const int j );
	const char* GetInIOName( const int idx, const int j );
	const char* GetOutIOName( const int idx, const int j );
	const char* GetInWeekdays( const int idx, const int j );
	const char* GetOutWeekdays( const int idx, const int j );
	const time_t GetOutOnStartTime( const int idx, const int j );
	const int GetInOnPeriod( const int idx, const int j );
	const int GetOutOnPeriod( const int idx, const int j );
	const int GetStartTime( const int idx, const int j );
	const char GetAnalogType( const int idx, const int j );
	const double GetCalcFactor( const int idx, const int j );
	const double GetOffset( const int idx, const int j );
	double& GetTriggerVoltage( const int idx, const int j );
	void GetEventTime( const int idx, const int j, struct timespec& ts );
	void GetEventTimeNow( struct timespec& tNow );
	int GetTotalDevices();
	const enum E_DAY_NIGHT_STATE GetDayNightState() { return m_eDayNightState; };
	const double GetDayNightVoltage( const enum E_DAY_NIGHT_STATE eState );
	const char* GetDayNightStateName();
	const bool IsMcuDevice( const int idx );

	void SetContext( const int idx, modbus_t* pCtx );
	void SetDeviceNo( const int idx, const int iDeviceNo );
	void SetComPort( const int idx, const char* szPort );
	void SetDeviceName( const int idx, const char* szName );
	void SetDeviceHostname( const int idx, const char* szHostname );
	void SetMcuResponseMsg( const char* szMcuName, const char* szResponseMsg );
	void SetDeviceType( const int idx, const enum E_DEVICE_TYPE eType );
	bool SetDeviceStatus( const int idx, const enum E_DEVICE_STATUS eStatus );
	void SetAddress( const int idx, const int iAddr );
	void SetNumInputs( const int idx, const int iNum );
	void SetNumOutputs( const int idx, const int iNum );
	void SetInIOName( const int idx, const int j, const char* pszName );
	void SetOutIOName( const int idx, const int j, const char* pszName );
	void SetInWeekdays( const int idx, const int j, const char* pszDays );
	void SetOutWeekdays( const int idx, const int j, const char* pszDays );
	void SetOutOnStartTime( const int idx, const int j, const time_t tt );
	void SetInOnPeriod( const int idx, const int j, const int iSec );
	void SetOutOnPeriod( const int idx, const int j, const int iSec );
	void SetStartTime( const int idx, const int j, const int iMin );
	void SetAnalogType( const int idx, const int j, const char cType );
	void SetCalcFactor( const int idx, const int j, const double dFactor );
	void SetOffset( const int idx, const int j, const double dOffset );
	void SetEventTime( const int idx, const int j, const struct timespec ts );
	void SetDayNightState( const double dVoltage );
	const long GetEventTimeDiff( const int idx, const int i, const struct timespec tNow );

	bool UpdateDeviceStatus( CMysql& myDB, const int idx );
	bool UpdateDeviceOutOnStartTime( CMysql& myDB, const int iOutIdx, const int iOutChannel );
	bool UpdateDBDeviceComPort( CMysql& myDB, const int idx );
	bool ReadDeviceConfig( CMysql& myDB );
	bool IsDeviceAlive( modbus_t* ctx, const char* szHostComPort, const int iAddr );
	void UpdateDeviceComPort( CMysql& myDB, const char* szNewComPort, const char* szOldComPort, char szPortList[MAX_DEVICES][MAX_COMPORT_LEN+1] );
};



#endif

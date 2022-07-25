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
#define MAX_CONDITIONS				10		// per each iolink
#define ESP_MSG_SIZE				22
#define MAX_ESP_QUEUE				4
#define MAX_DATA_BUFFER				10
#define MAX_TEMPERATURE_DIFF		5.0		// max temperature value change between readings
#define MAX_PIN_FAILURES			3


enum E_DEVICE_TYPE {
	E_DT_UNUSED = 0,
	E_DT_DIGITAL_IO,		// 1: digital input and/or output
	E_DT_TEMPERATURE_DS,	// 2: temperature DS18B20 devices
	E_DT_TIMER,				// 3: timer
	E_DT_VOLTAGE,			// 4: analogue voltage
	E_DT_TEMPERATURE_K1,	// 5: temperature, PD3064 K thermocouple
	E_DT_LEVEL_K02,			// 6: ultrasonic level measurement type K02
	E_DT_LEVEL_HDL,			// 7: water level sensor type HDL300
	E_DT_ROTARY_ENC_12BIT,	// 8: rotary encoder 12 bit
	E_DT_VIPF_MON,			// 9: PZEM-016 VIPF Monitor
	E_DT_CARD_READER,		// 10: HID card reader with pin pad
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
	E_IO_TEMP_HIGHLOW,		// 13:	temperature too high or too low
	E_IO_VOLT_HIGHLOW,		// 14:	voltage too high or too low
	E_IO_LEVEL_MONITOR,		// 15:	level measurement K02
	E_IO_LEVEL_HIGH,		// 16:	level too high
	E_IO_LEVEL_LOW,			// 17:	level too low
	E_IO_LEVEL_HIGHLOW,		// 18:	level too high or too low
	E_IO_ROTENC_MONITOR,	// 19:	rotary encoder measurement
	E_IO_ROTENC_HIGH,		// 20:	rotary encoder too high
	E_IO_ROTENC_LOW,		// 21:	rotary encoder too low
	E_IO_ROTENC_HIGHLOW,	// 22:	rotary too high or too low
	E_IO_CURRENT_MONITOR,	// 23:	current monitor only
	E_IO_CURRENT_HIGH,		// 24:	current too high
	E_IO_CURRENT_LOW,		// 25:	current too low
	E_IO_CURRENT_HIGHLOW,	// 26:	current too high or too low
	E_IO_FREQ_MONITOR,		// 27:	frequency monitor only
	E_IO_FREQ_HIGH,			// 28:	frequency too high
	E_IO_FREQ_LOW,			// 29:	frequency too low
	E_IO_FREQ_HIGHLOW,		// 30:	frequency too high or too low
	E_IO_PWRFACT_MONITOR,	// 31:	power factor monitor only
	E_IO_POWER_MONITOR,		// 32:	power monitor only
	E_IO_POWER_HIGH,		// 33:	power too high
	E_IO_POWER_LOW,			// 34:	power too low
	E_IO_POWER_HIGHLOW,		// 35:	power too high or too low
	E_IO_ON_OFF_INV,		// 36:	manual on off switch, inverted levels
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
	E_ET_LEVEL,				// 9:	level event
	E_ET_ROTARY_ENC,		// 10:	rotary encode
	E_ET_CURRENT,			// 11:	current
	E_ET_FREQUENCY,			// 12:	frequency
	E_ET_POWERFACTOR,		// 13:	power factor
	E_ET_POWER,				// 14:	power
	E_ET_CARDREADER,		// 15: 	card reader
	E_ET_PLCEVENT,			// 16:	plc events
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


/* Ultrasonic distance measurment module: AJ-SR04M ($5 from AliExpress)
 * - Output is TTL level RS-232.
 * - The module runs on 5VDC and can be powered directly from a USB/RS232 TTL converter
 * - USB/RS232 converter wiring. Red = 5V, Black = 0V, White = Rx (wire the module Tx/Echo pin), Green = Tx (wire to module Rx/Trg pin)
 * - Nimrod software is written to use mode 5 (R19 = 0 or short circuit) ascii data
 *
 * K02 example
 * module has 5modes:
 *
 * mode1 R19 not instaled: module wil send pulse on echo line after at least 10us pulse on trigger line (tested, works with external pulldown - 4k7)
 *
 * mode2 R19 = 300k: module will send pulse on echo line after at least 10ms pulse on trigger line (tested, works with external pulldown - 4k7)
 *
 * mode3 R19 = 120k(100k works): module will send serial data at 9600 each 100ms
 * data format:
 * 0xFF
 * upper 8bit
 * lower 8bit
 * check sum = ((upper+lower)&0xff)
 *
 * mode3 R19 = 47k: module will send serial data at 9600  after receivind any data on RX line
 * data format:
 * 0xFF
 * upper 8bit
 * lower 8bit
 * check sum = ((upper+lower)&0xff)
 *
 * mode5 R19 = 0: module will send continuesly asci data (seems to only send when it receives something, not continuously):
 * Gap=1234mm<CRLF>
 */



class CMysql;
class CDeviceList;
class CInOutLinks {
private:
	int m_iLinkNo[MAX_IO_LINKS];
	int m_iInDeviceNo[MAX_IO_LINKS];
	int m_iInChannel[MAX_IO_LINKS];
	int m_iOutDeviceNo[MAX_IO_LINKS];
	int m_iOutChannel[MAX_IO_LINKS];
	enum E_EVENT_TYPE m_eEventType[MAX_IO_LINKS];
	int m_iOnPeriod[MAX_IO_LINKS];
	int m_iLinkDeviceNo[MAX_IO_LINKS][MAX_CONDITIONS];
	int m_iLinkChannel[MAX_IO_LINKS][MAX_CONDITIONS];
	char m_szLinkTest[MAX_IO_LINKS][MAX_CONDITIONS][6];
	double m_dLinkValue[MAX_IO_LINKS][MAX_CONDITIONS];

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
	const int GetLinkDeviceNo( const int idx, const int cn );
	const int GetLinkChannel( const int idx, const int cn );
	const char* GetLinkTest( const int idx, const int cn );
	const double GetLinkValue( const int idx, const int cn );

	void SetLinkNo( const int idx, const int iVal );
	void SetInDeviceNo( const int idx, const int iVal );
	void SetInChannel( const int idx, const int iVal );
	void SetOutDeviceNo( const int idx, const int iVal );
	void SetOutChannel( const int idx, const int iVal );
	void SetEventType( const int idx, const enum E_EVENT_TYPE eVal );
	void SetOnPeriod( const int idx, const int iVal );
	void SetLinkDeviceNo( const int idx, const int cn, const int iVal );
	void SetLinkChannel( const int idx, const int cn, const int iVal );
	void SetLinkTest( const int idx, const int cn, const char* szTest );
	void SetLinkValue( const int idx, const int cn, const double dVal );

	bool ReadIOLinks( CMysql& myDB );

	const int FindEmptyConditionSlot( const int idx );
	bool Find( const int iInAddress, const int iInSwitch, int &idx, int& iOutADdress, int& iOutChannel, int& iOnPeriod, bool& bLinkTestPassed, bool& bInvertState, CDeviceList* m_pmyDevices );
};



class CMyDevice {
private:
	char m_szComPort[MAX_COMPORT_LEN+1];
	char m_szDeviceName[MAX_DEVICE_NAME_LEN+1];
	char m_szDeviceHostname[MAX_HOSTNAME_LEN+1];
	int m_iBaudRate;
	int m_iComHandle;
	modbus_t* m_pCtx;
	int m_iDeviceNo;
	int m_iAddress;
	int m_iNumInputs;
	int m_iTimeoutCount;
	enum E_DEVICE_STATUS m_eDeviceStatus;
	enum E_DEVICE_TYPE m_eDeviceType;
	bool m_bAlarmTriggered[MAX_IO_PORTS];
	int m_iHysteresis[MAX_IO_PORTS];
	double m_dMonitorHi[MAX_IO_PORTS];
	double m_dMonitorLo[MAX_IO_PORTS];
	time_t m_tLastRecorded[MAX_IO_PORTS];
	uint8_t m_uLastInput[MAX_IO_PORTS];
	uint8_t m_uNewInput[MAX_IO_PORTS];
	uint16_t m_uLastData[MAX_IO_PORTS];
	uint16_t m_uLastLogData[MAX_IO_PORTS];
	uint16_t m_uNewData[MAX_IO_PORTS];
	uint16_t m_uDataBuffer[MAX_IO_PORTS][MAX_DATA_BUFFER];
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
	double m_dOffset[MAX_IO_PORTS];
	char m_szMonitorPos[MAX_IO_PORTS][15+1];	// max of 5 sets of 3 chars
	struct timespec m_tEventTime[MAX_IO_PORTS];

public:
	CMyDevice();
	~CMyDevice();

	void Init();
	const bool IsOffTime( const int j );
	const double CalculateTemperature( const uint16_t uVal );
	const double CalcTemperature( const int iChannel, const bool bNew );
	const double CalcVoltage( const int iChannel, const bool bNew );
	const double CalcLevel( const int iChannel, const bool bNew );
	const double CalcRotaryEncoderDistance( const int iChannel, const bool bNew );
	const double CalcVIPFValue( const int iChannel, const bool bNew );
	const bool IsSensorConnected( const int iChannel );
	const bool WasSensorConnected( const int iChannel );
	const bool IsTimerEnabledToday( const int iChannel );
	const bool LinkTestPassed( const int iLinkChannel, const char* szLinkTest, const double dLinkValue, bool& bInvertState );
	const bool TestValue( const char* szLinkTest, const double dLinkValue, const double dVal, bool& bInvertState );

	const int GetComHandle(){ return m_iComHandle; };
	modbus_t* GetContext(){ return m_pCtx; };
	const int GetDeviceNo(){ return m_iDeviceNo; };
	const int GetBaudRate(){ return m_iBaudRate; };
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
	double& GetMonitorValueLo( const int i ){ return m_dMonitorLo[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	double& GetMonitorValueHi( const int i ){ return m_dMonitorHi[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	time_t& GetLastRecorded( const int i ){ return m_tLastRecorded[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint8_t& GetLastInput( const int i ){ return m_uLastInput[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint8_t& GetNewInput( const int i ){ return m_uNewInput[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint16_t& GetLastData( const int i ){ return m_uLastData[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint16_t& GetLastLogData( const int i ){ return m_uLastLogData[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint16_t& GetNewData( const int i ){ return m_uNewData[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	uint8_t& GetOutput( const int i ){ return m_uOutput[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	void SaveDataValue( const int i );
	void ClearDataBuffer( const int i );
	bool DataBufferIsStable( const int j );
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
	const char* GetMonitorPos( const int i ){ return m_szMonitorPos[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };
	void GetEventTime( const int i, struct timespec& ts ){ ts = m_tEventTime[(i >= 0 && i < MAX_IO_PORTS ? i : 0)]; };

	void SetComHandle( int iHandle ){ m_iComHandle = iHandle; };
	void SetContext( modbus_t* pCtx ){ m_pCtx = pCtx; };
	void SetDeviceNo( const int iDeviceNo ){ m_iDeviceNo = iDeviceNo; };
	void SetBaudRate( const int iBaudRate ){ m_iBaudRate = iBaudRate; };
	void SetComPort( const char* szPort );
	void SetDeviceName( const char* szName );
	void SetDeviceHostname( const char* szDeviceHostname );
	void SetAddress( const int iAddr ){ m_iAddress = iAddr; };
	void SetDeviceType( const enum E_DEVICE_TYPE eType ){ m_eDeviceType = eType; };
	void SetDeviceStatus( const enum E_DEVICE_STATUS eStatus, const bool bNoTimeout );
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
	void SetMonitorPos( const int i, const char* szMonPos );
	void SetEventTime( const int i, const struct timespec ts );
};

class CDeviceList {
private:
	int m_iEspMessageCount;
	bool m_bHostComPortModbus[MAX_DEVICES];
	enum E_DAY_NIGHT_STATE m_eDayNightState;
	CMyDevice m_Device[MAX_DEVICES];
	char m_szDummy[2];
	char m_szHostComPort[MAX_DEVICES][MAX_COMPORT_LEN+1];
	char m_szEspResponseMsg[MAX_ESP_QUEUE][ESP_MSG_SIZE+1];	// TODO: handle multiple messages
	char m_szEspName[MAX_ESP_QUEUE][MAX_DEVICE_NAME_LEN+1];
	modbus_t* m_pHostCtx[MAX_DEVICES];

public:
	CDeviceList();
	~CDeviceList();

	void Init();
	bool InitContext();
	const int GetEspMessageCount();
	const bool IsShared( const int idx );
	const bool IsSharedWithNext( const int idx );
	void FreeAllContexts();
	const int GetBaudRateForPort( const char* szComPort );
	const bool GetComPortsOnHost( CMysql& myDB, char szPort[MAX_DEVICES][MAX_COMPORT_LEN+1], bool bSwapBaud );
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
	const double CalcLevel( const int idx, const int iChannel, const bool bNew );
	const double CalcRotaryEncoderDistance( const int idx, const int iChannel, const bool bNew );
	const double CalcVIPFValue( const int idx, const int iChannel, const bool bNew );
	const bool IsSensorConnected( const int idx, const int iChannel );
	const bool WasSensorConnected( const int idx, const int iChannel );
	const bool IsTimerEnabledToday( const int idx, const int iChannel );
	const char* GetEventTypeDesc( const enum E_EVENT_TYPE eType );
	const bool LinkTestPassed( const int iLinkIdx, const int iLinkChannel, const char* szLinkTest, const double dLinkValue, bool& bInvertState );

	int GetComHandle( const int idx );
	modbus_t* GetContext( const int idx );
	const int GetDeviceNo( const int idx );
	const int GetBaudRate( const int idx );
	const char* GetComPort( const int idx );
	const char* GetDeviceName( const int idx );
	const char* GetDeviceHostname( const int idx );
	void GetEspResponseMsg( char* szEspName, size_t uNameLen, char* szEspResponseMsg, size_t uMsgLen );
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
	double& GetMonitorValueLo( const int idx, const int j );
	double& GetMonitorValueHi( const int idx, const int j );
	time_t& GetLastRecorded( const int idx, const int j );
	uint8_t& GetLastInput( const int idx, const int j );
	uint8_t& GetNewInput( const int idx, const int j );
	uint16_t& GetLastData( const int idx, const int j );
	uint16_t& GetLastLogData( const int idx, const int j );
	uint16_t& GetNewData( const int idx, const int j );
	void SaveDataValue( const int idx, const int j );
	bool DataBufferIsStable( const int idx, const int j );
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
	const char* GetMonitorPos( const int idx, const int j );
	void GetEventTime( const int idx, const int j, struct timespec& ts );
	void GetEventTimeNow( struct timespec& tNow );
	int GetTotalDevices();
	const enum E_DAY_NIGHT_STATE GetDayNightState() { return m_eDayNightState; };
	const double GetDayNightVoltage( const enum E_DAY_NIGHT_STATE eState );
	const char* GetDayNightStateName();
	const bool IsEspDevice( const int idx );

	void SetContext( const int idx, modbus_t* pCtx );
	void SetDeviceNo( const int idx, const int iDeviceNo );
	void SetBaudRate( const int idx, const int iBaudRate );
	void SetComPort( const int idx, const char* szPort );
	void SetDeviceName( const int idx, const char* szName );
	void SetDeviceHostname( const int idx, const char* szHostname );
	void SetEspResponseMsg( const char* szEspName, const char* szResponseMsg );
	void SetDeviceType( const int idx, const enum E_DEVICE_TYPE eType );
	bool SetDeviceStatus( const int idx, const enum E_DEVICE_STATUS eStatus, const bool bNoTimeout = false );
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
	void SetMonitorPos( const int idx, const int j, const char* szMonPos );
	void SetEventTime( const int idx, const int j, const struct timespec ts );
	void SetDayNightState( const double dVoltage );
	const long GetEventTimeDiff( const int idx, const int i, const struct timespec tNow );

	bool UpdatePinFailCount( CMysql& myDB, const char* szCardNumber, const int iPinFailCount );
	bool SelectCardNumber( CMysql& myDB, const char* szCardNumber, char* szCardPin, const int iLen, bool& bEnabled, int& iPinFailCount );
	bool UpdateDeviceStatus( CMysql& myDB, const int idx );
	bool UpdateDeviceOutOnStartTime( CMysql& myDB, const int iOutIdx, const int iOutChannel );
	bool UpdateDBDeviceComPort( CMysql& myDB, const int idx );
	bool ReadDeviceConfig( CMysql& myDB );
	bool IsDeviceAlive( modbus_t* ctx, const char* szHostComPort, const int iAddr );
	void UpdateDeviceComPort( CMysql& myDB, const char* szNewComPort, const char* szOldComPort, char szPortList[MAX_DEVICES][MAX_COMPORT_LEN+1] );
	void ReadSimpleComPort( const char* szPort, char* szSimplePort, size_t uLen );
};



#endif

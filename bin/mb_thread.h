#ifndef _INC_MB_THREAD_H
#define _INC_MB_THREAD_H

#include <stdint.h>
#include <modbus/modbus.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <libwebsockets.h>

#include "mb_socket.h"


#define TEMPERATURE_CHECK_PERIOD			5		// seconds
#define VOLTAGE_CHECK_PERIOD				2		// seconds
#define LEVEL_CHECK_PERIOD					5		// seconds
#define MAX_TCPIP_SOCKETS					16
#define ESP_PING_TIMEOUT					90
#define CAMERA_SNAPSHOT_PERIOD				60	// seconds



enum E_LOCK_TYPES {
	E_LT_LOGGING = 0,
	E_LT_MODBUS,
	E_LT_NODEESP,
	E_LT_WEBSOCKET,
	E_LT_MAX_LOCKS
};
extern pthread_mutex_t mutexLock[E_LT_MAX_LOCKS];

enum E_THREAD_TYPE {
	E_TT_UNKNOWN = 0,
	E_TT_TCPIP,
	E_TT_TIMER,
	E_TT_WEBSOCKET,
	E_TT_COMPORT
};




class CPlcState {
private:
	int m_iStateNo;
	char m_szOperation[51];
	char m_szStateName[51];
	char m_szStateIsActive[2];
	time_t m_tStateTimestamp;
	char m_szRuleType[2];
	int m_iDeviceNo;
	int m_iIOChannel;
	int m_iValue;
	char m_szTest[6];
	char m_szNextStateName[51];
	int m_iOrder;
	int m_iDelayTime;

public:
	CPlcState();
	~CPlcState();
	void Init();

	const int GetStateNo(){ return m_iStateNo; }
	const char* GetOperation(){ return m_szOperation; }
	const char* GetStateName(){ return m_szStateName; }
	const bool GetStateIsActive(){ return (m_szStateIsActive[0] == 'Y' ? true : false); }
	const time_t GetStateTimestamp(){ return m_tStateTimestamp; }
	const char* GetRuleType(){ return m_szRuleType; }
	const int GetDeviceNo(){ return m_iDeviceNo; }
	const int GetIOChannel(){ return m_iIOChannel; }
	const int GetValue(){ return m_iValue; }
	const char* GetTest(){ return m_szTest; }
	const char* GetNextStateName(){ return m_szNextStateName; }
	const int GetOrder(){ return m_iOrder; }
	const int GetDelayTime(){ return m_iDelayTime; }

	void SetStateNo( const int iStateNo );
	void SetOperation( const char* szOperation );
	void SetStateName( const char* szStateName );
	void SetStateIsActive( const char* szActive );
	void SetStateTimestamp( const time_t ts );
	void SetRuleType( const char* szRuleType );
	void SetDeviceNo( const int iDeviceNo );
	void SetIOChannel( const int iIOChannel );
	void SetValue( const int iValue );
	void SetTest( const char* szTest );
	void SetNextStateName( const char* szNextStateName );
	void SetOrder( const int iOrder );
	void SetDelayTime( const int iDelayTime );
};

#define MAX_PLC_STATES				500
#define MAX_PLC_INPUT_EVENTS		20
class CPlcStates {
private:
	int m_iStateCount;
	int m_iActiveStateIdx;
	int m_iInputDeviceNo[MAX_PLC_INPUT_EVENTS];
	int m_iInputIOChannel[MAX_PLC_INPUT_EVENTS];
	CPlcState m_State[MAX_PLC_STATES];

public:
	CPlcStates();
	~CPlcStates();
	void Init();
	const int GetStateCount();
	CPlcState& GetState( const int i );
	CPlcState& GetActiveState(){ return GetState( m_iActiveStateIdx ); }
	void AddState();
	const bool IsActive();
	void SetActiveStateIdx( int idx );
	const int FindStateNo( int iStateNo );
	const int GetInitialAction( const int iStartIdx );
	const int GetActiveStateIdx();
	const bool FindInputDevice( const int iDeviceNo, const int iIOChannel );
	void AddInputEvent( const int iDeviceNo, const int iIOChannel );
	const int ReadInputEvent( int& iDeviceNo, int& iIOChannel );
	void SetNextStateActive( const int idx, const time_t tTimenow );
};



class CCamera {
private:
	int m_iCameraNo;
	char m_szName[51];
	char m_szIPAddress[16];
	char m_szPTZ[2];
	char m_szEncoding[11];
	char m_szDirectory[251];
	char m_szUserId[21];
	char m_szPassword[51];
	char m_szModel[21];
	char m_szMJpeg[2];

public:
	CCamera();
	~CCamera();

	void Init();

	void SetCameraNo( const int iCameraNo );
	void SetName( const char* szName );
	void SetIPAddress( const char* szIPAddress );
	void SetPTZ( const char* szPTZ );
	void SetEncoding( const char* szEncoding );
	void SetDirectory( const char* szDirectory );
	void SetUserId( const char* szUserId );
	void SetPassword( const char* szPassword );
	void SetModel( const char* szModel );
	void SetMJpeg( const char* szMJpeg );

	const int GetCameraNo();
	const char* GetName();
	const char* GetIPAddress();
	const char* GetPTZ();
	const char* GetEncoding();
	const char* GetDirectory();
	const char* GetUserId();
	const char* GetPassword();
	const char* GetModel();
	const char* GetMJpeg();
};

#define MAX_CAMERAS		20
class CCameraList {
private:
	int m_iNumCameras;
	int m_iSnapshotIdx;
	CCamera m_List[MAX_CAMERAS];

public:
	CCameraList();
	~CCameraList();

	void Init();
	void AddCamera();
	void NextSnapshotIdx();
	const int GetSnapshotIdx();
	const int GetNumCameras();
	CCamera& GetCamera( const int idx );
	CCamera& GetSnapshotCamera();
};

class CThread {
private:
	bool* m_pbThreadRunning;
	bool* m_pbAllDevicesDead;
	enum E_THREAD_TYPE m_eThreadType;
	int m_iLevelMessage;
	int m_iClientCount;
	int m_iClientFd[MAX_TCPIP_SOCKETS];
	int m_Sockfd;
	char m_szComBuffer[100];
	char m_szMyComPort[MAX_COMPORT_LEN+1];
	char m_szEspResponseMsg[ESP_MSG_SIZE+1];
	char m_szClientEspName[MAX_TCPIP_SOCKETS][MAX_DEVICE_NAME_LEN+1];
	time_t m_tClientLastMsg[MAX_TCPIP_SOCKETS];
	time_t m_tConfigTime;
	time_t m_tPlcStatesTime;
	time_t m_tLastConfigCheck;
	time_t m_tLastPlcStatesCheck;
	time_t m_tLastCameraSnapshot;
	struct in_addr m_xClientInAddr[MAX_TCPIP_SOCKETS];
	SSL* m_xClientSSL[MAX_TCPIP_SOCKETS];
	CDeviceList* m_pmyDevices;
	CInOutLinks* m_pmyIOLinks;
	CPlcStates* m_pmyPlcStates;
	CCameraList m_CameraList;
	SSL_CTX *m_sslServerCtx;
	SSL_CTX *m_sslClientCtx;
	struct lws_context *m_WSContext;

public:
	CThread( const char* szPort, CDeviceList* pmyDevices, CInOutLinks* pmyIOLinks, CPlcStates* pmyPlcStates, const enum E_THREAD_TYPE eThreadType, bool* pbThreadRunning, bool* pbAllDevicesDead );
	~CThread();

	void Worker();
	void ChangeOutput( CMysql& myDB, const int iInAddress, const int iInChannel, const uint8_t iState, const enum E_EVENT_TYPE eEvent );
	void ChangeOutputState( CMysql& myDB, const int iInIdx, const int iInAddress, const int iInChannel, const int iOutIdx, const int iOutAddress, const int iOutChannel, const uint8_t uState,
			const enum E_IO_TYPE eSwType, int iOutOnPeriod );
	const bool ClickFileExists( CMysql& myDB, const int iDeviceNo, const int iChannel );
	const bool IsComPortThread();
	const bool IsTimerThread();
	const bool IsTcpipThread();
	const bool IsWebsocketThread();
	const bool IsMyComPort( const char* szPort );
	SSL_CTX* InitServerCTX(void);
	SSL_CTX* InitClientCTX(void);
	void LoadCertificates( SSL_CTX* ctx, const char* szCertFile, const char* szKeyFile );
	void ShowCerts( SSL* ssl );
	void LogSSLError();
	void CreateListenerSocket();
	void AcceptTcpipClient();
	void CloseSocket( int& fd, const int fdx );
	void SendEspMessage();
	void ReadTcpipMessage( CDeviceList* pmyDevices, CMysql& myDB );
	size_t ReadTcpipMsgBytes( SSL* ssl, const int newfd, NIMROD_MSGBUF_TYPE& msgBuf, const bool bBlock );
	bool SendTcpipChangeOutputToHost( const char* szHostname, const int iInIdx, const int iInAddress, const int iInChannel, const int iOutIdx, const int iOutAddress, const int iOutChannel,
			const uint8_t uState, const enum E_IO_TYPE eSwType, const int iOutOnPeriod );
	void HandleTemperatureDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleSwitchDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleVoltageDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleHdlLevelDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleOutputDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void ProcessEspSwitchEvent( CMysql& myDB, const char* szName, const int iButton );
	void CheckForTimerOffTime( CMysql& myDB, const int idx );
	void HandleLevelDevice( CMysql& myDB, const int idx, const bool bSendPrompt, bool& bAllDead );
	void GetCameraSnapshots( CMysql& myDB, CCameraList& CameraList );
	void ReadCameraRecords( CMysql& myDB, CCameraList& CameraList );
	void ReadPlcStatesTable( CMysql& myDB, CPlcStates* pmyPlcStates );
	void ProcessPlcStates( CMysql& myDB, CPlcStates* pmyPlcStates );

	void websocket_init();
	void websocket_process( const char* szMsg );
	void websocket_destroy();

	static const char* GetThreadType( const enum E_THREAD_TYPE eTT );
	static const char* GetTcpipMsgType( const enum E_MESSAGE_TYPE eMT );
};



/*
Voltage device (0-30V), reference voltage = 0.256V

Register
Addr		Description					Data Type		Units
0x20		Ch1 analog input value		IEEE-754
0x22		Ch2 analog input value		floating-point	mA/V
...			...							format
0x3e		Ch16 analog input value

0x40		Ch1 digital output			uint32_t
0x42		Ch2 digital output			MSB=1=negative
...
0x5e		Ch16 digital output

0x60		CH1 analog input value		int16_t			uA/mV		25432 = 25.432V
0x62		Ch2 analog input			-32768 to 32767
...
0x7e		Ch16 analog input

*/

#endif

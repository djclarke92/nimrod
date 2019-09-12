#ifndef _INC_MB_THREAD_H
#define _INC_MB_THREAD_H

#include <stdint.h>
#include <modbus/modbus.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>

#include "mb_socket.h"


#define TEMPERATURE_CHECK_PERIOD			5		// seconds
#define VOLTAGE_CHECK_PERIOD				2		// seconds
#define MAX_TCPIP_SOCKETS					16
#define MCU_PING_TIMEOUT					90



enum E_LOCK_TYPES {
	E_LT_LOGGING = 0,
	E_LT_MODBUS,
	E_LT_NODEMCU,
	E_LT_MAX_LOCKS
};
extern pthread_mutex_t mutexLock[E_LT_MAX_LOCKS];

enum E_THREAD_TYPE {
	E_TT_UNKNOWN = 0,
	E_TT_TCPIP,
	E_TT_TIMER,
	E_TT_COMPORT
};



class CThread {
private:
	bool* m_pbThreadRunning;
	enum E_THREAD_TYPE m_eThreadType;
	int m_iClientCount;
	int m_iClientFd[MAX_TCPIP_SOCKETS];
	int m_Sockfd;
	char m_szMyComPort[50];
	char m_szMcuResponseMsg[MCU_MSG_SIZE+1];
	char m_szClientMcuName[MAX_TCPIP_SOCKETS][MAX_DEVICE_NAME_LEN+1];
	time_t m_tClientLastMsg[MAX_TCPIP_SOCKETS];
	time_t m_tConfigTime;
	time_t m_tLastConfigCheck;
	struct in_addr m_xClientInAddr[MAX_TCPIP_SOCKETS];
	SSL* m_xClientSSL[MAX_TCPIP_SOCKETS];
	CDeviceList* m_pmyDevices;
	CInOutLinks* m_pmyIOLinks;
	SSL_CTX *m_sslServerCtx;
	SSL_CTX *m_sslClientCtx;

public:
	CThread( const char* szPort, CDeviceList* pmyDevices, CInOutLinks* pmyIOLinks, const enum E_THREAD_TYPE eThreadType, bool* pbThreadRunning );
	~CThread();

	void Worker();
	void ChangeOutput( CMysql& myDB, const int iInAddress, const int iInChannel, const uint8_t iState, const enum E_EVENT_TYPE eEvent );
	void ChangeOutputState( CMysql& myDB, const int iInIdx, const int iInAddress, const int iInChannel, const int iOutIdx, const int iOutAddress, const int iOutChannel, const uint8_t uState,
			const enum E_IO_TYPE eSwType, int iOutOnPeriod );
	const bool ClickFileExists( const int iDeviceNo, const int iChannel );
	const bool IsComPortThread();
	const bool IsTimerThread();
	const bool IsTcpipThread();
	const bool IsMyComPort( const char* szPort );
	SSL_CTX* InitServerCTX(void);
	SSL_CTX* InitClientCTX(void);
	void LoadCertificates( SSL_CTX* ctx, const char* CertFile, const char* KeyFile );
	void ShowCerts( SSL* ssl );
	void LogSSLError();
	void CreateListenerSocket();
	void AcceptTcpipClient();
	void CloseSocket( int& fd, const int fdx );
	void SendMcuMessage();
	void ReadTcpipMessage( CDeviceList* pmyDevices, CMysql& myDB );
	size_t ReadTcpipMsgBytes( SSL* ssl, const int newfd, NIMROD_MSGBUF_TYPE& msgBuf, const bool bBlock );
	bool SendTcpipChangeOutputToHost( const char* szHostname, const int iInIdx, const int iInAddress, const int iInChannel, const int iOutIdx, const int iOutAddress, const int iOutChannel,
			const uint8_t uState, const enum E_IO_TYPE eSwType, const int iOutOnPeriod );
	void HandleTemperatureDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleSwitchDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleVoltageDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void HandleOutputDevice( CMysql& myDB, modbus_t* ctx, const int idx, bool& bAllDead );
	void ProcessMcuSwitchEvent( CMysql& myDB, const char* szName, const int iButton );
	void CheckForTimerOffTime( CMysql& myDB, const int idx );

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

#ifndef _INC_MB_SOCKET_H
#define _INC_MB_SOCKET_H


#define NIMROD_PORT							54011


enum E_MESSAGE_TYPE {
	E_MT_UNKNOWN = 0,
	E_MT_HELLO,
	E_MT_ACK,
	E_MT_TOTAL_DEVICES,
	E_MT_DEVICE_INFO,
	E_MT_CHANGE_OUTPUT,
	E_MT_MCU_MSG,
	E_MT_MAX_MESSAGE
};

typedef struct {
	int iTotalDevices;
} MSG_TOTAL_DEVICES_TYPE;

typedef struct {
	enum E_DEVICE_STATUS m_eDeviceStatus;
	enum E_DEVICE_TYPE m_eDeviceType;
	int iAddress;
	int iNumInputs;
	int iNumOutputs;
	char szName[MAX_DEVICE_NAME_LEN];
} MSG_DEVICE_INFO_TYPE;

typedef struct {
	int iInIdx;
	int iInAddress;
	int iInChannel;
	int iOutIdx;
	int iOutAddress;
	int iOutChannel;
	uint8_t uState;
	enum E_IO_TYPE eSwType;
	int iOutOnPeriod;
} MSG_CHANGE_OUTPUT;


// char(20) msg data
// 01234567890123456789
// MCUxxxCLKx				MCUxxx is the device name
//							CLK1 is which button was pressed, 1-4
// OKyxxxxx					Ack returned to device where y is the output to activate 1-4
//							and xxxxxx is the on period in seconds
// PG000000					ping
// MCUxxxCIDyyyyyyy			startup message with chip id if xxx == 000 then we reply with a new unique device name, e.g. MCU002
// NNMCUyyy					new name for device to use
typedef struct {
	char szBuf[MCU_MSG_SIZE];
	char szMcuName[7];
	char szEvent[4];
	int iButton;
	long lChipId;
} MSG_NODEMCU_TYPE;

typedef struct {
	enum E_MESSAGE_TYPE eMsgType;
	int iSegCount;
	int iSegTotal;
	int iAddress;
	union {
		MSG_TOTAL_DEVICES_TYPE totalDevices;
		MSG_DEVICE_INFO_TYPE listDevices;
		MSG_CHANGE_OUTPUT changeOutput;
		MSG_NODEMCU_TYPE mcu;
	};
} NIMROD_MSG_TYPE;

typedef struct {
	union {
		unsigned char szBuf[512];
		NIMROD_MSG_TYPE msg;
	};
} NIMROD_MSGBUF_TYPE;




#endif

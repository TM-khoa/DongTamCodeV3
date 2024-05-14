/*
 * Protocol.h
 *
 *  Created on: May 11, 2024
 *      Author: SpiritBoi
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "main.h"
#include <CRC16.h>
#include <string>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

typedef enum ProtocolListID {
	PROTOCOL_ID_HANDSHAKE,
	PROTOCOL_ID_VALVE,
	PROTOCOL_ID_PULSE_TIME,
	PROTOCOL_ID_TOTAL_VALVE,
	PROTOCOL_ID_CYCLE_INTERVAL_TIME,
	PROTOCOL_ID_INTERVAL_TIME,
	PROTOCOL_ID_TRIGGER_VALVE,
	PROTOCOL_ID_RTC_TIME,
	PROTOCOL_ID_PRESSURE,
	PROTOCOL_ID_ERROR,
	PROTOCOL_ID_END,
} ProtocolListID;

typedef enum GetSetFlag {
	SET_DATA_TO_THIS_DEVICE,
	GET_DATA_FROM_THIS_DEVICE,
} GetSetFlag;

typedef enum ProtocolErrorCode {
	PROTOCOL_ERR_OK,
	PROTOCOL_ERR_BOARD_NOT_FOUND,
	PROTOCOL_ERR_BOARD_FEATURE_NOT_SUPPORT,
	PROTOCOL_ERR_COMMAND_NOT_FOUND,
	PROTOCOL_ERR_OUT_OF_COMMAND_LIST,
	PROTOCOL_ERR_NULL_CALLBACK_FUNCTION,
	PROTOCOL_ERR_CRC_FAIL,
	PROTOCOL_ERR_FRAME_ERROR,
	PROTOCOL_ERR_OUT_OF_BUFFER_SIZE,
	PROTOCOL_ERR_PORT_NULL,
	PROTOCOL_ERR_SEND_FRAME_FAIL,
	PROTOCOL_ERR_MISSING_SEND_FUNCTION,
	PROTOCOL_ERR_MISSING_RECEIVE_FUNCTION,
	PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND,
	PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE,
	PROTOCOL_ERR_STORE_BUFFER_IS_NULL,
	PROTOCOL_ERR_END,
} ProtocolErrorCode;

typedef struct FrameData {
		uint8_t payloadLength;
		GetSetFlag getSetFlag;
		uint16_t crc16;
		// avoid critical section by sending and receiving frame in the same FrameData variable
		bool isOnProcess;
		ProtocolListID protocolID;
} FrameData;

typedef struct ArgumentOfProtocolList_t {
		void *pArg;
		uint8_t sizeArgument;
} ArgumentOfProtocolList_t;

#define PROTOCOL_COMMAND_LIST_LENGTH 1
#define PROTOCOL_CRC_LENGTH	2
#define PROTOCOL_DATA_LENGTH 1
#define PROTOCOL_GET_SET_LENGTH 1
#define PROTOCOL_TOTAL_LENGTH(_DATASIZE_) (PROTOCOL_COMMAND_LIST_LENGTH + PROTOCOL_DATA_LENGTH + PROTOCOL_GET_SET_LENGTH + (_DATASIZE_) + PROTOCOL_CRC_LENGTH)

#define PROTOCOL_DATA_LENGTH_FIELD 0
#define PROTOCOL_ID_FIELD (PROTOCOL_DATA_LENGTH_FIELD + PROTOCOL_DATA_LENGTH)
#define PROTOCOL_GET_SET_FIELD (PROTOCOL_ID_FIELD + PROTOCOL_COMMAND_LIST_LENGTH)
#define PROTOCOL_PAYLOAD_FIELD (PROTOCOL_GET_SET_FIELD + PROTOCOL_GET_SET_LENGTH)
#define PROTOCOL_CRC16_FIELD(_DATASIZE_) (PROTOCOL_PAYLOAD_FIELD + (_DATASIZE_))

typedef void (*pProtocolCpltCallback)(ProtocolListID protocolID, GetSetFlag getOrSet);
typedef void (*pProtocolErrorCallback)(ProtocolErrorCode err);
typedef void (*pProtocolSend)(void *portCommunicate, uint8_t *pData, uint16_t Size);
typedef void (*pProtocolReceiveInterrupt)(void *portCommunicate, uint8_t *pData, uint16_t Size);

class Protocol {

	private:
		void *_port;
		uint8_t *_pTxBuffer;
		uint8_t *_pRxBuffer;
		uint8_t _rxBufSize;
		uint8_t _txBufSize;
		pProtocolCpltCallback _pProlCallback;
		pProtocolErrorCallback _pProlErr;
		pProtocolSend _pSend;
		pProtocolReceiveInterrupt _pReceiveIRQ;
		bool isRxBufferEmpty = true;
		bool isTxBufferEmpty = true;
		FrameData fd;
		ArgumentOfProtocolList_t argID[PROTOCOL_ID_END - 1];

		ProtocolErrorCode DecodeFrameAndCheckCRC();
		void JumpToError(ProtocolErrorCode err);
		void ResetFrame();
		bool IsPassCRC();

	public:

		void Init(void *portCommunicate, uint8_t *pTxBuffer, uint8_t txSize, uint8_t *pRxBuffer, uint8_t rxSize);
		void RegisterArgument(void *arg, uint8_t sizeOfArgument, ProtocolListID protocolID);
		void RegisterReceivedCallbackEvent(void (*pComCpltCallback)(ProtocolListID protocolID, GetSetFlag getOrSet));
		void RegisterErrorEvent(void (*pComErrorCallback)(ProtocolErrorCode err));
		void RegisterSendFrameFunction(void (*pProtocolSend)(void *portCommunicate, uint8_t *pData, uint16_t Size));
		void RegisterReceiveFrameFunction(void (*pProtocolReceiveInterrupt)(void *portCommunicate, uint8_t *pData, uint16_t Size));
		void GetValueFromPayload();
		void GetValueFromPayload(void *outData, uint8_t sizeOfData);
		void HandleReceiveDataInterrupt(void *portCommunicate);
		void MakeFrame(ProtocolListID protocolID, GetSetFlag getOrSet);
		void MakeFrame(void *payload, uint16_t sizeOfPayload, ProtocolListID protocolID, GetSetFlag getOrSet);
		void SendFrame();

};

#endif /* INC_PROTOCOL_H_ */

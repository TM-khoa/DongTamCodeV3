/*
 * Protocol.h
 *
 *  Created on: May 11, 2024
 *      Author: SpiritBoi
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "main.h"
#include "CRC16.h"
#include <string>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

typedef enum CommandList {
	CMD_HANDSHAKE,
	CMD_VALVE,
	CMD_PULSE_TIME,
	CMD_TOTAL_VALVE,
	CMD_CYCLE_INTERVAL_TIME,
	CMD_INTERVAL_TIME,
	CMD_TRIGGER_VALVE,
	CMD_RTC_TIME,
	CMD_END,
} CommandList;

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
	PROTOCOL_ERR_UART_PORT_NULL,
	PROTOCOL_ERR_SEND_FRAME_FAIL,
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
		CommandList cmdList;
} FrameData;

typedef struct ArgumentOfCommandList_t {
		void *pArg;
		uint8_t sizeArgument;
} ArgumentOfCommandList_t;

#define PROTOCOL_COMMAND_LIST_LENGTH 1
#define PROTOCOL_CRC_LENGTH	2
#define PROTOCOL_DATA_LENGTH 1
#define PROTOCOL_GET_SET_LENGTH 1
#define PROTOCOL_TOTAL_LENGTH(_DATASIZE_) (PROTOCOL_COMMAND_LIST_LENGTH + PROTOCOL_DATA_LENGTH + PROTOCOL_GET_SET_LENGTH + (_DATASIZE_) + PROTOCOL_CRC_LENGTH)

#define PROTOCOL_DATA_LENGTH_FIELD 0
#define PROTOCOL_COMMANDLIST_FIELD (PROTOCOL_DATA_LENGTH_FIELD + PROTOCOL_DATA_LENGTH)
#define PROTOCOL_GET_SET_FIELD (PROTOCOL_COMMANDLIST_FIELD + PROTOCOL_COMMAND_LIST_LENGTH)
#define PROTOCOL_PAYLOAD_FIELD (PROTOCOL_GET_SET_FIELD + PROTOCOL_GET_SET_LENGTH)
#define PROTOCOL_CRC16_FIELD(_DATASIZE_) (PROTOCOL_PAYLOAD_FIELD + (_DATASIZE_))

typedef void (*pProtocolCpltCallback)(CommandList cmdlist);
typedef void (*pProtocolErrorCallback)(ProtocolErrorCode err);
typedef void (*pProtocolSend)(uint8_t *pData, uint16_t Size);
typedef void (*pProtocolReceiveInterrupt)(uint8_t *pData, uint16_t Size);

class Protocol {

	private:
		UART_HandleTypeDef *_port;
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
		ArgumentOfCommandList_t argcmd[CMD_END - 1];

		ProtocolErrorCode decodeFrameAndCheckCRC();
		void jumpToError(ProtocolErrorCode err);
		void resetFrame();
		bool isPassCRC();

	public:

		void init(UART_HandleTypeDef *huart, uint8_t *pTxBuffer, uint8_t txSize, uint8_t *pRxBuffer, uint8_t rxSize);
		void registerSendFunction(uint8_t *pData, uint16_t Size);
		void registerReceiveFunction(uint8_t *pData, uint16_t Size);
		void registerArgument(void *arg, uint8_t sizeOfArgument, CommandList cmdID);
		void registerReceivedCallbackEvent(void (*pComCpltCallback)(CommandList cmdlist));
		void registerErrorEvent(void (*pComErrorCallback)(ProtocolErrorCode err));
		void getValueFromPayload();
		void getValueFromPayload(void *outData, uint8_t sizeOfData);
		void receiveDataInterrupt(UART_HandleTypeDef *huart);
		void makeFrame(CommandList cmdID, GetSetFlag getOrSet);
		void makeFrame(void *payload, uint16_t sizeOfPayload, CommandList cmdID, GetSetFlag getOrSet);
		void sendFrame();

};

#endif /* INC_PROTOCOL_H_ */

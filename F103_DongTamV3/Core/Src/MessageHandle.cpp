/*
 * MessageHandle.cpp
 *
 *  Created on: May 14, 2024
 *      Author: KHOA
 */

#include <MessageHandle.h>
#include "Protocol.h"
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

uint8_t logTxBuf[70] = {0};
uint8_t logRxBuf[70] = {0};
uint8_t espTxBuf[70] = {0};
uint8_t espRxBuf[70] = {0};

Protocol pLog, pESP32;

void MessageSend(void *portCommunicate, uint8_t *data, uint16_t size) {
	HAL_UART_Transmit((UART_HandleTypeDef*) portCommunicate, data, size, HAL_MAX_DELAY);
}

void MessageReceive(void *portCommunicate, uint8_t *data, uint16_t size) {
	HAL_UART_Receive_IT((UART_HandleTypeDef*) portCommunicate, data, size);
}

void Message_HandleCommandID(ProtocolListID protocolID, GetSetFlag getOrSet) {

}

void MessageError(ProtocolErrorCode err) {
	while (1);
}

MessageHandle::MessageHandle() {
	// TODO Auto-generated constructor stub

	pLog.RegisterReceiveFrameFunction(&MessageReceive);
	pLog.RegisterSendFrameFunction(&MessageSend);
	pLog.RegisterErrorEvent(&MessageError);
	pLog.RegisterReceivedCallbackEvent(&Message_HandleCommandID);
	pLog.Init((void*) &huart3, logTxBuf, sizeof(logTxBuf), logRxBuf, sizeof(logRxBuf));

	pESP32.RegisterReceiveFrameFunction(&MessageReceive);
	pESP32.RegisterSendFrameFunction(&MessageSend);
	pESP32.RegisterErrorEvent(&MessageError);
	pESP32.RegisterReceivedCallbackEvent(&Message_HandleCommandID);
	pESP32.Init((void*) &huart1, espTxBuf, sizeof(espTxBuf), espRxBuf, sizeof(espRxBuf));

}

MessageHandle::~MessageHandle() {
	// TODO Auto-generated destructor stub
}


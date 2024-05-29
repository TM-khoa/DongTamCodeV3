/*
 * MessageHandle.cpp
 *
 *  Created on: May 14, 2024
 *      Author: KHOA
 */

#include <MessageHandle.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
MessageHandle mesg;

void HandleMessageCallback(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID id, GetSetFlag getSetFlag) {
	if (id == PROTOCOL_ID_HANDSHAKE && getSetFlag == GET_DATA_FROM_THIS_DEVICE) {
		mesg.SetHandshakeStatus(true);
		mesg.SendFrame(PROTOCOL_ID_HANDSHAKE, SET_DATA_TO_THIS_DEVICE);
	}
}

void MessageError(ProtocolErrorCode err) {
	while (1);
}


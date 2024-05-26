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

void HandleMessageCallback(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID protocolID, GetSetFlag getOrSet) {

}

void MessageError(ProtocolErrorCode err) {
	while (1);
}


/*
 * MessageHandle.cpp
 *
 *  Created on: May 14, 2024
 *      Author: KHOA
 */

#include <MessageHandle.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern PCF8563 pcf;
MessageHandle mesg;

void HandleMessageCallback(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID id, GetSetFlag getSetFlag) {
	if (id == PROTOCOL_ID_HANDSHAKE && getSetFlag == GET_DATA_FROM_THIS_DEVICE) {
		mesg.SetHandshakeStatus(true);
		mesg.SendFrame(PROTOCOL_ID_HANDSHAKE, SET_DATA_TO_THIS_DEVICE);
	}
	switch (id) {
	case PROTOCOL_ID_RTC_TIME:
		if (getSetFlag == SET_DATA_TO_THIS_DEVICE) {
			RTC_t rtc;
			mesg.GetValueFromPayload((void*) &rtc, sizeof(rtc));
			pcf.WriteTimeRegisters(rtc);
		}
		break;
	default:
		break;
	}
}

void MessageError(ProtocolErrorCode err) {
	while (1);
}


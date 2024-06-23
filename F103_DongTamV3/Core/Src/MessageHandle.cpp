/*
 * MessageHandle.cpp
 *
 *  Created on: May 14, 2024
 *      Author: KHOA
 */

#include <MessageHandle.h>
#include "ValveControlProcess.h"
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern ValveControl vc;
extern PCF8563 pcf;
MessageHandle mesg;

void HandleMessageCallback(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolID id, GetSetFlag getSetFlag) {
	if (id == (uint8_t) PROTOCOL_ID_HANDSHAKE && getSetFlag == GET_DATA_FROM_THIS_DEVICE) {
		mesg.SetHandshakeStatus(true);
		mesg.SendFrame((uint8_t) PROTOCOL_ID_HANDSHAKE, SET_DATA_TO_THIS_DEVICE);
	}
	switch (id) {
	case PROTOCOL_ID_RTC_TIME:
		if (getSetFlag == SET_DATA_TO_THIS_DEVICE) {
			RTC_t rtc;
			mesg.GetValueFromPayload((void*) &rtc, sizeof(rtc));
			pcf.WriteTimeRegisters(rtc);
		}
		break;
	case PROTOCOL_ID_ON_PROCESS:
		if (getSetFlag == SET_DATA_TO_THIS_DEVICE) {
			bool onProcessStatus;
			mesg.GetValueFromPayload((void*) &onProcessStatus, sizeof(onProcessStatus));
			if (onProcessStatus == true)
				vc.StartValveProcess();
			else
				vc.StopValveProcess();
		}
		else if (getSetFlag == GET_DATA_FROM_THIS_DEVICE) {
			bool valveProcessStatus = vc.GetValveProcess();
			mesg.SendFrame((void*) &valveProcessStatus, 1, PROTOCOL_ID_ON_PROCESS, SET_DATA_TO_THIS_DEVICE);
		}
		break;
	case PROTOCOL_ID_SETTING_PARAMS:
		if (getSetFlag == SET_DATA_TO_THIS_DEVICE) {
			mesg.GetValueFromPayload();
			SettingParameter sp = mesg.GetSettingParams();
			vc.SetCycleIntervalTime(sp.cycleIntervalTime);
			vc.SetIntervalTime(sp.intervalTime);
			vc.SetPulseTime(sp.pulseTime);
			vc.SetTotalValve(sp.totalValve);
		}
		break;
	default:
		break;
	}
}

void MessageError(ProtocolErrorCode err) {
	while (1);
}


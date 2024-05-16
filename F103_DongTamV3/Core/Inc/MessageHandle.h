/*
 * MessageHandle.h
 *
 *  Created on: May 14, 2024
 *      Author: KHOA
 */

#ifndef INC_MESSAGEHANDLE_H_
#define INC_MESSAGEHANDLE_H_

#include "main.h"
#include "Protocol.h"

class MessageHandle: Protocol {
	public:
		void HandleCommandID(ProtocolListID protocolID, GetSetFlag getOrSet);
		void CallbackFromUART(UART_HandleTypeDef *huart);
		MessageHandle();
		virtual ~MessageHandle();

	private:
		bool _isHandShake;
};

#endif /* INC_MESSAGEHANDLE_H_ */

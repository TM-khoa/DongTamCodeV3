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
#include "RTC_Format.h"

typedef HAL_StatusTypeDef (*pfnSend)(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
typedef HAL_StatusTypeDef (*pfnReceive)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

typedef struct PressureTwoSensorValue {
		float sensorAMS5915;
		float sensorSP100;
} PressureTwoSensorValue;

typedef struct ValveData {
		uint16_t valveStatus;
		uint8_t currentValveTrigger;
} ValveData;

void HandleMessageCallback(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID id, GetSetFlag getSetFlag);

class MessageHandle: public Protocol {
	private:
		bool _isHandShake;
		UART_HandleTypeDef *_targetUART;
		UART_HandleTypeDef *portArray[2];
		uint8_t _rxBuf[50];
		uint8_t _txBuf[50];
		pfnSend pSend;
		pfnReceive pReceive;
		uint16_t _handshakeCode;
		PressureTwoSensorValue _pressure;
	public:
		MessageHandle() {
			pSend = &HAL_UART_Transmit;
			pReceive = &HAL_UART_Receive_IT;
			Protocol::RegisterStorageBuffer(_txBuf, sizeof(_txBuf), _rxBuf, sizeof(_rxBuf));
			Protocol::RegisterReceivedCallbackEvent(&HandleMessageCallback);

		}

		void HandleFramDataInterrupt(UART_HandleTypeDef *huart) {
			_targetUART = huart;
			static bool isOnFrameReceived = false;
			if (!isOnFrameReceived) {
				isOnFrameReceived = true;
				uint8_t totalLength = Protocol::GetTotalLengthFromFirstCallback();
				if (totalLength < sizeof(_rxBuf)) {
					pReceive(_targetUART, (uint8_t*) _rxBuf + PROTOCOL_TOTAL_LENGTH_FIELD_SIZE, totalLength - PROTOCOL_TOTAL_LENGTH_FIELD_SIZE);
				}
				else
					JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
			}
			else {
				Protocol::DecodeFrameAndCheckCRC();
				isOnFrameReceived = false;
				FrameData fd = GetFrameDataInfo();
				memset(_rxBuf, 0, fd.totalLength);
				Protocol::ResetFrame();
				pReceive(huart, _rxBuf, 1);
			}
		}
		void Begin(UART_HandleTypeDef *huartESP32, UART_HandleTypeDef *huartLog) {
			portArray[0] = huartESP32;
			portArray[1] = huartLog;
			_targetUART = portArray[0];
			pReceive(*(portArray + 0), _rxBuf, 1);
			pReceive(*(portArray + 1), _rxBuf, 1);

			Protocol::RegisterArgument((void*) &_pressure, sizeof(PressureTwoSensorValue), PROTOCOL_ID_PRESSURE);
			Protocol::RegisterArgument((void*) &_handshakeCode, 0, PROTOCOL_ID_HANDSHAKE);
			SendFrame(PROTOCOL_ID_HANDSHAKE, GET_DATA_FROM_THIS_DEVICE);

		}
		void SetPortUART(UART_HandleTypeDef *huart) {
			_targetUART = huart;
		}

		uint8_t* GetPtrRxBuffer() {
			return _rxBuf;
		}
		uint8_t* GetPtrTxBuffer() {
			return _rxBuf;
		}

		void UpdatePressureAndSend(UART_HandleTypeDef *huart, float pAMS5915, float pSP100) {
			_pressure.sensorAMS5915 = pAMS5915;
			_pressure.sensorSP100 = pSP100;
			SendFrame(PROTOCOL_ID_PRESSURE, SET_DATA_TO_THIS_DEVICE);
		}

		/**
		 * @brief Lấy dữ liệu đã đăng ký từ trước đưa đi đóng gói thành khung truyền và gửi đi
		 * @note Yêu cầu đối số cần gửi phải đăng ký trước bằng phương thức Protocol::RegisterArgument
		 * @note Yêu cầu bộ đệm gửi phải đăng ký trước bằng phương thức Protocol::RegisterStorageBuffer
		 * @param protocolID Mã định dạng khung truyền
		 * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
		 */
		void SendFrame(ProtocolListID protocolID, GetSetFlag getSetFlag) {
			Protocol::MakeFrame(protocolID, getSetFlag);
			FrameData fd = GetFrameDataInfo();
			pSend(_targetUART, _txBuf, fd.totalLength, HAL_MAX_DELAY);
			Protocol::ResetFrame();
			memset(_txBuf, 0, fd.totalLength);
		}

		/**
		 * @brief Lấy dữ liệu do người dùng cung cấp đóng gói thành khung truyền và gửi đi
		 * @note Yêu cầu bộ đệm gửi phải đăng ký trước bằng phương thức Protocol::RegisterStorageBuffer
		 * @param protocolID Mã định dạng khung truyền
		 * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
		 */
		void SendFrame(void *payload, uint16_t sizeOfPayload, ProtocolListID protocolID, GetSetFlag getSetFlag) {
			Protocol::MakeFrame(payload, sizeOfPayload, protocolID, getSetFlag);
			FrameData fd = GetFrameDataInfo();
			pSend(_targetUART, _txBuf, fd.totalLength, HAL_MAX_DELAY);
			Protocol::ResetFrame();
			memset(_txBuf, 0, fd.totalLength);
		}

		void SetHandshakeStatus(bool isHandshake) {
			_isHandShake = isHandshake;
		}

		bool GetHandshakeStatus() {
			return _isHandShake;
		}

}
;

#endif /* INC_MESSAGEHANDLE_H_ */

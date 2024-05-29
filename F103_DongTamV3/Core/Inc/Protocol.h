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
	GET_SET_NONE,
	SET_DATA_TO_THIS_DEVICE,
	GET_DATA_FROM_THIS_DEVICE,
} GetSetFlag;

typedef enum ProtocolErrorCode {
	PROTOCOL_ERR_OK,
	PROTOCOL_ERR_BOARD_NOT_FOUND,
	PROTOCOL_ERR_BOARD_FEATURE_NOT_SUPPORT,
	PROTOCOL_ERR_ID_NOT_FOUND,
	PROTOCOL_ERR_NULL_CALLBACK_FUNCTION,
	PROTOCOL_ERR_CRC_FAIL,
	PROTOCOL_ERR_FRAME_ERROR,
	PROTOCOL_ERR_OUT_OF_BUFFER_SIZE,
	PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND,
	PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE,
	PROTOCOL_ERR_STORE_BUFFER_IS_NULL,
	PROTOCOL_ERR_END,
} ProtocolErrorCode;

typedef struct FrameData {
		uint8_t payloadLength;
		uint8_t totalLength;
		uint16_t crc16;
		GetSetFlag getSetFlag;
		ProtocolListID protocolID;
} FrameData;

typedef struct ArgumentOfProtocolList_t {
		void *pArg;
		uint8_t sizeArgument;
} ArgumentOfProtocolList_t;

#define PROTOCOL_TOTAL_LENGTH_FIELD_SIZE 1
#define PROTOCOL_GET_SET_FIELD_SIZE 1
#define PROTOCOL_ID_FIELD_SIZE 1
#define PROTOCOL_PAYLOAD_LENGTH(TOTAL_LENGTH) ((TOTAL_LENGTH) - PROTOCOL_TOTAL_LENGTH_FIELD_SIZE - PROTOCOL_ID_FIELD_SIZE - PROTOCOL_GET_SET_FIELD_SIZE - PROTOCOL_CRC_FIELD_SIZE)
#define PROTOCOL_CRC_FIELD_SIZE	2

#define PROTOCOL_TOTAL_LENGTH_FIELD 0
#define PROTOCOL_ID_FIELD (PROTOCOL_TOTAL_LENGTH_FIELD + PROTOCOL_TOTAL_LENGTH_FIELD_SIZE)
#define PROTOCOL_GET_SET_FIELD (PROTOCOL_ID_FIELD + PROTOCOL_ID_FIELD_SIZE)
#define PROTOCOL_PAYLOAD_FIELD (PROTOCOL_GET_SET_FIELD + PROTOCOL_GET_SET_FIELD_SIZE)
#define PROTOCOL_CRC16_FIELD(_DATASIZE_) (PROTOCOL_PAYLOAD_FIELD + (_DATASIZE_))

#define PROTOCOL_TOTAL_LENGTH(_DATASIZE_) (PROTOCOL_ID_FIELD_SIZE + PROTOCOL_TOTAL_LENGTH_FIELD_SIZE + PROTOCOL_GET_SET_FIELD_SIZE + (_DATASIZE_) + PROTOCOL_CRC_FIELD_SIZE)

typedef void (*pProtocolCpltCallback)(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID protocolID, GetSetFlag getOrSet);
typedef void (*pProtocolErrorCallback)(ProtocolErrorCode err);

class Protocol {
	private:
		uint8_t *_pTxBuffer;
		uint8_t *_pRxBuffer;
		uint16_t _rxBufSize;
		uint16_t _txBufSize;
		pProtocolCpltCallback _pProlCallback;
		pProtocolErrorCallback _pProlErr;
		FrameData _fd;
		ArgumentOfProtocolList_t argID[PROTOCOL_ID_END - 1];

		/**
		 * @brief Kiểm tra mã CRC từ bộ đệm đầu vào, nếu khác 0 thì khung truyền bị lỗi.
		 * @param data Bộ đệm đầu vào chứa khung truyền
		 * @return
		 */
		bool IsPassCRC(uint8_t *data) {

			if (crc16_Unreflected(data, PROTOCOL_TOTAL_LENGTH(_fd.payloadLength), 0) != 0)
				return false;
			return true;
		}
	public:

		/**
		 * @brief Tự động cập nhật dữ liệu từ bộ đệm cho trước vào đối số tương ứng đã được đăng ký.
		 * @note - Yêu cầu đăng ký đầy đủ bộ đệm input, output với RegisterStorageBuffer và đối số đã được đăng ký trước bởi
		 * RegisterArgument.
		 * @note - Phải gọi DecodeFrameAndCheckCRC trước.
		 */
		void GetValueFromPayload() {
			if (argID[_fd.protocolID].pArg == NULL)
				JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
			if (_fd.payloadLength != argID[_fd.protocolID].sizeArgument)
				JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
			//offset to payloadField in receive buffer
			if (argID[_fd.protocolID].pArg != NULL && _pRxBuffer != NULL)
				memcpy(argID[_fd.protocolID].pArg, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, _fd.payloadLength);
			else
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
		}

		/**
		 * @brief Lấy payload từ khung truyền và đưa vào bộ đệm đầu ra do người dùng tự cung cấp.
		 * @note - Yêu cầu đăng ký RegisterStorageBuffer để có bộ đệm input nhận dữ liệu.
		 * @note - Không cần đăng ký đối số RegisterArgument.
		 * @note - Phải gọi DecodeFrameAndCheckCRC trước.
		 * @param outputBuffer con trỏ trỏ tới bộ đệm đầu ra sẽ được chứa payload.
		 * @param sizeOfOutputBuffer kích thước bộ đệm (lớn hơn hoặc bằng độ dài payload).
		 */
		void GetValueFromPayload(void *outputBuffer, uint8_t sizeOfOutputBuffer) {
			if (sizeOfOutputBuffer < _fd.payloadLength)
				JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
			if (outputBuffer != NULL && _pRxBuffer != NULL)
				memcpy(outputBuffer, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, _fd.payloadLength);
			else {
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			}
		}

		/**
		 * @brief Tách lấy payload từ bộ đệm đầu vào và tự động cập nhật vào đối số tương ứng đã được đăng ký trước.
		 * @note - Yêu cầu đăng ký đối số (RegisterArgument).
		 * @note - Không cần đăng ký bộ đệm (RegisterStorageBuffer).
		 * @note - Phải gọi DecodeFrameAndCheckCRC trước.
		 * @param inputBuffer Địa chỉ bộ đệm khung truyền dữ liệu có chứa payload cần tách.
		 * @param sizeOfInputBuffer Kích thước bộ đệm chứa payload.
		 */
		void GetValueFromPayload(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer) {
			if (argID[_fd.protocolID].pArg == NULL)
				JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
			if (_fd.payloadLength != argID[_fd.protocolID].sizeArgument)
				JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
			if (inputBuffer != NULL)
				memcpy(argID[_fd.protocolID].pArg, inputBuffer + PROTOCOL_PAYLOAD_FIELD, _fd.payloadLength);
			else {
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			}
		}

		/**
		 * @brief Tách lấy payload từ bộ đệm đầu vào và đưa tới bộ đệm đầu ra.
		 * @note Không cần đăng ký bộ đệm (RegisterStorageBuffer) và đối số (RegisterArgument).
		 * @param inputBuffer Địa chỉ bộ đệm khung truyền dữ liệu có chứa payload cần tách.
		 * @param sizeOfInputBuffer Kích thước bộ đệm chứa payload.
		 * @param outputBuffer Địa chỉ bộ đệm dùng để nhận payload được tách ra.
		 * @param sizeOfOutputBuffer Kích thước bộ đệm dùng để nhận payload.
		 */
		void GetValueFromPayload(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, void *outputBuffer, uint8_t sizeOfOutputBuffer) {
			if (_fd.payloadLength > sizeOfOutputBuffer || sizeOfInputBuffer < sizeOfOutputBuffer)
				JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
			if (outputBuffer != NULL && inputBuffer != NULL)
				memcpy(outputBuffer, inputBuffer + PROTOCOL_PAYLOAD_FIELD, _fd.payloadLength);
			else {
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			}
		}

		/**
		 * @brief Tự động tạo khung truyền dựa trên mã định dạng đã cho.
		 * @note Chỉ dùng khi mã định dạng đã được đăng ký đối số dùng hàm RegisterArgument.
		 * @param protocolID Mã định dạng khung truyền.
		 * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
		 */
		void MakeFrame(ProtocolListID protocolID, GetSetFlag getSetFlag) {
			_fd.totalLength = PROTOCOL_TOTAL_LENGTH(argID[protocolID].sizeArgument);
			if (_fd.totalLength > _txBufSize)
				JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
			if (argID[protocolID].pArg == NULL)
				JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
			if (_pTxBuffer == NULL)
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			memset(_pTxBuffer, 0, _txBufSize);
			_fd.payloadLength = argID[protocolID].sizeArgument;
			_fd.getSetFlag = getSetFlag;
			_fd.protocolID = protocolID;
			*(_pTxBuffer + PROTOCOL_TOTAL_LENGTH_FIELD) = _fd.totalLength; // first byte of frame is data length
			*(_pTxBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocolID; // The second byte is command list
			*(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getSetFlag; // Third byte is get or set flag
			memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, argID[protocolID].pArg, argID[protocolID].sizeArgument); // Remain bytes are payload data
			// Calculate CRC16 of _pTxBuffer from begining to payload data
			uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(argID[protocolID].sizeArgument) - PROTOCOL_CRC_FIELD_SIZE, 0);
			_fd.crc16 = crc16Result;
			uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),
					(uint8_t) (crc16Result & 0xff)};
			memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(argID[protocolID].sizeArgument), temp, 2);
		}

		/**
		 * @brief Tạo khung truyền từ đầu vào dữ liệu do người dùng cung cấp với mã định dạng tương ứng
		 *
		 * @param payload Dữ liệu cần đóng gói vào khung truyền
		 * @param sizeOfPayload Kích thước dữ liệu cần đóng gói
		 * @param protocolID Mã định dạng khung truyền
		 * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
		 */
		void MakeFrame(void *payload, uint16_t sizeOfPayload, ProtocolListID protocolID, GetSetFlag getSetFlag) {
			if (PROTOCOL_TOTAL_LENGTH(sizeOfPayload) > _txBufSize)
				JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
			if (payload == NULL || _pTxBuffer == NULL)
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			memset(_pTxBuffer, 0, _txBufSize);
			_fd.payloadLength = sizeOfPayload;
			_fd.totalLength = PROTOCOL_TOTAL_LENGTH(sizeOfPayload);
			_fd.getSetFlag = getSetFlag;
			_fd.protocolID = protocolID;
			*(_pTxBuffer + PROTOCOL_TOTAL_LENGTH_FIELD) = PROTOCOL_TOTAL_LENGTH(sizeOfPayload); // first byte of frame is total length
			*(_pTxBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocolID; // The second byte is command list
			*(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getSetFlag; // Third byte is get or set flag
			memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, payload, sizeOfPayload); // Remain bytes are payload data
			// Calculate CRC16 of _pTxBuffer from begining to payload data
			// Calculate CRC16 of _pTxBuffer from begining to payload data
			uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(sizeOfPayload) - PROTOCOL_CRC_FIELD_SIZE, 0);
			_fd.crc16 = crc16Result;
			uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),
					(uint8_t) (crc16Result & 0xff)};
			memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(sizeOfPayload), temp, 2);
		}

		/**
		 * @brief Tạo ra khung truyền dữ liệu và chứa tại bộ đệm đầu ra
		 * @param outputBuffer Con trỏ trỏ tới địa chỉ bộ đệm đầu ra
		 * @param sizeOfOutputBuffer Kích thước bộ đệm đầu ra chứa khung truyền
		 * @param payload Dữ liệu cần đóng gói vào khung truyền
		 * @param sizeOfPayload Kích thước dữ liệu cần đóng gói
		 * @param protocolID Mã định dạng khung truyền
		 * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
		 */
		void MakeFrame(uint8_t *outputBuffer, uint16_t sizeOfOutputBuffer, void *payload, uint16_t sizeOfPayload, ProtocolListID protocolID, GetSetFlag getSetFlag) {
			if (PROTOCOL_TOTAL_LENGTH(sizeOfPayload) > sizeOfOutputBuffer)
				JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
			if (payload == NULL || outputBuffer == NULL)
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			memset(outputBuffer, 0, sizeOfOutputBuffer);
			_fd.totalLength = PROTOCOL_TOTAL_LENGTH(sizeOfPayload);
			_fd.protocolID = protocolID;
			_fd.getSetFlag = getSetFlag;
			_fd.payloadLength = sizeOfPayload;
			*(outputBuffer + PROTOCOL_TOTAL_LENGTH_FIELD) = PROTOCOL_TOTAL_LENGTH(sizeOfPayload); // first byte of frame is total length
			*(outputBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocolID; // The second byte is command list
			*(outputBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getSetFlag; // Third byte is get or set flag
			memcpy(outputBuffer + PROTOCOL_PAYLOAD_FIELD, payload, sizeOfPayload); // Remain bytes are payload data
			// Calculate CRC16 of outputBuffer from begining to payload data
			uint16_t crc16Result = crc16_Unreflected(outputBuffer, PROTOCOL_TOTAL_LENGTH(sizeOfPayload) - PROTOCOL_CRC_FIELD_SIZE, 0);
			_fd.crc16 = crc16Result;
			uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),
					(uint8_t) (crc16Result & 0xff)};
			memcpy(outputBuffer + PROTOCOL_CRC16_FIELD(sizeOfPayload), temp, 2);
		}

		uint8_t GetTotalLengthFromFirstCallback() {
			if (_pRxBuffer == NULL)
				return PROTOCOL_ERR_STORE_BUFFER_IS_NULL;
			_fd.totalLength = *(_pRxBuffer + 0);
			return _fd.totalLength;
		}

		void DecodeFrameAndCheckCRC() {
			if (_pRxBuffer == NULL)
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			_fd.payloadLength = PROTOCOL_PAYLOAD_LENGTH(_fd.totalLength);
			_fd.protocolID = (ProtocolListID) *(_pRxBuffer + PROTOCOL_ID_FIELD);
			_fd.getSetFlag = (GetSetFlag) *(_pRxBuffer + PROTOCOL_GET_SET_FIELD);
			if (_fd.payloadLength != argID[_fd.protocolID].sizeArgument)
				JumpToError(PROTOCOL_ERR_FRAME_ERROR);
			uint32_t crcNibbleByteMSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(_fd.payloadLength)) << 8;
			uint32_t crcNibbleByteLSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(_fd.payloadLength) + 1);
			_fd.crc16 = crcNibbleByteMSB | crcNibbleByteLSB;
			if (!IsPassCRC(_pRxBuffer))
				JumpToError(PROTOCOL_ERR_CRC_FAIL);
			if (_pProlCallback != NULL)
				_pProlCallback(NULL, 0, _fd.protocolID, _fd.getSetFlag);
			else
				JumpToError(PROTOCOL_ERR_NULL_CALLBACK_FUNCTION);
		}

		/**
		 * @brief Phân tích khung truyền từ bộ đệm đầu vào và gọi tới hàm callback đã được đăng ký
		 * @param inputBuffer con trỏ trỏ tới bộ đệm chứa khung truyền
		 * @param sizeOfBuffer kích thước khung truyền
		 */
		void DecodeFrameAndCheckCRC(uint8_t *inputBuffer, uint16_t lengthOfFrameData) {
			if (inputBuffer == NULL)
				JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
			_fd.totalLength = *(inputBuffer + 0);
			_fd.payloadLength = PROTOCOL_PAYLOAD_LENGTH(_fd.totalLength);
			if (PROTOCOL_TOTAL_LENGTH(_fd.payloadLength) != lengthOfFrameData)
				JumpToError(PROTOCOL_ERR_FRAME_ERROR);
			_fd.protocolID = (ProtocolListID) *(inputBuffer + PROTOCOL_ID_FIELD);
			_fd.getSetFlag = (GetSetFlag) *(inputBuffer + PROTOCOL_GET_SET_FIELD);
			uint32_t crcNibbleByteMSB = *(inputBuffer + PROTOCOL_CRC16_FIELD(_fd.payloadLength)) << 8;
			uint32_t crcNibbleByteLSB = *(inputBuffer + PROTOCOL_CRC16_FIELD(_fd.payloadLength) + 1);
			_fd.crc16 = crcNibbleByteMSB | crcNibbleByteLSB;
			if (!IsPassCRC(inputBuffer))
				JumpToError(PROTOCOL_ERR_CRC_FAIL);
			if (_pProlCallback != NULL)
				_pProlCallback(inputBuffer, lengthOfFrameData, _fd.protocolID, _fd.getSetFlag);
			else
				JumpToError(PROTOCOL_ERR_NULL_CALLBACK_FUNCTION);
		}

		/**
		 * @brief Lấy thông tin từ FrameData
		 * @return FrameData chứa thông tin nhận được
		 */
		FrameData GetFrameDataInfo() {
			return _fd;
		}

		void FrameData_SetPayloadLengthData(uint16_t payloadLength) {
			_fd.payloadLength = payloadLength;
		}

		ArgumentOfProtocolList_t GetArgumentID(ProtocolListID id) {
			return argID[id];
		}

		void ResetFrame() {
			FrameData fdTemp;
			fdTemp.crc16 = 0;
			fdTemp.getSetFlag = GET_SET_NONE;
			fdTemp.payloadLength = 0;
			fdTemp.protocolID = (ProtocolListID) 0, _fd = fdTemp;
		}

		void RegisterStorageBuffer(uint8_t *pTxBuffer, uint16_t txBufferSize, uint8_t *pRxBuffer, uint16_t rxBufferSize) {
			_pTxBuffer = pTxBuffer;
			_txBufSize = txBufferSize;
			_pRxBuffer = pRxBuffer;
			_rxBufSize = rxBufferSize;
		}

		void RegisterArgument(void *arg, uint8_t sizeOfArgument, ProtocolListID protocolID) {
			argID[protocolID].pArg = arg;
			argID[protocolID].sizeArgument = sizeOfArgument;
		}

		void RegisterReceivedCallbackEvent(void (*pProtocolCpltCallback)(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID protocolID, GetSetFlag getOrSet)) {
			_pProlCallback = pProtocolCpltCallback;
		}

		void RegisterErrorEvent(void (*pProtocolErrorCallback)(ProtocolErrorCode err)) {
			_pProlErr = pProtocolErrorCallback;
		}

		void JumpToError(ProtocolErrorCode err) {
			if (_pProlErr != NULL)
				_pProlErr(err);
			else
				while (1);
		}

};

#endif /* INC_PROTOCOL_H_ */

/*
 * Protocol.cpp
 *
 *  Created on: May 11, 2024
 *      Author: SpiritBoi
 */

#include <Protocol.h>
#include "CRC16.h"

void Protocol::Init(void *portCommunicate, uint8_t *pTxBuffer, uint8_t txSize, uint8_t *pRxBuffer, uint8_t rxSize) {
	_port = portCommunicate;
	_pTxBuffer = pTxBuffer;
	_pRxBuffer = pRxBuffer;
	_txBufSize = txSize;
	_rxBufSize = rxSize;
	if (_pReceiveIRQ != NULL)
		_pReceiveIRQ(_port, _pRxBuffer, PROTOCOL_DATA_LENGTH);
	else
		JumpToError(PROTOCOL_ERR_MISSING_RECEIVE_FUNCTION);
}

void Protocol::RegisterArgument(void *arg, uint8_t sizeOfArgument, ProtocolListID protocoID) {
	argID[protocoID].pArg = arg;
	argID[protocoID].sizeArgument = sizeOfArgument;
}

void Protocol::RegisterSendFrameFunction(void (*pProtocolSend)(void *portCommunicate, uint8_t *pData, uint16_t Size)) {
	_pSend = pProtocolSend;
}
void Protocol::RegisterReceiveFrameFunction(void (*pProtocolReceiveInterrupt)(void *portCommunicate, uint8_t *pData, uint16_t Size)) {
	_pReceiveIRQ = pProtocolReceiveInterrupt;
}

void Protocol::RegisterReceivedCallbackEvent(void (*pProtocolCpltCallback)(ProtocolListID protocoID, GetSetFlag getOrSet)) {
	_pProlCallback = pProtocolCpltCallback;
}

void Protocol::RegisterErrorEvent(void (*pProtocolErrorCallback)(ProtocolErrorCode err)) {
	_pProlErr = pProtocolErrorCallback;
}

void Protocol::GetValueFromPayload() {
	if (fd.payloadLength != argID[fd.protocolID].sizeArgument) JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
	//offset to payloadField in receive buffer
	if (argID[fd.protocolID].pArg != NULL) memcpy(argID[fd.protocolID].pArg, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
}

void Protocol::GetValueFromPayload(void *outData, uint8_t sizeOfData) {
	if (fd.payloadLength != argID[fd.protocolID].sizeArgument) JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
	if (outData != NULL)
		memcpy(outData, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
	else {
		JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
	}
}

void Protocol::MakeFrame(ProtocolListID protocoID, GetSetFlag getOrSet) {

	if (PROTOCOL_TOTAL_LENGTH(argID[protocoID].sizeArgument) > _txBufSize) JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
	if (argID[protocoID].pArg == NULL) JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
	if (isTxBufferEmpty == false) memset(_pTxBuffer, 0, _txBufSize);
	fd.isOnProcess = true;
	fd.payloadLength = argID[protocoID].sizeArgument;
	*(_pTxBuffer + PROTOCOL_DATA_LENGTH_FIELD) = argID[protocoID].sizeArgument; // first byte of frame is data length
	*(_pTxBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocoID; // The second byte is command list
	*(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
	memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, argID[protocoID].pArg, argID[protocoID].sizeArgument); // Remain bytes are payload data
	// Calculate CRC16 of _pTxBuffer from begining to payload data
	uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(argID[protocoID].sizeArgument) - PROTOCOL_CRC_LENGTH, 0);
	uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),
			(uint8_t) (crc16Result & 0xff)};
	memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(argID[protocoID].sizeArgument), temp, 2);
}

void Protocol::MakeFrame(void *payload, uint16_t sizeOfPayload, ProtocolListID protocoID, GetSetFlag getOrSet) {
	if (PROTOCOL_TOTAL_LENGTH(sizeOfPayload) > _txBufSize) JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
	if (payload == NULL) JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
	if (isTxBufferEmpty == false) memset(_pTxBuffer, 0, _txBufSize);
	fd.isOnProcess = true;
	fd.payloadLength = sizeOfPayload;
	*(_pTxBuffer + PROTOCOL_DATA_LENGTH_FIELD) = sizeOfPayload; // first byte of frame is data length
	*(_pTxBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocoID; // The second byte is command list
	*(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
	memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, payload, sizeOfPayload); // Remain bytes are payload data
	// Calculate CRC16 of _pTxBuffer from begining to payload data
	// Calculate CRC16 of _pTxBuffer from begining to payload data
	uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(sizeOfPayload) - PROTOCOL_CRC_LENGTH, 0);
	uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),
			(uint8_t) (crc16Result & 0xff)};
	memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(sizeOfPayload), temp, 2);
}

void Protocol::HandleReceiveDataInterrupt(void *portCommunicate) {
	if (_port == NULL) JumpToError(PROTOCOL_ERR_PORT_NULL);
	if (portCommunicate != _port) return;
	static bool isOnFrameReceived = false;
	// if this is a new frame data, extract data length and receive all remain data in interrupt indicated by data length
	if (isOnFrameReceived == false) {
		isOnFrameReceived = true;
		fd.payloadLength = *(_pRxBuffer + 0);
		if (PROTOCOL_TOTAL_LENGTH(fd.payloadLength) < _rxBufSize) {
			if (_pReceiveIRQ != NULL)
				_pReceiveIRQ(_port, _pRxBuffer + PROTOCOL_DATA_LENGTH, PROTOCOL_TOTAL_LENGTH(fd.payloadLength) - PROTOCOL_DATA_LENGTH);
			else
				JumpToError(PROTOCOL_ERR_MISSING_RECEIVE_FUNCTION);
		}
		else
			JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
	}
	// if all data of the frame have been received, begin to extract data and reset to receive new frame, and calling to user callback function
	else {
		isOnFrameReceived = false;
		if (DecodeFrameAndCheckCRC() == PROTOCOL_ERR_OK) {
			if (_pProlCallback != NULL)
				_pProlCallback(fd.protocolID, fd.getSetFlag);
			else
				JumpToError(PROTOCOL_ERR_NULL_CALLBACK_FUNCTION);
		}
		if (_pReceiveIRQ != NULL)
			_pReceiveIRQ(_port, _pRxBuffer, PROTOCOL_DATA_LENGTH);
		else
			JumpToError(PROTOCOL_ERR_MISSING_RECEIVE_FUNCTION);
		memset(_pRxBuffer, 0, _rxBufSize);
		ResetFrame();
	}
}

void Protocol::SendFrame() {
	// sending data length first, delay amount of time for app to process and send the rest of packet
	if (_pSend != NULL)
		_pSend(_port, _pTxBuffer, 1);
	else
		JumpToError(PROTOCOL_ERR_MISSING_SEND_FUNCTION);
	for (uint16_t i = 0; i < 20000; i++)		// delay để app phân biệt 2 gói tin gửi liền nhau
		__NOP();
	if (_pSend != NULL)
		_pSend(_port, _pTxBuffer + 1, PROTOCOL_TOTAL_LENGTH(fd.payloadLength) - 1);
	else
		JumpToError(PROTOCOL_ERR_MISSING_SEND_FUNCTION);
	memset(_pTxBuffer, 0, _txBufSize);
	isTxBufferEmpty = true;
	ResetFrame();
}

ProtocolErrorCode Protocol::DecodeFrameAndCheckCRC() {
	uint32_t crcNibbleByteMSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength)) << 8;
	uint32_t crcNibbleByteLSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength) + 1);
	fd.protocolID = (ProtocolListID) *(_pRxBuffer + PROTOCOL_ID_FIELD);
	fd.crc16 = crcNibbleByteMSB | crcNibbleByteLSB;
	fd.getSetFlag = (GetSetFlag) *(_pRxBuffer + PROTOCOL_GET_SET_FIELD);
	if (!IsPassCRC()) return PROTOCOL_ERR_CRC_FAIL;
	return PROTOCOL_ERR_OK;
}

void Protocol::ResetFrame() {
	FrameData fdTemp = {0};
	fd = fdTemp;
}

void Protocol::JumpToError(ProtocolErrorCode err) {
	if (_pProlErr != NULL) {
		_pProlErr(err);
	}
	else
		while (1);
}

bool Protocol::IsPassCRC() {
	if (crc16_Unreflected(_pRxBuffer, PROTOCOL_CRC16_FIELD(fd.payloadLength) + PROTOCOL_CRC_LENGTH, 0) != 0) {
		return false;
	}
	return true;
}


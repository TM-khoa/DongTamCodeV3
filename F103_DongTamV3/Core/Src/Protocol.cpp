/*
 * Protocol.cpp
 *
 *  Created on: May 11, 2024
 *      Author: SpiritBoi
 */

#include <Protocol.h>

void Protocol::jumpToError(ProtocolErrorCode err) {
	if (_pProlErr != NULL) {
		_pProlErr(err);
	}
	else
		while (1);
}

bool Protocol::isPassCRC() {
	if (crc16_Unreflected(_pRxBuffer, PROTOCOL_CRC16_FIELD(fd.payloadLength) + PROTOCOL_CRC_LENGTH, 0) != 0) {
		return false;
	}
	return true;
}

ProtocolErrorCode Protocol::decodeFrameAndCheckCRC() {
	uint32_t crcNibbleByteMSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength)) << 8;
	uint32_t crcNibbleByteLSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength) + 1);
	fd.cmdList = (CommandList) *(_pRxBuffer + PROTOCOL_COMMANDLIST_FIELD);
	fd.crc16 = crcNibbleByteMSB | crcNibbleByteLSB;
	fd.getSetFlag = (GetSetFlag) *(_pRxBuffer + PROTOCOL_GET_SET_FIELD);
	if (!isPassCRC()) return PROTOCOL_ERR_CRC_FAIL;
	return PROTOCOL_ERR_OK;
}

void Protocol::resetFrame() {
	FrameData fdTemp = {0};
	fd = fdTemp;
}

void Protocol::init(UART_HandleTypeDef *huart, uint8_t *pTxBuffer, uint8_t txSize, uint8_t *pRxBuffer, uint8_t rxSize) {
	_port = huart;
	_pTxBuffer = pTxBuffer;
	_pRxBuffer = pRxBuffer;
	_txBufSize = txSize;
	_rxBufSize = rxSize;
	_pReceiveIRQ(_pRxBuffer, PROTOCOL_DATA_LENGTH);
}

void Protocol::receiveDataInterrupt(UART_HandleTypeDef *huart) {
	if (huart != _port) return;
	if (_port == NULL) jumpToError(PROTOCOL_ERR_UART_PORT_NULL);
	static bool isOnFrameReceived = false;
	// if this is a new frame data, extract data length and receive all remain data in interrupt indicated by data length
	if (isOnFrameReceived == false) {
		isOnFrameReceived = true;
		fd.payloadLength = *(_pRxBuffer + 0);
		if (PROTOCOL_TOTAL_LENGTH(fd.payloadLength) < _rxBufSize)
			_pReceiveIRQ(_pRxBuffer + PROTOCOL_DATA_LENGTH, PROTOCOL_TOTAL_LENGTH(fd.payloadLength) - PROTOCOL_DATA_LENGTH);
		else
			jumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
	}
	// if all data of the frame have been received, begin to extract data and reset to receive new frame, and calling to user callback function
	else {
		isOnFrameReceived = false;
		if (decodeFrameAndCheckCRC() == PROTOCOL_ERR_OK) {
			if (_pProlCallback != NULL)
				_pProlCallback(fd.cmdList);
			else
				jumpToError(PROTOCOL_ERR_NULL_CALLBACK_FUNCTION);
		}
		_pReceiveIRQ(_pRxBuffer, PROTOCOL_DATA_LENGTH);
		memset(_pRxBuffer, 0, _rxBufSize);
		resetFrame();
	}
}

void Protocol::registerArgument(void *arg, uint8_t sizeOfArgument, CommandList cmdID) {
	argcmd[cmdID].pArg = arg;
	argcmd[cmdID].sizeArgument = sizeOfArgument;
}

void Protocol::registerReceivedCallbackEvent(void (*pProtocolCpltCallback)(CommandList cmdID)) {
	_pProlCallback = pProtocolCpltCallback;
}

void Protocol::registerErrorEvent(void (*pProtocolErrorCallback)(ProtocolErrorCode err)) {
	_pProlErr = pProtocolErrorCallback;
}

void Protocol::getValueFromPayload() {
	if (fd.payloadLength != argcmd[fd.cmdList].sizeArgument) jumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
	//offset to payloadField in receive buffer
	if (argcmd[fd.cmdList].pArg != NULL) memcpy(argcmd[fd.cmdList].pArg, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
}

void Protocol::getValueFromPayload(void *outData, uint8_t sizeOfData) {
	if (fd.payloadLength != argcmd[fd.cmdList].sizeArgument) jumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
	if (outData != NULL)
		memcpy(outData, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
	else {
		jumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
	}
}

void Protocol::makeFrame(CommandList cmdID, GetSetFlag getOrSet) {

	if (PROTOCOL_TOTAL_LENGTH(argcmd[cmdID].sizeArgument) > _txBufSize) jumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
	if (argcmd[cmdID].pArg == NULL) jumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
	if (isTxBufferEmpty == false) memset(_pTxBuffer, 0, _txBufSize);
	fd.isOnProcess = true;
	fd.payloadLength = argcmd[cmdID].sizeArgument;
	*(_pTxBuffer + PROTOCOL_DATA_LENGTH_FIELD) = argcmd[cmdID].sizeArgument; // first byte of frame is data length
	*(_pTxBuffer + PROTOCOL_COMMANDLIST_FIELD) = (uint8_t) cmdID; // The second byte is command list
	*(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
	memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, argcmd[cmdID].pArg, argcmd[cmdID].sizeArgument); // Remain bytes are payload data
	// Calculate CRC16 of _pTxBuffer from begining to payload data
	uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(argcmd[cmdID].sizeArgument) - PROTOCOL_CRC_LENGTH, 0);
	uint8_t temp[2] = {(uint8_t) (crc16Result >> 8), (uint8_t) (crc16Result & 0xff)};
	memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(argcmd[cmdID].sizeArgument), temp, 2);
}

void Protocol::makeFrame(void *payload, uint16_t sizeOfPayload, CommandList cmdID, GetSetFlag getOrSet) {
	if (PROTOCOL_TOTAL_LENGTH(sizeOfPayload) > _txBufSize) jumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
	if (payload == NULL) jumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
	if (isTxBufferEmpty == false) memset(_pTxBuffer, 0, _txBufSize);
	fd.isOnProcess = true;
	fd.payloadLength = sizeOfPayload;
	*(_pTxBuffer + PROTOCOL_DATA_LENGTH_FIELD) = sizeOfPayload; // first byte of frame is data length
	*(_pTxBuffer + PROTOCOL_COMMANDLIST_FIELD) = (uint8_t) cmdID; // The second byte is command list
	*(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
	memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, payload, sizeOfPayload); // Remain bytes are payload data
	// Calculate CRC16 of _pTxBuffer from begining to payload data
	// Calculate CRC16 of _pTxBuffer from begining to payload data
	uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(sizeOfPayload) - PROTOCOL_CRC_LENGTH, 0);
	uint8_t temp[2] = {(uint8_t) (crc16Result >> 8), (uint8_t) (crc16Result & 0xff)};
	memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(sizeOfPayload), temp, 2);
}

void Protocol::sendFrame() {
	// sending data length first, delay amount of time for app to process and send the rest of packet
	_pSend(_pTxBuffer, 1);
	for (uint16_t i = 0; i < 20000; i++)		// delay để app phân biệt 2 gói tin gửi liền nhau
		__NOP();
	_pSend(_pTxBuffer + 1, PROTOCOL_TOTAL_LENGTH(fd.payloadLength) - 1);
	memset(_pTxBuffer, 0, _txBufSize);
	isTxBufferEmpty = true;
	resetFrame();
}


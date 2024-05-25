/*
 * Protocol.h
 *
 *  Created on: May 11, 2024
 *      Author: SpiritBoi
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "../main.h"
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
	PROTOCOL_ERR_OUT_OF_PROTOCOL_ID,
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
		GetSetFlag getSetFlag;
		uint16_t crc16;
		// avoid critical section by sending and receiving frame in the same FrameData variable
		bool isOnProcess;
		ProtocolListID protocolID;
} FrameData;

typedef struct ArgumentOfProtocolList_t {
		void *pArg;
		uint8_t sizeArgument;
} ArgumentOfProtocolList_t;

#define PROTOCOL_ID_LENGTH 1
#define PROTOCOL_CRC_LENGTH	2
#define PROTOCOL_DATA_LENGTH 1
#define PROTOCOL_GET_SET_LENGTH 1
#define PROTOCOL_TOTAL_LENGTH(_DATASIZE_) (PROTOCOL_ID_LENGTH + PROTOCOL_DATA_LENGTH + PROTOCOL_GET_SET_LENGTH + (_DATASIZE_) + PROTOCOL_CRC_LENGTH)

#define PROTOCOL_DATA_LENGTH_FIELD 0
#define PROTOCOL_ID_FIELD (PROTOCOL_DATA_LENGTH_FIELD + PROTOCOL_DATA_LENGTH)
#define PROTOCOL_GET_SET_FIELD (PROTOCOL_ID_FIELD + PROTOCOL_ID_LENGTH)
#define PROTOCOL_PAYLOAD_FIELD (PROTOCOL_GET_SET_FIELD + PROTOCOL_GET_SET_LENGTH)
#define PROTOCOL_CRC16_FIELD(_DATASIZE_) (PROTOCOL_PAYLOAD_FIELD + (_DATASIZE_))

typedef void (*pProtocolCpltCallback)(ProtocolListID protocolID, GetSetFlag getOrSet);
typedef void (*pProtocolErrorCallback)(ProtocolErrorCode err);

class Protocol {


public:


    /**
     * @brief Cập nhật dữ liệu tự động với đầu vào và đầu ra đã chỉ định sẵn
     */
    void GetValueFromPayload(){
        if (fd.payloadLength != argID[fd.protocolID].sizeArgument)
            JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
        //offset to payloadField in receive buffer
        if (argID[fd.protocolID].pArg != NULL)
            memcpy(argID[fd.protocolID].pArg, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
    }
    
    /**
     * @brief Lấy payload từ khung truyền và đưa vào bộ đệm đầu ra do người dùng tự cung cấp
     * @param outData con trỏ trỏ tới bộ đệm đầu ra chứa payload
     * @param sizeOfData kích thước bộ đệm
     */
    void GetValueFromPayload(void *outData, uint8_t sizeOfData){
        if (fd.payloadLength != argID[fd.protocolID].sizeArgument)
            JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
        if(sizeOfData < fd.payloadLength) 
            JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
        if (outData != NULL && _pRxBuffer != NULL)
            memcpy(outData, _pRxBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
        else {
            JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
        }
    }

    void GetValueFromPayload(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, void *outputBuffer, uint8_t sizeOfOutputBuffer){
        if (fd.payloadLength != sizeOfOutputBuffer)
            JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
        if (outputBuffer != NULL && inputBuffer != NULL)
            memcpy(outputBuffer, inputBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
        else {
            JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
        }
    }

    /**
     * @brief Tách lấy payload từ bộ đệm đầu vào và đưa tới bộ đệm đầu ra
     * @param inputBuffer Địa chỉ bộ đệm khung truyền dữ liệu có chứa payload cần tách 
     * @param sizeOfInputBuffer Kích thước bộ đệm chứa payload
     * @param outputBuffer Địa chỉ bộ đệm dùng để nhận payload được tách ra
     * @param sizeOfOutputBuffer Kích thước bộ đệm dùng để nhận payload
     */
    void GetValueFromPayload(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, void *outputBuffer, uint8_t sizeOfOutputBuffer){
        if (fd.payloadLength != sizeOfOutputBuffer || sizeOfInputBuffer < sizeOfOutputBuffer)
            JumpToError(PROTOCOL_ERR_PAYLOAD_NOT_RECOGNIZE);
        if (outputBuffer != NULL)
            memcpy(outputBuffer, inputBuffer + PROTOCOL_PAYLOAD_FIELD, fd.payloadLength);
        else {
            JumpToError(PROTOCOL_ERR_STORE_BUFFER_IS_NULL);
        }
    }
  
    void MakeFrame(ProtocolListID protocolID, GetSetFlag getOrSet){
        if (PROTOCOL_TOTAL_LENGTH(argID[protocolID].sizeArgument) > _txBufSize)
            JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
        if (argID[protocolID].pArg == NULL)
            JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
        if (isTxBufferEmpty == false)
            memset(_pTxBuffer, 0, _txBufSize);
        fd.isOnProcess = true;
        fd.payloadLength = argID[protocolID].sizeArgument;
        *(_pTxBuffer + PROTOCOL_DATA_LENGTH_FIELD) = argID[protocolID].sizeArgument; // first byte of frame is data length
        *(_pTxBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocolID; // The second byte is command list
        *(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
        memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, argID[protocolID].pArg, argID[protocolID].sizeArgument); // Remain bytes are payload data
        // Calculate CRC16 of _pTxBuffer from begining to payload data
        uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(argID[protocolID].sizeArgument) - PROTOCOL_CRC_LENGTH, 0);
        uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),(uint8_t) (crc16Result & 0xff)};
        memcpy(_pTxBuffer + PROTOCOL_CRC16_FIELD(argID[protocolID].sizeArgument), temp, 2);
    }
  
    void MakeFrame(void *payload, uint16_t sizeOfPayload, ProtocolListID protocolID, GetSetFlag getOrSet){
        if (PROTOCOL_TOTAL_LENGTH(sizeOfPayload) > _txBufSize)
            JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
        if (payload == NULL)
            JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
        if (isTxBufferEmpty == false)
            memset(_pTxBuffer, 0, _txBufSize);
        fd.isOnProcess = true;
        fd.payloadLength = sizeOfPayload;
        *(_pTxBuffer + PROTOCOL_DATA_LENGTH_FIELD) = sizeOfPayload; // first byte of frame is data length
        *(_pTxBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocolID; // The second byte is command list
        *(_pTxBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
        memcpy(_pTxBuffer + PROTOCOL_PAYLOAD_FIELD, payload, sizeOfPayload); // Remain bytes are payload data
        // Calculate CRC16 of _pTxBuffer from begining to payload data
        // Calculate CRC16 of _pTxBuffer from begining to payload data
        uint16_t crc16Result = crc16_Unreflected(_pTxBuffer, PROTOCOL_TOTAL_LENGTH(sizeOfPayload) - PROTOCOL_CRC_LENGTH, 0);
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
     * @param protocolID mã định dạng khung truyền
     * @param getOrSet Quy định lấy dữ liệu ra hay nạp dữ liệu vào hoặc không
     */
    void MakeFrame(uint8_t *outputBuffer, uint16_t sizeOfOutputBuffer, void *payload, uint16_t sizeOfPayload, ProtocolListID protocolID, GetSetFlag getOrSet){  
        if (PROTOCOL_TOTAL_LENGTH(sizeOfPayload) > sizeOfOutputBuffer)
            JumpToError(PROTOCOL_ERR_OUT_OF_BUFFER_SIZE);
        if (payload == NULL)
            JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
        memset(outputBuffer, 0, sizeOfOutputBuffer);
        fd.isOnProcess = true;
        fd.payloadLength = sizeOfPayload;
        *(outputBuffer + PROTOCOL_DATA_LENGTH_FIELD) = sizeOfPayload; // first byte of frame is data length
        *(outputBuffer + PROTOCOL_ID_FIELD) = (uint8_t) protocolID; // The second byte is command list
        *(outputBuffer + PROTOCOL_GET_SET_FIELD) = (uint8_t) getOrSet; // Third byte is get or set flag
        memcpy(outputBuffer + PROTOCOL_PAYLOAD_FIELD, payload, sizeOfPayload); // Remain bytes are payload data
        // Calculate CRC16 of outputBuffer from begining to payload data
        uint16_t crc16Result = crc16_Unreflected(outputBuffer, PROTOCOL_TOTAL_LENGTH(sizeOfPayload) - PROTOCOL_CRC_LENGTH, 0);
        uint8_t temp[2] = {(uint8_t) (crc16Result >> 8),
                (uint8_t) (crc16Result & 0xff)};
        memcpy(outputBuffer + PROTOCOL_CRC16_FIELD(sizeOfPayload), temp, 2);
    }

    ProtocolErrorCode DecodeFrameAndCheckCRC(){
        fd.payloadLength = *(_pRxBuffer + 0);
        fd.protocolID = (ProtocolListID) *(_pRxBuffer + PROTOCOL_ID_FIELD);
        fd.getSetFlag = (GetSetFlag) *(_pRxBuffer + PROTOCOL_GET_SET_FIELD);
        if(PROTOCOL_TOTAL_LENGTH(fd.payloadLength) != argID[fd.protocolID].sizeArgument) return PROTOCOL_ERR_FRAME_ERROR;
        uint32_t crcNibbleByteMSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength)) << 8;
        uint32_t crcNibbleByteLSB = *(_pRxBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength) + 1);
        fd.crc16 = crcNibbleByteMSB | crcNibbleByteLSB;
        if (!IsPassCRC(_pRxBuffer)) return PROTOCOL_ERR_CRC_FAIL;
        return PROTOCOL_ERR_OK;
    }


    /**
     * @brief Phân tích khung truyền từ bộ đệm đầu vào và gọi tới hàm callback đã được đăng ký
     * @param inputBuffer con trỏ trỏ tới bộ đệm chứa khung truyền
     * @param sizeOfBuffer kích thước khung truyền
     * @return 
     */
    ProtocolErrorCode DecodeFrameAndCheckCRC(uint8_t *inputBuffer, uint16_t lengthOfFrameData){
        fd.payloadLength = *(inputBuffer + 0);
        if(PROTOCOL_TOTAL_LENGTH(fd.payloadLength) != lengthOfFrameData) return PROTOCOL_ERR_FRAME_ERROR;
        fd.protocolID = (ProtocolListID) *(inputBuffer + PROTOCOL_ID_FIELD);
        fd.getSetFlag = (GetSetFlag) *(inputBuffer + PROTOCOL_GET_SET_FIELD);
        uint32_t crcNibbleByteMSB = *(inputBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength)) << 8;
        uint32_t crcNibbleByteLSB = *(inputBuffer + PROTOCOL_CRC16_FIELD(fd.payloadLength) + 1);
        fd.crc16 = crcNibbleByteMSB | crcNibbleByteLSB;
        if (!IsPassCRC(inputBuffer)) return PROTOCOL_ERR_CRC_FAIL;
            if (_pProlCallback != NULL)
            _pProlCallback(fd.protocolID, fd.getSetFlag);
        else
            JumpToError(PROTOCOL_ERR_NULL_CALLBACK_FUNCTION);
        return PROTOCOL_ERR_OK;
    }

    /**
     * @brief Lấy thông tin từ FrameData
     * @return FrameData chứa thông tin nhận được
     */
    FrameData GetFrameDataInfo(){
        return fd;
    }

    ArgumentOfProtocolList_t GetArgumentID(ProtocolListID id){
        return argID[id];
    }

    void ResetFrame(){
        FrameData fdTemp;
        fdTemp.crc16 = 0;
        fdTemp.getSetFlag = GET_SET_NONE;
        fdTemp.payloadLength = 0;
        fdTemp.protocolID = (ProtocolListID)0,
        fd = fdTemp;
    }

    void RegisterArgument(void *arg, uint8_t sizeOfArgument, ProtocolListID protocolID){
        argID[protocolID].pArg = arg;
	    argID[protocolID].sizeArgument = sizeOfArgument;
    }
    
    void RegisterReceivedCallbackEvent(void (*pProtocolCpltCallback)(ProtocolListID protocolID, GetSetFlag getOrSet)){
        _pProlCallback = pProtocolCpltCallback;
    }
    
    void RegisterErrorEvent(void (*pProtocolErrorCallback)(ProtocolErrorCode err)){
        _pProlErr = pProtocolErrorCallback;
    }


private:
    uint8_t *_pTxBuffer;
    uint8_t *_pRxBuffer;
    uint8_t _rxBufSize;
    uint8_t _txBufSize;
    pProtocolCpltCallback _pProlCallback;
    pProtocolErrorCallback _pProlErr;
    bool isRxBufferEmpty = true;
    bool isTxBufferEmpty = true;
    FrameData fd;
    ArgumentOfProtocolList_t argID[PROTOCOL_ID_END - 1];


    void JumpToError(ProtocolErrorCode err){
        if (_pProlErr != NULL) 
            _pProlErr(err);
        else
            while (1);
    }

    bool IsPassCRC(uint8_t *data){
        
        if (crc16_Unreflected(data, PROTOCOL_TOTAL_LENGTH(fd.payloadLength), 0) != 0) 
            return false;
        return true;
    }


    
};

#endif /* INC_PROTOCOL_H_ */

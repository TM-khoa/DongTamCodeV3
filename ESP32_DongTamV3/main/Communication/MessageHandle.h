#ifndef _MESSAGE_HANDLE_H
#define _MESSAGE_HANDLE_H
#include "../main.h"
#include "Protocol.h"
#include "UART.h"
typedef struct PressureTwoSensorValue {
		float sensorAMS5915;
		float sensorSP100;
} PressureTwoSensorValue;

typedef struct ValveData {
		uint16_t valveStatus;
		uint8_t currentValveTrigger;
} ValveData;

typedef struct SettingParameter {
		uint8_t totalValve;
		uint16_t pulseTime;
		uint16_t intervalTime;
		uint8_t cycleIntervalTime;
} SettingParameter;

class MessageHandle: public PortUART {
public:
    void Begin();


    /**
     * @brief Tự cấp phát vùng nhớ chứa khung truyền và gửi đi tới cổng UART được chỉ định.
     * @note - Yêu cầu phải đăng ký trước đối số với Protocol::RegisterArgument.
     * @param port Cổng UART chỉ định gửi khung truyền.
     * @param id Mã định danh thông số cần gửi.
     * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
     */
    void TransmitMessage(uart_port_t port,ProtocolID id, GetSetFlag getSetFlag){
        // Lấy thông tin đối số cần truyền đi
        ArgumentOfProtocolList_t arg = Protocol::GetArgumentID(id);
        if(arg.pArg == NULL) Protocol::JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
        // Tính toán kích thước frame truyền tương ứng với kích thước payload lấy từ đối số
        uint16_t frameDataLength = PROTOCOL_TOTAL_LENGTH(arg.sizeArgument);
        // Cấp phát vùng nhớ có kích thước bằng khung truyền
        uint8_t *senderBuffer = (uint8_t*)malloc(frameDataLength);
        // Lắp ghép payload từ đối số, ID truyền, cờ get set vào khung truyền
        Protocol::MakeFrame(senderBuffer,frameDataLength,arg.pArg,arg.sizeArgument,id,getSetFlag);
        // Gửi khung truyền tới cổng UART được chỉ định
        uart_write_bytes(port,senderBuffer,frameDataLength);
        // Hủy vùng nhớ
        free(senderBuffer);
    }

    /**
     * @brief Tự cấp phát vùng nhớ chứa khung truyền và gửi đi tới cổng UART được chỉ định với dữ liệu đầu vào do người dùng cung cấp.
     * @param port Cổng UART chỉ định gửi khung truyền.
     * @param id Mã định danh thông số cần gửi.
     * @param getSetFlag Cờ báo yêu cầu đối tượng sẽ nhận dữ liệu hay phản hồi dữ liệu về.
     */
    void TransmitMessage(uart_port_t port, ProtocolID id, GetSetFlag getSetFlag, void *data, uint16_t sizeOfData){
        if(data == NULL) Protocol::JumpToError(PROTOCOL_ERR_REFERENCE_PAYLOAD_NOT_FOUND);
        // Tính toán kích thước frame truyền tương ứng với kích thước sizeOfData
        uint16_t frameDataLength = PROTOCOL_TOTAL_LENGTH(sizeOfData);
        // Cấp phát vùng nhớ có kích thước bằng khung truyền
        uint8_t *senderBuffer = (uint8_t*)malloc(frameDataLength);
        // Lắp ghép payload từ đối số, ID truyền, cờ get set vào khung truyền
        Protocol::MakeFrame(senderBuffer,frameDataLength,data,sizeOfData,id,getSetFlag);
        // Gửi khung truyền tới cổng UART được chỉ định
        uart_write_bytes(port,senderBuffer,frameDataLength);
        // Hủy vùng nhớ
        free(senderBuffer);
    }

    void HandleErrorMessage(ProtocolErrorCode err)
    {
        if(_isHandshake == false) return;
        ESP_LOGE("MesgHandle","err:%d",err);
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        while(1);
    }

    PressureTwoSensorValue GetTwoSensorvalue(){return _pressure;}
    void SetTwoSensorValue(PressureTwoSensorValue p){_pressure = p;}

    ValveData GetValveData(){return _vData;}

    void LoadSettingParam(uint8_t totalValve, uint16_t pulseTime, uint16_t intervalTime, uint16_t cycleIntervalTime)    {
        _settingParams.totalValve = totalValve;
        _settingParams.intervalTime = intervalTime;
        _settingParams.cycleIntervalTime = cycleIntervalTime;
        _settingParams.pulseTime = pulseTime;
    }

    void SetHandshake(bool handshake) {_isHandshake = handshake;}
    bool IsHandshake(){ return _isHandshake;}
private:
    uint16_t _handshakeCode = 0;
    bool _isHandshake;
    PressureTwoSensorValue _pressure;
    ValveData _vData;
    SettingParameter _settingParams;
};

void TaskUART(void *pvParameters);
void ErrorMessage(ProtocolErrorCode err);
#endif


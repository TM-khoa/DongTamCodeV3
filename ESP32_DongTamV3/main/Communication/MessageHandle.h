#ifndef _MESSAGE_HANDLE_H
#define _MESSAGE_HANDLE_H
#include "../main.h"
#include "Protocol.h"
#include "UART.h"


class MessageHandle: public PortUART {
public:
    void Begin();

    void TransmitMessage(uart_port_t port,ProtocolListID id, GetSetFlag getSetFlag){
        // Lấy thông tin đối số cần truyền đi
        ArgumentOfProtocolList_t arg = Protocol::GetArgumentID(id);
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

    void TransmitMessage(uart_port_t port, ProtocolListID id, GetSetFlag getSetFlag, void *data, uint16_t sizeOfData){
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

    void HandleReceiveMessage(ProtocolListID id, GetSetFlag getSetFlag){
        switch (id){
        case PROTOCOL_ID_HANDSHAKE:
            ESP_LOGI("HandShake","Receive");
            break;
        case PROTOCOL_ID_VALVE:
            
            break;            
        case PROTOCOL_ID_PULSE_TIME:
            
            break;
        case PROTOCOL_ID_INTERVAL_TIME:
            
            break;                    
        case PROTOCOL_ID_CYCLE_INTERVAL_TIME:
            
            break;            
        case PROTOCOL_ID_TOTAL_VALVE:
            
            break;   
        case PROTOCOL_ID_PRESSURE:
            break;
        case PROTOCOL_ID_RTC_TIME:
            break;
        default:
            break;
        }
    }

private:
};

void HandleReceivedMessage(ProtocolListID ID, GetSetFlag getSetFlag);
void HandleErrorMessage(ProtocolErrorCode err);
#endif


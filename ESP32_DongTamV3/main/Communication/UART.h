#ifndef _UART_H_
#define _UART_H_

/**
 * @brief Dùng để truyền và nhận dữ liệu được đóng gói bởi module Protocol
*/

#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include "../main.h"
#include "Protocol.h"

#define UART_RX GPIO_NUM_16
#define UART_TX GPIO_NUM_17
#define UART_QUEUE_EVENT_SIZE 34
#define UART_BUFFER_EVENT_SIZE 1000
#define QUEUE_SIZE_OF_QBIG_SIZE 10

typedef enum ExtendEventUART {
    EVT_UART_TASK_BIG_SIZE,
    EVT_UART_DELETE_TASK_BIG_SIZE,
    EVT_UART_OVERSIZE_HW_FIFO,
    EVT_UART_STM32_READY,
    EVT_UART_IS_ON_PROCESS,
    EVT_UART_NOT_ENOUGH_SPACE_BIG_SIZE,
}ExtendEventUART;

class PortUART: public Protocol{
private:
    QueueHandle_t qRxEventSTM32, qRxEventLog, qTxMesg, qPtrBigSize;
    uart_port_t _uartTarget; 
    uart_event_t _uartEvent;
    EventGroupHandle_t _evgExtendEventUART;
    EventBits_t e;
    uint8_t _countQueueBigSize = 0;
public:
    PortUART(){
        uart_config_t uartConfig = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_DEFAULT,
        };
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, UART_BUFFER_EVENT_SIZE, UART_BUFFER_EVENT_SIZE, UART_QUEUE_EVENT_SIZE, &qRxEventSTM32, 0));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uartConfig));
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, UART_BUFFER_EVENT_SIZE, UART_BUFFER_EVENT_SIZE, UART_QUEUE_EVENT_SIZE, &qRxEventLog, 0));
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uartConfig));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

        qPtrBigSize = xQueueCreate(QUEUE_SIZE_OF_QBIG_SIZE,sizeof(uint8_t*));
        _evgExtendEventUART = xEventGroupCreate();
    }
    void WaitForEventUART (){
        
        if(xQueueReceive(qRxEventSTM32, (void * )&_uartEvent, 0))
            _uartTarget = UART_NUM_2;
        else if(xQueueReceive(qRxEventLog,(void * )&_uartEvent, 0))
            _uartTarget = UART_NUM_0;
        else return;
        switch(_uartEvent.type) {
            case UART_DATA:
                e = xEventGroupGetBits(_evgExtendEventUART);
                /*
                    Nếu dữ liệu nhận được không vượt quá bộ đệm cứng của ESP32 là 120 byte hoặc
                    không có tín hiệu báo đang trong chế độ nhận dữ liệu lớn (EVT_UART_OVERSIZE_HW_FIFO)
                    thì nhận bình thường
                */
                if(_uartEvent.size < 120 && CHECKFLAG(e,SHIFT_BIT_LEFT(EVT_UART_OVERSIZE_HW_FIFO)) == false){
                    uint8_t *dtmp = (uint8_t*) malloc(_uartEvent.size + 1);
                    uart_read_bytes(UART_NUM_2, dtmp, _uartEvent.size, portMAX_DELAY);
                    ProtocolErrorCode pErrCode = Protocol::DecodeFrameAndCheckCRC(dtmp,_uartEvent.size);
                    if(pErrCode == PROTOCOL_ERR_OK){
                        FrameData fd = Protocol::GetFrameDataInfo();
                        ESP_LOGI("Frame info","payloadLength:%d, GetSet:%d, crc16:%d, ID:%d",fd.payloadLength,fd.getSetFlag,fd.crc16,fd.protocolID);
                        Protocol::ResetFrame();
                        free(dtmp);
                    }
                    else if(pErrCode == PROTOCOL_ERR_CRC_FAIL){
                        ESP_LOGE("Frame info","CRC fail");
                        Protocol::ResetFrame();
                        free(dtmp);
                    }
                    else if(pErrCode == PROTOCOL_ERR_FRAME_ERROR){
                        ESP_LOGE("Frame info","Frame error");
                        Protocol::ResetFrame();
                        free(dtmp);
                    }

                } 
                else {
                    // Đang ở chế độ nhận dữ liệu lớn
                    xEventGroupSetBits(_evgExtendEventUART,EVT_UART_OVERSIZE_HW_FIFO);
                    // Cấp phát vùng nhớ để nhận từng khối 120 byte
                    uint8_t *dtmp = (uint8_t*) malloc(_uartEvent.size);
                    uart_read_bytes(UART_NUM_0, dtmp, _uartEvent.size, portMAX_DELAY);
                    /*
                        Con trỏ dtmp tới vùng nhớ chứa dữ liệu sẽ được lưu vào queue qPtrBigSize như một item
                        khi nhận xong toàn bộ dữ liệu thì sẽ tạo một task và đẩy hết số item (con trỏ trỏ tới
                        vùng nhớ được cấp phát cho mỗi khối dữ liệu 120 byte) để ghép thành khối dữ liệu lớn
                    */ 
                    xQueueSend(qPtrBigSize,(void*)dtmp,2/portTICK_PERIOD_MS);
                    _countQueueBigSize++;
                    if(_countQueueBigSize >= QUEUE_SIZE_OF_QBIG_SIZE){
                        ESP_LOGI("BigSize","Message over queue size");
                        while(1);
                    }
                }

                break;
            default: 
            break;
        }
    }

    void SetTarget(uart_port_t port){
        _uartTarget = port;
    }

    void SendData(uart_port_t port){
        uint8_t *s;
        if(xQueueReceive(qTxMesg, (void*)&s, (TickType_t)10/portTICK_PERIOD_MS)) {
            // uart_write_bytes(port,s,strlen(s));
            free(s);
        }
    }

};

#endif
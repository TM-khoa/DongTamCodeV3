#include "MessageHandle.h"
#include "../BoardParameter.h"
#include "../GUI/GUI_Navigator.h"
#include "UART.h"
#include "esp_log.h"

MessageHandle mesg;
extern BoardParameter brdParam;
extern EventGroupHandle_t evgGUI;
void ErrorMessage(ProtocolErrorCode err){mesg.HandleErrorMessage(err);}
void HandleReceiveMessage(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolListID id, GetSetFlag getSetFlag){
    FrameData fd = mesg.GetFrameDataInfo();
    // ESP_LOGI("Receive Message", "totalLength:%u,ID:%d,GetSetFlag:%d",fd.totalLength,id,getSetFlag);

    if(id == PROTOCOL_ID_HANDSHAKE){
        // ESP_LOGI("HandShake","Receive");
        mesg.SetHandshake(true);   
    } else if (mesg.IsHandshake() == false) {
        // ESP_LOGW("HandShake","Send Request");
        mesg.TransmitMessage(mesg.GetPortCurrent(),PROTOCOL_ID_HANDSHAKE,GET_DATA_FROM_THIS_DEVICE);
        return;
    }
    switch (id){
    
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
    {
        if(getSetFlag == SET_DATA_TO_THIS_DEVICE){         
            mesg.GetValueFromPayload(inputBuffer,sizeOfInputBuffer);
            PressureTwoSensorValue p = mesg.GetTwoSensorvalue();
            brdParam.SetPressureAMS5915(p.sensorAMS5915);
            brdParam.SetPressureSP100(p.sensorSP100);
            ESP_LOGI("Pressure","AMS5915:%.2f, SP100:%.2f",p.sensorAMS5915,p.sensorSP100);
        }
    }
        break;
    case PROTOCOL_ID_RTC_TIME:
    {
        if(getSetFlag == SET_DATA_TO_THIS_DEVICE){
            mesg.GetValueFromPayload(inputBuffer,sizeOfInputBuffer);
            RTC_t t = brdParam.GetRTC();
            ESP_LOGI("RTC","h:%u, m:%u, s:%u, d:%u, mth:%u, y:%u",t.hour,t.minute,t.second,t.day,t.month,t.year);
        }
    }
        break;
    default:
        break;
    }
    xEventGroupSetBits(evgGUI,SHIFT_BIT_LEFT(GUI_EVT_UPDATE_VALUE_FROM_UART));
}

void MessageHandle::Begin(){
    RegisterArgument(brdParam.GetValueAddress(PARAM_TOTAL_VALVE),sizeof(uint16_t),PROTOCOL_ID_TOTAL_VALVE);
    RegisterArgument((void*)&_pressure,sizeof(PressureTwoSensorValue),PROTOCOL_ID_PRESSURE);
    RegisterArgument((void*)brdParam.GetAddrRTC(),sizeof(RTC_t),PROTOCOL_ID_RTC_TIME);
    RegisterArgument((void*)&handshakeCode, 0, PROTOCOL_ID_HANDSHAKE);
    RegisterReceivedCallbackEvent(&HandleReceiveMessage);
    RegisterErrorEvent(&ErrorMessage);
}




void TaskUART(void *pvParameters)
{
    mesg.Begin();
    while(1){
        mesg.WaitForEventUART();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

// void TaskUartHandleBigSize(void *pvParameters){
//     uint8_t *queueAreWaiting = (uint8_t*)pvParameters;
//     ESP_LOGI("TaskUartHandleBigSize","Created, total queue:%u",*queueAreWaiting);
//     uint8_t *blockMessage = (uint8_t *)calloc(sizeof(char),((*queueAreWaiting)*120 + 70));
//     // Nếu không cấp phát được vùng nhớ
//     if(blockMessage == NULL){

//     }


//     while(1){
//         if(xQueueReceive(qUART_BigSize,&a,10/portTICK_PERIOD_MS)){
//             strcat(s,a);
//             free(a);
//             *QueueAreWaiting-=1;  
//             if(!*QueueAreWaiting) strcat(s,"\0");
//         }
//         EventBits_t e = xEventGroupGetBits(evgUART);
//         // Check event to avoid send queue (only send queue once) after this task had been deleted
//         if(!*QueueAreWaiting && !CHECKFLAG(e,EVT_UART_DELETE_TASK_BIG_SIZE)){
//             ESP_LOGI("TaskUartHandleBigSize","All queue item had been received, about to free task");
//             // ESP_LOGI("TaskUartHandleBigSize ","Send:%p",s);
//             xQueueSend(QUEUE_RX,(void*)&s,2/portTICK_PERIOD_MS);
//             xEventGroupSetBits(evgUART,EVT_UART_DELETE_TASK_BIG_SIZE);
//             vTaskDelete(TaskHandleBigSize);
//         }
//     }

// }
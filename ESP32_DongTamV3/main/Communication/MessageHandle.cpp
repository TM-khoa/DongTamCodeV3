#include "MessageHandle.h"
#include "../BoardParameter.h"
#include "../OnlineManage/OnlineManage.h"
#include "../GUI/GUI_Navigator.h"
#include "../GUI/GUI.h"
#include "UART.h"
#include "esp_log.h"

MessageHandle mesg;
extern BoardParameter brdParam;
void ErrorMessage(ProtocolErrorCode err){mesg.HandleErrorMessage(err);}
void HandleReceiveMessage(uint8_t *inputBuffer, uint16_t sizeOfInputBuffer, ProtocolID id, GetSetFlag getSetFlag){
    // FrameData fd = mesg.GetFrameDataInfo();
    // ESP_LOGI("Receive Message", "totalLength:%u,ID:%d,GetSetFlag:%d",fd.totalLength,id,getSetFlag);

    if(id == PROTOCOL_ID_HANDSHAKE){
        // ESP_LOGI("HandShake","Receive");
        mesg.SetHandshake(true);   
        mesg.LoadSettingParam(
            *(uint16_t*)brdParam.GetValueAddress(PARAM_TOTAL_VALVE),
            *(uint16_t*)brdParam.GetValueAddress(PARAM_PULSE_TIME),
            *(uint16_t*)brdParam.GetValueAddress(PARAM_INTERVAL_TIME),
            *(uint16_t*)brdParam.GetValueAddress(PARAM_CYCLE_INTERVAL_TIME)
        );
        ESP_LOGI("MesgHandle","SendSettingParam, totalValve: %u, pulseTime: %u,intervalTime: %u, cycleIntervalTime: %u,",
        *(uint16_t*)brdParam.GetValueAddress(PARAM_TOTAL_VALVE),
        *(uint16_t*)brdParam.GetValueAddress(PARAM_PULSE_TIME),
        *(uint16_t*)brdParam.GetValueAddress(PARAM_INTERVAL_TIME),
        *(uint16_t*)brdParam.GetValueAddress(PARAM_CYCLE_INTERVAL_TIME));
        mesg.TransmitMessage(UART_NUM_2,PROTOCOL_ID_SETTING_PARAMS,SET_DATA_TO_THIS_DEVICE);
        vTaskDelay(10/portTICK_PERIOD_MS);
        bool onProcessStatus = brdParam.GetOnProcessValveStatus();
        mesg.TransmitMessage(UART_NUM_2,PROTOCOL_ID_ON_PROCESS,SET_DATA_TO_THIS_DEVICE,(void*)&onProcessStatus,sizeof(onProcessStatus));
    } 
    else if (mesg.IsHandshake() == false) {
        // ESP_LOGW("HandShake","Send Request");
        mesg.TransmitMessage(mesg.GetCurrentPort(),PROTOCOL_ID_HANDSHAKE,GET_DATA_FROM_THIS_DEVICE);
        return;
    }
    switch (id){
    case PROTOCOL_ID_ON_PROCESS:
        if(getSetFlag == SET_DATA_TO_THIS_DEVICE){
            bool isOnProcessStatus;
            mesg.GetValueFromPayload((void*)&isOnProcessStatus,(uint8_t)sizeof(bool));
            brdParam.IsOnProcessValve(isOnProcessStatus);
        }
        break;
    case PROTOCOL_ID_VALVE:
        if(getSetFlag == SET_DATA_TO_THIS_DEVICE){
            mesg.GetValueFromPayload(inputBuffer,sizeOfInputBuffer);
            ValveData vData = mesg.GetValveData();
            ESP_ERROR_CHECK(brdParam.SetCurrentValveTrigger(vData.currentValveTrigger));
            brdParam.SetValveStatus(vData.valveStatus);
            // Những thông số cần hiển thị lên màn hình thì mới set event
            GUI_SetEvent(GUI_EVT_UPDATE_VALUE_FROM_UART);
            // Khi nào van kích thì mới gửi thông tin lên server
            OnlineManage_SetEvent(ONLEVT_SEND_DATA_TO_SERVER);
        }
        break;
    case PROTOCOL_ID_PRESSURE:
    {
        if(getSetFlag == SET_DATA_TO_THIS_DEVICE){         
            mesg.GetValueFromPayload(inputBuffer,sizeOfInputBuffer);
            PressureTwoSensorValue p = mesg.GetTwoSensorvalue();
            brdParam.SetPressureAMS5915(p.sensorAMS5915);
            brdParam.SetPressureSP100(p.sensorSP100);
            // Những thông số cần hiển thị lên màn hình thì mới set event
            GUI_SetEvent(GUI_EVT_UPDATE_VALUE_FROM_UART);
            // ESP_LOGI("Pressure","AMS5915:%.2f, SP100:%.2f",p.sensorAMS5915,p.sensorSP100);
        }
    }
        break;
    case PROTOCOL_ID_RTC_TIME:
    {
        if(getSetFlag == SET_DATA_TO_THIS_DEVICE){
            mesg.GetValueFromPayload(inputBuffer,sizeOfInputBuffer);
            // Những thông số cần hiển thị lên màn hình thì mới set event
            GUI_SetEvent(GUI_EVT_UPDATE_VALUE_FROM_UART);
            OnlineManage_SetEvent(ONLEVT_RTC_RECEIVED_FROM_STM32);
            GUI_SetEvent(GUI_EVT_UPDATE_VALUE_FROM_UART);
            // RTC_t t = brdParam.GetRTC();
            // ESP_LOGI("RTC","h:%u, m:%u, s:%u, d:%u, mth:%u, y:%u",t.hour,t.minute,t.second,t.day,t.month,t.year);
        }
    }
        break;
    default:
        break;
    }
    
}

void MessageHandle::Begin(){
    RegisterArgument((void*)&_pressure,sizeof(PressureTwoSensorValue),(ProtocolID)PROTOCOL_ID_PRESSURE);
    RegisterArgument((void*)brdParam.GetAddrRTC(),sizeof(RTC_t),(ProtocolID)PROTOCOL_ID_RTC_TIME);
    RegisterArgument((void*)&_handshakeCode, sizeof(_handshakeCode), (ProtocolID)PROTOCOL_ID_HANDSHAKE);
    RegisterArgument((void*)&_vData,sizeof(ValveData),(ProtocolID)PROTOCOL_ID_VALVE);
    RegisterArgument((void*)&_settingParams,sizeof(_settingParams),(uint8_t)PROTOCOL_ID_SETTING_PARAMS);
    RegisterReceivedCallbackEvent(&HandleReceiveMessage);
    RegisterErrorEvent(&ErrorMessage);
}




void TaskUART(void *pvParameters)
{
    mesg.Begin();
    vTaskDelay(10/portTICK_PERIOD_MS);// wait STM32 init done
    bool onProcessStatus = brdParam.GetOnProcessValveStatus();
    mesg.TransmitMessage(UART_NUM_2,PROTOCOL_ID_ON_PROCESS,SET_DATA_TO_THIS_DEVICE,(void*)&onProcessStatus,sizeof(onProcessStatus));
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
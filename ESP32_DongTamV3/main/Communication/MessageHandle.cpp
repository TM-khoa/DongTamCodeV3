#include "MessageHandle.h"
#include "../BoardParameter.h"
#include "UART.h"
#include "esp_log.h"

MessageHandle mesg;
extern BoardParameter brdParam;

void MessageHandle::Begin(){
    RegisterArgument(brdParam.GetValueAddress(PARAM_TOTAL_VALVE),sizeof(uint16_t),PROTOCOL_ID_TOTAL_VALVE);
    RegisterReceivedCallbackEvent(&HandleReceivedMessage);
    RegisterErrorEvent(&HandleErrorMessage);
}



void HandleErrorMessage(ProtocolErrorCode err)
{
    ESP_LOGE("ErrMesg","%d",err);
    while(1);
}

void HandleReceivedMessage(ProtocolListID id, GetSetFlag getSetFlag)
{
    mesg.HandleReceiveMessage(id,getSetFlag);
}

void TaskUART(void *pvParameters)
{
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
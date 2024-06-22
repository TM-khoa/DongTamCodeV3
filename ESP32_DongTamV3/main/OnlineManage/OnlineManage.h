#ifndef _ONLINE_MANAGE_H_
#define _ONLINE_MANAGE_H_
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "time.h"
#include "../main.h"
#include "wifi_manager.h"
#include "../BoardParameter.h"
#include "../Communication/MessageHandle.h"
#include "POSTGET.h"


typedef enum OnlineEvent{
    ONLEVT_STA_GOT_IP,
    ONLEVT_DISCONNECT,
    ONLEVT_TIME_SYNC,
    ONLEVT_RTC_RECEIVED_FROM_STM32,
    ONLEVT_SEND_DATA_TO_SERVER,
}OnlineEvent;

#define URL_POST_IOTVISION_DONGTAM "http://app.iotvision.vn/api/DongTam_DuLieu"

void TaskOnlineManager(void *pvParameter);
esp_err_t OnlineManage_RequestSyncTimeFromServerNTP();
bool OnlineManage_CheckEvent(OnlineEvent evt);
void OnlineManage_SetEvent(OnlineEvent evt);
TaskHandle_t* OnlineManage_GetOnlineTaskHandle();
char* OnlineManage_GetStationSSID();
HTTP_CODE_e OnlineManage_GetCodeHTTP();

class OnlineManage {
private:
    bool isAllowSyncTime = false;
public:
    void Init(); 

    void AllowSyncTime(){isAllowSyncTime = true;}
    void SyncTimeNTP()
    {
        if(isAllowSyncTime == false) return;
        if(esp_netif_sntp_sync_wait(1000/portTICK_PERIOD_MS) == ESP_OK){
            ESP_LOGI("SyncTime", "wait sync");
            isAllowSyncTime = false;
        }
    }
};

#endif
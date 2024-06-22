#include "OnlineManage.h"
#include "math.h"
#include "stdlib.h"
#include "../GUI/PressureBar.h"
#include "../GUI/GUI.h"



static OnlineManage onlmng;
static EventGroupHandle_t evgOnline;
static TaskHandle_t taskOnlineHandle;
void WifiGetStationIP_Callback(void *pvParameter);
static void WifiDisconnectCallback(void *pvParameter);
static void TimeServerSyncNotifyCallback(struct timeval *tv);
static void SendTimeSyncToSTM32();
static void SendDataToServer();
static HTTP_CODE_e http_code;
static char stationSSID[32] = {0};
extern BoardParameter brdParam;
extern MessageHandle mesg;

void TaskOnlineManager(void *pvParameter)
{
    onlmng.Init();
    for(;;){
        onlmng.SyncTimeNTP();
        SendTimeSyncToSTM32();
        SendDataToServer();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}


void OnlineManage::Init() {
    esp_sntp_config_t sntpConfig = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    sntpConfig.sync_cb = &TimeServerSyncNotifyCallback;
    ESP_ERROR_CHECK(esp_netif_sntp_init(&sntpConfig));
    evgOnline = xEventGroupCreate();
    OnlineManage_RequestSyncTimeFromServerNTP();
    wifi_manager_start();
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP,&WifiGetStationIP_Callback);
    wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED,&WifiDisconnectCallback);
}

static void SendTimeSyncToSTM32()
{
    EventBits_t e = xEventGroupGetBits(evgOnline);
    if(CHECKFLAG(e,ONLEVT_TIME_SYNC) == false 
    || CHECKFLAG(e,ONLEVT_RTC_RECEIVED_FROM_STM32) == false
    || mesg.IsHandshake() == false) return;
    RTC_t rtc = brdParam.GetRTC();
    time_t sntpTimeNow = 0;
    struct tm timeCalendarInfo;
    ESP_LOGI("Before send RTC to STM32", "hour:%u, min:%u, sec:%u, weekday:%u, day:%u, month:%u, year:%u"
    ,rtc.hour
    ,rtc.minute
    ,rtc.second
    ,rtc.weekday
    ,rtc.day
    ,rtc.month
    ,rtc.year);

    // Set timezone to IndoChina Time
    setenv("TZ", "ICT-7", 1);
    tzset();
    time(&sntpTimeNow);
    localtime_r(&sntpTimeNow,&timeCalendarInfo);
    if((uint8_t)timeCalendarInfo.tm_hour != rtc.hour
    || (uint8_t)timeCalendarInfo.tm_min != rtc.minute
    || (abs((uint8_t)timeCalendarInfo.tm_sec - rtc.second) > 50)
    || (uint8_t)timeCalendarInfo.tm_mday != rtc.day
    || (uint8_t)timeCalendarInfo.tm_mon + 1 != rtc.month
    || (uint8_t)timeCalendarInfo.tm_year % 100 != rtc.year
    || (uint8_t)timeCalendarInfo.tm_wday != rtc.weekday
    ) {
        rtc.hour = (uint8_t)timeCalendarInfo.tm_hour;
        rtc.minute = (uint8_t)timeCalendarInfo.tm_min;
        rtc.second = (uint8_t)timeCalendarInfo.tm_sec;
        rtc.weekday = (uint8_t)timeCalendarInfo.tm_wday;
        rtc.day = (uint8_t)timeCalendarInfo.tm_mday;
        rtc.month = (uint8_t)timeCalendarInfo.tm_mon + 1;
        rtc.year = (uint8_t)timeCalendarInfo.tm_year % 100;
        ESP_LOGI("Send RTC to STM32", "hour:%u, min:%u, sec:%u, weekday:%u, day:%u, month:%u, year:%u"
        ,rtc.hour
        ,rtc.minute
        ,rtc.second
        ,rtc.weekday
        ,rtc.day
        ,rtc.month
        ,rtc.year);
        mesg.TransmitMessage(UART_NUM_2,(ProtocolID)PROTOCOL_ID_RTC_TIME,SET_DATA_TO_THIS_DEVICE,(void*)&rtc,sizeof(rtc));
    }
    xEventGroupClearBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_TIME_SYNC));
}

static void SendDataToServer() 
{
    EventBits_t e = xEventGroupGetBits(evgOnline);
    if(CHECKFLAG(e,ONLEVT_STA_GOT_IP) == false
    || CHECKFLAG(e,ONLEVT_SEND_DATA_TO_SERVER) == false) return;
    uint16_t *dpHigh = (uint16_t*)brdParam.GetValueAddress(PARAM_DP_HIGH);
    uint16_t *dpLow = (uint16_t*)brdParam.GetValueAddress(PARAM_DP_LOW);
    float pAMS5915 = brdParam.GetPressureAMS5915();
    uint8_t currentValve = brdParam.GetCurrentValveTrigger();
    uint16_t valveStatus = brdParam.GetValveStatus();
    PressureBar pBar;
    uint8_t pressureLevel = pBar.CalculateLevelFromPressure(*dpHigh,*dpLow,pAMS5915);
    char s[300] = {0};
    RTC_t t = brdParam.GetRTC();
    snprintf(s,300,  
    "{'IMEI': \"AC67B2F6E568\", 'Power':1, 'FAN':1, 'ODCMode':1, 'ValveError':%d, 'VanKich':%u, 'DeltaPH':%d, 'DeltaP':%.2f,'DeltaPL':%d,'LED10Bar':%u,'RTC': \"%d/%d/%d %d:%d:%d\"}",
    valveStatus > 0 ? 1 : 0,
    currentValve,
    *dpHigh,
    pAMS5915,
    *dpLow,
    pressureLevel,
    t.day, t.month, t.year + 2000, t.hour, t.minute, t.second);
    http_code = http_post((char*)URL_POST_IOTVISION_DONGTAM,s);
    if(http_code != HTTP_OK) ESP_LOGE("HTTP Send","Err:%d", http_code);
    xEventGroupClearBits(evgOnline,ONLEVT_SEND_DATA_TO_SERVER);
    vTaskDelay(3000/portTICK_PERIOD_MS);
}

esp_err_t OnlineManage_RequestSyncTimeFromServerNTP()
{
    //Nếu không có kết nối wifi thì return lỗi
    if(OnlineManage_CheckEvent(ONLEVT_STA_GOT_IP) == false) return ESP_ERR_WIFI_NOT_CONNECT;
    //Xóa cờ báo đồng bộ thời gian (phòng trường hợp gửi thời gian xuống stm32 nhưng không xóa cờ), cờ này sẽ được set khi có callback đồng bộ thời gian
    xEventGroupClearBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_TIME_SYNC));
    onlmng.AllowSyncTime();
    return ESP_OK;
}

char* OnlineManage_GetStationSSID()
{
    wifi_config_t *wifiStation = wifi_manager_get_wifi_sta_config();
    strcpy(stationSSID,(const char*)wifiStation->sta.ssid);
    return stationSSID; 
}

HTTP_CODE_e OnlineManage_GetCodeHTTP()
{
    return http_code;
}

bool OnlineManage_CheckEvent(OnlineEvent evt)
{
    EventBits_t e = xEventGroupGetBits(evgOnline);
    return CHECKFLAG(e,evt);
}
void OnlineManage_SetEvent(OnlineEvent evt){
    xEventGroupSetBits(evgOnline,SHIFT_BIT_LEFT(evt));
}


void WifiGetStationIP_Callback(void *pvParameter)
{
    
    xEventGroupSetBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_STA_GOT_IP));
    xEventGroupClearBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_DISCONNECT));
}

static void WifiDisconnectCallback(void *pvParameter)
{
    xEventGroupSetBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_DISCONNECT));
    xEventGroupClearBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_STA_GOT_IP));
}

static void TimeServerSyncNotifyCallback(struct timeval *tv)
{
    xEventGroupSetBits(evgOnline,SHIFT_BIT_LEFT(ONLEVT_TIME_SYNC));
}

TaskHandle_t* OnlineManage_GetOnlineTaskHandle(){return &taskOnlineHandle;}
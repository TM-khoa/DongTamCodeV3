#include "GUI.h"

PressureBar pBar;
StatusLED ledError(LED_ERROR_MASK),ledStatus(LED_STATUS_MASK);

void TaskManageGUI(void *pvParameter)
{
    for(;;){
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void InitGUI()
{
    pBar.Begin();
    // Cần phải gọi pBar.Begin(); trước vì class PressureBar có khai báo chân 74HC595, cần thiết để xuất LED status
    TestLedStatus(ledError,ledStatus,5,100);
    ESP_LOGI("PressureBar","init");
}
#include "GUI.h"

PressureBar pBar;

void TaskManageGUI(void *pvParameter)
{
    for(;;){
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void InitGUI()
{
    pBar.Begin();
    ESP_LOGI("PressureBar","init");
}
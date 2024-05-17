#include "GUI.h"

PressureBar pBar;
StatusLED ledError(LED_ERROR_MASK),ledStatus(LED_STATUS_MASK);
ButtonGUI btnGUI;

void TaskManageGUI(void *pvParameter)
{
    EventBits_t e;
    for(;;){
        if(xTaskNotifyWait(pdFALSE,pdTRUE,&e,10/portTICK_PERIOD_MS)){

        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void InitGUI()
{
    /***********************************************Init section************************************************/
    pBar.Begin(); 
    btnGUI.Begin();
    /***********************************************Test section************************************************/

    // Cần phải gọi pBar.Begin(); trước vì class PressureBar có khai báo chân 74HC595, cần thiết để xuất LED status
    TestLedStatus(ledError,ledStatus,5,100);
    // Chạy vòng lặp không thoát trong phần test nút nhấn GUI, chỉ dùng để test
    btnGUI.TestScanBtn();
}
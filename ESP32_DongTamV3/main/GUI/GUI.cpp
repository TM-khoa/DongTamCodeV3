#include "GUI.h"

PressureBar pBar;
StatusLED ledError(LED_ERROR_MASK),ledStatus(LED_STATUS_MASK);
ButtonGUI btnGUI;
ClassLCDI2C lcdI2C;

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
    lcdI2C.begin();
    /***********************************************Test section************************************************/

    // Cần phải gọi pBar.Begin(); trước vì class PressureBar có khai báo chân 74HC595, cần thiết để xuất LED status
    TestLedStatus(ledError,ledStatus,5,100);
    // Chạy vòng lặp không thoát trong phần test nút nhấn GUI, chỉ dùng để test
    // btnGUI.TestScanBtn();

    lcdI2C.TurnOnBackLight();
    lcdI2C.print("DongTamV3",0,0);
    lcdI2C.print("DongTamV3",0,1);
    lcdI2C.print("DongTamV3",0,2);
    lcdI2C.print("DongTamV3",0,3);
}
#include "GUI.h"

PressureBar pBar;
StatusLED ledError(LED_ERROR_MASK),ledStatus(LED_STATUS_MASK);
extern ButtonGUI btnGUI;
ClassLCDI2C lcdI2C;
TaskHandle_t taskHandleGUI;

/**
 * @brief Dùng để nhận thông tin từ các task khác thông qua phương pháp TaskNotify
 * Đây là đối số truyền vào khi khởi tạo GUITask dùng hàm xTaskCreate
 * 
 * @return TaskHandle_t* 
 */
TaskHandle_t* GUI_GetTaskHandle(){ return &taskHandleGUI;}

void TaskManageGUI(void *pvParameter)
{
    EventBits_t e;
    for(;;){
        if(xTaskNotifyWait(pdFALSE,pdTRUE,&e,10/portTICK_PERIOD_MS)){

        }
    }
}

void InitGUI()
{
    /***********************************************Init section************************************************/
    pBar.Begin(); 
    
    lcdI2C.begin();
    lcdI2C.TurnOnBackLight();
    /***********************************************Test section************************************************/

    // Cần phải gọi pBar.Begin(); trước vì class PressureBar có khai báo chân 74HC595, cần thiết để xuất LED status
    TestLedStatus(ledError,ledStatus,5,100);
    // Chạy vòng lặp không thoát trong phần test nút nhấn GUI, chỉ dùng để test
    // btnGUI.TestScanBtn();

    lcdI2C.print("DongTamV3",0,0);
    lcdI2C.print("DongTamV3",0,1);
    lcdI2C.print("DongTamV3",0,2);
    lcdI2C.print("DongTamV3",0,3);
}
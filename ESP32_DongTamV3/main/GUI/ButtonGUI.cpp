#include "ButtonGUI.h"
#include "GUI.h"

extern GUI_Manager gui;

void TaskScanButton(void *pvParameter){
    while(1){
        gui.Read(BTN_MENU,EVT_BTN_MENU);
        gui.Read(BTN_SET,EVT_BTN_SET);
        gui.Read(BTN_UP,EVT_BTN_UP);
        gui.Read(BTN_DOWN_RIGHT,EVT_BTN_DOWN_RIGHT);
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}





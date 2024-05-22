#include "ButtonGUI.h"
#include "GUI.h"

extern GUI_Manager gui;

void TaskScanButton(void *pvParameter){
    while(1){
        gui.Read(BTN_MENU,SHIFT_BIT_LEFT(EVT_BTN_MENU));
        gui.Read(BTN_SET,SHIFT_BIT_LEFT(EVT_BTN_SET));
        gui.Read(BTN_UP,SHIFT_BIT_LEFT(EVT_BTN_UP));
        gui.Read(BTN_DOWN_RIGHT,SHIFT_BIT_LEFT(EVT_BTN_DOWN_RIGHT));
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}





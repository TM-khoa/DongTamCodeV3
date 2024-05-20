#ifndef __GUI_H__
#define __GUI_H__

#include "esp_err.h"
#include "esp_check.h"
#include "PressureBar.h"
#include "StatusLED.h"
#include "ButtonGUI.h"
#include "i2cdev.h"
#include "PCF8574.hpp"
#include "LCD_I2C.hpp"
#include "freertos/FreeRTOS.h"



class GUI_Manager
{
private:
    /* private methods */
    void ClearPointer();
    void ShowPointer();
    void LoadPageInit();
    /* data */

public:

};


void TaskManageGUI(void *pvParameter);
TaskHandle_t* GUI_GetTaskHandle();
void InitGUI();
#endif
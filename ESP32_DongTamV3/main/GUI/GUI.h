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
#include "GUI_Navigator.h"



class GUI_Manager : public ClassLCDI2C, public GUI_Navigator, public PressureBar, public StatusLED, public ButtonGUI
{
private:
    /* private methods */
    /* data */
    Parameter_t _paramDisplayBuffer[4];
public:

    /**
     * @brief Lấy toàn bộ thông tin của struct Parameter_t từ BoardParameter nạp vào 
     * @param param 
     * @param index 
     */
    esp_err_t LoadToBufferGUI(Parameter_t param, uint8_t index){
        if(index > 4) return ESP_ERR_INVALID_ARG;
        _paramDisplayBuffer[index] = param;
        return ESP_OK;
    }

    /**
     * @brief Xóa con trỏ màn hình
     */
    void ClearPointer(){
        uint8_t px = GUI_Navigator::GetPx();
        uint8_t py = GUI_Navigator::GetPy();
        ClassLCDI2C::print(" ",px,py);
    }

    /**
     * @brief Hiển thị con trỏ màn hình
     */
    void ShowPointer(){
        uint8_t px = GUI_Navigator::GetPx();
        uint8_t py = GUI_Navigator::GetPy();
        ClassLCDI2C::print(">",px,py);
    }

    void Begin(TaskHandle_t taskHandleGUI, EventGroupHandle_t evgGUI, uint8_t maxParamMappingDisplay){
        ButtonGUI::Begin(taskHandleGUI);
        GUI_Navigator::Begin(maxParamMappingDisplay,evgGUI);
        ClassLCDI2C::begin();
        PressureBar::Begin();
        // Cần phải gọi PressureBar::Begin(); trước vì class PressureBar có khai báo chân 74HC595, cần thiết để xuất LED status
        StatusLED::Begin(LED_ERROR_MASK,LED_STATUS_MASK);
        StatusLED::Test(4,100);
    };

    void AllowSpeedUpValueIfPointerNowIsValue(ParamID id){
        if(GUI_Navigator::GetPointerNow() == IS_VALUE && id < PARAM_STRING_PARAM_OFFSET) ButtonGUI::AllowToSpeedUp(true);
        else ButtonGUI::AllowToSpeedUp(false);
    }

    void ResetLCD(){
        ClassLCDI2C::begin();
    }
};


void TaskManageGUI(void *pvParameter);
TaskHandle_t* GUI_GetTaskHandle();
void InitGUI();
void GUI_LoadParamsToBuffer();
#endif
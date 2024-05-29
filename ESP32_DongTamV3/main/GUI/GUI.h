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

    // Chỉ nắm giữ địa chỉ trỏ tới thông số, không tạo thêm cấu trúc dữ liệu chứa thông số đó
    Parameter_t *_paramDisplayBuffer[4];
public:

    /**
     * @brief Lấy toàn bộ thông tin của struct Parameter_t từ BoardParameter nạp vào 
     * @param param 
     * @param index 
     */
    esp_err_t LoadParamAddressToBufferGUI(Parameter_t *param, uint8_t index){
        if(index > 4) return ESP_ERR_INVALID_ARG;
        _paramDisplayBuffer[index] = param;
        return ESP_OK;
    }

    /**
     * @brief Reset toàn bộ bộ đệm sau khi có sự kiện refresh thông số để tránh trường hợp
     * tồn đọng thông số cũ mà không được load bởi thông số mới vào bộ đệm (ví dụ như vượt quá phạm vi của Mảng ánh xạ thông số
     * thì sẽ không cho phép nạp thông số mới vào bộ đệm, nếu không xóa thì sẽ in ra thông số cũ)
     */
    void ResetBufferGUI(){
        for(uint8_t i = 0; i < LCD_ROWS; i++){
            _paramDisplayBuffer[i] = NULL;
        }
    }

    void PrintParamsToLCD(){
        clear();
        vTaskDelay(20/portTICK_PERIOD_MS); // chờ xóa màn hình
        for(uint8_t i = 0; i < LCD_ROWS; i++){
            // Nếu thông số không được load thì con trỏ sẽ mang giá trị NULL
            if(_paramDisplayBuffer[i] == NULL) break;
            // Trỏ tới tên thông số
            const char *keyName = _paramDisplayBuffer[i]->keyName;
            const char *unit = NULL;
            // Tạo bộ đệm chuỗi để ghép giá trị thông số và đơn vị đo(nếu có)
            char strBuffer[20] = {0};
            print(keyName,POINTER_SLOT,i);print(":",LENGTH_OF_PARAM,i);
            if(_paramDisplayBuffer[i]->dataType == TYPE_STRING){
                // Tạo con trỏ chuỗi lấy giá trị từ con trỏ cấp 2 ép kiểu từ void* cộng với thứ tự phần tử index để trỏ tới chuỗi cần lấy giá trị
                const char *strVal = *((const char**)_paramDisplayBuffer[i]->value + _paramDisplayBuffer[i]->index);
                strcpy(strBuffer,strVal);
            } 
            else {
                // ép kiểu con trỏ void* của value về uint16_t* để lấy ra giá trị của thông số
                uint16_t *value = (uint16_t*)_paramDisplayBuffer[i]->value;
                sprintf(strBuffer,"%d",*value);
            }
            // Nếu có đơn vị đo thì ghép vào giá trị thông số
            if(_paramDisplayBuffer[i]->unit != NULL){
                unit = _paramDisplayBuffer[i]->unit;
                strcat(strBuffer,unit);
            }
            print(strBuffer,POINTER_SLOT + LENGTH_OF_PARAM + POINTER_SLOT,i);
        }
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
    /**
     * @brief Cho phép tăng tốc giá trị thông số khi nhấn giữ nút và con trỏ phải đang trỏ tới thông số có giá trị số thì mới được tăng tốc
     * @param id mã thông số kiểm tra giá trị có được phép tăng tốc hay không
     */
    void AllowSpeedUpValueIfPointerNowIsValue(ParamID id){
        if(GUI_Navigator::GetCurrentPage() != PAGE_SETTING) return;
        if(GUI_Navigator::GetPointerNow() == IS_VALUE && id < PARAM_STRING_PARAM_OFFSET) ButtonGUI::AllowToSpeedUp(true);
        else ButtonGUI::AllowToSpeedUp(false);
    }



};


void TaskManageGUI(void *pvParameter);
TaskHandle_t* GUI_GetTaskHandle();
void InitGUI();
void GUI_LoadParamsToBuffer();
#endif
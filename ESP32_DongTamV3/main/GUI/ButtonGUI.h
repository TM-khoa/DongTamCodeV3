#ifndef _BUTTON_GUI_H
#define _BUTTON_GUI_H

#include "../main.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"


#define BTN_MENU  GPIO_NUM_39
#define BTN_SET GPIO_NUM_36
#define BTN_UP  GPIO_NUM_35
#define BTN_DOWN_RIGHT GPIO_NUM_34

#define BT1_MASK (1ULL<<GPIO_NUM_36)
#define BT2_MASK (1ULL<<GPIO_NUM_39)
#define BT3_MASK (1ULL<<GPIO_NUM_34)
#define BT4_MASK (1ULL<<GPIO_NUM_35)

typedef enum EventButton{
    EVT_BTN_MENU,
    EVT_BTN_SET,
    EVT_BTN_UP,
    EVT_BTN_DOWN_RIGHT,
    EVT_BTN_LCD_RESET,
    EVT_BTN_NEXT_PAGE,
}EventButton;

class ButtonGUI{

private:
    /*********************Data*************************/

    TaskHandle_t _taskHandleGUI = NULL;
    bool isAllowToSpeedUp = false; // Chỉ cho phép tăng tốc giá trị nếu đó là số, còn nếu giá trị là chuỗi thì không được phép
    /**
     * @brief Khi người dùng nhấn giữ, liên tục gửi sự kiện với tốc độ nhanh dần cho TaskManageGUI
     * @param gpio Chân đọc nút nhấn
     * @param e Sự kiện nhấn nút gửi đi, sử dụng SET_BIT_EVENT(enum EventButton)
     */
    void DoThingWhenHoldingButton(gpio_num_t gpio, EventBits_t e){
    #define BTN_HOLD_DELAY_MAX 300
    #define BTN_HOLD_DELAY_MIN 50
    #define BTN_HOLD_DELAY_DECREASE_STEP 50
    #define DELAY_COUNT_LOOP_THRESHOLD 3
        //Delay each loop when user keep holding button
        //Quy định tốc độ thay đổi giá trị của thông số
        static uint16_t delay = BTN_HOLD_DELAY_MAX;
        /* if this number is bigger than certain threshold, 
        Delay will be decrease to speed up sending notify to GUITask
        Each time call xTaskNotify, DelayCountLoop increase by 1

        Nếu số đếm delayCountLoop lớn hơn ngưỡng DELAY_COUNT_LOOP_THRESHOLD thì sẽ giảm thời gian delay lại
        */
        static uint16_t delayCountLoop = 0;
        static uint8_t resetWhenErrorLCD_Count = 0;
        // reset lại thông số nếu thoát khỏi chế độ nhấn giữ
        if(e == 0){
            delayCountLoop = 0;
            resetWhenErrorLCD_Count = 0;
            delay = BTN_HOLD_DELAY_MAX;
        }

        /*if user keep holding down button, check button is UP or DOWN-RIGHT and if only currently selected is value 
        to make delay shorter, the rest is just delay
        Chỉ có nút Up và nút DownRight và hiện tại thông số GUI đang điều chỉnh giá trị là số (giá trị là string không được phép) 
        thay vì điều hướng các thông số thì mới được phép gửi
        */
        if((((gpio == BTN_UP) || (gpio == BTN_DOWN_RIGHT)) && isAllowToSpeedUp) 
        // && (GUINAV_GetCurrentSelected() == IS_VALUE)
        ) {
            vTaskDelay(delay/portTICK_PERIOD_MS);
            delayCountLoop += 1;
            /**
             *  if user only press BTN_UP and BTN_DOWN_RIGHT just only one,
             * interpret it as normal button and not send notify, if user holding down button and DelayCountLoop increase, 
             * this time interpret it as speed up count
             * Nếu người dùng chỉ nhấn một lần thì điều kiện này không được xem là nhấn giữ, do đó không Notify cho GUI liên tục

            */
            if(delayCountLoop > DELAY_COUNT_LOOP_THRESHOLD - 1) {
                xTaskNotify(_taskHandleGUI,e,eSetValueWithoutOverwrite);
            }
        }
         /** Button MENU should be handle to reset LCD if it is error
          * Khi nhấn giữ 5s nút Menu thì sẽ reset LCD nếu bị lỗi hiển thị  
          * @note Lưu ý không được để ESP_LOGI trong này, nếu không LCD sẽ bị lỗi ghi I2C vào PCF8574  (dự đoán có thể do tràn stack)
          * */ 
        if(gpio == BTN_MENU){
            vTaskDelay(500/portTICK_PERIOD_MS);
            resetWhenErrorLCD_Count++;
            // Wait for 5 seconds or above to send notify 
            if(resetWhenErrorLCD_Count >= 5){
                xTaskNotify(_taskHandleGUI, EVT_BTN_LCD_RESET, eSetValueWithoutOverwrite);
                resetWhenErrorLCD_Count = 0;
            }
        }
        else {
            // button SET are nothing to hanle, just delay and return
            vTaskDelay(100/portTICK_PERIOD_MS);
            return;
        }

        /**
         * Nếu chương trình quét nhiều lần đạt đến DELAY_COUNT_LOOP_THRESHOLD thì bắt đầu giảm delay
        */
        if(delayCountLoop >= DELAY_COUNT_LOOP_THRESHOLD && delay > BTN_HOLD_DELAY_MIN){
            delayCountLoop = 0;
            int temp = delay - BTN_HOLD_DELAY_DECREASE_STEP;
            // Saturate low delay speed
            if(temp < BTN_HOLD_DELAY_MIN) delay = BTN_HOLD_DELAY_MIN;
            else delay -= BTN_HOLD_DELAY_DECREASE_STEP; 
        }

    }
    /**
     * @brief Test đọc từng nút nhấn, nếu có nhấn thì in ra màn hình logMessage
     * @param gpio chân đọc nút nhấn
     * @param logMessage Thông tin in lên màn hình Terminal của máy tính khi nhấn nút
     */
    void TestRead(gpio_num_t gpio,const char* logMessage){
        if(!gpio_get_level(gpio)){
            vTaskDelay(100/portTICK_PERIOD_MS);
            ESP_LOGI("TestReadBtnGUI","%s",logMessage);
            while (!gpio_get_level(gpio))
            {
                vTaskDelay(50/portTICK_PERIOD_MS);
            }
        }
    }
    public:
    /**
     * @brief Khi có sự kiện nhấn nút thì gửi một EventBits tới TaskManageGUI thông qua Notify task handle của TaskMangeGUI
     * @param gpio Chân đọc nút nhấn
     * @param e Sự kiện nhấn nút gửi đi, sử dụng SET_BIT_EVENT(enum EventButton)
     * @example Khi nhấn nút Menu, sự kiện gửi đi - đối số truyền vào e là SET_BIT_EVENT(EVT_BTN_MENU)
     */
    void Read(gpio_num_t gpio, EventBits_t e){
        if(!gpio_get_level(gpio)){
            vTaskDelay(100/portTICK_PERIOD_MS);
            while(!gpio_get_level(gpio)){
                DoThingWhenHoldingButton(gpio,e);
            } 
            DoThingWhenHoldingButton(gpio,0);
            xTaskNotify(_taskHandleGUI,e,eSetValueWithoutOverwrite);
        }
    }
    
    void AllowToSpeedUp(bool allow){
        if(allow) isAllowToSpeedUp = true;
        else isAllowToSpeedUp = false;
    }


    /**
     * @brief Chạy liên tục trong vòng lặp không thoát để quét đọc nút nhấn
     */
    void TestScanBtn(){
        while(1){
            TestRead(BTN_MENU,"BtnMenu");
            TestRead(BTN_SET,"BtnSet");
            TestRead(BTN_UP,"BtnUp");
            TestRead(BTN_DOWN_RIGHT,"BtnDownRight");
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }

    /**
     * @brief Khởi tạo chân đọc nút nhấn từ màn hình GUI
     * @return mã lỗi esp_err_t
     */
    esp_err_t Begin(TaskHandle_t taskHandleGUI)
    {
        gpio_config_t cfg = {
        .pin_bit_mask = BT1_MASK |
                        BT2_MASK |
                        BT3_MASK|
                        BT4_MASK,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        };
        _taskHandleGUI = taskHandleGUI;
        return gpio_config(&cfg);
    }
        
};

void TaskScanButton(void *pvParameter);
#endif
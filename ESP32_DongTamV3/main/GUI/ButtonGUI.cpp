#include "ButtonGUI.h"

void TaskScanButton(void *pvParameter){
    while(1){
        // (BTN_MENU,EVT_BTN_MENU);
        // ReadGuiButton(BTN_UP,EVT_BTN_UP);
        // ReadGuiButton(BTN_DOWN_RIGHT,EVT_BTN_DOWN_RIGHT);
        // ReadGuiButton(BTN_SET,EVT_BTN_SET);
        // vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
/**
 * @brief Khi có sự kiện nhấn nút thì gửi một EventBits tới TaskManageGUI thông qua Notify task handle của TaskMangeGUI
 * @param gpio Chân đọc nút nhấn
 * @param e Sự kiện nhấn nút gửi đi, sử dụng SET_BIT_EVENT(enum EventButton)
 * @example Khi nhấn nút Menu, sự kiện gửi đi - đối số truyền vào e là SET_BIT_EVENT(EVT_BTN_MENU)
 */
void ButtonGUI::Read(gpio_num_t gpio, EventBits_t e){
    if(!gpio_get_level(gpio)){
       vTaskDelay(100/portTICK_PERIOD_MS);
       while(!gpio_get_level(gpio)){
            DoThingWhenHoldingButton(gpio,e);
       } 
    }
}

/**
 * @brief Khi người dùng nhấn giữ, liên tục gửi sự kiện với tốc độ nhanh dần cho TaskManageGUI
 * @param gpio Chân đọc nút nhấn
 * @param e Sự kiện nhấn nút gửi đi, sử dụng SET_BIT_EVENT(enum EventButton)
 */
void ButtonGUI::DoThingWhenHoldingButton(gpio_num_t gpio, EventBits_t e){

}

void ButtonGUI::TestScanBtn(){
    while(1){
        TestRead(BTN_MENU,"BtnMenu");
        TestRead(BTN_SET,"BtnSet");
        TestRead(BTN_UP,"BtnUp");
        TestRead(BTN_DOWN_RIGHT,"BtnDownRight");
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

/**
 * @brief Test đọc từng nút nhấn, nếu có nhấn thì in ra màn hình logMessage
 * @param gpio chân đọc nút nhấn
 * @param logMessage Thông tin in lên màn hình Terminal của máy tính khi nhấn nút
 */
void ButtonGUI::TestRead(gpio_num_t gpio,const char* logMessage){
    if(!gpio_get_level(gpio)){
        vTaskDelay(100/portTICK_PERIOD_MS);
        ESP_LOGI("TestReadBtnGUI","%s",logMessage);
        while (!gpio_get_level(gpio))
        {
            vTaskDelay(50/portTICK_PERIOD_MS);
        }
    }
}

/**
 * @brief Khởi tạo chân đọc nút nhấn từ màn hình GUI
 * @return mã lỗi esp_err_t
 */
esp_err_t ButtonGUI::Begin()
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
    return gpio_config(&cfg);
}
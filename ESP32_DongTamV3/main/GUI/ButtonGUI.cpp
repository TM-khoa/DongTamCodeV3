#include "ButtonGUI.cpp"

void TaskScanButton(void *pvParameter){
    while(1){
        ReadGuiButton(BTN_MENU,EVT_BTN_MENU);
        ReadGuiButton(BTN_UP,EVT_BTN_UP);
        ReadGuiButton(BTN_DOWN_RIGHT,EVT_BTN_DOWN_RIGHT);
        ReadGuiButton(BTN_SET,EVT_BTN_SET);
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

esp_err_t ButtonGUI::()
{
    gpio_config_t cfg = {
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = BT1_MASK |
                    BT2_MASK |
                    BT3_MASK|
                    BT4_MASK,
    };
    return gpio_config(&cfg);
}
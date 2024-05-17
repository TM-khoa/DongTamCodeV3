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
}EventButton;

#define SET_BIT_EVENT(_EVT_BTN_) (1 << (_EVT_BTN_))

class ButtonGUI{
    public:
    void Read(gpio_num_t gpio, EventBits_t e);
    void TestScanBtn();
    esp_err_t Begin();
    private:
    void DoThingWhenHoldingButton(gpio_num_t gpio, EventBits_t e);
    void TestRead(gpio_num_t gpio,const char* logMessage);
};
#endif
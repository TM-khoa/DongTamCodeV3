#ifndef _BUTTON_GUI_H
#define _BUTTON_GUI_H

#include "main.h"
#include "gpio.h"
#include "freertos/event_groups.h"

class ButtonGUI{
    public:
    void Read(gpio_num_t gpio, EventBits_t e);
    void TestRead();
    void HoldingHandle();
    esp_err_t Begin();
    private:


};
#endif
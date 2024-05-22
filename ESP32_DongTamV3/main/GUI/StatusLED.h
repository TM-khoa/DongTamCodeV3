#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

#include "../main.h"
#include "74HC595.h"
#define LED_ERROR_MASK 10
#define LED_STATUS_MASK 11


class StatusLED
{
private:
    /* data */
    uint8_t _ledErrorBitmask;
    uint8_t _ledStatusBitmask;
public:
    void Begin(uint32_t ledErrorBitmask, uint32_t ledStatusBitmask);
    // Hàm này cần phải khai báo chân 74HC595 ở PressureBar trước khi sử dụng vì class này không có method khởi tạo và chọn chân 74HC595
    void OutputStatusLED(bool on);
    void OutputErrorLED(bool on);
    void Test(uint8_t blinkNum, uint16_t delay);

};
#endif
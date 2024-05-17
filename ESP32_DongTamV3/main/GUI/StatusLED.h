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
    uint8_t _bitmask;
public:
    // Hàm này cần phải khai báo chân 74HC595 ở PressureBar trước khi sử dụng vì class này không có method khởi tạo và chọn chân 74HC595
    void Write(bool on);
StatusLED(uint8_t bitmask);

};



void TestLedStatus(StatusLED ledError, StatusLED ledStatus, uint8_t blinkNum, uint16_t delay);

#endif
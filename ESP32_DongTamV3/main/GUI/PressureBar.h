#ifndef PRESSURE_BAR_H_
#define PRESSURE_BAR_H_

#include "74HC595.h"
#include "freertos/FreeRTOS.h"
#define PRESSURE_BAR_MAX_BITMASK 0x01ff
#define PRESSURE_BAR_MIN_BITMASK 0x0100
#define PRESSURE_BAR_MAX_LEVEL 10

#define _74HC595_OE_MASK (1ULL<<GPIO_NUM_4)
#define _74HC595_LATCH_MASK (1ULL<<GPIO_NUM_5)
#define _74HC595_CLK_MASK (1ULL<<GPIO_NUM_18)
#define _74HC595_DS_MASK (1ULL<<GPIO_NUM_23)

class PressureBar{
private:
    HC595 _hc595;    
public:
    void Begin();
    void SetLevel(uint8_t level);
    uint8_t CalculateLevelFromPressure(uint32_t max, uint32_t min, float value);
};

#endif /* end of PRESSURE_BAR_H_ */
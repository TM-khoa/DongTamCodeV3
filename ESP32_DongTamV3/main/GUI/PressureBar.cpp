#include "PressureBar.h"
#include "math.h"

void PressureBar::SetLevel(uint8_t level){
    if(level > 10) return;
    for(uint8_t i = 0; i <= level; i++){
        HC595_SetBitOutput(10 - i);
    }
    HC595_ShiftOut(NULL,2,1);
    HC595_ClearByteOutput(PRESSURE_BAR_MAX_BITMASK);
}

uint8_t PressureBar::CalculateLevelFromPressure(uint32_t max, uint32_t min, float value){
    uint32_t step = (uint32_t)((max - min)/PRESSURE_BAR_MAX_LEVEL); 
    uint32_t a = min + step;
    if(abs(value) > max) return PRESSURE_BAR_MAX_LEVEL;
    else if(abs(value) < min) return 1;
    for(uint8_t i=2; i <= 10;i++){
		if(a > abs(value)){
			return i; // return level of pressure indicator
		} else {
			a += step;
		}
		if(a > max) return i;
		else if(a < min) return 2;
	} 
    return a;
}

void PressureBar::Begin(){
    gpio_config_t cfg = {
        .pin_bit_mask = _74HC595_DS_MASK |
                        _74HC595_OE_MASK |
                        _74HC595_CLK_MASK|
                        _74HC595_LATCH_MASK,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    HC595_AssignPin(&_hc595, GPIO_NUM_18, HC595_CLK);
	HC595_AssignPin(&_hc595, GPIO_NUM_23, HC595_DS);
	HC595_AssignPin(&_hc595, GPIO_NUM_5, HC595_LATCH);
    HC595_AssignPin(&_hc595, GPIO_NUM_4, HC595_OE);
	
    HC595_EnableOutput();
    HC595_ClearByteOutput(0xffffffff);
    HC595_ShiftOut(NULL,2,1); 
    

    for(uint8_t i=0; i < 11; i++){
        SetLevel(i);
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
    for(uint8_t i = 0; i < 11; i++){
        SetLevel(10 - i);
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}


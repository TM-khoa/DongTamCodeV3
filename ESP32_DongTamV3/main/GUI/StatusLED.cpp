#include "StatusLED.h"

void StatusLED::Begin(uint32_t ledErrorBitmask, uint32_t ledStatusBitmask){
    _ledErrorBitmask = ledErrorBitmask;
    _ledStatusBitmask = ledStatusBitmask;
}

void StatusLED::OutputStatusLED(bool on)
{
    if(on) HC595_SetBitOutput(_ledStatusBitmask);
    else HC595_ClearBitOutput(_ledStatusBitmask);
}

void StatusLED::OutputErrorLED(bool on)
{
    if(on) HC595_SetBitOutput(_ledErrorBitmask);
    else HC595_ClearBitOutput(_ledErrorBitmask);
}

void StatusLED::Test(uint8_t blinkNum, uint16_t delay)
{
    for(uint8_t j = 0; j < 2; j++){
        for(uint8_t i = 0; i < blinkNum; i++){
            OutputErrorLED(1);
            OutputStatusLED(0);
            HC595_ShiftOut(NULL,2,1);
            vTaskDelay(delay/portTICK_PERIOD_MS);
            OutputErrorLED(0);
            OutputStatusLED(1);
            HC595_ShiftOut(NULL,2,1);
            vTaskDelay((delay)/portTICK_PERIOD_MS);
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
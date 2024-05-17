#include "StatusLED.h"

StatusLED::StatusLED(uint8_t bitmask)
{
    _bitmask = bitmask;
}

void StatusLED::Write(bool on)
{
    if(on) HC595_SetBitOutput(_bitmask);
    else HC595_ClearBitOutput(_bitmask);
}

void TestLedStatus(StatusLED ledError, StatusLED ledStatus, uint8_t blinkNum, uint16_t delay)
{
    for(uint8_t i=0;i<blinkNum;i++){
        ledError.Write(1);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        ledError.Write(0);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
    }
    for(uint8_t i=0;i<blinkNum;i++){
        ledStatus.Write(1);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        ledStatus.Write(0);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        
    }
    for(uint8_t i=0;i<blinkNum;i++){
        ledStatus.Write(1);
        ledError.Write(1);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        ledStatus.Write(0);
        ledError.Write(0);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
    }
}
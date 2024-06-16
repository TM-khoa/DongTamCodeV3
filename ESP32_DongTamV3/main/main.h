#ifndef _MAIN_H_
#define _MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"


#define SHIFT_BIT_LEFT(STEP) (1UL << (STEP))
#define CHECKFLAG(FlagGroup,FlagBit) ((((FlagGroup) & (1UL << FlagBit)) == (1UL << FlagBit)) ? 1 : 0)
#define SETFLAG(FlagGroup,FlagBit) ((FlagGroup) |= (1UL << FlagBit))
#define CLEARFLAG(FlagGroup,FlagBit) (FlagGroup &= ~(1UL << FlagBit))

#define CONFIG_USE_74HC595
#define CONFIG_USE_PCF8574
#define CONFIG_USE_I2CDEV
#define CONFIG_USE_LCDI2C
#define CONFIG_USE_HD44780
#define CONFIG_USE_CPP
#define CONFIG_USE_POSTGET

#endif
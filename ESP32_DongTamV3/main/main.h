#ifndef _MAIN_H_
#define _MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

typedef enum ProtocolListID {
	PROTOCOL_ID_HANDSHAKE,
	PROTOCOL_ID_VALVE,
	PROTOCOL_ID_PULSE_TIME,
	PROTOCOL_ID_TOTAL_VALVE,
	PROTOCOL_ID_CYCLE_INTERVAL_TIME,
	PROTOCOL_ID_INTERVAL_TIME,
	PROTOCOL_ID_TRIGGER_VALVE,
	PROTOCOL_ID_RTC_TIME,
	PROTOCOL_ID_PRESSURE,
    PROTOCOL_ID_IS_ON_PROCESS,
	PROTOCOL_ID_ERROR,
	PROTOCOL_ID_END,
} ProtocolListID;


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
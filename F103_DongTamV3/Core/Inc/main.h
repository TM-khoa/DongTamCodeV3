/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum ProtocolListID {
	PROTOCOL_ID_HANDSHAKE,
	PROTOCOL_ID_VALVE,
	PROTOCOL_ID_SETTING_PARAMS, // bao gồm totalValve, pulseTime, intervalTime, cycleIntervalTime
	PROTOCOL_ID_RTC_TIME,
	PROTOCOL_ID_PRESSURE,
	PROTOCOL_ID_ON_PROCESS,
	PROTOCOL_ID_ERROR,
	PROTOCOL_ID_END,
} ProtocolListID;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define CONFIG_USE_74HC595
#define CONFIG_USE_74HC165
#define CONFIG_USE_PCF8563
#define CONFIG_USE_AMS5915

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BlueLED_Pin GPIO_PIN_13
#define BlueLED_GPIO_Port GPIOC
#define _74HC595_CLK_Pin GPIO_PIN_5
#define _74HC595_CLK_GPIO_Port GPIOA
#define _74HC595_STORE_Pin GPIO_PIN_6
#define _74HC595_STORE_GPIO_Port GPIOA
#define _74HC595_DATA_Pin GPIO_PIN_7
#define _74HC595_DATA_GPIO_Port GPIOA
#define OE_Pin GPIO_PIN_8
#define OE_GPIO_Port GPIOA
#define _74HC165_DATA_Pin GPIO_PIN_3
#define _74HC165_DATA_GPIO_Port GPIOB
#define _74HC165_CLK_Pin GPIO_PIN_4
#define _74HC165_CLK_GPIO_Port GPIOB
#define _74HC165_LOAD_Pin GPIO_PIN_5
#define _74HC165_LOAD_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

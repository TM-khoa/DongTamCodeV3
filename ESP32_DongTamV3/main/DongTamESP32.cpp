/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "GUI/GUI.h"
#include "GUI/ButtonGUI.h"
#include "GUI/PressureBar.h"
#include "BoardParameter.h"
#include "Communication/MessageHandle.h"

// #pragma warn -par /* parameter never used */ 

extern "C" void app_main(void)
{
    InitBoardParameter();
    TaskHandle_t *taskHandleGUI = GUI_GetTaskHandle();
    xTaskCreate(TaskManageGUI, "TaskGUI", 4096, NULL, 2, taskHandleGUI);
    xTaskCreate(TaskScanButton,"TaskBtnGUI",1024,NULL,1,NULL);
    xTaskCreate(TaskUART,"TaskUART",2048,NULL,3,NULL);
    while(1){
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

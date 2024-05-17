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
#include "GUI/PressureBar.h"

extern "C" void app_main(void)
{
    InitGUI();
    xTaskCreate(TaskManageGUI, "GUITask", 4096, NULL, 2, NULL);
    while(1){
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

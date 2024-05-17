#ifndef __GUI_H__
#define __GUI_H__

#include "esp_err.h"
#include "esp_check.h"
#include "PressureBar.h"
#include "StatusLED.h"

void TaskManageGUI(void *pvParameter);
void InitGUI();
#endif
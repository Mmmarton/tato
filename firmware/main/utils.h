#ifndef _UTILS_H
#define _UTILS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define _delay_ms(x) (vTaskDelay(x / portTICK_PERIOD_MS))

#endif
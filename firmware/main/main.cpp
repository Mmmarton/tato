#include "delay.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>

#define VALVE_PIN GPIO_NUM_26

static const char *TAG = "Main";

extern "C" {
void app_main(void);
}

void setupValve(void);
void openValve(void);
void closeValve(void);

void app_main(void) {

    setupValve();

    while (1) {
        closeValve();
        delay(5000);
        openValve();
        delay(1000);
    }
}

void setupValve(void) {
    gpio_set_direction(VALVE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(VALVE_PIN, 1);
}

void openValve(void) {
    gpio_set_level(VALVE_PIN, 0);
}

void closeValve(void) {
    gpio_set_level(VALVE_PIN, 1);
}
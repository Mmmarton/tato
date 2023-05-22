#include "delay.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "tcp/tcp.h"
#include "wifi/wifi.h"
#include <stdint.h>
#include <string.h>

#define VALVE_PIN GPIO_NUM_26

#define COMMAND_REQUEST_ID 1
#define COMMAND_SET_ID     2

static const char *TAG = "Main";

extern "C" {
void app_main(void);
}

void     setupValve(void);
void     openValve(void);
void     closeValve(void);
void     saveId(uint16_t id);
uint16_t loadId(void);
uint16_t getId(void);

void app_main(void) {
    ESP_LOGI(TAG, "Start application");
    nvs_flash_init();
    setupValve();
    WIFI::init("Oppai", "nem68akarok92menni");
    TCP::init("192.168.1.103", 8082);

    uint16_t id = loadId();
    if (!id) {
        id = getId();
        saveId(id);
    }

    printf("%d\n", id);

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

void saveId(uint16_t id) {
    nvs_handle_t nvs;
    nvs_open("storage", NVS_READWRITE, &nvs);
    nvs_set_u8(nvs, "isConfigured", 19);
    nvs_set_u16(nvs, "id", id);
    nvs_close(nvs);
}

uint16_t loadId(void) {
    uint8_t      isConfigured;
    uint16_t     id;
    nvs_handle_t nvs;
    nvs_open("storage", NVS_READWRITE, &nvs);
    nvs_get_u8(nvs, "isConfigured", &isConfigured);
    nvs_get_u16(nvs, "id", &id);
    nvs_close(nvs);

    if (isConfigured == 19) {
        return id;
    }
    else {
        return 0;
    }
}

uint16_t getId(void) {
    while (1) {
        char request[] = {COMMAND_REQUEST_ID, 0};
        TCP::write(request);

        char countDown = 100;
        while (countDown--) {
            char    response[10];
            uint8_t length = 0;
            TCP::read(response, length);

            if (length && response[0] == COMMAND_SET_ID) {
                return (uint16_t)response[1] << 8 | response[2];
            }
            delay(10);
        }
    }
}
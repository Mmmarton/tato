#include "badge_defines.h"
#include "error_manager.h"
#include "log.h"
#include "spi_driver.h"
#include "utilities.h"

#include <esp_sleep.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "configuration_manager.h"
#include "dwm_driver.h"
#include "dwm_manager.h"

#define MODULE_TAG         "STRESS_TESTING"
#define DEVICE_UUID_LENGTH 16
#define BADGES_COUNT       50
#define BLINK_DELAY        250

static bool                   is_init = false;
static device_configuration_t device_configuration;

static uint8_t last_id = 100;
static uint8_t id_list[255][16];

void check_device_id_configuration(uint8_t device_number);

void send_blinks(uint16_t number_of_badges, uint8_t delay);

status_t stress_testing_init(void) {
    if (is_init == false) {
        SET_OUTPUT_PIN(5);
        SET_PIN_HIGH(5);
        spi_driver_init();
        set_use_dwm_spi();
        dwm_manager_init(NULL);
        is_init = true;
    }

    return STATUS_OK;
}

status_t stress_testing_main(void) {
    for (size_t i = 0; i < BADGES_COUNT; i++) {
        check_device_id_configuration(i);
    }
    while (1) {
        LOG_INFO("SENDING BLINK REPORT");

        send_blinks(BADGES_COUNT, BLINK_DELAY);
    }
}

void check_device_id_configuration(uint8_t device_number) {
    uint8_t device_id[16];
    memset(device_id, 0, 16);
    memset(device_configuration.config.device_id, 0, 16);

    if (memcmp(device_id, device_configuration.config.device_id, 16) == 0) {
        LOG_INFO("Requesting badge ID");
        int rand_num;
        srand(time(0));
        rand_num = rand();

        uint8_t  buffer[DWM_MANAGER_MAXIMUM_FRAME_LENGTH] = {SYSTEM_CMD_UWB_REQUEST_NEW_BADGE_ID, rand_num % 256};
        uint32_t buffer_size                              = 2;
        uint8_t  counter                                  = 100;

        dwm_manager_request_data(buffer, &buffer_size);

        while (counter--) {
            dwm_manager_poll_irq();

            dwm_manager_retrieve_data(buffer, &buffer_size);
            if (buffer_size != 0) {
                if (buffer[0] == SYSTEM_CMD_UWB_SEND_NEW_BADGE_ID) {
                    if (buffer[1] == (uint8_t)(rand_num % 256)) {
                        memcpy(id_list[device_number], buffer + 2, DEVICE_UUID_LENGTH);
                        break;
                    }
                }

                buffer_size = 0;
            }
            _delay_ms(10);
        }
    }
}

void send_blinks(uint16_t number_of_badges, uint8_t delay) {
    for (size_t i = 0; i < number_of_badges; i++) {
        dwm_manager_transmit_blink(id_list[i], DWM_MANAGER_BLINK_TRANSMIT, false);
    }
    last_id = 0;
    _delay_ms(delay);
}

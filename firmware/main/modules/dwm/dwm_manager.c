#include "dwm_manager.h"
#include "badge_defines.h"
#include "configuration_manager.h"
#include "dwm_driver.h"
#include "log.h"

#include <stdio.h>
#include <string.h>

#define MODULE_TAG "DWM_MANAGER"

static bool is_init = false;

static uint8_t tx_blink[DWM_MANAGER_MAXIMUM_FRAME_LENGTH];
static uint8_t tx_id = 0;

status_t dwm_manager_init(dwm_manager_config_t *dwm_config) {
    if (is_init == true) {
        LOG_WARNING("module already initialized");
        return STATUS_ALREADY_INIT;
    }

    dwm_configuration_t config = DWM_DEFAULT_CONFIGURATION();
    if (dwm_config != NULL) {
        config.channel      = dwm_config->channel;
        config.data_rate    = dwm_config->data_rate;
        config.preamble_len = dwm_config->preamble_len;
        config.sfd_type     = dwm_config->sfd_type;
    }
    CHECK_FUNCTION(dwm_driver_init(&config));

    is_init = true;
    return STATUS_OK;
}

status_t dwm_manager_transmit_blink(uint8_t *device_id, dwm_manager_blink_type_e type, bool is_sos_triggered) {
    CHECK_INIT(is_init);

    tx_blink[0] = SYSTEM_CMD_UWB_BLINK;
    tx_blink[1] = tx_id++;

    for (size_t i = 0; i < 16; i++) {
        tx_blink[i + 2] = device_id[i];
    }

    tx_blink[18] = is_sos_triggered;

    uint32_t size = 19;
    switch (type) {
        case DWM_MANAGER_BLINK_REQUEST:
            CHECK_FUNCTION(dwm_driver_request_data(tx_blink, &size));
            break;

        case DWM_MANAGER_BLINK_TRANSMIT:
        default:
            CHECK_FUNCTION(dwm_driver_transmit_data(tx_blink, size));
            break;
    }

    return STATUS_OK;
}

status_t dwm_manager_transmit_data(uint8_t *buffer, uint32_t buffer_size) {
    CHECK_INIT(is_init);
    CHECK_FUNCTION(dwm_driver_transmit_data(buffer, buffer_size));

    return STATUS_OK;
}

status_t dwm_manager_retrieve_data(uint8_t *buffer, uint32_t *buffer_size) {
    CHECK_INIT(is_init);
    CHECK_FUNCTION(dwm_driver_retrieve_data(buffer, buffer_size));

    return STATUS_OK;
}

status_t dwm_manager_request_data(uint8_t *buffer, uint32_t *buffer_size) {
    CHECK_INIT(is_init);
    CHECK_FUNCTION(dwm_driver_request_data(buffer, buffer_size));

    return STATUS_OK;
}

status_t dwm_manager_enable_sleep(void) {
    CHECK_INIT(is_init);
    CHECK_FUNCTION(dwm_driver_enable_sleep());

    return STATUS_OK;
}

void dwm_manager_poll_irq(void) {
    dwm_driver_poll_irq();
}

status_t dwm_manager_report_status(status_t status, uint8_t *device_id) {
    LOG_INFO("transmitting status report");
    uint8_t status_message[21];
    status_message[0] = SYSTEM_CMD_UWB_STATUS;

    for (size_t i = 0; i < 4; i++) {
        status_message[i + 1] = (status >> (i * 8)) & 0xFF;
    }

    memcpy(status_message + 5, device_id, 16);

    CHECK_FUNCTION(dwm_driver_transmit_data(status_message, sizeof(status_message)));
    return STATUS_OK;
}
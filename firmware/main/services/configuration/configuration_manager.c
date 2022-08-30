#include "configuration_manager.h"
#include "badge_defines.h"
#include "display_manager.h"
#include "driver/gpio.h"
#include "log.h"
#include "m24lr_driver.h"
#include "neopixel_driver.h"
#include "notification_manager.h"
#include "storage.h"
#include "utilities.h"

#include "nvs.h"
#include "nvs_flash.h"

#include <stdlib.h>
#include <string.h>

#define MODULE_TAG "CONFIGURATION"

#define NVS_DEFAULT_PARTITION_NAME "storage" /*!< NVS default partition name */

#define MAXIMUM_WRITE_BUFFER_SIZE  128
#define MAXIMUM_RECORD_BUFFER_SIZE 24

#define CLEAR(x) memset(x, 0, sizeof(x))

static uint8_t current_date[3];
static uint8_t current_date_string[20];

uint16_t get_position_update_period(uint8_t byte);
uint16_t get_notification_poll_period(uint8_t byte);
uint16_t get_awake_period(uint8_t byte);
uint8_t  get_weather_type(uint8_t byte);

status_t parse_retrieved_string(configuration_record_t *configurations, uint8_t *ndef_string, uint8_t length);

status_t configuration_manager_init(void) {
    profile_configuration_t       profile;
    weather_configuration_t       weather;
    notifications_configuration_t notification;
    device_configuration_t        device_configuration;

    CLEAR(profile.buffer);
    CLEAR(weather.buffer);
    CLEAR(notification.buffer);
    CLEAR(device_configuration.buffer);

    uint32_t buffer_size = 0;

    buffer_size = sizeof(device_configuration.buffer);
    if (storage_read_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_READ_BINARY_MODE, device_configuration.buffer,
                              &buffer_size)
        != STATUS_OK) {
        device_configuration.config.dwm_channel      = 5;
        device_configuration.config.dwm_preamble_len = 5;
        device_configuration.config.dwm_sfd_type     = 1;
        device_configuration.config.dwm_data_rate    = 1;

        device_configuration.config.awake_period             = 5;
        device_configuration.config.wakeup_sensitivity       = 7;
        device_configuration.config.notification_poll_period = 30;
        device_configuration.config.position_update_period   = 200;

        device_configuration.config.badge_state = BADGE_STATE_NONE;

        CHECK_FUNCTION(storage_write_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                              device_configuration.buffer, sizeof(device_configuration.buffer)));
    }

    buffer_size = sizeof(profile.buffer);
    if (storage_read_txt_file(PROFILE_SCREEN_TXT_FILENAME, STORAGE_READ_BINARY_MODE, profile.buffer, &buffer_size)
        != STATUS_OK) {
        memcpy(profile.config.name, "NAME", 5);
        memcpy(profile.config.surname, "Surname", 8);
        memcpy(profile.config.function, "Job Title", 10);
        memcpy(profile.config.issue_date, "issue date", 11);

        CHECK_FUNCTION(storage_write_txt_file(PROFILE_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                              profile.buffer, sizeof(profile.buffer)));
    }

    buffer_size = sizeof(weather.buffer);
    if (storage_read_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_READ_BINARY_MODE, weather.buffer, &buffer_size)
        != STATUS_OK) {
        memcpy(weather.config.location, "City", 5);
        memcpy(weather.config.area, "State", 6);
        memcpy(weather.config.date, "date", 5);

        CHECK_FUNCTION(storage_write_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                              weather.buffer, sizeof(weather.buffer)));
    }

    buffer_size = sizeof(notification.buffer);
    if (storage_read_txt_file(NOTIFICATIONS_SCREEN_TXT_FILENAME, STORAGE_READ_BINARY_MODE, notification.buffer,
                              &buffer_size)
        != STATUS_OK) {
        memcpy(notification.config.date, "date", 5);
        memcpy(notification.config.notification[0], "notification 1", 15);
        memcpy(notification.config.notification[1], "notification 2", 15);
        memcpy(notification.config.notification[2], "notification 3", 15);
        memcpy(notification.config.notification[3], "notification 4", 15);

        CHECK_FUNCTION(storage_write_txt_file(NOTIFICATIONS_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                              notification.buffer, sizeof(notification.buffer)));
    }

    return STATUS_OK;
}

status_t configuration_manager_read_nfc_memory(configuration_record_t *configurations) {
    uint8_t configuration[255];
    uint8_t length = 0;

    m24lr_driver_read_ndef(M24LR_DRIVER_NDEF_TYPE_FORUM_WKT, configuration, &length);
    if (length == 0) {
        LOG_WARNING("could not retrieve data");
        return STATUS_CONFIG_MANAGER_NFC_READ_ERROR;
    }

    CHECK_FUNCTION(parse_retrieved_string(configurations, configuration, length));
    return STATUS_OK;
}

status_t configuration_manager_get_configuration_data(device_configuration_t *device_data) {
    uint32_t buffer_size = sizeof(device_data->buffer);
    CHECK_FUNCTION(storage_read_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_READ_BINARY_MODE, device_data->buffer,
                                         &buffer_size));

    return STATUS_OK;
}

status_t configuration_manager_store_configuration_data(device_configuration_t *device_data) {
    CHECK_FUNCTION(storage_write_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                          device_data->buffer, sizeof(device_data->buffer)));

    return STATUS_OK;
}

status_t configuration_manager_update_device_id(uint8_t *device_id) {
    device_configuration_t data;
    uint32_t               buffer_size = sizeof(data.buffer);
    storage_read_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_READ_BINARY_MODE, data.buffer, &buffer_size);

    memset(data.config.device_id, 0, 16);
    memcpy(data.config.device_id, device_id, 16);
    CHECK_FUNCTION(storage_write_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                          data.buffer, sizeof(data.buffer)));

    return STATUS_OK;
}

status_t configuration_manager_get_profile_data(profile_configuration_t *profile) {
    uint32_t buffer_size = sizeof(profile->buffer);
    CHECK_FUNCTION(
        storage_read_txt_file(PROFILE_SCREEN_TXT_FILENAME, STORAGE_READ_BINARY_MODE, profile->buffer, &buffer_size));

    return STATUS_OK;
}

status_t configuration_manager_get_notifications_data(notifications_configuration_t *notifications) {
    uint32_t buffer_size = sizeof(notifications->buffer);
    CHECK_FUNCTION(storage_read_txt_file(NOTIFICATIONS_SCREEN_TXT_FILENAME, STORAGE_READ_BINARY_MODE,
                                         notifications->buffer, &buffer_size));

    memset(notifications->config.date, 0, sizeof(notifications->config.date));
    memcpy(notifications->config.date, current_date_string, strlen((char *)current_date_string));

    return STATUS_OK;
}

status_t configuration_manager_get_weather_data(weather_configuration_t *weather) {
    uint32_t buffer_size = sizeof(weather->buffer);
    CHECK_FUNCTION(
        storage_read_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_READ_BINARY_MODE, weather->buffer, &buffer_size));

    memset(weather->config.date, 0, sizeof(weather->config.date));
    memcpy(weather->config.date, current_date_string, strlen((char *)current_date_string));

    return STATUS_OK;
}

status_t configuration_manager_read_user_data(configuration_record_t *configurations) {

    full_configuration_t badge_configuration;
    uint32_t             badge_config_buffer_size = 0;
    storage_read_txt_file(FULL_CONFIGURATION_TXT_FILENAME, STORAGE_READ_BINARY_MODE, badge_configuration.buffer,
                          &badge_config_buffer_size);

    sprintf((char *)configurations[SD_PROPERTY_INDEX_DWM_CHANNEL].text, "%d", badge_configuration.config.dwm_channel);
    configurations[SD_PROPERTY_INDEX_DWM_CHANNEL].length
        = strlen((char *)configurations[SD_PROPERTY_INDEX_DWM_CHANNEL].text);

    sprintf((char *)configurations[SD_PROPERTY_INDEX_DWM_DATA_RATE].text, "%d",
            badge_configuration.config.dwm_data_rate);
    configurations[SD_PROPERTY_INDEX_DWM_DATA_RATE].length
        = strlen((char *)configurations[SD_PROPERTY_INDEX_DWM_DATA_RATE].text);

    sprintf((char *)configurations[SD_PROPERTY_INDEX_DWM_PREAMBLE_LENGTH].text, "%d",
            badge_configuration.config.dwm_preamble_len);
    configurations[SD_PROPERTY_INDEX_DWM_PREAMBLE_LENGTH].length
        = strlen((char *)configurations[SD_PROPERTY_INDEX_DWM_PREAMBLE_LENGTH].text);

    sprintf((char *)configurations[SD_PROPERTY_INDEX_DWM_SFD_TYPE].text, "%d", badge_configuration.config.dwm_sfd_type);
    configurations[SD_PROPERTY_INDEX_DWM_SFD_TYPE].length
        = strlen((char *)configurations[SD_PROPERTY_INDEX_DWM_SFD_TYPE].text);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_WIFI_SSID].text, (char *)badge_configuration.config.wifi_ssid,
           sizeof(badge_configuration.config.wifi_ssid));
    configurations[SD_PROPERTY_INDEX_WIFI_SSID].length = strlen((char *)badge_configuration.config.wifi_ssid);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_WIFI_PASSWORD].text, (char *)badge_configuration.config.wifi_pass,
           sizeof(badge_configuration.config.wifi_pass));
    configurations[SD_PROPERTY_INDEX_WIFI_PASSWORD].length = strlen((char *)badge_configuration.config.wifi_pass);

    CLEAR(configurations[SD_PROPERTY_INDEX_DEVICE_ID].text);
    for (int i = 0; i < 16; i++) {
        sprintf((char *)(configurations[SD_PROPERTY_INDEX_DEVICE_ID].text
                         + (int)strlen((char *)configurations[SD_PROPERTY_INDEX_DEVICE_ID].text)),
                "%.2x", badge_configuration.config.device_id[i]);
    }
    configurations[SD_PROPERTY_INDEX_DEVICE_ID].length
        = strlen((char *)configurations[SD_PROPERTY_INDEX_DEVICE_ID].text);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_NAME].text, (char *)badge_configuration.config.name,
           sizeof(badge_configuration.config.name));
    configurations[SD_PROPERTY_INDEX_NAME].length = strlen((char *)badge_configuration.config.name);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_SURNAME].text, (char *)badge_configuration.config.surname,
           sizeof(badge_configuration.config.surname));
    configurations[SD_PROPERTY_INDEX_SURNAME].length = strlen((char *)badge_configuration.config.surname);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_FUNCTION].text, (char *)badge_configuration.config.function,
           sizeof(badge_configuration.config.function));
    configurations[SD_PROPERTY_INDEX_FUNCTION].length = strlen((char *)badge_configuration.config.function);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_ISSUE_DATE].text, (char *)badge_configuration.config.issue_date,
           sizeof(badge_configuration.config.issue_date));
    configurations[SD_PROPERTY_INDEX_ISSUE_DATE].length = strlen((char *)badge_configuration.config.issue_date);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_CITY].text, (char *)badge_configuration.config.location,
           sizeof(badge_configuration.config.location));
    configurations[SD_PROPERTY_INDEX_CITY].length = strlen((char *)badge_configuration.config.location);

    memcpy((char *)configurations[SD_PROPERTY_INDEX_AREA].text, (char *)badge_configuration.config.area,
           sizeof(badge_configuration.config.area));
    configurations[SD_PROPERTY_INDEX_AREA].length = strlen((char *)badge_configuration.config.area);

    return STATUS_OK;
}

status_t configuration_manager_update_profile_data(configuration_record_t *configurations) {
    profile_configuration_t data;

    CLEAR(data.buffer);

    memcpy(data.config.name, configurations[NFC_PROPERTY_INDEX_NAME].text,
           configurations[NFC_PROPERTY_INDEX_NAME].length);
    memcpy(data.config.surname, configurations[NFC_PROPERTY_INDEX_SURNAME].text,
           configurations[NFC_PROPERTY_INDEX_SURNAME].length);
    memcpy(data.config.function, configurations[NFC_PROPERTY_INDEX_FUNCTION].text,
           configurations[NFC_PROPERTY_INDEX_FUNCTION].length);
    memcpy(data.config.issue_date, configurations[NFC_PROPERTY_INDEX_ISSUE_DATE].text,
           configurations[NFC_PROPERTY_INDEX_ISSUE_DATE].length);

    storage_write_txt_file(PROFILE_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, data.buffer,
                           sizeof(data.buffer));
    return STATUS_OK;
}

status_t configuration_manager_update_notifications_data(uint8_t *buffer, uint8_t buffer_size) {
    notifications_configuration_t data;
    device_configuration_t        device_data;

    memset(data.config.date, 0, sizeof(data.config.date));

    CHECK_FUNCTION(configuration_manager_get_notifications_data(&data));
    if (current_date[0] == 0 || current_date[1] == 0 || current_date[2] == 0) {
        CHECK_FUNCTION(configuration_manager_get_configuration_data(&device_data));
        memcpy(data.config.date, device_data.config.date, strlen((char *)device_data.config.date));
    }
    else {
        memcpy(data.config.date, current_date_string, strlen((char *)current_date_string));
    }

    for (size_t i = 3; i > 0; i--) {
        memcpy(data.config.notification[i], data.config.notification[i - 1], sizeof(data.config.notification[i]));
        memcpy(data.config.timestamp_date[i], data.config.timestamp_date[i - 1], sizeof(data.config.timestamp_date[i]));
        memcpy(data.config.timestamp[i], data.config.timestamp[i - 1], sizeof(data.config.timestamp[i]));
    }

    CLEAR(data.config.notification[0]);
    CLEAR(data.config.timestamp[0]);
    CLEAR(data.config.timestamp_date[0]);

    for (size_t i = 3; i < buffer[2]; i++) {
        if (buffer[i] < 32 || buffer[i] > 126) {
            buffer[i] = 0;
        }
    }

    char aux_buffer[30];

    if (buffer[0] < 12) {
        if (buffer[0] == 0) {
            buffer[0] = 12;
        }
        sprintf(aux_buffer, "%02d:%02d AM - %02d-%02d-20%02d", buffer[0], buffer[1], current_date[1], current_date[0],
                current_date[2]);
    }
    else {
        if (buffer[0] == 12) {
            buffer[0] = 12;
        }
        else {
            buffer[0] = buffer[0] % 12;
        }
        sprintf(aux_buffer, "%02d:%02d PM - %02d-%02d-20%02d", buffer[0], buffer[1], current_date[1], current_date[0],
                current_date[2]);
    }

    memcpy(data.config.notification[0], buffer + 3, buffer[2]);
    memcpy(data.config.timestamp[0], aux_buffer, strlen(aux_buffer));
    memcpy(data.config.timestamp_date[0], current_date, sizeof(current_date));

    storage_write_txt_file(NOTIFICATIONS_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, data.buffer,
                           sizeof(data.buffer));
    return STATUS_OK;
}

status_t configuration_manager_update_weather_data_nfc(configuration_record_t *configurations) {
    weather_configuration_t data;
    CHECK_FUNCTION(configuration_manager_get_weather_data(&data));

    CLEAR(data.config.location);
    CLEAR(data.config.area);

    memcpy(data.config.location, configurations[NFC_PROPERTY_INDEX_CITY].text,
           configurations[NFC_PROPERTY_INDEX_CITY].length);
    memcpy(data.config.area, configurations[NFC_PROPERTY_INDEX_AREA].text,
           configurations[NFC_PROPERTY_INDEX_AREA].length);

    storage_write_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, data.buffer,
                           sizeof(data.buffer));
    return STATUS_OK;
}

status_t configuration_manager_update_weather_data_dwm(uint8_t *buffer) {
    weather_configuration_t data;
    CHECK_FUNCTION(configuration_manager_get_weather_data(&data));

    data.config.temperature = buffer[8];

    if (buffer[8] == 255 && buffer[9] == 255) {
        data.config.weather_type = buffer[9];

        storage_write_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, data.buffer,
                               sizeof(data.buffer));
        return STATUS_OK;
    }
    if (buffer[9] & CONFIGURATION_MANAGER_WEATHER_SIGN_BITMASK) {
        data.config.weather_type = get_weather_type(buffer[9] & CONFIGURATION_MANAGER_WEATHER_IMAGE_BITMASK)
                                   | CONFIGURATION_MANAGER_WEATHER_SIGN_BITMASK;
    }
    else {
        data.config.weather_type = get_weather_type(buffer[9]);
    }

    storage_write_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, data.buffer,
                           sizeof(data.buffer));
    return STATUS_OK;
}

status_t configuration_manager_update_configuration_data_nfc(configuration_record_t *configurations) {
    device_configuration_t data;
    CHECK_FUNCTION(configuration_manager_get_configuration_data(&data));

    data.config.dwm_channel      = atoi((char *)configurations[NFC_PROPERTY_INDEX_DWM_CHANNEL].text);
    data.config.dwm_data_rate    = atoi((char *)configurations[NFC_PROPERTY_INDEX_DWM_DATA_RATE].text);
    data.config.dwm_preamble_len = atoi((char *)configurations[NFC_PROPERTY_INDEX_DWM_PREAMBLE_LENGTH].text);
    data.config.dwm_sfd_type     = atoi((char *)configurations[NFC_PROPERTY_INDEX_DWM_SFD_TYPE].text);

    memset(data.config.wifi_ssid, 0, sizeof(data.config.wifi_ssid));
    memset(data.config.wifi_pass, 0, sizeof(data.config.wifi_pass));

    memcpy(data.config.wifi_ssid, configurations[NFC_PROPERTY_INDEX_WIFI_SSID].text,
           configurations[NFC_PROPERTY_INDEX_WIFI_SSID].length);
    memcpy(data.config.wifi_pass, configurations[NFC_PROPERTY_INDEX_WIFI_PASSWORD].text,
           configurations[NFC_PROPERTY_INDEX_WIFI_PASSWORD].length);

    storage_write_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, data.buffer,
                           sizeof(data.buffer));
    return STATUS_OK;
}

status_t configuration_manager_update_configuration_data_dwm(device_configuration_t *device_cfg, uint8_t *buffer) {

    for (size_t i = 0; i < 3; i++) {
        current_date[i] = buffer[i + 1];
    }

    sprintf((char *)current_date_string, "%02d-%02d-20%02d", current_date[1], current_date[0], current_date[2]);
    memcpy(device_cfg->config.date, current_date_string, strlen((char *)current_date_string));

    device_cfg->config.position_update_period   = get_position_update_period(buffer[4]);
    device_cfg->config.wakeup_sensitivity       = buffer[5];
    device_cfg->config.notification_poll_period = get_notification_poll_period(buffer[6]);
    device_cfg->config.awake_period             = get_awake_period(buffer[7]);

    if (device_cfg->config.notification_poll_period == 0) {
        device_cfg->config.notification_poll_period = 10;
    }

    if (device_cfg->config.position_update_period == 0) {
        device_cfg->config.position_update_period = 250;
    }

    if (device_cfg->config.awake_period == 0) {
        device_cfg->config.awake_period = 5;
    }

    configuration_manager_update_weather_data_dwm(buffer);

    configuration_manager_store_configuration_data(device_cfg);

    return STATUS_OK;
}

status_t configuration_manager_load_full_configuration() {
    full_configuration_t device_configuration;

    device_configuration_t dev_cfg;

    profile_configuration_t profile_configuration;
    weather_configuration_t weather_configuration;

    uint32_t device_configuration_buffer_size = sizeof(device_configuration.buffer);
    storage_read_txt_file(FULL_CONFIGURATION_TXT_FILENAME, STORAGE_READ_BINARY_MODE, device_configuration.buffer,
                          &device_configuration_buffer_size);

    uint32_t dev_cfg_buffer_size = sizeof(dev_cfg.buffer);
    storage_read_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_READ_BINARY_MODE, dev_cfg.buffer,
                          &dev_cfg_buffer_size);

    dev_cfg.config.dwm_channel      = device_configuration.config.dwm_channel;
    dev_cfg.config.dwm_data_rate    = device_configuration.config.dwm_data_rate;
    dev_cfg.config.dwm_preamble_len = device_configuration.config.dwm_preamble_len;
    dev_cfg.config.dwm_sfd_type     = device_configuration.config.dwm_sfd_type;
    memcpy(dev_cfg.config.device_id, device_configuration.config.device_id, 16);
    memcpy(dev_cfg.config.date, device_configuration.config.date, 20);
    dev_cfg.config.position_update_period   = device_configuration.config.position_update_period;
    dev_cfg.config.wakeup_sensitivity       = device_configuration.config.wakeup_sensitivity;
    dev_cfg.config.notification_poll_period = device_configuration.config.notification_poll_period;
    dev_cfg.config.awake_period             = device_configuration.config.awake_period;
    memcpy(dev_cfg.config.alert_led, device_configuration.config.alert_led, 3);
    memcpy(dev_cfg.config.danger_led, device_configuration.config.danger_led, 3);
    memcpy(dev_cfg.config.notification_led, device_configuration.config.notification_led, 3);
    dev_cfg.config.badge_state                   = BADGE_STATE_NONE;
    dev_cfg.config.is_profile_screen_built       = device_configuration.config.is_profile_screen_built;
    dev_cfg.config.is_weather_screen_built       = device_configuration.config.is_weather_screen_built;
    dev_cfg.config.is_notifications_screen_built = device_configuration.config.is_notifications_screen_built;
    dev_cfg.config.notification_state            = device_configuration.config.notification_state;
    dev_cfg.config.targeted_notification_state   = device_configuration.config.targeted_notification_state;
    memcpy(dev_cfg.config.wifi_ssid, device_configuration.config.wifi_ssid, 40);
    memcpy(dev_cfg.config.wifi_pass, device_configuration.config.wifi_pass, 40);

    memcpy(profile_configuration.config.name, device_configuration.config.name, CONFIGURATION_MAX_BUFFER_SIZE);
    memcpy(profile_configuration.config.surname, device_configuration.config.surname, CONFIGURATION_MAX_BUFFER_SIZE);
    memcpy(profile_configuration.config.function, device_configuration.config.function, CONFIGURATION_MAX_BUFFER_SIZE);
    memcpy(profile_configuration.config.issue_date, device_configuration.config.issue_date, 20);

    memcpy(weather_configuration.config.location, device_configuration.config.location, CONFIGURATION_MAX_BUFFER_SIZE);
    memcpy(weather_configuration.config.area, device_configuration.config.area, CONFIGURATION_MAX_BUFFER_SIZE);
    memcpy(weather_configuration.config.date, device_configuration.config.weather_date, 20);
    weather_configuration.config.temperature  = device_configuration.config.temperature;
    weather_configuration.config.weather_type = device_configuration.config.weather_type;

    dev_cfg_buffer_size = sizeof(dev_cfg.buffer);
    storage_write_txt_file(CONFIGURATION_FILE_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, dev_cfg.buffer,
                           dev_cfg_buffer_size);

    uint16_t weather_configuration_buffer_size = sizeof(weather_configuration.buffer);
    storage_write_txt_file(WEATHER_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, weather_configuration.buffer,
                           weather_configuration_buffer_size);

    uint16_t profile_configuration_buffer_size = sizeof(profile_configuration.buffer);
    storage_write_txt_file(PROFILE_SCREEN_TXT_FILENAME, STORAGE_WRITE_BINARY_MODE_REPLACE, profile_configuration.buffer,
                           profile_configuration_buffer_size);

    return STATUS_OK;
}

status_t parse_retrieved_string(configuration_record_t *configurations, uint8_t *ndef_string, uint8_t length) {
    uint8_t config_count  = 0;
    uint8_t record_length = 0;
    uint8_t record[MAXIMUM_RECORD_BUFFER_SIZE];

    for (size_t i = 0; i < length; i++) {
        if (ndef_string[i] == CONFIGURATION_NEW_TEXT_ENTRY) {
            record_length = ndef_string[i - 1] - 3;
            i += 4;

            uint8_t record_iterator = 0;
            for (size_t j = i; j < i + record_length; j++) {
                record[record_iterator] = ndef_string[j];
                record_iterator++;
            }

            strncpy((char *)configurations[config_count].text, (char *)record, record_length);
            configurations[config_count].length = record_length;
            config_count++;
            i = i + record_length;
        }
    }

    if (config_count != CONFIGURATION_ENTRIES_NUMBER) {
        return STATUS_CONFIG_MANAGER_INVALID_ENTRY_COUNT;
    }

    return STATUS_OK;
}

uint16_t get_position_update_period(uint8_t byte) {
    switch (byte) {
        case 1:
            return 100;
        case 2:
            return 200;
        case 3:
            return 250;
        case 4:
            return 500;
        case 5:
            return 1000;
        case 6:
            return 2000;
        case 7:
            return 5000;
        case 8:
            return 10000;

        default:
            return 250;
    }

    return 250;
}

uint16_t get_notification_poll_period(uint8_t byte) {
    switch (byte) {
        case 1:
            return 5;
        case 2:
            return 10;
        case 3:
            return 20;
        case 4:
            return 30;
        case 5:
            return 60;
        case 6:
            return 120;
        case 7:
            return 300;
        case 8:
            return 600;
        case 9:
            return 1800;
        case 10:
            return 3600;

        default:
            return 10;
    }

    return 10;
}

uint16_t get_awake_period(uint8_t byte) {
    switch (byte) {
        case 1:
            return 0;
        case 2:
            return 10;
        case 3:
            return 25;
        case 4:
            return 50;
        case 5:
            return 100;
        case 6:
            return 150;
        case 7:
            return 300;

        default:
            return 50;
    }

    return 50;
}

uint8_t get_weather_type(uint8_t byte) {
    switch (byte) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        case 4:
            return 3;
        case 9:
            return 4;
        case 10:
            return 5;
        case 11:
            return 6;
        case 13:
            return 7;
        case 50:
            return 8;
        default:
            return 0;
    }
}
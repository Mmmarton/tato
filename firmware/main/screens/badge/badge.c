#include "badge.h"
#include "adxl345_driver.h"
#include "badge_defines.h"
#include "badge_test.h"
#include "client.h"
#include "configuration_manager.h"
#include "configuration_menu.h"
#include "controls.h"
#include "display_manager.h"
#include "driver/gpio.h"
#include "dwm_manager.h"
#include "log.h"
#include "m24lr_driver.h"
#include "max_driver.h"
#include "notification_manager.h"
#include "ota.h"
#include "reports.h"
#include "sleep.h"
#include "spi_driver.h"
#include "storage.h"
#include "utilities.h"
#include "wifi.h"

#include "client.h"
#include "wifi.h"

#include <esp_sleep.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MODULE_TAG "BADGE MENU"

#define PROFILE_SCREEN_LINE_COUNT       4 /** number of lines to be read for the profile screen */
#define NOTIFICATIONS_SCREEN_LINE_COUNT 8 /** number of lines to be read for the notifications screen */
#define WEATHER_SCREEN_LINE_COUNT       3 /** number of lines to be read for the weatehr screen */

#define WAKEUP_PIN 33 /*!< pin for avr wake up command */

static bool is_spi_free      = true;
static bool is_controls_free = true;
static bool is_sos_triggered = false;

static controls_button_e      action        = 0;
static uint16_t               sleep_counter = 0;
static device_configuration_t device_configuration;

static RTC_NOINIT_ATTR int32_t last_request_timestamp = 0;
static RTC_NOINIT_ATTR int32_t last_report_timestamp  = 0;
static RTC_NOINIT_ATTR bool    is_first_start         = true;

static uint8_t *weather_conditions[]
    = {WEATHER_SCREEN_MSG_CLEAR,         WEATHER_SCREEN_MSG_FEW_CLOUDS,  WEATHER_SCREEN_MSG_SCATTERED_CLOUDS,
       WEATHER_SCREEN_MSG_BROKEN_CLOUDS, WEATHER_SCREEN_MSG_SHOWER_RAIN, WEATHER_SCREEN_MSG_RAIN,
       WEATHER_SCREEN_MSG_THUNDERSTORM,  WEATHER_SCREEN_MSG_SNOW,        WEATHER_SCREEN_MSG_FOG};

void display_profile_screen(void);
void display_notifications_screen(void);
void display_weather_screen(void);
void display_sos_screen(void);
void display_battery_status(void);

void check_notificaton_status(void);
void check_report_status(void);
void check_device_id_configuration(void);

void check_configuration_menu_init(void);
void check_test_menu_init(void);

void format_token(uint8_t *output, uint8_t *buffer);
void format_name(uint8_t *output, uint8_t *buffer);

void check_alert_mode_conditions(uint8_t settings, bool is_zone_notification);
bool request_notifications(uint8_t *buffer, uint32_t buffer_size);
bool check_targeted_notification_status(void);

#define DEVICE_UUID_LENGTH 16

typedef union {
    struct PACKED config_data {
        uint8_t current_day;
        uint8_t current_month;
        uint8_t current_year;
        uint8_t position_update_period;
        uint8_t wakeup_sensitivity;
        uint8_t notification_poll_period;
        uint8_t device_awake_period;
        uint8_t temperature;
        uint8_t temperature_data;
    } fields;
    uint8_t buffer[sizeof(struct config_data) + 1];
} badge_update_config_t;

typedef union {
    struct PACKED update_data {
        uint8_t               command;
        badge_update_config_t configuration;
        bool                  has_notif_data;
        bool                  has_file_data;
        uint8_t               notif_target_type;
        uint8_t               file_target_type;
        uint8_t               target[DEVICE_UUID_LENGTH];
        uint8_t               notification_settings;
        uint8_t               hour;
        uint8_t               minute;
        uint8_t               message[30];
        uint8_t               file_update_settings;
        uint8_t               server_address[4];
        uint16_t              port;
        uint8_t               token[16];
        uint32_t              checksum;
        uint8_t               filename[24];
    } fields;
    uint8_t buffer[sizeof(struct update_data) + 1];
} badge_update_t;

status_t essentials_init(void);
status_t complete_init(void);

status_t sleep_setup(void);
status_t accelerometer_setup(void);
status_t dwm_setup(void);

void task_dwm(void *arg);
void task_controls(void *arg);

void           hibernate(void);
void           initialize_sleep_reset_interrupt(void);
void IRAM_ATTR sleep_counter_reset_interrupt_handler(void *arg);

status_t check_bmp_file(uint8_t *name, uint8_t *msg, uint8_t msg_size);

static badge_update_t update;

status_t badge_init(void) {
    LOG_INFO("badge started. running firmware version: %d.%d.%d", BADGE_VERSION_MAJOR, BADGE_VERSION_MINOR,
             BADGE_VERSION_PATCH);

    /** enable LDO */
    SET_OUTPUT_PIN(5);
    SET_PIN_HIGH(5);

    CHECK_FUNCTION(spi_driver_init());
    CHECK_FUNCTION(storage_init());

    CHECK_FUNCTION(controls_driver_init());
    check_configuration_menu_init();
    check_test_menu_init();

    xTaskCreate(task_controls, "task_controls", 2048, NULL, 4, NULL);

    CHECK_FUNCTION(essentials_init());
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        esp_enable_sleep_timer_seconds(device_configuration.config.notification_poll_period);
        last_request_timestamp = get_time();
        _delay_ms(100);

        uint8_t  buffer[DWM_MANAGER_MAXIMUM_FRAME_LENGTH];
        uint32_t buffer_size = 1;

        memset(buffer, 0, DWM_MANAGER_MAXIMUM_FRAME_LENGTH);
        if (!request_notifications(buffer, buffer_size)) {
            accelerometer_setup();
            sleep_setup();

            _delay_ms(10);
            hibernate();
        }
        else {
            accelerometer_setup();
            sleep_setup();

            is_spi_free = false;
            configuration_manager_update_configuration_data_dwm(&device_configuration, buffer);

            uint8_t target = buffer[10];
            uint8_t offset = 0;
            if (target != 0) {
                offset += 16;

                if (memcmp(buffer + 11, device_configuration.config.device_id, 16) != 0) {
                    return STATUS_CONFIG_MANAGER_INVALID_TARGET_ERROR;
                }
            }

            uint8_t settings = buffer[11 + offset];
            if (settings != 0) {
                if ((device_configuration.config.notification_state & NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK)
                    != (settings & NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK)) {
                    complete_init();

                    configuration_manager_update_notifications_data(buffer + 12 + offset, buffer[14 + offset]);
                    device_configuration.config.notification_state = settings;
                    configuration_manager_store_configuration_data(&device_configuration);
                    check_alert_mode_conditions(settings, false);
                }
            }

            _delay_ms(1000);
            hibernate();
        }
    }
    else {
        accelerometer_setup();
        sleep_setup();
    }

    return STATUS_OK;
}

void badge_loop(void) {
    complete_init();

    xTaskCreate(task_dwm, "task_dwm", 2048, NULL, 5, NULL);
    sleep_counter = 0;

    while (1) {
        adxl345_driver_reset_interrupt();

        if (is_spi_free == true && action != 0 && is_sos_triggered == false) {
            switch (action) {
                case CONTROLS_BUTTON_1:
                    if (device_configuration.config.badge_state != BADGE_STATE_WEATHER) {
                        display_weather_screen();
                    }
                    break;
                case CONTROLS_BUTTON_2:
                    if (device_configuration.config.badge_state != BADGE_STATE_NOTIFICATIONS
                        || notification_manager_get_has_new_notification()) {
                        display_notifications_screen();
                    }
                    break;
                case CONTROLS_BUTTON_3:
                    if (device_configuration.config.badge_state != BADGE_STATE_PROFILE) {
                        display_profile_screen();
                    }
                    break;
                case CONTROLS_BUTTON_SOS:
                    display_sos_screen();
                    action = 0;
                    break;
                default:
                    break;
            }
            sleep_counter = 0;
            action        = 0;
        }
        sleep_counter++;
        _delay_ms(10);
        check_report_status();
        if ((sleep_counter % 100 == 0) && (is_sos_triggered == false)) {
            check_notificaton_status();
            if (sleep_counter >= device_configuration.config.awake_period * 10) {
                esp_enable_sleep_timer_seconds(
                    (device_configuration.config.notification_poll_period - (sleep_counter / 100)));
                _delay_ms(100);
                hibernate();
            }
        }
        else if (is_sos_triggered == true) {
            is_spi_free = true;
        }
    }
}

void badge_hibernate(void) {
    display_manager_init();
    display_manager_clear_display_buffers();
    display_manager_write_centered_message((uint8_t *)"NO VALID SD CARD", 16, 200, 1, false);
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    _delay_ms(1000);
    hibernate();
}

void display_profile_screen(void) {
    is_spi_free = false;

    display_manager_clear_display_buffers();
    if (check_bmp_file(PROFILE_SCREEN_BMP_FILENAME, (uint8_t *)"NO SCREEN FILE", 14) == -1) {
        configuration_manager_store_configuration_data(&device_configuration);

        is_spi_free = true;
        return;
    }

    display_manager_write_image_to_buffer(PROFILE_SCREEN_BMP_FILENAME, 0, 0, true);

    profile_configuration_t data;
    status_t                ret = configuration_manager_get_profile_data(&data);
    if (ret == STATUS_OK) {
        display_manager_write_text_to_buffer(data.config.name, strlen((char *)data.config.name), 20, 330, 2, true);

        display_manager_write_text_to_buffer(data.config.surname, strlen((char *)data.config.surname), 20, 350, 1,
                                             true);
        display_manager_write_text_to_buffer(data.config.function, strlen((char *)data.config.function), 20, 380, 0,
                                             true);

        display_manager_write_text_to_buffer((uint8_t *)"Issued", 6, 180, 340, 1, true);
        display_manager_write_text_to_buffer(data.config.issue_date, strlen((char *)data.config.issue_date), 160, 360,
                                             0, true);
    }
    else if (ret == STATUS_STORAGE_NULL_TXT_FILE) {
        display_manager_write_centered_message((uint8_t *)"no data file", 12, 340, 1, true);
    }
    else {
        display_manager_write_centered_message((uint8_t *)"invalid data", 12, 340, 1, true);
    }

    display_manager_write_image_to_buffer(PROFILE_SCREEN_QR_BMP_FILENAME, 170, 110, false);

    if (display_manager_write_image_to_buffer(PROFILE_SCREEN_USER_BMP_FILENAME, 0, 100, false) != 0) {
        display_manager_write_image_to_buffer(PROFILE_SCREEN_PALCEHOLDER_BMP_FILENAME, 0, 100, false);
    }

    display_battery_status();
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    device_configuration.config.badge_state = BADGE_STATE_PROFILE;
    configuration_manager_store_configuration_data(&device_configuration);
    is_spi_free = true;
}

void display_notifications_screen(void) {
    device_configuration.config.notification_state &= NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK;
    is_spi_free = false;

    display_manager_clear_display_buffers();
    if (check_bmp_file(NOTIFICATIONS_SCREEN_BMP_FILENAME, (uint8_t *)"NO NOTIFICATION FILE", 20) == -1) {
        configuration_manager_store_configuration_data(&device_configuration);

        is_spi_free = true;
        return;
    }

    display_manager_write_image_to_buffer(NOTIFICATIONS_SCREEN_BMP_FILENAME, 0, 0, true);
    display_manager_write_centered_message((uint8_t *)"Today", 5, 120, 2, true);
    display_manager_write_centered_message(device_configuration.config.date,
                                           strlen((char *)device_configuration.config.date), 150, 1, true);

    notifications_configuration_t data;
    status_t                      ret = configuration_manager_get_notifications_data(&data);
    if (ret == STATUS_OK) {

        for (size_t i = 0; i < 4; i++) {
            display_manager_write_text_to_buffer(
                data.config.notification[i], strlen((char *)data.config.notification[i]), 20, 210 + (i * 50), 1, true);

            display_manager_write_text_to_buffer(data.config.timestamp[i], strlen((char *)data.config.timestamp[i]), 25,
                                                 230 + (i * 50), 0, true);
            if (i != 3) {
                display_manager_write_horizontal_separator(10, 245 + (i * 50), 220, GDEW0371W7_COLOR_BW_BLACK);
            }
        }
    }
    else if (ret == STATUS_STORAGE_NULL_TXT_FILE) {
        display_manager_write_centered_message((uint8_t *)"no data file", 12, 340, 1, true);
    }
    else {
        display_manager_write_centered_message((uint8_t *)"invalid data", 12, 340, 1, true);
    }

    display_battery_status();
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    device_configuration.config.badge_state = BADGE_STATE_NOTIFICATIONS;
    configuration_manager_store_configuration_data(&device_configuration);
    is_spi_free = true;
}

void display_weather_screen(void) {
    is_spi_free = false;

    display_manager_clear_display_buffers();
    if (check_bmp_file(WEATHER_SCREEN_BMP_FILENAME, (uint8_t *)"NO WEATHER FILE", 15) == -1) {
        configuration_manager_store_configuration_data(&device_configuration);

        is_spi_free = true;
        return;
    }

    display_manager_write_image_to_buffer(WEATHER_SCREEN_BMP_FILENAME, 0, 0, true);
    display_manager_write_centered_message(device_configuration.config.date,
                                           strlen((char *)device_configuration.config.date), 150, 1, true);

    weather_configuration_t data;
    status_t                ret = configuration_manager_get_weather_data(&data);
    if (ret == STATUS_OK) {
        display_manager_write_centered_message(data.config.location, strlen((char *)data.config.location), 90, 2, true);
        display_manager_write_centered_message(data.config.area, strlen((char *)data.config.area), 120, 3, true);

        if (data.config.temperature == 255 && data.config.weather_type == 255) {
            display_manager_write_text_to_buffer((uint8_t *)"No weather data available", 25, 31, 250, 1, false);
        }
        else {
            uint8_t weather_index    = data.config.weather_type;
            bool    is_negative_sign = false;

            if (weather_index & CONFIGURATION_MANAGER_WEATHER_SIGN_BITMASK) {
                is_negative_sign = true;
                weather_index    = weather_index & CONFIGURATION_MANAGER_WEATHER_IMAGE_BITMASK;
            }
            uint8_t text_width = strlen((char *)weather_conditions[weather_index]);

            uint8_t aux_string_formatter[10];
            sprintf((char *)aux_string_formatter, "/%d.bmp", weather_index);
            display_manager_write_image_to_buffer(aux_string_formatter, 60, 180, true);
            display_manager_write_centered_message(weather_conditions[weather_index], text_width, 330, 1, true);

            if (is_negative_sign == true) {
                sprintf((char *)aux_string_formatter, "-%d F", data.config.temperature);
            }
            else {
                sprintf((char *)aux_string_formatter, "%d F", data.config.temperature);
            }
            display_manager_write_centered_message(aux_string_formatter, strlen((char *)aux_string_formatter), 360, 2,
                                                   true);
        }
    }
    else if (ret == STATUS_STORAGE_NULL_TXT_FILE) {
        display_manager_write_centered_message((uint8_t *)"no data file", 12, 340, 1, true);
    }
    else {
        display_manager_write_centered_message((uint8_t *)"invalid data", 12, 340, 1, true);
    }

    display_battery_status();
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    device_configuration.config.badge_state = BADGE_STATE_WEATHER;
    configuration_manager_store_configuration_data(&device_configuration);
    is_spi_free = true;
}

void display_sos_screen(void) {
    is_spi_free      = false;
    is_controls_free = false;
    is_sos_triggered = true;

    display_manager_clear_display_buffers();
    display_manager_write_centered_message((uint8_t *)"sos mode started", 16, 210, 1, true);

    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    device_configuration.config.badge_state = BADGE_STATE_NONE;
    configuration_manager_store_configuration_data(&device_configuration);

    is_spi_free = true;
}

void display_battery_status(void) {
    float soc = 0.0f;
    max_driver_read_soc(&soc);

    uint16_t x = BATTERY_ICON_X_COORD;
    uint16_t y = BATTERY_ICON_Y_COORD;

    if (soc > BATTERY_4_LINES_THRESHOLD) {
        display_manager_write_image_to_buffer(BATTERY_4_LINES_IMAGE_BMP_FILENAME, x, y, false);
    }
    else if (soc > BATTERY_3_LINES_THRESHOLD) {
        display_manager_write_image_to_buffer(BATTERY_3_LINES_IMAGE_BMP_FILENAME, x, y, false);
    }
    else if (soc > BATTERY_2_LINES_THRESHOLD) {
        display_manager_write_image_to_buffer(BATTERY_2_LINES_IMAGE_BMP_FILENAME, x, y, false);
    }
    else if (soc > BATTERY_1_LINES_THRESHOLD) {
        display_manager_write_image_to_buffer(BATTERY_1_LINES_IMAGE_BMP_FILENAME, x, y, false);
    }
    else {
        display_manager_write_image_to_buffer(BATTERY_0_LINES_IMAGE_BMP_FILENAME, x, y, false);
    }
}

void task_dwm(void *arg) {
    while (1) {
        if (is_spi_free == true) {
            is_spi_free = false;
            dwm_manager_transmit_blink(device_configuration.config.device_id, DWM_MANAGER_BLINK_REQUEST,
                                       is_sos_triggered);
            is_spi_free = true;
        }
        _delay_ms(device_configuration.config.position_update_period);
    }
}

void task_controls(void *arg) {
    controls_button_e new_action      = 0;
    controls_button_e previous_action = 0;

    uint8_t inactive_counter = 0;
    uint8_t press_counter    = 0;

    while (1) {
        if (is_controls_free == true) {
            check_for_new_action(&new_action, true);
            if (inactive_counter > 150) {
                action           = previous_action;
                inactive_counter = 0;
                press_counter    = 0;
            }

            if (new_action == 0) {
                inactive_counter++;
            }
            else {
                if (new_action == CONTROLS_BUTTON_2 && previous_action == new_action) {
                    press_counter++;
                }

                if (press_counter >= 1) {
                    action        = CONTROLS_BUTTON_SOS;
                    press_counter = 0;
                }

                previous_action  = new_action;
                inactive_counter = 0;
            }
        }

        _delay_ms(10);
    }
}

status_t essentials_init(void) {
    status_t config_file_status = configuration_manager_get_configuration_data(&device_configuration);
    if (config_file_status == STATUS_STORAGE_NULL_TXT_FILE) {
        display_manager_init();
        display_manager_clear_display_buffers();
        display_manager_write_centered_message((uint8_t *)"INVALID CONFIGURATION FILE", 26, 200, 1, false);
        display_manager_write_centered_message((uint8_t *)"please reset", 12, 220, 1, false);
        display_manager_write_centered_message((uint8_t *)"the device will be reverted to", 28, 240, 0, false);
        display_manager_write_centered_message((uint8_t *)"the default configurations", 26, 255, 0, false);
        display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

        configuration_manager_init();

        _delay_ms(1000);
        hibernate();
    }

    dwm_setup();
    check_device_id_configuration();
    CHECK_FUNCTION(max_driver_init());

    is_spi_free = true;
    return STATUS_OK;
}

status_t complete_init(void) {
    CHECK_FUNCTION(display_manager_init());
    CHECK_FUNCTION(notification_manager_init());

    if (is_first_start == true) {
        is_first_start         = false;
        last_request_timestamp = get_time();
        last_report_timestamp  = get_time();
    }

    if ((device_configuration.config.notification_state & NOTIFICATION_MANAGER_ALERT_MODE_BITMASK) != 0) {
        check_alert_mode_conditions(device_configuration.config.notification_state, false);
    }
    else {
        uint8_t  buffer[DWM_MANAGER_MAXIMUM_FRAME_LENGTH];
        uint32_t buffer_size = 1;

        memset(buffer, 0, DWM_MANAGER_MAXIMUM_FRAME_LENGTH);
        if (request_notifications(buffer, buffer_size)) {
            configuration_manager_update_configuration_data_dwm(&device_configuration, buffer);
        }

        uint8_t wakeup_pin = esp_get_wakeup_pin();
        if (wakeup_pin == CONTROLS_BUTTON_1 && device_configuration.config.badge_state != BADGE_STATE_WEATHER) {
            display_weather_screen();
        }
        else if (wakeup_pin == CONTROLS_BUTTON_2
                 && device_configuration.config.badge_state != BADGE_STATE_NOTIFICATIONS) {
            display_notifications_screen();
        }
        else if (wakeup_pin == CONTROLS_BUTTON_3 && device_configuration.config.badge_state != BADGE_STATE_PROFILE) {
            display_profile_screen();
        }
        else if (device_configuration.config.badge_state == BADGE_STATE_NONE) {
            display_profile_screen();
        }
    }

    return STATUS_OK;
}

status_t dwm_setup(void) {
    dwm_manager_config_t dwm_configuration;

    if (device_configuration.config.dwm_channel == 5 || device_configuration.config.dwm_channel == 9) {
        dwm_configuration.channel      = device_configuration.config.dwm_channel;
        dwm_configuration.data_rate    = device_configuration.config.dwm_data_rate;
        dwm_configuration.preamble_len = device_configuration.config.dwm_preamble_len;
        dwm_configuration.sfd_type     = device_configuration.config.dwm_sfd_type;
        CHECK_FUNCTION(dwm_manager_init(&dwm_configuration));
    }
    else {
        CHECK_FUNCTION(dwm_manager_init(NULL));
    }

    return STATUS_OK;
}

status_t accelerometer_setup(void) {
    adxl345_driver_configuration_t adxl_configuration = ADXL345_DEFAULT_CONFIGURATION();
    adxl_configuration.activity_threshold             = device_configuration.config.wakeup_sensitivity;
    CHECK_FUNCTION(adxl345_driver_init(&adxl_configuration));

    return STATUS_OK;
}

status_t sleep_setup(void) {
    wakeup_pin_configuration_t wakeup_configuration = {
        .gpio_wakeup_pins           = {CONTROLS_BUTTON_1, CONTROLS_BUTTON_2, CONTROLS_BUTTON_3},
        .interrupt_wakeup_pins      = {ADXL345_DRIVER_INTERRUPT_PIN},
        .gpio_wakeup_pin_count      = 3,
        .interrupt_wakeup_pin_count = 1,
    };

    esp_sleep_configuration(&wakeup_configuration, ESP_SLEEP_MODE_DEEP_SLEEP);
    initialize_sleep_reset_interrupt();

    return STATUS_OK;
}

void check_device_id_configuration(void) {
    uint8_t device_id[16];
    memset(device_id, 0, 16);

    if (memcmp(device_id, device_configuration.config.device_id, 16) == 0) {
        int rand_num;
        srand(time(0));
        rand_num = rand();

        uint8_t  buffer[DWM_MANAGER_MAXIMUM_FRAME_LENGTH] = {SYSTEM_CMD_UWB_REQUEST_NEW_BADGE_ID, rand_num % 256};
        uint32_t buffer_size                              = 2;
        uint8_t  counter                                  = 100;

        is_spi_free = false;
        dwm_manager_request_data(buffer, &buffer_size);

        while (counter--) {
            dwm_manager_poll_irq();

            dwm_manager_retrieve_data(buffer, &buffer_size);
            if (buffer_size != 0) {
                if (buffer[0] == SYSTEM_CMD_UWB_SEND_NEW_BADGE_ID) {
                    if (buffer[1] == (uint8_t)(rand_num % 256)) {
                        memcpy(device_configuration.config.device_id, buffer + 2, DEVICE_UUID_LENGTH);
                        configuration_manager_store_configuration_data(&device_configuration);

                        break;
                    }
                }

                buffer_size = 0;
            }
            _delay_ms(10);
        }
    }

    is_spi_free = true;
}

void check_report_status(void) {
    int32_t current_time = get_time();

    if (current_time - last_report_timestamp >= REPORT_TIME_DIFFERENCE) {
        reports_transmit_bme68x_data(&device_configuration);
        last_report_timestamp = current_time;
    }
}

void check_notificaton_status(void) {
    int32_t current_time = get_time();

    if (current_time - last_request_timestamp >= device_configuration.config.notification_poll_period) {
        uint8_t  buffer[DWM_MANAGER_MAXIMUM_FRAME_LENGTH];
        uint32_t buffer_size = 1;

        memset(buffer, 0, DWM_MANAGER_MAXIMUM_FRAME_LENGTH);
        if (request_notifications(buffer, buffer_size)) {
            is_spi_free = false;
            configuration_manager_update_configuration_data_dwm(&device_configuration, buffer);

            uint8_t target = buffer[10];
            uint8_t offset = 0;
            if (target != 0) {
                offset += 16;

                if (memcmp(buffer + 11, device_configuration.config.device_id, 16) != 0) {
                    return;
                }
            }

            uint8_t settings = buffer[11 + offset];
            if (settings != 0) {
                if ((device_configuration.config.notification_state & NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK)
                    != (settings & NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK)) {
                    configuration_manager_update_notifications_data(buffer + 12 + offset, buffer[14 + offset]);
                    device_configuration.config.notification_state = settings;
                    configuration_manager_store_configuration_data(&device_configuration);
                    check_alert_mode_conditions(settings, false);
                }
            }
            is_spi_free = true;
        }

        last_request_timestamp = current_time;
    }
}

bool check_targeted_notification_status(void) {
    if (update.fields.message[0] == 1) {
        update.fields.message[0] = '/';
    }
    else {
        return false;
    }

    uint8_t file_name[40];
    memset(file_name, 0, 40);

    sprintf((char *)file_name, "%s.bmp", update.fields.message);

    if ((device_configuration.config.targeted_notification_state & NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK)
        != (update.fields.notification_settings & NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK)) {

        display_manager_init();
        notification_manager_init();

        device_configuration.config.targeted_notification_state = update.fields.notification_settings;
        is_spi_free                                             = false;

        display_manager_clear_display_buffers();
        if (display_manager_write_image_to_buffer(file_name, 0, 0, true) != STATUS_OK) {
            display_manager_write_centered_message((uint8_t *)"no valid image", 14, 200, 2, true);
        }
        else {
            display_battery_status();
        }
        display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

        configuration_manager_store_configuration_data(&device_configuration);
        is_spi_free = true;

        check_alert_mode_conditions(update.fields.notification_settings, true);

        return true;
    }

    return false;
}

bool request_notifications(uint8_t *buffer, uint32_t buffer_size) {
    LOG_INFO("requesting notifications");
    uint8_t retries = 10;

    buffer_size = 21;
    buffer[0]   = SYSTEM_CMD_UWB_REQUEST_NOTIFICATION;
    memcpy(&buffer[1], device_configuration.config.device_id, 16);

    float raw_battery_percentage;
    max_driver_read_soc(&raw_battery_percentage);

    uint32_t battery_percentage = (uint32_t)(raw_battery_percentage * 100);

    if (battery_percentage > 100 * 100) {
        battery_percentage = 0;
    }

    buffer[17] = (uint8_t)(battery_percentage / 100);
    buffer[18] = BADGE_VERSION_MAJOR;
    buffer[19] = BADGE_VERSION_MINOR;
    buffer[20] = BADGE_VERSION_PATCH;

    is_spi_free = false;

    dwm_manager_request_data(buffer, &buffer_size);
    while (retries--) {
        dwm_manager_poll_irq();

        uint8_t  buffer_aux[DWM_MANAGER_MAXIMUM_FRAME_LENGTH];
        uint32_t buffer_size_aux;
        dwm_manager_retrieve_data(buffer_aux, &buffer_size_aux);

        if (buffer_size_aux != 0) {

            if (buffer_aux[0] == SYSTEM_CMD_UWB_SEND_NOTIFICATION) {
                memcpy(update.buffer, buffer_aux, buffer_size_aux - 2);

                memcpy(buffer, update.buffer, 10);
                buffer[10] = 1;
                memcpy(buffer + 11, update.fields.target, DEVICE_UUID_LENGTH);

                configuration_manager_get_configuration_data(&device_configuration);

                if (update.fields.has_file_data == true
                    && update.fields.file_update_settings != device_configuration.config.file_update_state) {

                    device_configuration.config.badge_state       = BADGE_STATE_NONE;
                    device_configuration.config.file_update_state = update.fields.file_update_settings;
                    configuration_manager_store_configuration_data(&device_configuration);

                    if (strcmp((char *)device_configuration.config.wifi_ssid, "")
                        && strcmp((char *)device_configuration.config.wifi_pass, "")) {
                        display_manager_clear_display_buffers();
                        display_manager_write_centered_message((uint8_t *)"file update in progress", 23, 210, 1, false);
                        display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

                        uint8_t address[40];
                        memset(address, 0, 40);

                        uint8_t formatted_filename[64];
                        memset(formatted_filename, 0, 64);

                        uint8_t formatted_token[40];
                        memset(formatted_token, 0, 40);

                        client_request_t req = {
                            .port         = update.fields.port,
                            .path         = "/badgeUploads",
                            .file_command = BADGE_FILE_COMMAND_DOWNLOAD,
                        };

                        sprintf((char *)address, "%d.%d.%d.%d", update.fields.server_address[0],
                                update.fields.server_address[1], update.fields.server_address[2],
                                update.fields.server_address[3]);

                        memcpy(req.filename, "/", 1);
                        memcpy(req.filename + 1, update.fields.filename, strlen((char *)update.fields.filename));

                        uint8_t command  = (update.fields.file_update_settings & BADGE_FILE_SETTINGS_BITMASK) >> 4;
                        req.file_command = command;

                        if (command == BADGE_FILE_COMMAND_FIRMWARE_UPDATE) {
                            req.file_command = BADGE_FILE_COMMAND_FIRMWARE_UPDATE;
                            memcpy(formatted_filename, update.fields.filename, 24);
                        }
                        else if (command == BADGE_FILE_COMMAND_UPLOAD) {
                            memcpy(req.path, "/badgeDownload", 15);
                            memset(formatted_filename, 0, sizeof(formatted_filename));
                            memcpy(req.badge_id, device_configuration.config.device_id, DEVICE_UUID_LENGTH);
                            sprintf((char *)formatted_filename, "/%s", (char *)update.fields.filename);
                        }
                        else if (update.fields.file_target_type != 0) {
                            format_name(formatted_filename, update.fields.filename);
                        }
                        else {
                            memcpy(formatted_filename, update.fields.filename, 24);
                        }

                        format_token(formatted_token, update.fields.token);

                        CHECK_FUNCTION(
                            wifi_init(device_configuration.config.wifi_ssid, device_configuration.config.wifi_pass));

                        memcpy(req.formatted_filename, formatted_filename, strlen((char *)formatted_filename));
                        memcpy(req.host, address, 40);
                        memcpy(req.token, formatted_token, 40);

                        if (command == BADGE_FILE_COMMAND_FIRMWARE_UPDATE) {
                            CHECK_FUNCTION(client_http_stream(&req));
                            ota_update_firmware(req.filename);
                        }
                        else if (command == BADGE_FILE_COMMAND_UPLOAD) {
                            CHECK_FUNCTION(client_http_post(&req));
                        }
                        else {
                            CHECK_FUNCTION(client_http_get(&req));
                            if (command == BADGE_FILE_COMMAND_UPDATE_CONFIGURATION) {
                                configuration_manager_get_configuration_data(&device_configuration);
                                device_configuration.config.badge_state       = BADGE_STATE_NONE;
                                device_configuration.config.file_update_state = update.fields.file_update_settings;
                                configuration_manager_store_configuration_data(&device_configuration);
                            }
                            esp_restart();
                        }
                    }
                    else {
                        LOG_WARNING("wifi and password not specified");
                        display_manager_render_error_message((uint8_t *)"Wi-Fi password not specified", 29);
                    }
                }

                if (update.fields.has_notif_data == true) {
                    buffer[11 + DEVICE_UUID_LENGTH] = update.fields.notification_settings;
                    buffer[12 + DEVICE_UUID_LENGTH] = update.fields.hour;
                    buffer[13 + DEVICE_UUID_LENGTH] = update.fields.minute;
                    buffer[14 + DEVICE_UUID_LENGTH] = strlen((char *)update.fields.message);
                    if (check_targeted_notification_status() == true) {
                        return false;
                    }

                    memcpy(buffer + 15 + DEVICE_UUID_LENGTH, update.fields.message, buffer[14 + DEVICE_UUID_LENGTH]);

                    return true;
                }

                is_spi_free = true;
            }
        }

        _delay_ms(10);
    }

    is_spi_free = true;
    return false;
}

void format_name(uint8_t *output, uint8_t *buffer) {
    uint8_t uuid[32];

    for (size_t i = 0; i < DEVICE_UUID_LENGTH; i++) {
        sprintf((char *)uuid + (i * 2), "%02x", device_configuration.config.device_id[i]);
    }

    char *name      = strtok((char *)buffer, ".");
    char *extension = strtok(NULL, ".");

    memcpy(output, name, strlen(name));
    memcpy(output + strlen(name), uuid, 32);
    memcpy(output + strlen(name) + 32, ".", 1);
    memcpy(output + strlen(name) + 32 + 1, extension, strlen(extension));

    output[strlen((char *)output)] = 0;
}

void format_token(uint8_t *output, uint8_t *buffer) {
    for (size_t i = 0; i < DEVICE_UUID_LENGTH; i++) {
        sprintf((char *)output + (i * 2), "%02x", buffer[i]);
    }
    output[32] = 0;
}

void check_alert_mode_conditions(uint8_t settings, bool is_zone_notification) {
    is_controls_free = false;
    is_spi_free      = false;

    notification_manager_set_has_new_notification(true);
    notification_manager_enter_alert_mode(settings);
    notification_manager_set_has_new_notification(false);

    if (is_zone_notification == true) {
        if (device_configuration.config.badge_state == BADGE_STATE_WEATHER) {
            display_weather_screen();
        }
        else if (device_configuration.config.badge_state == BADGE_STATE_NOTIFICATIONS) {
            display_notifications_screen();
        }
        else if (device_configuration.config.badge_state == BADGE_STATE_PROFILE) {
            display_profile_screen();
        }
    }
    else {
        device_configuration.config.is_notifications_screen_built = false;
        display_notifications_screen();
    }

    is_controls_free = true;
    is_spi_free      = true;
}

void check_configuration_menu_init(void) {
    uint16_t counter = 0;
    while (gpio_get_level(CONTROLS_BUTTON_1) == 1) {
        counter++;
        _delay_ms(10);
    }

    if (counter > CONFIGURATION_MENU_BUTTON_THRESHOLD * 100) {
        essentials_init();
        display_manager_init();
        m24lr_driver_init();

        configuration_manager_get_configuration_data(&device_configuration);
        device_configuration.config.is_profile_screen_built = false;
        device_configuration.config.is_weather_screen_built = false;
        device_configuration.config.badge_state             = BADGE_STATE_NONE;
        configuration_manager_store_configuration_data(&device_configuration);

        configuration_menu_loop();
    }
}

void check_test_menu_init(void) {
    uint16_t counter = 0;
    while (gpio_get_level(CONTROLS_BUTTON_2) == 1) {
        counter++;
        _delay_ms(10);
    }

    if (counter > CONFIGURATION_MENU_BUTTON_THRESHOLD * 100) {
        configuration_manager_get_configuration_data(&device_configuration);
        device_configuration.config.badge_state = BADGE_STATE_NONE;
        configuration_manager_store_configuration_data(&device_configuration);

        badge_test_init();
        badge_test_loop();
    }
}

void hibernate(void) {
    if (is_sos_triggered == false) {
        _delay_ms(10);
        dwm_manager_enable_sleep();

        SET_PIN_LOW(5);
        LOG_INFO("hibernate mode start");
        esp_hibernate_enable();
    }
}

void initialize_sleep_reset_interrupt(void) {
    gpio_config_t io_conf = {};
    io_conf.intr_type     = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask  = (1ULL << ADXL345_DRIVER_INTERRUPT_PIN);
    io_conf.mode          = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(ADXL345_DRIVER_INTERRUPT_PIN, sleep_counter_reset_interrupt_handler, NULL);
}

void IRAM_ATTR sleep_counter_reset_interrupt_handler(void *arg) {
    sleep_counter = 0;
}

status_t check_bmp_file(uint8_t *name, uint8_t *msg, uint8_t msg_size) {
    status_t bmp_file_status = display_manager_write_image_to_buffer(name, 0, 0, true);
    if (bmp_file_status != STATUS_OK) {
        display_manager_clear_display_buffers();
        display_manager_write_centered_message(msg, msg_size, 210, 1, false);
        display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

        return -1;
    }

    return STATUS_OK;
}

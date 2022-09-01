#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "utils.h"
#include "webserver.h"
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ACTIVATE       'F'
#define IS_ACTIVE      'X'
#define GET_LAST_ON    'O'
#define MINIMUM_PERIOD 600
#define FILL_PERIOD    10
#define VALVE_PIN      GPIO_NUM_26

static const char *TAG = "Main";
httpd_ws_frame_t   ws_response;
uint8_t            ws_payload[64];

uint8_t  is_active;
uint16_t last_active_time;
uint32_t minutes_since_active = 0;

uint16_t get_time(void) {
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);

    return timestamp.tv_sec;
}

uint16_t get_last_on() {
    uint16_t current_time = get_time();
    if (current_time > last_active_time) {
        return current_time - last_active_time;
    }
    else {
        return UINT16_MAX - last_active_time + current_time;
    }
}

uint8_t can_activate() {
    return get_last_on() > MINIMUM_PERIOD;
}

void format_response(uint8_t *message) {
    memset(&ws_response, 0, sizeof(httpd_ws_frame_t));
    ws_response.type    = HTTPD_WS_TYPE_TEXT;
    ws_response.payload = message;
    ws_response.len     = strlen((char *)message);
}

httpd_ws_frame_t *ws_event_handler(uint8_t *message) {
    sprintf((char *)ws_payload, "nope");

    if (message[0] == ACTIVATE) {
        if (can_activate()) {
            last_active_time     = get_time();
            minutes_since_active = 0;
            is_active            = FILL_PERIOD;
            gpio_set_level(VALVE_PIN, 1);
            sprintf((char *)ws_payload, "F:OK");
            ESP_LOGI(TAG, "Start");
        }
        else {
            sprintf((char *)ws_payload, "F:NO");
            ESP_LOGI(TAG, "Need to wait %d", MINIMUM_PERIOD - get_last_on());
        }
    }
    else if (message[0] == IS_ACTIVE) {
        sprintf((char *)ws_payload, "X:%d", is_active);
        ESP_LOGI(TAG, "Is active: %d", is_active);
    }
    else if (message[0] == GET_LAST_ON) {
        sprintf((char *)ws_payload, "O:%d", minutes_since_active);
        ESP_LOGI(TAG, "Minutes since last active: %d", minutes_since_active);
    }

    format_response(ws_payload);

    return &ws_response;
}

void app_main(void) {

    gpio_set_direction(VALVE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(VALVE_PIN, 0);

    start_websocket();

    uint8_t seconds = 0;

    while (1) {
        _delay_ms(1000);

        if (is_active) {
            is_active--;
        }
        else {
            gpio_set_level(VALVE_PIN, 0);
        }

        if (seconds++ >= 60) {
            minutes_since_active++;
            seconds = 0;
        }
    }
}
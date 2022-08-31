#include "esp_log.h"
#include "utils.h"
#include "webserver.h"
#include <string.h>

static const char *TAG = "Main";
httpd_ws_frame_t   ws_response;
uint8_t            ws_payload[64];

uint8_t counter = 1;

httpd_ws_frame_t *ws_event_handler(uint8_t *message) {
    ESP_LOGW(TAG, "got: %s", message);

    sprintf((const char *)ws_payload, "%d", counter);

    ws_response.payload = ws_payload;
    ws_response.len     = strlen((const char *)ws_payload);

    return &ws_response;
}

void app_main(void) {

    start_websocket();

    memset(&ws_response, 0, sizeof(httpd_ws_frame_t));
    ws_response.type = HTTPD_WS_TYPE_TEXT;

    while (1) {

        ESP_LOGE(TAG, "Count: %d", counter++);

        _delay_ms(1000);
    }
}
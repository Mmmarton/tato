#include "esp_log.h"
#include "utils.h"
#include "webserver.h"

static const char *TAG = "Main";

void app_main(void) {

    start_websocket();

    while (1) {

        ESP_LOGE(TAG, "Hello");

        _delay_ms(200);
    }
}
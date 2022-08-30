#include "log.h"
#include "esp_log.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const uint8_t log_message_overhead = 128;

char *format_log(char *format, ...) {
    char *  buffer = (char *)malloc(strlen(format) + log_message_overhead + 1);
    va_list log_arguments;

    va_start(log_arguments, format);
    vsprintf(buffer, format, log_arguments);
    va_end(log_arguments);

    return buffer;
}

void log_event_handler(log_levels_e level, char *tag, char *message) {
#ifdef LOG_LEVEL
    if (level >= LOG_LEVEL) {
        switch (level) {
            case LEVEL_DEBUG:
                printf("%s:%s\n", tag, message);
                break;
            case LEVEL_INFO:
                ESP_LOGI(tag, "%s", message);
                break;
            case LEVEL_WARNING:
                ESP_LOGW(tag, "%s", message);
                break;
            case LEVEL_ERROR:
                ESP_LOGE(tag, "%s", message);
                break;

            default:
                break;
        }
    }
#endif
    free(message);
}
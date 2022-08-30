#include "neopixel_driver.h"
#include "log.h"
#include "utilities.h"

#define MODULE_TAG "LED_DRIVER" /** tag used as argument for the LOG functions */

#define START_FRAME_SIGNAL 0x00
#define END_FRAME_SIGNAL   0x01
#define MAXIMUM_BRIGHTNESS 31

#if NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING == NEOPIXEL_DRIVER_BRIGHTNESS_ENABLED
    #define SET_BRIGHTNESS(x) (send_byte(0xe0 + x))
#endif

static bool is_init = false;

void send_byte(uint8_t c);
void send_signal(uint8_t signal);

status_t neopixel_driver_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    SET_OUTPUT_PIN(NEOPIXEL_DRIVER_DATA_PIN);
    SET_OUTPUT_PIN(NEOPIXEL_DRIVER_CLOCK_PIN);

    SET_PIN_LOW(NEOPIXEL_DRIVER_DATA_PIN);

    is_init = true;

    LOG_INFO("driver init ok");
    return STATUS_OK;
}

#if NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING == NEOPIXEL_DRIVER_BRIGHTNESS_ENABLED
status_t neopixel_driver_send_led_array(rgb_color_t *ledarray, uint8_t size, uint8_t brightness) {
    CHECK_INIT(is_init);
    send_signal(START_FRAME_SIGNAL);

    for (size_t i = 0; i < size; i++) {
        SET_BRIGHTNESS(brightness);
        send_byte(ledarray[i].blue);
        send_byte(ledarray[i].green);
        send_byte(ledarray[i].red);
    }

    send_signal(END_FRAME_SIGNAL);
    return STATUS_OK;
}

#elif NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING == LED_DRIVER_BRIGHTNESS_DISABLED
status_t neopixel_driver_send_led_array(rgb_color_t *ledarray, uint8_t size) {
    CHECK_INIT(is_init);
    send_signal(START_FRAME_SIGNAL);

    for (size_t i = 0; i < size; i++) {
        SET_BRIGHTNESS(NEOPIXEL_DEFAULT_BRIGHTNESS);
        send_byte(ledarray[i].blue);
        send_byte(ledarray[i].green);
        send_byte(ledarray[i].red);
    }

    send_signal(END_FRAME_SIGNAL);
    return STATUS_OK;
}
#endif

void send_byte(uint8_t data_byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (!(data_byte & 0x80)) {
            SET_PIN_LOW(NEOPIXEL_DRIVER_DATA_PIN);
        }
        else {
            SET_PIN_HIGH(NEOPIXEL_DRIVER_DATA_PIN);
        }

        SET_PIN_HIGH(NEOPIXEL_DRIVER_CLOCK_PIN);
        data_byte <<= 1;
        SET_PIN_LOW(NEOPIXEL_DRIVER_CLOCK_PIN);
    }
}

void send_signal(uint8_t signal) {
    for (int i = 0; i < 4; i++) {
        send_byte(signal);
    }
}
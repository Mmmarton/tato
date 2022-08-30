#include "controls.h"
#include "error_manager.h"
#include "log.h"
#include "utilities.h"

#include <stdint.h>
#include <stdio.h>

#define MODULE_TAG "CONTROLS"

uint8_t debounce(uint8_t pin, bool debounce_on);

static uint8_t pins[]  = {CONTROLS_BUTTON_1, CONTROLS_BUTTON_2, CONTROLS_BUTTON_3};
static bool    is_init = false;

status_t controls_driver_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    SET_INPUT_PIN(CONTROLS_BUTTON_1);
    SET_INPUT_PIN(CONTROLS_BUTTON_2);
    SET_INPUT_PIN(CONTROLS_BUTTON_3);

    LOG_DEBUG("driver init ok");
    is_init = true;
    return STATUS_OK;
}

status_t check_for_new_action(controls_button_e *action, bool debounce_on) {
    CHECK_INIT(is_init);

    for (uint8_t i = 0; i < sizeof(pins); i++) {
        uint8_t debounce_value = debounce(pins[i], debounce_on);
        if (debounce_value != 0) {
            (*action) = pins[i];
            return STATUS_OK;
        }
    }

    (*action) = 0;
    return 1;
}

uint8_t debounce(uint8_t pin, bool debounce_on) {
    bool current_state = true;
    bool button_state  = true;

    uint16_t press_count  = 0;
    uint8_t  return_state = 0;

    current_state = (!(gpio_get_level(pin)));
    if (current_state != button_state) {
        button_state = current_state;
        while (button_state == current_state) {
            if (press_count > PRESS_COUNT) {
                return_state = !button_state;
                
                if (debounce_on == false) {
                    break;
                }
            }
            press_count++;
            current_state = (!(gpio_get_level(pin)));
        }
    }
    else {
        press_count = 0;
    }

    return return_state;
}
#include "notification_manager.h"
#include "configuration_manager.h"
#include "controls.h"
#include "dwm_manager.h"
#include "log.h"
#include "utilities.h"

#include "neopixel_driver.h"
#include "pwm_driver.h"

#define MODULE_TAG "NOTIFICATION_MANAGER"

#define LED_COUNT              4
#define BUZZER_DELAY           20
#define SETTINGS_BUZZER_MASK   (1 << 5)
#define SETTINGS_HAPTICS_MASK  (1 << 4)
#define SETTINGS_SEVERITY_MASK 0b11000000
#define BUZZER_PIN             17
#define HAPTICS_PIN            4

static pwm_channel_e haptics;
static pwm_channel_e buzzer;

static rgb_color_t notification_danger[LED_COUNT];
static rgb_color_t notification_warning[LED_COUNT];
static rgb_color_t notification_success[LED_COUNT];
static rgb_color_t notification_info[LED_COUNT];

static bool has_new_notification = false;
static bool is_init              = false;

void set_led_color(rgb_color_t *led_array, rgb_color_t led_color);
void set_leds(uint8_t type);
void set_buzzer(uint8_t value);
void set_haptics(uint8_t value);
bool loop_period(uint8_t on_time, uint8_t off_time, uint8_t toggle_num, uint16_t period, uint8_t settings);
bool loop_delay(uint16_t counter);

status_t notification_manager_init(void) {
    if (is_init == true) {
        LOG_WARNING("module already init");
        return STATUS_ALREADY_INIT;
    }

    pwm_driver_init(&haptics, HAPTICS_PIN, 2000);
    pwm_driver_init(&buzzer, BUZZER_PIN, 2000);

    CHECK_FUNCTION(neopixel_driver_init());

    // @TODO: read color values from configuration
    set_led_color(notification_danger, (rgb_color_t){3, 3, 100});
    set_led_color(notification_warning, (rgb_color_t){3, 70, 100});
    set_led_color(notification_success, (rgb_color_t){3, 100, 3});
    set_led_color(notification_info, (rgb_color_t){100, 100, 3});

    is_init = true;
    return STATUS_OK;
}

status_t notification_manager_enter_alert_mode(uint8_t settings) {
    CHECK_INIT(is_init);

    while (has_new_notification == true) {
        _delay_ms(BUZZER_DELAY);
        if (loop_period(ON_TIME, OFF_TIME, TOGGLE_COUNT, PERIOD - BUZZER_DELAY, settings) == true) {
            set_leds(false);
            set_haptics(false);
            set_buzzer(false);
            break;
        }
    }

    return STATUS_OK;
}

void notification_manager_set_has_new_notification(bool state) {
    has_new_notification = state;
}

bool notification_manager_get_has_new_notification(void) {
    return has_new_notification;
}

bool loop_period(uint8_t on_time, uint8_t off_time, uint8_t toggle_num, uint16_t period, uint8_t settings) {
    uint8_t toggle_count = toggle_num;

    while (toggle_count--) {
        set_leds(((settings & SETTINGS_SEVERITY_MASK) >> 6) + 1);
        set_haptics(settings & SETTINGS_HAPTICS_MASK);
        set_buzzer(settings & SETTINGS_BUZZER_MASK);
        if (loop_delay(on_time)) {
            return true;
        }

        set_leds(false);
        set_haptics(false);
        set_buzzer(false);
        if (loop_delay(off_time)) {
            return true;
        }
    }

    uint16_t wait_time = period - toggle_num * (on_time + off_time);
    if (loop_delay(wait_time)) {
        set_leds(false);
        set_haptics(false);
        set_buzzer(false);
        return true;
    }

    return false;
}

bool loop_delay(uint16_t counter) {
    controls_button_e action = 0;

    while (counter--) {
        _delay_ms(10);
        check_for_new_action(&action, false);
        if (action == CONTROLS_BUTTON_2) {
            return true;
        }
    }
    return false;
}

void set_buzzer(uint8_t value) {
    if (buzzer == PWM_DRIVER_INVALID_CHANNEL) {
        return;
    }

    if (value) {
        pwm_driver_set_duty_cycle(buzzer, 50);
    }
    else {
        pwm_driver_set_duty_cycle(buzzer, 0);
    }
}

void set_haptics(uint8_t value) {
    if (haptics == PWM_DRIVER_INVALID_CHANNEL) {
        return;
    }

    if (value) {
        pwm_driver_set_duty_cycle(haptics, 30);
    }
    else {
        pwm_driver_set_duty_cycle(haptics, 0);
    }
}

void set_leds(uint8_t type) {
    if (type) {
        switch (type) {
            case NOTIFICATION_TYPE_DANGER:
                neopixel_driver_send_led_array(notification_danger, sizeof(notification_danger), 5);
                break;

            case NOTIFICATION_TYPE_WARNING:
                neopixel_driver_send_led_array(notification_warning, sizeof(notification_warning), 5);
                break;

            case NOTIFICATION_TYPE_SUCCESS:
                neopixel_driver_send_led_array(notification_success, sizeof(notification_success), 5);
                break;

            case NOTIFICATION_TYPE_INFO:
            default:
                neopixel_driver_send_led_array(notification_info, sizeof(notification_info), 5);
                break;
        }
    }
    else {
        neopixel_driver_send_led_array(notification_info, sizeof(notification_info), 0);
    }
}

void set_led_color(rgb_color_t *led_array, rgb_color_t led_color) {
    led_array[0] = led_color;
    led_array[1] = led_color;
    led_array[2] = led_color;
    led_array[3] = led_color;
}

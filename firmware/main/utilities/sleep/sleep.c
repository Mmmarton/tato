#include "sleep.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "log.h"

#define MODULE_TAG "SLEEP"

void enable_light_sleep(wakeup_pin_configuration_t *configuration);
void enable_deep_sleep(wakeup_pin_configuration_t *configuration);

void esp_sleep_configuration(wakeup_pin_configuration_t *configuration, esp_sleep_mode_e mode) {
    switch (mode) {
        case ESP_SLEEP_MODE_LIGHT_SLEEP:
            enable_light_sleep(configuration);
            break;

        case ESP_SLEEP_MODE_DEEP_SLEEP:
            enable_deep_sleep(configuration);
            break;

        default:
            break;
    }
}

void esp_light_sleep_enable(void) {
    esp_light_sleep_start();
}

void esp_deep_sleep_enable(void) {
    esp_deep_sleep_start();
}

void esp_hibernate_enable(void) {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);

    esp_deep_sleep_start();
}

void esp_print_wakeup_reason(void) {
    if (esp_sleep_get_ext1_wakeup_status() != 0) {
        printf("%lld\n", esp_sleep_get_ext1_wakeup_status());
    }

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            LOG_INFO("Wakeup caused by external signal using RTC_IO\n");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            LOG_INFO("Wakeup caused by external signal using RTC_CNTL\n");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            LOG_INFO("Wakeup caused by timer\n");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            LOG_INFO("Wakeup caused by touchpad\n");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            LOG_INFO("Wakeup caused by ULP program\n");
            break;
        default:
            break;
    }
}

void enable_light_sleep(wakeup_pin_configuration_t *configuration) {
    for (size_t i = 0; i < configuration->interrupt_wakeup_pin_count; i++) {
        rtc_gpio_pulldown_en(configuration->interrupt_wakeup_pins[i]);
        esp_sleep_enable_ext0_wakeup(configuration->interrupt_wakeup_pins[i], 0);
    }

    for (size_t i = 0; i < configuration->gpio_wakeup_pin_count; i++) {
        gpio_wakeup_enable(configuration->gpio_wakeup_pins[i], GPIO_INTR_LOW_LEVEL);
    }
    esp_sleep_enable_gpio_wakeup();
}

void enable_deep_sleep(wakeup_pin_configuration_t *configuration) {
    uint64_t pin_mask = 0;

    for (size_t i = 0; i < configuration->interrupt_wakeup_pin_count; i++) {
        pin_mask |= 1ULL << configuration->interrupt_wakeup_pins[i];
    }

    for (size_t i = 0; i < configuration->gpio_wakeup_pin_count; i++) {
        pin_mask |= 1ULL << configuration->gpio_wakeup_pins[i];
    }

    esp_sleep_enable_ext1_wakeup(pin_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
}

uint8_t esp_get_wakeup_pin(void) {
    uint8_t pin = 0;

    uint64_t mask = esp_sleep_get_ext1_wakeup_status();
    if (mask != 0) {
        while ((mask & 1) == 0) {
            pin++;
            mask >>= 1;
        }
    }

    return pin;
}

void esp_enable_sleep_timer_seconds(uint64_t timer) {
    esp_sleep_enable_timer_wakeup(timer * 1000 * 1000);
}
#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>

/** @brief This structure provides two arrays of up to 8 pins (4 GPIOs and 4 interrupts) that can trigger
 *         a wake-up on the ESP32 MCU. */
typedef struct {
    uint8_t gpio_wakeup_pin_count;      /*!< number of GPIO pins that can trigger a wake-up */
    uint8_t interrupt_wakeup_pin_count; /*!< number of interrupt pins that can trigger a wake-up */
    uint8_t gpio_wakeup_pins[4];        /*!< GPIO pins that can trigger a wake-up (only work in light sleep) */
    uint8_t interrupt_wakeup_pins[4];   /*!< interrupt pins that can trigger a wake-up (MUST be RTC_IO) */
} wakeup_pin_configuration_t;

typedef enum {
    ESP_SLEEP_MODE_LIGHT_SLEEP,
    ESP_SLEEP_MODE_DEEP_SLEEP,
} esp_sleep_mode_e;

/**
 * @brief This function is used to configure the wake-up triggers of the ESP32 MCU.
 *
 * @param[in]   configuration   Structure containing wake-up trigger data.
 * @param[in]   mode            Sleep mode configuration.
 *
 * @return      (void)
 */
void esp_sleep_configuration(wakeup_pin_configuration_t *configuration, esp_sleep_mode_e mode);

/**
 * @brief This function is used to put the ESP32 MCU into light sleep mode. Please note that `light_sleep` mode must be
 *        configured first.
 *
 * @return      (void)
 */
void esp_light_sleep_enable(void);

/**
 * @brief This function is used to put the ESP32 MCU into deep sleep mode. Please note that `deep_sleep` mode must be
 *        configured first.
 *
 * @return      (void)
 */
void esp_deep_sleep_enable(void);

/**
 * @brief This function is used to put the ESP32 MCU into hibernate mode. Please note that `deep_sleep` mode must be
 *        configured first.
 *
 * @return      (void)
 */
void esp_hibernate_enable(void);

/**
 * @brief This function is used to print the wake-up reason of the ESP32 MCU.
 *
 * @return (void)
 */
void esp_print_wakeup_reason(void);

/**
 * @brief This function is used to retrieve the pin that was pressed.
 *
 * @return (uint8_t)
 * @retval pin that caused the wakeup.
 */
uint8_t esp_get_wakeup_pin(void);

/**
 * @brief This function is used to configure wake-up timer for the ESP32 MCU.
 *
 * @param[in]   timer Delay until a wake-up is triggered (in seconds).
 *
 * @return      (void)
 */
void esp_enable_sleep_timer_seconds(uint64_t timer);

#endif
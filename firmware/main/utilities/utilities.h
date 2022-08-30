/*! @file utilities.h
 * @brief This module provides general use functions that might be use throught an entire project.
 */
/*!
 * @defgroup UTILITIES
 * @{*/

#ifndef UTILITIES_H
    #define UTILITIES_H

    #include "driver/gpio.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "sdkconfig.h"

    #define NULL_BYTE    0x00 /*!< byte mask: 0b00000000 */
    #define ENABLED_BYTE 0xFF /*!< byte mask: 0b11111111 */

    #define CONCAT(x, y) x##y /*!< concatenates 2 given entities */

    #define SET_OUTPUT_PIN(x) (gpio_set_direction(x, GPIO_MODE_OUTPUT)) /*!< set pin as output */
    #define SET_INPUT_PIN(x)  (gpio_set_direction(x, GPIO_MODE_INPUT))  /*!< set pin as input */
    #define SET_PIN_HIGH(x)   (gpio_set_level(x, 1))                    /*!< set pin level high */
    #define SET_PIN_LOW(x)    (gpio_set_level(x, 0))                    /*!< set pin level low */

    #define _delay_ms(x) (vTaskDelay(x / portTICK_PERIOD_MS)) /*!< delay function (ms) */

    #define RESET_REGISTER_BYTE(x) (x = 0x00)                       /*!< reset byte */
    #define MCU_HALT_EXECUTION     (SMCR |= (1 << SM1) | (1 << SE)) /*!< puts MCU to sleep */

    #define RETRY(condition, max_count, msg, err)  \
        {                                          \
            uint8_t attempt_counter = 0;           \
            while (condition) {                    \
                if (attempt_counter > max_count) { \
                    LOG_WARNING(msg);              \
                    return err;                    \
                }                                  \
                attempt_counter++;                 \
                _delay_ms(10);                     \
            }                                      \
        }

/**
 * @brief This function takes a number as input alongside the boundaries of the input and the
 *        boundaries of the output and does an interpolation of the `num` by making use of the
 *        given boundaries.
 *
 * @param[in]   num     number to be converted
 * @param[in]   in_min  minimum possible value of the input
 * @param[in]   in_max  maximum possible value of the input
 * @param[in]   out_min minimum possible value of the output
 * @param[in]   out_max maximum possible value of the output
 *
 * @return      (long)
 * @retval      result success
 * @retval      out_min error
 */
long map(long num, long in_min, long in_max, long out_min, long out_max);

/**
 * @brief This function is used to swap 2 integer values.
 *
 * @param[in]   x Integer value.
 * @param[in]   y Integer value.
 *
 * @return      (void)
 */
void swap_int(int *x, int *y);

/**
 * @brief This function is used to convert the temperature from degrees C to degrees F.
 *
 * @param[in]   temperature Temperature in C degrees.
 *
 * @return      (float)
 * @retval      Converted value of the input temperature.
 */
float celsius_to_fahrenheit(float temperature);

/**
 * @brief This function is used to retrieve the current system timestamp.
 *
 * @return      (int32_t)
 * @retval      CUrrent value of the system clock.
 */
int32_t get_time(void);

#endif
/** @}*/
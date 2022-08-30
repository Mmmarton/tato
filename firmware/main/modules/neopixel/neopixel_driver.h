/*! @file neopixel_driver.h
 *
 * @brief This driver is used to enable a neopixel array.
 */

/*!
 * @defgroup NEOPIXEL
 * @{*/

#ifndef NEOPIXEL_DRIVER_H
    #define NEOPIXEL_DRIVER_H

    #include "error_manager.h"
    #include "led_types.h"

    #define NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING \
        NEOPIXEL_DRIVER_BRIGHTNESS_ENABLED /*!< enable/disable LED brightness */

    /** @note: the default brightness macro only works when the runtime brigthness option is disabled */
    #if NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING == NEOPIXEL_DRIVER_BRIGHTNESS_DISABLED
        #define NEOPIXEL_DEFAULT_BRIGHTNESS 20 /*!< default brightness */
    #endif

    #define NEOPIXEL_DRIVER_DATA_PIN  19 /*!< data pin */
    #define NEOPIXEL_DRIVER_CLOCK_PIN 2  /*!< clock pin */

/**
 * @brief This function is used to initialize the neopixel_driver module by enabling the output on the specified pins.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t neopixel_driver_init(void);

    /**
     * @brief This function is used to set the color configuration of the LED strip by processing a rgb_color_t type
     * array passed as parameter. By default, the brightness is configurable, disabling the brightness setting resulting
     *        in global brightness being set to maximum (31).
     *
     * @param[in] ledarray   The array containing the RGB color configuration of the LED strip.
     * @param[in] size       Size of the array.
     * @param[in] brightness The global brightness of the LED strip.
     * @note: only works if the brightness setting is enabled
     *
     * @return (status_t)
     * @retval 0 success, others for errors
     */
    #if NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING == NEOPIXEL_DRIVER_BRIGHTNESS_ENABLED
status_t neopixel_driver_send_led_array(rgb_color_t *ledarray, uint8_t size, uint8_t brightness);
    #elif NEOPIXEL_DRIVER_ENABLE_BRIGHTNESS_SETTING == LED_DRIVER_BRIGHTNESS_DISABLED
status_t neopixel_driver_send_led_array(rgb_color_t *ledarray, uint8_t size);
    #endif

#endif
/** @}*/
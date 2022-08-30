/*! @file led_types.h
 *
 * @brief This module contains the neopixel LEDs configuration type and brightness options.
 */

/*!
 * @defgroup NEOPIXEL
 * @{*/

#ifndef LED_TYPES_H
    #define LED_TYPES_H

    #include <stdint.h>

    #define NEOPIXEL_DRIVER_BRIGHTNESS_ENABLED  0x01 /*!< enable LED brightness */
    #define NEOPIXEL_DRIVER_BRIGHTNESS_DISABLED 0x00 /*!< disable LED brightness */

/**
 * @brief This structure contains the BGR color codes for a particular LED.
 */
typedef struct {
    uint8_t blue;  /*!< BGR blue value */
    uint8_t green; /*!< BGR green value */
    uint8_t red;   /*!< red blue value */
} rgb_color_t;

#endif
/** @}*/
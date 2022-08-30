/*! @file adxl345_driver.h
 *
 * @brief This driver is used for the ADXL345 accelerometer.
 */

/*!
 * @defgroup ADXL345
 * @{*/

#ifndef ADXL345_DRIVER_H
    #define ADXL345_DRIVER_H

    #include <stdbool.h>
    #include <stdint.h>

    #include "error_manager.h"

    #define ADXL345_DRIVER_I2C_REG         0x1D /*!< I2C id of ADXL345 */
    #define ADXL345_DRIVER_READING_REG     0x32 /*!< starting address of the reading registers */
    #define ADXL345_DRIVER_BW_RATE_REG     0x2C /*!< register for low power reading control */
    #define ADXL345_DRIVER_POWER_CTRL_REG  0x2D /*!< power control configuration */
    #define ADXL345_DRIVER_DATA_FORMAT_REG 0x31 /*!< data format register */

    #define ADXL345_DRIVER_ACTIVITY_THRESHOLD_REG   0x24 /*!< address of activity threshold register */
    #define ADXL345_DRIVER_INACTIVITY_THRESHOLD_REG 0x25 /*!< address of inactivity threshold register */
    #define ADXL345_DRIVER_INACTIVITY_TIME_REG      0x26 /*!< address of inactivity time register */
    #define ADXL345_DRIVER_ACT_INACT_CTRL_REG       0x27 /*!< address of activity/inactivity control register */

    #define ADXL345_DRIVER_INTERRUPT_ENABLE_REG 0x2E /*!< interrupt enable register */
    #define ADXL345_DRIVER_INTERRUPT_MAP_REG    0x2F /*!< interrupt map register */
    #define ADXL345_DRIVER_INTERRUPT_SOURCE_REG 0x30 /*!< interrupt source register (read-only) */

    #define ADXL345_DRIVER_INTERRUPT_PIN 34 /*!< interrupt pin */

    /** @brief This macro initializes the adxl345_driver_configuration_t structure with the default settings. */
    #define ADXL345_DEFAULT_CONFIGURATION()                                                                   \
        {                                                                                                     \
            .range = 2, .activity_threshold = 4, .inactivity_threshold = 255, .inactivity_time = 0,           \
            .x_axis_enabled = true, .y_axis_enabled = true, .z_axis_enabled = true, .interrupt_level = false, \
        }

/**
 * @brief This structure contains the ADXL345 3-axis readings.
 */
typedef struct {
    float x_axis; /*!< value read for the x-axis */
    float y_axis; /*!< value read for the y-axis */
    float z_axis; /*!< value read for the z-axis */
} adxl345_reading_t;

/**
 * @brief This structure contains the default configuration of the ADXL345 driver.
 */
typedef struct {
    uint8_t range;                /*!< reading range, 2, 4, 8 or 16 */
    uint8_t activity_threshold;   /*!< threshold value that decides when the sensor wakes up */
    uint8_t inactivity_threshold; /*!< threshold value that decides when the sensor goes to sleep */
    uint8_t inactivity_time;      /*!< duration of inactivity before going to sleep */
    bool    x_axis_enabled;       /*!< checks x-axis for activity/inactivity */
    bool    y_axis_enabled;       /*!< checks y-axis for activity/inactivity */
    bool    z_axis_enabled;       /*!< checks z-axis for activity/inactivity */
    bool    interrupt_level;      /*!< interrupt level, true for active low, false for active high */
} adxl345_driver_configuration_t;

/**
 * @brief This function is used to initialize the ADXL345 driver.
 *
 * @param[in]   configuration   ADXL345 driver configuration
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t adxl345_driver_init(adxl345_driver_configuration_t *configuration);

/**
 * @brief This function is used to update the accelerometer activity/inactivity threshold, which in turn decides when
 *        the devices goes to/wakes up from sleep.
 *
 * @param[in]   threshold  Value between 0 and 100.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t adxl345_driver_update_activity_threshold(uint8_t threshold);

/**
 * @brief This function is used to retrieve a full data reading from the ADXL345 sensor.
 *
 * @param[out]  reading structure containing the full data reading
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t adxl345_driver_get_axis_data(adxl345_reading_t *reading);

/**
 * @brief This function is used to reset the interrupt registers of the ADXL345 sensor. This MUST be done in order to
 *        keep the interrupt line in the desired state.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t adxl345_driver_reset_interrupt(void);

/**
 * @brief This function is used to set the accelerometer to sleep mode.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t adxl345_driver_enter_sleep(void);

/**
 * @brief This function is used to set the accelerometer de standby mode.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t adxl345_driver_enter_standby(void);

#endif
/** @}*/
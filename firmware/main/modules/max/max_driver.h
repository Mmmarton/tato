/*! @file max_driver.h
 *
 * @brief This module acts as an API for a user-defined MAX fuel gauge.
 */

/*!
 * @defgroup MAX
 * @{*/

#ifndef MAX_DRIVER_H
    #define MAX_DRIVER_H

    #include <stdint.h>

    #include "error_manager.h"

    #define MAX17048_I2C_ADDRESS 0x36 /*!< MAX17048 I2C address */

/** @brief This enumeration contains the registers that are used by the MAX17048 driver. */
typedef enum {
    MAX17048_DRIVER_VCELL     = 0x02,
    MAX17048_DRIVER_SOC       = 0x04,
    MAX17048_DRIVER_MODE      = 0x06,
    MAX17048_DRIVER_VERSION   = 0x08,
    MAX17048_DRIVER_HIBRT     = 0x0A,
    MAX17048_DRIVER_CONFIG    = 0x0C,
    MAX17048_DRIVER_VALRT     = 0x14,
    MAX17048_DRIVER_CRATE     = 0x16,
    MAX17048_DRIVER_VRESET_ID = 0x18,
    MAX17048_DRIVER_STATUS    = 0x1A,
    MAX17048_DRIVER_TABLE     = 0x40,
    MAX17048_DRIVER_CMD       = 0xFE
} max17048_reg_e;

/**
 * @brief This function initializes the specified max driver.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t max_driver_init(void);

/**
 * @brief This function returns the voltage level.
 *
 * @param[out]  voltage Voltage value retrieved from the fuel gauge.
 * 
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t max_driver_read_voltage(float *voltage);

/**
 * @brief This function returns the SOC percent (as float).
 *
* @param[out]  soc  State of charge value retrieved from the fuel gauge.
 * 
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t max_driver_read_soc(float *soc);

#endif
/** @}*/
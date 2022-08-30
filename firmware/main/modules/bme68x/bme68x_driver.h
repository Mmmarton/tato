/*! @file bme68x_driver.h
 *
 * @brief This module acts as a wrapper for the BME68x sensor API provided by Bosch. It provides a
 *        simplified API that handles all the necessary steps internally, thus providing the user with
 *        a more streamlined experience.
 */

/*!
 * @defgroup BME68x
 * @{*/

#ifndef BME68X_DRIVER_H
    #define BME68X_DRIVER_H

    #include "bme68x.h"
    #include "error_manager.h"
    
    #define BME_GAS_HEAT_TEMPERATURE_DEFAULT 300 /*!< default gas heating temperature 	 = 300 (degC) */
    #define BME_GAS_HEAT_DURATION_DEFAULT    100 /*!< default gas heating duration    	 = 1.0 (s) */

    #define BME_DEFAULT_CONFIGURATION() {                               \
        .temperature_oversampling = BME68X_OS_2X,                       \
        .pressure_oversampling    = BME68X_OS_1X,                       \
        .humidity_oversampling    = BME68X_OS_16X,                      \
        .enable_gas_measurement   = BME68X_ENABLE,                      \
        .gas_heater_temperature   = BME_GAS_HEAT_TEMPERATURE_DEFAULT,   \
        .gas_heater_duration      = BME_GAS_HEAT_DURATION_DEFAULT,      \
        .filter_coefficient       = BME68X_FILTER_OFF,                  \
    }
    
/**
 * @brief This structure contains the THPG readings.
 */
typedef struct {
    float temperature;    /*!< sensor temperature reading */
    float humidity;       /*!< sensor humidity reading */
    float pressure;       /*!< sensor pressure reading */
    float gas_resistance; /*!< sensor gas resistance reading */
} bme_reading_t;

/**
 * @brief This structure contains the user configuration that will be passed to the
 *        sensor initializing function in order to set the desired settings. Can be
 *        initialized with default values.
 */
typedef struct {
    uint8_t  device_id;                /*!< I2C device address */
    uint8_t  temperature_oversampling; /*!< temperature oversampling value (usually default) */
    uint8_t  pressure_oversampling;    /*!< pressure oversampling value (usually default) */
    uint8_t  humidity_oversampling;    /*!< humidity oversampling value (usually default) */
    uint8_t  filter_coefficient;       /*!< filter coefficient value */
    uint8_t  enable_gas_measurement;   /*!< decides whether or not the gas resistance measurement takes place */
    uint8_t  gas_heater_control;       /*!< gas heater control register */
    uint16_t gas_heater_temperature;   /*!< gas heater temperature value */
    uint16_t gas_heater_duration;      /*!< gas heater duration value */
    uint32_t measurement_delay;        /*!< delay between measurement */
} bme_configuration_t;

/** give more elegant names to the device and field_data structures provided by the BME68x library */
typedef struct bme68x_dev        bme_device_t;
typedef struct bme68x_data       bme_data_t;
typedef struct bme68x_conf       bme_config_t;
typedef struct bme68x_heatr_conf bme_heater_config_t;

/**
 * @brief This function is used to initialize the driver using a configuration structure that can be initialized by the
 *        user. If the user wishes to skip the structure setting step, please consider using the provided macro
 *        `BME_DEFAULT_CONFIGURATION` which initializes the structure with default values.
 *
 * @param[in]   configuration          Contains the initialization structure.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t bme_driver_init(bme_configuration_t *configuration);

/**
 * @brief This function is used to retrieve data from the BME68x sensor, making use of the BME api.
 *
 * @param[out]  reading                 Contains data retrieved from the sensor.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t bme_driver_read_sensor_data(bme_reading_t *reading);

#endif
/** @}*/
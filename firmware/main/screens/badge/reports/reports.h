#ifndef REPORTS_H
#define REPORTS_H

#include "badge_defines.h"
#include "configuration_manager_defines.h"
#include "error_manager.h"

#include <stdint.h>
#include <stdio.h>

#define REPORT_DEVICE_ID_OFFSET       1  /*!< offset position in protocol of the device UUID */
#define REPORT_BME_DATA_BUFFER_LENGTH 29 /*!< buffer length for BME68x data report protocol */

#define REPORT_BME_DATA_TEMPERATURE_OFFSET    17 /*!< offset position in BME68x data report of the temperature */
#define REPORT_BME_DATA_HUMIDITY_OFFSET       19 /*!< offset position in BME68x data report of the humidity */
#define REPORT_BME_DATA_PRESSURE_OFFSET       21 /*!< offset position in BME68x data report of the pressure */
#define REPORT_BME_DATA_GAS_RESISTANCE_OFFSET 25 /*!< offset position in BME68x data report of the gas resistance */

/**
 * @brief This function is used to generate a BME68x data reading report and send it over UWB.
 *
 * @param[in] device Structure containing device data
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t reports_transmit_bme68x_data(device_configuration_t *device);

#endif
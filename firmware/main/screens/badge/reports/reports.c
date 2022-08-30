#include "reports.h"
#include "bme68x_driver.h"
#include "dwm_manager.h"
#include "log.h"

#define MODULE_TAG "REPORTS"

#define GAS_LOWER_LIMIT 5000
#define GAS_UPPER_LIMIT 50000

float compute_aiq_score(bme_reading_t *reading);

status_t reports_transmit_bme68x_data(device_configuration_t *device) {
    LOG_INFO("transmit BME68x data report");
    bme_configuration_t config = BME_DEFAULT_CONFIGURATION();

    status_t reading_status = bme_driver_init(&config);
    if ((reading_status != STATUS_OK) && (reading_status != STATUS_ALREADY_INIT)) {
        return reading_status;
    }

    bme_reading_t reading;
    CHECK_FUNCTION(bme_driver_read_sensor_data(&reading));

    int16_t temperature    = (int16_t)reading.temperature;
    int16_t humidity       = (int16_t)reading.humidity;
    int32_t pressure       = (int32_t)reading.pressure / 100;
    int32_t gas_resistance = (int32_t)compute_aiq_score(&reading);

    uint8_t report_buffer[DWM_MANAGER_MAXIMUM_FRAME_LENGTH];
    uint8_t report_size = REPORT_BME_DATA_BUFFER_LENGTH;

    report_buffer[0] = SYSTEM_CMD_UWB_SEND_REPORT_BME_DATA;
    memcpy(report_buffer + REPORT_DEVICE_ID_OFFSET, device->config.device_id, sizeof(device->config.device_id));
    memcpy(report_buffer + REPORT_BME_DATA_TEMPERATURE_OFFSET, (int8_t *)&temperature, sizeof(temperature));
    memcpy(report_buffer + REPORT_BME_DATA_HUMIDITY_OFFSET, (int8_t *)&humidity, sizeof(humidity));
    memcpy(report_buffer + REPORT_BME_DATA_PRESSURE_OFFSET, (int8_t *)&pressure, sizeof(pressure));
    memcpy(report_buffer + REPORT_BME_DATA_GAS_RESISTANCE_OFFSET, (int8_t *)&gas_resistance, sizeof(gas_resistance));

    return dwm_manager_transmit_data(report_buffer, report_size);
}

float compute_aiq_score(bme_reading_t *reading) {
    float humidity_score = 100;
    float gas_score      = 100;

    float gas_reference_value      = 250000;
    float humidity_reference_value = 40;

    float current_humidity = reading->humidity;
    if (current_humidity >= 38 && current_humidity <= 42) {
        humidity_score = 0.25 * 100;
    }
    else {
        if (current_humidity < 38) {
            humidity_score = 0.25 / humidity_reference_value * current_humidity * 100;
        }
        else {
            humidity_score = ((-0.25 / (100 - humidity_reference_value) * current_humidity) + 0.416666) * 100;
        }
    }

    if (gas_reference_value > GAS_UPPER_LIMIT) {
        gas_reference_value = GAS_UPPER_LIMIT;
    }
    if (gas_reference_value < GAS_LOWER_LIMIT) {
        gas_reference_value = GAS_LOWER_LIMIT;
    }
    gas_score = (0.75 / (GAS_UPPER_LIMIT - GAS_LOWER_LIMIT) * gas_reference_value
                 - (GAS_LOWER_LIMIT * (0.75 / (GAS_UPPER_LIMIT - GAS_LOWER_LIMIT))))
                * 100;

    float air_quality_score = humidity_score + gas_score;
    return (100 - air_quality_score) * 5;
}
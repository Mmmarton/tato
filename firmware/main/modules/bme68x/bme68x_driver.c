#include "bme68x_driver.h"
#include "i2c_driver.h"
#include "log.h"
#include "utilities.h"

#define TICKS_PER_US (int)80
#define MODULE_TAG   "BME68X_DRIVER"

static uint8_t      device_address = BME68X_I2C_ADDR_HIGH;
static uint32_t     measurement_delay;
static bme_device_t sensor;
static bool         is_init = false;

static bme_configuration_t inner_configuration;

BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

int8_t bme68x_interface_init(struct bme68x_dev *bme);
void   bme68x_delay_us(uint32_t period, void *intf_ptr);

status_t bme_driver_init(bme_configuration_t *configuration) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    status_t driver_status = i2c_driver_init_master();
    if (driver_status != STATUS_OK && driver_status != STATUS_ALREADY_INIT) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    inner_configuration.filter_coefficient       = configuration->filter_coefficient;
    inner_configuration.humidity_oversampling    = configuration->humidity_oversampling;
    inner_configuration.pressure_oversampling    = configuration->pressure_oversampling;
    inner_configuration.temperature_oversampling = configuration->temperature_oversampling;

    inner_configuration.enable_gas_measurement = configuration->enable_gas_measurement;
    inner_configuration.gas_heater_duration    = configuration->gas_heater_duration;
    inner_configuration.gas_heater_temperature = configuration->gas_heater_temperature;

    is_init = true;
    
    LOG_INFO("driver init ok");
    return STATUS_OK;
}

status_t bme_driver_read_sensor_data(bme_reading_t *reading) {
    CHECK_INIT(is_init);

    bme_config_t        sensor_configuration;
    bme_heater_config_t heater_control;

    bme68x_interface_init(&sensor);
    bme68x_init(&sensor);

    sensor_configuration.filter  = inner_configuration.filter_coefficient;
    sensor_configuration.odr     = BME68X_ODR_NONE;
    sensor_configuration.os_hum  = inner_configuration.humidity_oversampling;
    sensor_configuration.os_pres = inner_configuration.pressure_oversampling;
    sensor_configuration.os_temp = inner_configuration.temperature_oversampling;
    bme68x_set_conf(&sensor_configuration, &sensor);

    heater_control.enable     = inner_configuration.enable_gas_measurement;
    heater_control.heatr_dur  = inner_configuration.gas_heater_duration;
    heater_control.heatr_temp = inner_configuration.gas_heater_temperature;
    bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heater_control, &sensor);

    measurement_delay
        = bme68x_get_meas_dur(BME68X_FORCED_MODE, &sensor_configuration, &sensor) + (heater_control.heatr_dur * 1000);

    bme_data_t reading_data;
    uint8_t    reading_result;

    bme68x_set_op_mode(BME68X_FORCED_MODE, &sensor);
    sensor.delay_us(measurement_delay, sensor.intf_ptr);

    bme68x_get_data(BME68X_FORCED_MODE, &reading_data, &reading_result, &sensor);
    if (reading_result == 0) {
        LOG_ERROR("no data retrieved");
        return STATUS_BME68X_INVALID_DATA_ERROR;
    }

    reading->temperature    = reading_data.temperature;
    reading->humidity       = reading_data.humidity;
    reading->pressure       = reading_data.pressure;
    reading->gas_resistance = reading_data.gas_resistance;

    LOG_INFO("data retrieved");
    return STATUS_OK;
}

int8_t bme68x_interface_init(struct bme68x_dev *bme_device) {
    bme_device->chip_id = device_address;
    bme_device->read    = &bme68x_i2c_read;
    bme_device->write   = &bme68x_i2c_write;
    bme_device->intf    = BME68X_I2C_INTF;

    bme_device->delay_us = bme68x_delay_us;
    bme_device->intf_ptr = &device_address;

    bme_device->amb_temp = 25;
    return 0;
}

BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    if (i2c_driver_read(*(uint8_t *)intf_ptr, reg_addr, reg_data, (uint16_t)len) != STATUS_OK) {
        return -1;
    }

    return BME68X_OK;
}

BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    if (i2c_driver_write(*(uint8_t *)intf_ptr, reg_addr, (uint8_t *)reg_data, (uint16_t)len) != STATUS_OK) {
        return -1;
    }

    return BME68X_OK;
}

void bme68x_delay_us(uint32_t period, void *intf_ptr) {
    for (size_t i = 0; i < TICKS_PER_US * period; i++) {
        continue;
    }
}

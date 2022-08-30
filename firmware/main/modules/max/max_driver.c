#include "max_driver.h"
#include "i2c_driver.h"
#include "log.h"

#define MODULE_TAG "MAX17048_DRIVER"

static bool is_init = false;

status_t max_driver_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    status_t driver_status = i2c_driver_init_master();
    if (driver_status != STATUS_OK && driver_status != STATUS_ALREADY_INIT) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    is_init = true;

    LOG_INFO("driver init ok");
    return STATUS_OK;
}

status_t max_driver_read_voltage(float *voltage) {
    CHECK_INIT(is_init);

    int8_t  error_check;
    uint8_t data[2];

    error_check = i2c_driver_read(MAX17048_I2C_ADDRESS, MAX17048_DRIVER_VCELL, data, 2);
    if (error_check != STATUS_OK) {
        LOG_WARNING("voltage read failed");
        return STATUS_MAX17048_VOLTAGE_READ_ERROR;
    }

    uint16_t vcell_value = (uint16_t)(data[0] << 8 | data[1]);
    (*voltage)           = (((float)vcell_value) * 78.125f / 1000000.f);

    return STATUS_OK;
}

status_t max_driver_read_soc(float *soc) {
    CHECK_INIT(is_init);

    int8_t  error_check;
    uint8_t data[2];

    error_check = i2c_driver_read(MAX17048_I2C_ADDRESS, MAX17048_DRIVER_SOC, data, 2);
    if (error_check != STATUS_OK) {
        LOG_WARNING("soc (float) read failed");
        return STATUS_MAX17048_SOC_READ_ERROR;
    }

    uint16_t soc_value = (uint16_t)(data[0] << 8 | data[1]);
    (*soc)             = (((float)soc_value) / 256.f);

    return STATUS_OK;
}
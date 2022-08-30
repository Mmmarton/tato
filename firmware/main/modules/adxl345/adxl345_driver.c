#include "adxl345_driver.h"
#include "i2c_driver.h"
#include "log.h"

#define MODULE_TAG "ADXL345_DRIVER"
#define BYTE_LEN   1

#define POWER_CTRL_REG_STANDBY_MODE_BITMASK 0b00000000
#define POWER_CTRL_REG_AUTO_MODE_BITMASK    0b00111000
#define POWER_CTRL_REG_SLEEP_MODE_BITMASK   0b00000101

#define INTERRUPT_CTRL_REG_ENABLE_INT_BITMASK 0b00011000
#define BIT_RATE_CTRL_REG_LOW_POWER_BITMASK   0b00010111

#define BYTES_TO_FLOAT(target, first_byte, second_byte) ((target[first_byte] << 8) | target[second_byte])

static uint8_t range;
static bool    is_init = false;

float get_x_axis_data(uint8_t *output_data);
float get_y_axis_data(uint8_t *output_data);
float get_z_axis_data(uint8_t *output_data);
float map_axis_data(float data);

void set_range_settings(uint8_t val);
void set_interrupt_level(bool level);

void set_activity_threshold(uint8_t value);
void set_inactivity_threshold(uint8_t value);
void set_inactivity_timer(uint8_t value);

void set_activity_bits(bool x, bool y, bool z);
void set_inactivity_bits(bool x, bool y, bool z);

status_t adxl345_driver_init(adxl345_driver_configuration_t *configuration) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    status_t driver_status = i2c_driver_init_master();
    if (driver_status != STATUS_OK && driver_status != STATUS_ALREADY_INIT) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    uint8_t data[1];

    data[0] = POWER_CTRL_REG_STANDBY_MODE_BITMASK;
    set_interrupt_level(configuration->interrupt_level);

    set_activity_bits(configuration->x_axis_enabled, configuration->y_axis_enabled, configuration->z_axis_enabled);
    set_inactivity_bits(configuration->x_axis_enabled, configuration->y_axis_enabled, configuration->z_axis_enabled);
    set_activity_threshold(configuration->activity_threshold);
    set_inactivity_threshold(configuration->activity_threshold);

    set_inactivity_timer(configuration->inactivity_time);

    data[0] = BIT_RATE_CTRL_REG_LOW_POWER_BITMASK;
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_BW_RATE_REG, data, BYTE_LEN);
    data[0] = POWER_CTRL_REG_AUTO_MODE_BITMASK;
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_POWER_CTRL_REG, data, BYTE_LEN);

    LOG_INFO("driver init ok");
    is_init = true;
    return STATUS_OK;
}

status_t adxl345_driver_enter_sleep(void) {
    CHECK_INIT(is_init);

    uint8_t data[1];
    data[0] = POWER_CTRL_REG_SLEEP_MODE_BITMASK;

    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_POWER_CTRL_REG, data, BYTE_LEN);

    return STATUS_OK;
}

status_t adxl345_driver_enter_standby(void) {
    CHECK_INIT(is_init);

    uint8_t data[1];
    data[0] = POWER_CTRL_REG_STANDBY_MODE_BITMASK;

    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_POWER_CTRL_REG, data, BYTE_LEN);
    return STATUS_OK;
}

status_t adxl345_driver_update_activity_threshold(uint8_t threshold) {
    CHECK_INIT(is_init);

    set_activity_threshold(threshold);
    set_inactivity_threshold(threshold);

    return STATUS_OK;
}

status_t adxl345_driver_get_axis_data(adxl345_reading_t *reading) {
    CHECK_INIT(is_init);

    uint8_t output_data[6];
    i2c_driver_read(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_READING_REG, output_data, 6);

    if (reading != NULL) {
        reading->x_axis = get_x_axis_data(output_data);
        reading->y_axis = get_y_axis_data(output_data);
        reading->z_axis = get_z_axis_data(output_data);
    }
    adxl345_driver_reset_interrupt();

    return STATUS_OK;
}

status_t adxl345_driver_reset_interrupt() {
    CHECK_INIT(is_init);

    uint8_t intr[1];
    i2c_driver_read(ADXL345_DRIVER_I2C_REG, 0x30, intr, BYTE_LEN);

    return STATUS_OK;
}

float get_x_axis_data(uint8_t *output_data) {
    return map_axis_data(BYTES_TO_FLOAT(output_data, 1, 0));
}

float get_y_axis_data(uint8_t *output_data) {
    return map_axis_data(BYTES_TO_FLOAT(output_data, 3, 2));
}

float get_z_axis_data(uint8_t *output_data) {
    return map_axis_data(BYTES_TO_FLOAT(output_data, 5, 4));
}

void set_range_settings(uint8_t val) {
    uint8_t byte = 0;

    switch (val) {
        case 2:
            byte |= 0b00000000;
            break;

        case 4:
            byte |= 0b00000001;
            break;

        case 8:
            byte |= 0b00000010;
            break;

        case 16:
            byte |= 0b00000011;
            break;

        default:
            byte |= 0b00000000;
    }

    uint8_t data[1];
    i2c_driver_read(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_DATA_FORMAT_REG, data, BYTE_LEN);

    data[0] |= (byte & 0b11101100);
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_DATA_FORMAT_REG, data, BYTE_LEN);
}

void set_interrupt_level(bool level) {
    uint8_t data[1];
    data[0] = INTERRUPT_CTRL_REG_ENABLE_INT_BITMASK;
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_INTERRUPT_ENABLE_REG, data, BYTE_LEN);

    data[0] = (level << 5);
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_DATA_FORMAT_REG, data, BYTE_LEN);
}

void set_activity_threshold(uint8_t value) {
    uint8_t data[] = {value};
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_ACTIVITY_THRESHOLD_REG, data, BYTE_LEN);
}

void set_inactivity_threshold(uint8_t value) {
    uint8_t data[] = {value};
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_INACTIVITY_THRESHOLD_REG, data, BYTE_LEN);
}

void set_inactivity_timer(uint8_t value) {
    uint8_t data[] = {value};
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_INACTIVITY_TIME_REG, data, BYTE_LEN);
}

void set_activity_bits(bool x, bool y, bool z) {
    uint8_t data[1];
    i2c_driver_read(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_ACT_INACT_CTRL_REG, data, BYTE_LEN);

    data[0] |= (1 << 7) | (x << 6) | (y << 5) | (z << 4);
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_ACT_INACT_CTRL_REG, data, BYTE_LEN);
}

void set_inactivity_bits(bool x, bool y, bool z) {
    uint8_t data[1];
    i2c_driver_read(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_ACT_INACT_CTRL_REG, data, BYTE_LEN);

    data[0] |= (1 << 3) | (x << 2) | (y << 1) | (z << 0);
    i2c_driver_write(ADXL345_DRIVER_I2C_REG, ADXL345_DRIVER_ACT_INACT_CTRL_REG, data, BYTE_LEN);
}

float map_axis_data(float data) {
    if (data / 256 > range) {
        return (data / 256 - 256);
    }

    return data / 256;
}
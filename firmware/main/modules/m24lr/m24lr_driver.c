#include "m24lr_driver.h"
#include "i2c_driver.h"
#include "log.h"
#include "utilities.h"

#include <string.h>

#define MODULE_TAG "M24LR_DRIVER"

/** @todo: edit these constants based on what we actually need */
#define NDEF_TYPE_EMPTY_LENGTH     0
#define NDEF_TYPE_FORUM_WKT_LENGTH 8
#define NDEF_TYPE_MEDIA_LENGTH     0
#define NDEF_TYPE_AURI_LENGTH      0
#define NDEF_TYPE_FORUM_EXT_LENGTH 0
#define NDEF_TYPE_UNKNOWN_LENGTH   0
#define NDEF_TYPE_UNCHANGED_LENGTH 0
#define NDEF_TYPE_RESERVED_LENGTH  0

#define NDEF_MAXIMUM_STRING_LENGTH 255

/** this array defines the lengths of the NDEF formats. only FORUM WELL-KNOWN TYPE is used currently, so only this
 *  specific value will be stored. further updates will change the values inside the array (if needed) */
static uint8_t ndef_format_length[]
    = {NDEF_TYPE_EMPTY_LENGTH,     NDEF_TYPE_FORUM_WKT_LENGTH, NDEF_TYPE_MEDIA_LENGTH,     NDEF_TYPE_AURI_LENGTH,
       NDEF_TYPE_FORUM_EXT_LENGTH, NDEF_TYPE_UNKNOWN_LENGTH,   NDEF_TYPE_UNCHANGED_LENGTH, NDEF_TYPE_RESERVED_LENGTH};
static bool is_init = false;

int8_t read_byte(uint8_t dev_address, uint16_t address, uint8_t *data);
int8_t write_byte(uint8_t dev_address, uint16_t address, uint8_t data);

status_t m24lr_driver_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    status_t driver_status = i2c_driver_init_master();
    if (driver_status != STATUS_OK && driver_status != STATUS_ALREADY_INIT) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    write_byte(M24LR_DRIVER_I2C_SYS_ADDRESS, 2320, 244);
    is_init = true;

    LOG_INFO("driver init ok");
    return STATUS_OK;
}

status_t m24lr_driver_write_ndef(ndef_type_e type, uint8_t *ndef_message) {
    CHECK_INIT(is_init);

    uint8_t ndef_format[] = {0xE1, 0x40, 0x40, 0x05, 0x03, 0x00, 0xD1, 0x01, 0x00, 0x54, 0x02, 0x65, 0x6E};

    uint8_t ndef_msg_length = strlen((char *)ndef_message);
    ndef_format[5]          = ndef_msg_length + 7;
    ndef_format[8]          = ndef_msg_length + 3;

    for (size_t i = 0; i < ndef_format_length[type]; i++) {
        write_byte(M24LR_DRIVER_I2C_USER_ADDRESS, i, ndef_format[i]);
    }

    uint8_t ndef_fmt_length = strlen((char *)ndef_format);
    for (size_t i = 0; i < ndef_msg_length; i++) {
        write_byte(M24LR_DRIVER_I2C_USER_ADDRESS, i + ndef_fmt_length, ndef_message[i]);
    }
    write_byte(M24LR_DRIVER_I2C_USER_ADDRESS, ndef_msg_length + ndef_fmt_length, 0xFE);

    return STATUS_OK;
}

status_t m24lr_driver_read_ndef(ndef_type_e type, uint8_t *ndef_message, uint8_t *data_length) {
    CHECK_INIT(is_init);

    uint8_t dummy_data[NDEF_MAXIMUM_STRING_LENGTH];

    uint8_t array_size = NDEF_MAXIMUM_STRING_LENGTH;
    uint8_t byte_count = 0;
    for (size_t i = 0; i < array_size; i++) {
        if ((i >= ndef_format_length[type]) && (i < ndef_format_length[type] + (*data_length))) {
            if (read_byte(M24LR_DRIVER_I2C_USER_ADDRESS, i, &ndef_message[byte_count]) == -1) {
                (*data_length) = 0;
                return STATUS_M24LR_INVALID_DATA_ERROR;
            }
            byte_count++;
        }
        else {
            if (read_byte(M24LR_DRIVER_I2C_USER_ADDRESS, i, &dummy_data[i]) == -1) {
                (*data_length) = 0;
                return STATUS_M24LR_INVALID_DATA_ERROR;
            }
            if (i == 5) {
                (*data_length) = dummy_data[i] - 2;
                array_size     = dummy_data[i] + ndef_format_length[type];
            }
        }
    }

    return STATUS_OK;
}

status_t m24lr_driver_enable_energy_harvesting(void) {
    CHECK_INIT(is_init);

    uint8_t byte;
    read_byte(M24LR_DRIVER_I2C_SYS_ADDRESS, M24LR_DRIVER_CONTROL_ADDRESS, &byte);
    write_byte(M24LR_DRIVER_I2C_SYS_ADDRESS, M24LR_DRIVER_CONTROL_ADDRESS, byte | 0x01);

    return STATUS_OK;
}

status_t m24lr_driver_disable_energy_harvesting(void) {
    CHECK_INIT(is_init);

    uint8_t byte;
    read_byte(M24LR_DRIVER_I2C_SYS_ADDRESS, M24LR_DRIVER_CONTROL_ADDRESS, &byte);
    write_byte(M24LR_DRIVER_I2C_SYS_ADDRESS, M24LR_DRIVER_CONTROL_ADDRESS, byte | 0xFE);

    return STATUS_OK;
}

status_t m24lr_driver_clear_ndef_buffer(void) {
    CHECK_INIT(is_init);

    for (size_t i = 0; i < NDEF_MAXIMUM_STRING_LENGTH; i++) {
        write_byte(M24LR_DRIVER_I2C_USER_ADDRESS, i, 0xFF);
    }

    return STATUS_OK;
}

int8_t read_byte(uint8_t dev_address, uint16_t address, uint8_t *data) {
    i2c_driver_create_cmd_link();
    i2c_driver_write_start(dev_address, I2C_MASTER_WRITE);

    uint8_t address_data[2];
    address_data[0] = (address >> 8);
    address_data[1] = (address & 0xFF);
    i2c_driver_write_data(address_data, 2, I2C_MASTER_ACK);

    i2c_driver_write_start(M24LR_DRIVER_I2C_USER_ADDRESS, I2C_MASTER_READ);
    i2c_driver_read_data(data, 1);

    i2c_driver_write_stop();
    if (i2c_driver_commit_data() != STATUS_OK) {
        return -1;
    }

    return 0;
}

int8_t write_byte(uint8_t dev_address, uint16_t address, uint8_t data) {
    i2c_driver_create_cmd_link();
    i2c_driver_write_start(dev_address, I2C_MASTER_WRITE);

    uint8_t address_data[2];
    address_data[0] = (address >> 8);
    address_data[1] = (address & 0xFF);
    i2c_driver_write_data(address_data, 2, I2C_MASTER_ACK);

    uint8_t byte[1];
    byte[0] = data;
    i2c_driver_write_data(byte, 1, I2C_MASTER_ACK);

    i2c_driver_write_stop();
    if (i2c_driver_commit_data() != STATUS_OK) {
        return -1;
    }

    return 0;
}
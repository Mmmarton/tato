#include "i2c_driver.h"
#include "esp_err.h"
#include "log.h"

#include <stdbool.h>

#define MODULE_TAG "I2C_DRIVER"

static bool             is_init         = false;
static int              i2c_master_port = 0;
static i2c_cmd_handle_t cmd_handle;

status_t i2c_driver_init_master(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    i2c_config_t i2c_configuration = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = I2C_MASTER_SDA,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_io_num       = I2C_MASTER_SCL,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ * 2,
    };

    esp_err_t error_check = i2c_param_config(i2c_master_port, &i2c_configuration);
    if (error_check != ESP_OK) {
        return STATUS_INIT_FAILED;
    }

    error_check = i2c_driver_install(i2c_master_port, i2c_configuration.mode, I2C_MASTER_RX_BUF, I2C_MASTER_TX_BUF, 0);
    if (error_check != ESP_OK) {
        return STATUS_INIT_FAILED;
    }

    is_init = true;
    LOG_INFO("driver init ok");
    return STATUS_OK;
}

status_t i2c_driver_read(uint8_t dev_id, uint8_t address, uint8_t *data, uint16_t data_length) {
    CHECK_INIT(is_init);

    if (data_length == 0) {
        LOG_DEBUG("i2c read invalid data length");
        return STATUS_I2C_INVALID_DATA_LENGTH_ERROR;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, I2C_ACK_VAL);

    i2c_master_write_byte(cmd, address, I2C_ACK_VAL);
    if (data) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_READ, I2C_ACK_VAL);
        if (data_length > 1) {
            i2c_master_read(cmd, data, data_length - 1, I2C_MASTER_ACK);
        }
        i2c_master_read_byte(cmd, data + data_length - 1, I2C_MASTER_NACK);
    }

    i2c_master_stop(cmd);
    esp_err_t i2c_error = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (i2c_error == ESP_OK) {
        return ESP_OK;
    }

    LOG_ERROR("I2C reading error");
    return STATUS_OK;
}

status_t i2c_driver_write(uint8_t dev_id, uint8_t address, uint8_t *data, uint16_t data_length) {
    CHECK_INIT(is_init);

    if (data_length == 0) {
        LOG_DEBUG("i2c write invalid data length");
        return STATUS_I2C_INVALID_DATA_LENGTH_ERROR;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, I2C_ACK_VAL);

    i2c_master_write_byte(cmd, address, I2C_ACK_VAL);
    if (data) {
        i2c_master_write(cmd, data, data_length, I2C_ACK_VAL);
    }

    i2c_master_stop(cmd);
    esp_err_t i2c_error = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (i2c_error == ESP_OK) {
        return STATUS_OK;
    }

    LOG_ERROR("I2C writing error");
    return STATUS_I2C_FULL_WRITE_ERROR;
}

status_t i2c_driver_create_cmd_link(void) {
    CHECK_INIT(is_init);

    cmd_handle = i2c_cmd_link_create();
    if (cmd_handle == NULL) {
        return STATUS_I2C_CREATE_CMD_ERROR;
    }

    return STATUS_OK;
}

status_t i2c_driver_write_start(uint8_t dev_id, uint8_t flag) {
    CHECK_INIT(is_init);

    esp_err_t error_code = ESP_OK;
    error_code           = i2c_master_start(cmd_handle);
    if (error_code != ESP_OK) {
        return STATUS_I2C_WRITE_START_COND_ERROR;
    }

    error_code = i2c_master_write_byte(cmd_handle, (dev_id << 1) | flag, I2C_ACK_VAL);
    if (error_code != ESP_OK) {
        return STATUS_I2C_WRITE_START_COND_ERROR;
    }

    return STATUS_OK;
}

status_t i2c_driver_write_stop(void) {
    CHECK_INIT(is_init);

    esp_err_t error_code = i2c_master_stop(cmd_handle);
    if (error_code != ESP_OK) {
        return STATUS_I2C_WRITE_STOP_COND_ERROR;
    }

    return STATUS_OK;
}

status_t i2c_driver_write_data(uint8_t *data, uint8_t data_length, uint8_t flag) {
    CHECK_INIT(is_init);

    esp_err_t error_code = i2c_master_write(cmd_handle, data, data_length, flag);
    if (error_code != ESP_OK) {
        return STATUS_I2C_WRITE_DATA_ERROR;
    }

    return STATUS_OK;
}

status_t i2c_driver_read_data(uint8_t *data, uint8_t data_length) {
    CHECK_INIT(is_init);

    esp_err_t error_code = ESP_OK;
    if (data_length > 1) {
        error_code = i2c_master_read(cmd_handle, data, data_length - 1, I2C_MASTER_ACK);
        if (error_code != ESP_OK) {
            return STATUS_I2C_READ_DATA_ERROR;
        }
    }

    error_code = i2c_master_read_byte(cmd_handle, data + data_length - 1, I2C_MASTER_NACK);
    if (error_code != ESP_OK) {
        return STATUS_I2C_READ_DATA_ERROR;
    }

    return STATUS_OK;
}

status_t i2c_driver_commit_data(void) {
    CHECK_INIT(is_init);

    esp_err_t i2c_error = i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    if (i2c_error != ESP_OK) {
        LOG_WARNING("I2C commit error %d", i2c_error);
        return STATUS_I2C_COMMIT_ERROR;
    }

    return STATUS_OK;
}

status_t i2c_driver_scan_devices(void) {
    CHECK_INIT(is_init);

    esp_err_t error_check;

    for (uint8_t i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
        i2c_master_stop(cmd);

        error_check = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        if (i % 16 == 0) {
            printf("\n");
        }
        if (error_check == 0) {
            printf(" %3x", i);
        }
        else {
            printf(" %-3s", "--");
        }
        i2c_cmd_link_delete(cmd);
    }

    printf("\n");
    return STATUS_OK;
}
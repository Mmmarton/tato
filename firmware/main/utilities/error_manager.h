#ifndef ERROR_H
#define ERROR_H

#include <string.h>

/** @brief Typedef mask for function status checking. */
typedef int status_t;

/** general use error codes */
#define STATUS_OK           0x00
#define STATUS_NOT_INIT     0x01
#define STATUS_ALREADY_INIT 0x02
#define STATUS_INIT_FAILED  0x03

/** SPI related error codes */
#define STATUS_SPI_START_ERROR 0x11

/** DWM related error codes */
#define STATUS_DWM_IDLERC_ERROR               0x21
#define STATUS_DWM_API_INIT_ERROR             0x22
#define STATUS_DWM_API_CONFIG_ERROR           0x23
#define STATUS_DWM_TIMEOUT_ERROR              0x24
#define STATUS_DWM_DISTANCE_MEASUREMENT_ERROR 0x25

/** I2C related error codes */
#define STATUS_I2C_CREATE_CMD_ERROR          0x31
#define STATUS_I2C_WRITE_START_COND_ERROR    0x32
#define STATUS_I2C_WRITE_STOP_COND_ERROR     0x33
#define STATUS_I2C_WRITE_DATA_ERROR          0x34
#define STATUS_I2C_READ_DATA_ERROR           0x35
#define STATUS_I2C_COMMIT_ERROR              0x36
#define STATUS_I2C_INVALID_DATA_LENGTH_ERROR 0x37
#define STATUS_I2C_FULL_READ_ERROR           0x38
#define STATUS_I2C_FULL_WRITE_ERROR          0x39

/** PWM related error codes */
#define STATUS_PWM_INVALID_CHANNEL_ERROR 0x41
#define STATUS_PWM_OUT_OF_BOUNDS_ERROR   0x42
#define STATUS_PWM_SET_DUTY_CYCLE_ERROR  0x43

/** BME68x related error codes */
#define STATUS_BME68X_INVALID_DATA_ERROR 0x51

/** M24LR related error codes */
#define STATUS_M24LR_INVALID_DATA_ERROR 0x61

/** MAX17048 related error codes */
#define STATUS_MAX17048_VOLTAGE_READ_ERROR 0x71
#define STATUS_MAX17048_SOC_READ_ERROR     0x72

/** storage module related error codes */
#define STATUS_STORAGE_FILESYSTEM_MOUNT_ERROR 0x81
#define STATUS_STORAGE_FILESYSTEM_INIT_ERROR  0x82
#define STATUS_STORAGE_NULL_TXT_FILE          0x83
#define STATUS_STORAGE_NULL_BMP_FILE          0x84
#define STATUS_STORAGE_INVALID_WRITE_ERROR    0x85
#define STATUS_STORAGE_INVALID_READ_ERROR     0x86
#define STATUS_STORAGE_NULL_BINARY_FILE       0x87
#define STATUS_STORAGE_INVALID_BINARY_ACTION  0x88
#define STATUS_STORAGE_DELETE_FAIL            0x89
#define STATUS_STORAGE_LIST_FAIL              0x90

/** configuration manager related error codes */
#define STATUS_CONFIG_MANAGER_NFC_READ_ERROR         0x91
#define STATUS_CONFIG_MANAGER_INVALID_TARGET_ERROR   0x92
#define STATUS_CONFIG_MANAGER_READ_FILE_ERROR        0x93
#define STATUS_CONFIG_MANAGER_INVALID_LED_DATA_ERROR 0x94
#define STATUS_CONFIG_MANAGER_INVALID_ACTION_ERROR   0x95
#define STATUS_CONFIG_MANAGER_NULL_POINTER_ERROR     0x96
#define STATUS_CONFIG_MANAGER_INVALID_ENTRY_COUNT    0x97

/** GDEW0371W7 driver related error codes */
#define STATUS_GDEW_DRIVER_OUT_OF_BOUNDS_ERROR 0x101

/** WIFI driver related error codes */
#define STATUS_WIFI_COULD_NOT_CONNECT 0x181

/** http client driver related error codes */
#define STATUS_CLIENT_COULD_NOT_START        0x191
#define STATUS_CLIENT_COULD_NOT_GET          0x192
#define STATUS_CLIENT_COULD_NOT_POST         0x193
#define STATUS_CLIENT_COULD_NOT_SETUP_STREAM 0x194

/** OTA driver related error codes */
#define STATUS_OTA_NVS_INIT_FAIL           0x201
#define STATUS_OTA_NULL_PARTITION_FAIL     0x202
#define STATUS_OTA_UPDATE_START_FAIL       0x203
#define STATUS_OTA_INVALID_FILE_FAIL       0x204
#define STATUS_OTA_IMAGE_VALIDATION_FAIL   0x205
#define STATUS_OTA_UPDATE_END_FAIL         0x206
#define STATUS_OTA_BOOT_PARTITION_SET_FAIL 0x207

/** @brief This macro is used to check if a module has been initialized. */
#define CHECK_INIT(status)                  \
    {                                       \
        if (status == false) {              \
            LOG_WARNING("module not init"); \
            return STATUS_NOT_INIT;         \
        }                                   \
    }

/** @brief This macro is used to check if a function execution encountered any issue. */
#define CHECK_FUNCTION(target)                                          \
    {                                                                   \
        status_t ret = (target);                                        \
        if (ret != STATUS_OK) {                                         \
            LOG_WARNING("function execution failed, code 0x%.4x", ret); \
            return ret;                                                 \
        }                                                               \
    }

#endif
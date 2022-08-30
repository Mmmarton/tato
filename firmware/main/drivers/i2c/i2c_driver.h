/*! @file i2c_driver.h
 *
 * @brief This driver is used for I2C communication between modules.
 */

/*!
 * @defgroup I2C
 * @{*/

#ifndef I2C_DRIVER_H
    #define I2C_DRIVER_H

    #include "driver/i2c.h"
    #include "error_manager.h"

    #define I2C_ACK_VAL  0x00 /*!< slave  ACK bit */
    #define I2C_NACK_VAL 0x01 /*!< slave NACK bit */

    #define I2C_MASTER_SDA  21     /*!< data  pin for the I2C bus */
    #define I2C_MASTER_SCL  22     /*!< clock pin for the I2C bus */
    #define I2C_MASTER_FREQ 400000 /*!< frequency for the I2C bus */

    #define I2C_MASTER_RX_BUF 0 /*!< RX buffer length */
    #define I2C_MASTER_TX_BUF 0 /*!< TX buffer length */

/**
 * @brief This function is used to initialize the I2C bus.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_init_master(void);

/**
 * @brief This function is used to read an array of bytes to a user-selected device using the I2C communication
 *        protocol.
 *
 * @param[in]   dev_id      ID of the target device.
 * @param[in]   address     Target memory address.
 * @param[out]  data        Data buffer to store the retrieved data.
 * @param[in]   data_length Size of the data buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_read(uint8_t dev_id, uint8_t address, uint8_t *data, uint16_t data_length);

/**
 * @brief This function is used to write an array of bytes to a user-selected device using the I2C communication
 *        protocol.
 *
 * @param[in]   dev_id      ID of the target device.
 * @param[in]   address     Target memory address.
 * @param[in]   data        Data buffer to be sent through I2C.
 * @param[in]   data_length Size of the data buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_write(uint8_t dev_id, uint8_t address, uint8_t *data, uint16_t data_length);

/**
 * @brief This function is used to create a command link for the I2C bus used by the ESP32. This command link
 *        MUST be created by the used before calling any other I2C functions and MUST be destroyed (this is done)
 *        by commiting the command link (see `i2c_driver_commit_data`).
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_create_cmd_link(void);

/**
 * @brief This function is used to send an I2C start signal followed by the passed flag (read/write).
 *
 * @param[in]  dev_id  ID of the target device.
 * @param[in]  flag    read/write flag for the master device.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_write_start(uint8_t dev_id, uint8_t flag);

/**
 * @brief This function is used to send an I2C stop signal.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_write_stop(void);

/**
 * @brief This function is used to write data on the command link. The flag represent either an ACK bit or a
 *        NACK bit.
 *
 * @param[in]   data        data buffer to be sent to the I2C bus.
 * @param[in]   data_length length of the data buffer.
 * @param[in]   flag        ACK/NACK bit
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_write_data(uint8_t *data, uint8_t data_length, uint8_t flag);

/**
 * @brief This function is used to read data on the command link.
 *
 * @param[out]  data        data buffer to be sent to the I2C bus.
 * @param[in]   data_length length of the data buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_read_data(uint8_t *data, uint8_t data_length);

/**
 * @brief This function is used to commit the command link and delete it. Without this function, the data the user
 *        is trying to send on the I2C bus will NOT be sent.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_commit_data(void);

/**
 * @brief This function is used to scan and print the adresses of all the devices connected to the I2C bus.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t i2c_driver_scan_devices(void);

#endif
/** @}*/
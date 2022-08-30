/*! @file spi_driver.h
 *
 * @brief This driver is used for SPI communication between modules.
 */

/*!
 * @defgroup SPI
 * @{*/

#ifndef SPI_DRIVER_H
    #define SPI_DRIVER_H

    #include "driver/spi_master.h"
    #include "error_manager.h"

    #define SPI_PIN_MISO (int)12 /*!< MISO pin */
    #define SPI_PIN_MOSI (int)13 /*!< MOSI pin */
    #define SPI_PIN_CLK  (int)14 /*!< CLK pin */

    #define SPI_DISPLAY_CS (int)27 /*!< display CS pin */
    #define SPI_SD_CS      (int)23 /*!< sd card CS pin */
    #define SPI_DWM_CS     (int)15 /*!< dwm module CS pin */

    #define SPI_BLOCK_SIZE (int)512 /*!< maximum size of the SD block */

/** @brief This enumeration contains available SPI channels. */
typedef enum {
    SPI_DRIVER_CHANNEL_0,       /*!< SPI channel 0 */
    SPI_DRIVER_CHANNEL_1,       /*!< SPI channel 1 */
    SPI_DRIVER_CHANNEL_2,       /*!< SPI channel 2 */
    SPI_DRIVER_CHANNEL_3,       /*!< SPI channel 3 */
    SPI_DRIVER_CHANNEL_4,       /*!< SPI channel 4 */
    SPI_DRIVER_CHANNEL_5,       /*!< SPI channel 5 */
    SPI_DRIVER_CHANNEL_6,       /*!< SPI channel 6 */
    SPI_DRIVER_CHANNEL_7,       /*!< SPI channel 7 */
    SPI_DRIVER_INVALID_CHANNEL, /*!< invalid SPI channel */
} spi_channel_e;

/**
 * @brief This function initializes the SPI communication based on the mode set by the SPI_MODE flag.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t spi_driver_init(void);

/**
 * @brief This function is used to both read and write a single byte using the SPI communication protocol.
 *
 * @param[in]   data        Contains the data to be sent. If no value should be sent, this parameter must be 0xFF.
 *
 * @return      (uint8_t)   value at register SPDR.
 */
uint8_t spi_driver_transact(uint8_t data);

/**
 * @brief Select the display device on the SPI bus.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t set_use_display_spi(void);

/**
 * @brief Select the SD device on the SPI bus.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t set_use_sd_spi(void);

/**
 * @brief Select the dwm device on the SPI bus.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t set_use_dwm_spi(void);

#endif
/** @}*/
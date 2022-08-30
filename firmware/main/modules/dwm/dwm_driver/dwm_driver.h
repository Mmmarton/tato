/*! @file dwm_driver.h
 *
 * @brief This module acts as an API for a user-defined DWM module.
 */

/*!
 * @defgroup DWM
 * @{*/

#ifndef DWM_DRIVER_H
    #define DWM_DRIVER_H

    #include <stdint.h>

    #include "error_manager.h"
    #include "deca_regs.h"
    #include "deca_device_api.h"

    #define DWM3000_DRIVER_CS_PIN   15 /*!< chip select pin */
    #define DWM3000_DRIVER_IRQ_PIN  16 /*!< interrupt request pin */
    #define DWM3000_DRIVER_WAKE_PIN 17 /*!< wake up pin */
    #define DWM3000_DRIVER_RST_PIN  18 /*!< reset pin */

    #define DWM_DEFAULT_CONFIGURATION() {       \
        .channel      = DWM_CHANNEL_5,          \
        .preamble_len = DWM_PREM_LENGTH_128,    \
        .sfd_type     = DWM_SFD_TYPE_1,         \
        .data_rate    = DWM_DATA_RATE_6M8,      \
    }

/** @brief This enumeration contains the available channels. */
typedef enum {
    DWM_CHANNEL_5 = 5,
    DWM_CHANNEL_9 = 9,
} dwm_channel_e;

/** @brief This enumeration contains the available data rates. */
typedef enum {
    DWM_DATA_RATE_850K,
    DWM_DATA_RATE_6M8,
} dwm_data_rate_e;

/** @brief This enumeration contains the available preamble lengths. */
typedef enum {
    DWM_PREM_LENGTH_64   = DWT_PLEN_64,
    DWM_PREM_LENGTH_128  = DWT_PLEN_128,
    DWM_PREM_LENGTH_256  = DWT_PLEN_256,
    DWM_PREM_LENGTH_512  = DWT_PLEN_512,
    DWM_PREM_LENGTH_1024 = DWT_PLEN_1024,
    DWM_PREM_LENGTH_1536 = DWT_PLEN_1536,
    DWM_PREM_LENGTH_2048 = DWT_PLEN_2048,
    DWM_PREM_LENGTH_4096 = DWT_PLEN_4096,
} dwm_preamble_len_e;

/** @brief This enumeration contains the available SFD types. */
typedef enum {
    DWM_SFD_TYPE_0, /*!< IEEE 8-bit standard */
    DWM_SFD_TYPE_1, /*!< DW 8-bit standard */
    DWM_SFD_TYPE_2, /*!< DW 16-bit standard */
    DWM_SFD_TYPE_3, /*!< 4z BPRF 8-bit standard */
} dwm_sfd_type_e;

/**
 * @brief This structure is a container for the available configurations.
 */
typedef struct {
    dwm_channel_e      channel;      /*!< channel to be used */
    dwm_data_rate_e    data_rate;    /*!< data rate to be used */
    dwm_preamble_len_e preamble_len; /*!< message preamble length */
    dwm_sfd_type_e     sfd_type;     /*!< sfd type to be used */
} dwm_configuration_t;

/**
 * @brief This function is used to initialize the selected DWM driver.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_driver_init(dwm_configuration_t *config);

/**
 * @brief This function is used to transmit a buffer through the selected DWM mdoule.
 *
 * @param[in]   buffer  The buffer to be transmitted.
 * @param[in]   size    The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_driver_transmit_data(uint8_t *buffer, uint32_t size);

/**
 * @brief This function is used to transmit a buffer through the selected DWM mdoule and wait for a response.
 *
 * @param[in]   buffer  The buffer to be transmitted and the response buffer.
 * @param[in]   size    Pointer to the size of the transmission buffer and the response buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_driver_request_data(uint8_t *buffer, uint32_t *size);

/**
 * @brief This function is used to retrieve a buffer and its size through the selected DWM mdoule.
 *
 * @param[out]  buffer  The buffer to store the retrieved data.
 * @param[out]  size    The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_driver_retrieve_data(uint8_t *buffer, uint32_t *size);

/**
 * @brief This function is used to put the selected DWM module to sleep.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_driver_enable_sleep(void);

/**
 * @brief This function is used to wake-up the selected DWM module.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_driver_wakeup(dwm_configuration_t *config);

/**
 * @brief This function is used to poll the DWM interrupt registers in case no IRQ pin is present.
 * 
 * @return (void)
 */
void dwm_driver_poll_irq(void);

#endif
/** @}*/
/*! @file m24lr_driver.h
 *
 * @brief This driver is used to enable the NFC capability of the badge.
 */

/*!
 * @defgroup M24LR
 * @{*/

#ifndef M24LR_DRIVER_H
    #define M24LR_DRIVER_H

    #include <stdint.h>

    #include "error_manager.h"

    #define M24LR_DRIVER_I2C_USER_ADDRESS 0x53 /*!< I2C user address for the M24LR IC */
    #define M24LR_DRIVER_I2C_SYS_ADDRESS  0x57 /*!< I2C system address for the M24LR IC */
    #define M24LR_DRIVER_CONTROL_ADDRESS  2336 /*!< energy harvesting control register */

    #define M24LR_DRIVER_BUSY_PIN 33 /*!< BUSY pin for the NFC driver */

/**
 * @brief This enumeration contains the TNF(type name format) types as per NFC Data Exchange Format (NDEF)
 *        Technical Specification. See page 20, link to the document below:
 *
 *        (http://sweet.ua.pt/andre.zuquete/Aulas/IRFID/11-12/docs/NFC%20Data%20Exchange%20Format%20(NDEF).pdf).
 */
typedef enum {
    M24LR_DRIVER_NDEF_TYPE_EMPTY,     /*!< TNF - Empty - not implemented */
    M24LR_DRIVER_NDEF_TYPE_FORUM_WKT, /*!< TNF - NFC Forum well-known type*/
    M24LR_DRIVER_NDEF_TYPE_MEDIA,     /*!< TNF - Media-type as defined in RFC 2046 - not implemented */
    M24LR_DRIVER_NDEF_TYPE_AURI,      /*!< TNF - Absolute URI as defined in RFC 3986 - not implemented */
    M24LR_DRIVER_NDEF_TYPE_FORUM_EXT, /*!< TNF - NFC Forum external type - not implemented */
    M24LR_DRIVER_NDEF_TYPE_UNKNOWN,   /*!< TNF - Unknown - not implemented */
    M24LR_DRIVER_NDEF_TYPE_UNCHANGED, /*!< TNF - Unchanged - not implemented */
    M24LR_DRIVER_NDEF_TYPE_RESERVED,  /*!< TNF - Reserved - not implemented */
} ndef_type_e;

/**
 * @brief This function is used to initialize the M24LR driver.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t m24lr_driver_init(void);

/**
 * @brief This function is used to write NDEF data based on a specified TNF-compliant type.
 *
 * @param[in]   type         TNF-compliant type.
 * @param[in]   ndef_message buffer containing the data to be sent.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t m24lr_driver_write_ndef(ndef_type_e type, uint8_t *ndef_message);

/**
 * @brief This function is used to read the NDEF data that is currently stored based on a specified TNF-compliant type.
 *
 * @param[in]   type         TNF-compliant type.
 * @param[out]  ndef_message buffer for retrieved data.
 * @param[out]  data_length  length of the data buffer
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t m24lr_driver_read_ndef(ndef_type_e type, uint8_t *ndef_message, uint8_t *data_length);

/**
 * @brief This function is used to enable energy harvesting. Without this function, the NFC tag will not receive enough
 *        power to resolve a request, thus rendering the NFC tag unreadable.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t m24lr_driver_enable_energy_harvesting(void);

/**
 * @brief This function is used to disable energy harvesting.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t m24lr_driver_disable_energy_harvesting(void);

/**
 * @brief This function is used to clear the buffer containing the NDEF data.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t m24lr_driver_clear_ndef_buffer(void);

#endif
/** @}*/
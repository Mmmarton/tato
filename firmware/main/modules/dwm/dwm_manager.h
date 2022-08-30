#ifndef DWM_MANAGER_H
#define DWM_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "error_manager.h"

#define DWM_MANAGER_MAXIMUM_FRAME_LENGTH 127 /*!< maximum frame length (for data transmission) */

typedef enum {
    DWM_MANAGER_BLINK_TRANSMIT, /*!< transmit simple blink */
    DWM_MANAGER_BLINK_REQUEST,  /*!< transmit blink and request response */
} dwm_manager_blink_type_e;

/**
 * @brief This structure contains data about the how the DWM driver should be configured.
 */
typedef struct {
    uint8_t channel;      /*!< DWM channel used for UWB comm */
    uint8_t data_rate;    /*!< DWM data rate used for UWB comm */
    uint8_t preamble_len; /*!< DWM SFD type used for UWB comm */
    uint8_t sfd_type;     /*!< DWM preamble length used for UWB comm */
} dwm_manager_config_t;

/**
 * @brief This function is used to initialize the DWM manager.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_init(dwm_manager_config_t *config);

/**
 * @brief This function is used to transmit a blink.
 *
 * @param[in]   device_id ID of the badge.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_transmit_blink(uint8_t *device_id, dwm_manager_blink_type_e type, bool is_sos_triggered);

/**
 * @brief This function is used to transmit data over the UWB using the DWM module.
 *
 * @param[in] buffer      buffer containing the ID of the badge.
 * @param[in] buffer_size size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_transmit_data(uint8_t *buffer, uint32_t buffer_size);

/**
 * @brief This function is used to retrieve data over the UWB using the DWM module.
 *
 * @param[in] buffer      buffer to be retrieved.
 * @param[in] buffer_size size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_retrieve_data(uint8_t *buffer, uint32_t *buffer_size);

/**
 * @brief This function is used to transmit a buffer over UWB and wait for a response.
 *
 * @param[in] buffer      the buffer to be transmitted and the response buffer.
 * @param[in] buffer_size pointer to the size of the transmission buffer and the response buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_request_data(uint8_t *buffer, uint32_t *buffer_size);

/**
 * @brief This function is used to put the module into sleep mode.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_enable_sleep(void);

/**
 * @brief This function offers manual control over the DWM interrupt line. Calling this function will check if any
 *        interrupt events have taken place between calls.
 *
 * @return (void)
 */
void dwm_manager_poll_irq(void);

/**
 * @brief This function sends status of the badge over UWB
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t dwm_manager_report_status(status_t status, uint8_t *device_id);

#endif
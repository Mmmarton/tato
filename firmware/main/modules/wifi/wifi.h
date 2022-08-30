#ifndef WIFI_H
#define WIFI_H

#include "log.h"
#include "error_manager.h"

#include <stdint.h>

/**
 * @brief This function is used to start a WIFI connection to the specified network.
 *
 * @param[in] network_ssid The name of the WIFI network.
 * @param[in] network_pass The password of the WIFI network.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t wifi_init(uint8_t *network_ssid, uint8_t *network_pass);

/**
 * @brief This function is used to stop an existing WIFI connection.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t wifi_stop(void);

#endif
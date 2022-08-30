#ifndef OTA_H
#define OTA_H

#include <stdint.h>
#include <string.h>

#include "error_manager.h"

#define OTA_FILE_CHUNK_SIZE 4096

typedef struct binary_data_t {
    uint32_t size;
    uint32_t remaining_size;
    uint8_t  data[OTA_FILE_CHUNK_SIZE];
} binary_data_t;

/**
 * @brief This function is used to start an OTA firmware update by providing the name of the firmware file stored on the
 *        SD card.
 *
 * @param[in] filename The name of the firmware file.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t ota_update_firmware(uint8_t *filename);

#endif
#include "ota.h"
#include "log.h"

#include "esp_event.h"
#include "esp_ota_ops.h"
#include "nvs.h"
#include "nvs_flash.h"

#define MODULE_TAG "OTA"

void get_fw_file_data(binary_data_t *data, FILE *fp);
void read_chunk(uint8_t *buffer, size_t size, size_t nitems, size_t offset, FILE *fp);

status_t ota_update_firmware(uint8_t *name) {
    LOG_INFO("starting firmware update");

    esp_err_t status = nvs_flash_init();
    if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        status = nvs_flash_init();
    }
    if (status != ESP_OK) {
        LOG_WARNING("nvs init failed %d", status);
        return STATUS_OTA_NVS_INIT_FAIL;
    }

    esp_ota_handle_t       handler   = 0;
    const esp_partition_t *partition = esp_ota_get_next_update_partition(NULL);
    if (partition == NULL) {
        LOG_WARNING("partition null");
        return STATUS_OTA_NULL_PARTITION_FAIL;
    }

    status = esp_ota_begin(partition, OTA_SIZE_UNKNOWN, &handler);
    if (status != ESP_OK) {
        LOG_WARNING("could not start OTA");
        return STATUS_OTA_UPDATE_START_FAIL;
    }

    char filename[64];
    sprintf(filename, "/sdcard%s", (char *)name);

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fclose(file);
        return STATUS_OTA_INVALID_FILE_FAIL;
    }

    binary_data_t data;
    get_fw_file_data(&data, file);

    while (data.remaining_size > 0) {
        size_t size = data.remaining_size <= OTA_FILE_CHUNK_SIZE ? data.remaining_size : OTA_FILE_CHUNK_SIZE;
        read_chunk(data.data, size, 1, data.size - data.remaining_size, file);
        status = esp_ota_write(handler, data.data, size);
        if (data.remaining_size <= OTA_FILE_CHUNK_SIZE) {
            break;
        }
        data.remaining_size -= OTA_FILE_CHUNK_SIZE;
    }

    status = esp_ota_end(handler);
    if (status != ESP_OK) {
        if (status == ESP_ERR_OTA_VALIDATE_FAILED) {
            LOG_ERROR("image validation failed");
            return STATUS_OTA_IMAGE_VALIDATION_FAIL;
        }
        LOG_ERROR("esp_ota_end failed (%s)!", esp_err_to_name(status));
        return STATUS_OTA_UPDATE_END_FAIL;
    }

    status = esp_ota_set_boot_partition(partition);
    if (status != ESP_OK) {
        LOG_ERROR("esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(status));
        return STATUS_OTA_BOOT_PARTITION_SET_FAIL;
    }

    fclose(file);
    LOG_INFO("firmware update done. restarting...");
    esp_restart();

    return STATUS_OK;
}

void get_fw_file_data(binary_data_t *data, FILE *fp) {
    fseek(fp, 0, SEEK_END);

    data->size           = ftell(fp);
    data->remaining_size = data->size;
    memset(data->data, 0, OTA_FILE_CHUNK_SIZE);

    fseek(fp, 0, SEEK_SET);
}

void read_chunk(uint8_t *buffer, size_t size, size_t nitems, size_t offset, FILE *fp) {
    if (fseek(fp, offset, SEEK_SET) != 0) {
        return;
    }

    fread(buffer, size, nitems, fp);
}

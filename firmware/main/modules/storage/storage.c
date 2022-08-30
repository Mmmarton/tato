#include "storage.h"
#include "esp_vfs_fat.h"
#include "log.h"
#include "sdmmc_cmd.h"
#include "spi_driver.h"
#include "utilities.h"
#include <dirent.h>
#include <sys/stat.h>

#include <string.h>

#define MODULE_TAG  "STORAGE"
#define MOUNT_POINT "/sdcard"

#define BMP_WHITE_INDEX 3
#define BMP_LIGHT_INDEX 2
#define BMP_DARK_INDEX  1
#define BMP_BLACK_INDEX 0

#define FILE_ACCESS_MODE_WRITE_BINARY  "wb"
#define FILE_ACCESS_MODE_APPEND_BINARY "ab"
#define FILE_ACCESS_MODE_WRITE_ASCII   "w"
#define FILE_ACCESS_MODE_APPEND_ASCII  "a"
#define FILE_ACCESS_MODE_READ_BINARY   "rb"
#define FILE_ACCESS_MODE_READ_ASCII    "r"

static bool  is_init = false;
static char *file_access_modes[]
    = {FILE_ACCESS_MODE_WRITE_BINARY, FILE_ACCESS_MODE_APPEND_BINARY, FILE_ACCESS_MODE_WRITE_ASCII,
       FILE_ACCESS_MODE_APPEND_ASCII, FILE_ACCESS_MODE_READ_BINARY,   FILE_ACCESS_MODE_READ_ASCII};

storage_bmp_image_t storage_get_image_from_buffer(uint8_t *buffer);

uint8_t  storage_get_old_data_byte(uint8_t *buffer);
uint8_t  storage_get_new_data_byte(uint8_t *buffer);
uint8_t  storage_get_old_color(uint8_t byte);
uint8_t  storage_get_new_color(uint8_t byte);
status_t storage_clear_path(char *path, char *path_to_skip, uint8_t path_length);

void get_16bit_bmp_image(storage_bmp_image_t *output, uint8_t *file_buffer, long file_size, uint8_t *image_buffer,
                         uint8_t *new_data_buffer);

status_t storage_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdmmc_card_t                    *card;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files              = 2,
        .allocation_unit_size   = 16 * 1024,
    };

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs               = SPI_SD_CS;
    slot_config.host_id               = host.slot;

    esp_err_t ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            LOG_WARNING("failed to mount filesystem");
            return STATUS_STORAGE_FILESYSTEM_MOUNT_ERROR;
        }

        LOG_WARNING("failed to initialize the card (%s)", esp_err_to_name(ret));
        return STATUS_STORAGE_FILESYSTEM_INIT_ERROR;
    }

    is_init = true;
    return STATUS_OK;
}

status_t storage_read_bmp_file(storage_bmp_image_t *output, uint8_t *name, uint8_t *image_buffer,
                               uint8_t *new_data_buffer) {
    CHECK_INIT(is_init);

    char filename[100];
    sprintf(filename, "%s%s", (char *)MOUNT_POINT, (char *)name);

    FILE *f = fopen(filename, file_access_modes[STORAGE_READ_BINARY_MODE]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_BMP_FILE;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t file_buffer[file_size];
    fread(file_buffer, sizeof(uint8_t), file_size, f);

    get_16bit_bmp_image(output, file_buffer, file_size, image_buffer, new_data_buffer);

    fclose(f);
    return STATUS_OK;
}

status_t storage_get_file_size(uint8_t *name, storage_mode_e mode, uint32_t *size) {
    CHECK_INIT(is_init);

    char filename[100];
    sprintf(filename, "%s%s", (char *)MOUNT_POINT, (char *)name);
    FILE *f = fopen(filename, file_access_modes[mode]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_TXT_FILE;
    }
    fseek(f, 0, SEEK_END);
    (*size) = ftell(f);
    fclose(f);
    return STATUS_OK;
}

status_t storage_read_txt_file(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint32_t *size) {
    CHECK_INIT(is_init);

    char filename[100];
    sprintf(filename, "%s%s", (char *)MOUNT_POINT, (char *)name);

    FILE *f = fopen(filename, file_access_modes[mode]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_TXT_FILE;
    }

    long file_size = (*size);
    if (file_size == 0) {
        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        fseek(f, 0, SEEK_SET);
    }

    uint8_t file_buffer[file_size];
    fread(file_buffer, sizeof(uint8_t), file_size, f);

    memcpy(buffer, file_buffer, file_size);
    fclose(f);
    return STATUS_OK;
}

status_t storage_read_file_sequences(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint32_t size,
                                     uint32_t offset, uint32_t *read_size) {
    CHECK_INIT(is_init);

    char filename[100];
    sprintf(filename, "%s%s", (char *)MOUNT_POINT, (char *)name);

    FILE *f = fopen(filename, file_access_modes[mode]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_TXT_FILE;
    }

    fseek(f, offset, SEEK_SET);

    *read_size = fread(buffer, sizeof(uint8_t), size, f);

    fclose(f);
    return STATUS_OK;
}

status_t storage_write_txt_file(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint16_t size) {
    CHECK_INIT(is_init);

    char filename[100];
    sprintf(filename, "%s%s", (char *)MOUNT_POINT, (char *)name);

    FILE *f = fopen(filename, file_access_modes[mode]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_TXT_FILE;
    }

    fwrite(buffer, sizeof(char), size, f);

    fclose(f);
    return STATUS_OK;
}

status_t storage_write_binary_file(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint16_t size) {
    CHECK_INIT(is_init);

    if (!(mode == STORAGE_WRITE_BINARY_MODE_REPLACE) && !(mode == STORAGE_WRITE_BINARY_MODE_APPEND)) {
        return STATUS_STORAGE_INVALID_BINARY_ACTION;
    }

    char filename[100];
    sprintf(filename, "%s%s", (char *)MOUNT_POINT, (char *)name);

    FILE *f = fopen(filename, file_access_modes[mode]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_BINARY_FILE;
    }

    fwrite(buffer, sizeof(char), size, f);

    fclose(f);
    return STATUS_OK;
}

status_t storage_clear(char *path_to_skip) {
    CHECK_INIT(is_init);

    char full_path[257];

    if (path_to_skip == NULL) {
        full_path[0] = '\0';
    }
    sprintf(full_path, "%s/%s", (char *)MOUNT_POINT, (char *)path_to_skip);

    return storage_clear_path(MOUNT_POINT, full_path, strlen(full_path));
}

status_t storage_clear_path(char *path, char *path_to_skip, uint8_t path_length) {
    if (memcmp(path, path_to_skip, path_length) == 0) {
        return STATUS_OK;
    }

    struct dirent *files;
    DIR           *dir = opendir(path);

    if (dir == NULL) {
        LOG_ERROR("Failed to delete path: %s", path);
        return STATUS_STORAGE_LIST_FAIL;
    }

    while ((files = readdir(dir)) != NULL) {
        struct stat buffer;
        char        full_path[257];
        sprintf(full_path, "%s/%s", path, files->d_name);
        stat(full_path, &buffer);

        if (S_ISDIR(buffer.st_mode)) {
            status_t status = storage_clear_path(full_path, path_to_skip, path_length);
            if (status != 0) {
                closedir(dir);
                return status;
            }
        }
        else {
            if (remove(full_path) != 0) {
                closedir(dir);
                return STATUS_STORAGE_DELETE_FAIL;
            }
        }
    }

    closedir(dir);
    if (memcmp(path, MOUNT_POINT, strlen(path)) != 0) {
        if (remove(path) != 0) {
            return STATUS_STORAGE_DELETE_FAIL;
        }
    }

    return STATUS_OK;
}

status_t storage_copy_file(char *from, char *to) {
    CHECK_INIT(is_init);

    char from_path[257];
    sprintf(from_path, "%s/%s", (char *)MOUNT_POINT, (char *)from);

    char to_path[257];
    sprintf(to_path, "%s/%s", (char *)MOUNT_POINT, (char *)to);

    FILE *file_from = fopen(from_path, FILE_ACCESS_MODE_READ_BINARY);
    FILE *file_to   = fopen(to_path, FILE_ACCESS_MODE_WRITE_BINARY);

    if (file_from == NULL || file_to == NULL) {
        fclose(file_from);
        fclose(file_to);
        return STATUS_STORAGE_NULL_BINARY_FILE;
    }

    int byte = fgetc(file_from);
    while (byte != EOF) {
        fputc(byte, file_to);
        byte = fgetc(file_from);
    }

    fclose(file_from);
    fclose(file_to);
    return STATUS_OK;
}

status_t storage_copy_directory_content(char *from, char *to) {
    CHECK_INIT(is_init);

    char from_path[257];
    char to_path[257];
    sprintf(from_path, "%s/%s", MOUNT_POINT, from);

    struct dirent *files;
    DIR           *dir = opendir(from_path);

    if (dir == NULL) {
        return STATUS_STORAGE_LIST_FAIL;
    }

    char    filenames[30][100];
    uint8_t file_count = 0;

    while ((files = readdir(dir)) != NULL) {
        memcpy(filenames[file_count++], files->d_name, strlen(files->d_name) + 1);
    }

    closedir(dir);

    for (uint8_t i = 0; i < file_count; i++) {
        sprintf(from_path, "%s/%s", from, filenames[i]);
        sprintf(to_path, "%s/%s", to, filenames[i]);
        storage_copy_file(from_path, to_path);
    }

    return STATUS_OK;
}

status_t storage_write_bmp_file(uint8_t *name, uint8_t *buffer, uint16_t size) {
    CHECK_INIT(is_init);

    char filename[100];
    sprintf(filename, "%s%s", MOUNT_POINT, name);

    FILE *f = fopen(filename, file_access_modes[0]);
    if (f == NULL) {
        fclose(f);
        return STATUS_STORAGE_NULL_TXT_FILE;
    }

    fwrite(buffer, sizeof(char), size, f);

    fclose(f);
    return STATUS_OK;
}

void get_16bit_bmp_image(storage_bmp_image_t *output, uint8_t *file_buffer, long file_size, uint8_t *image_buffer,
                         uint8_t *new_data_buffer) {

    storage_bmp_image_t image          = storage_get_image_from_buffer(file_buffer);
    uint16_t            buffer_counter = 0;
    for (size_t i = 0; i < file_size; i++) {
        if (i < image.file_offset) {
            continue;
        }
        uint8_t byte[]                  = {file_buffer[i], file_buffer[i + 1], file_buffer[i + 2], file_buffer[i + 3]};
        image_buffer[buffer_counter]    = storage_get_old_data_byte(byte);
        new_data_buffer[buffer_counter] = storage_get_new_data_byte(byte);
        buffer_counter++;
        i += 3;
        if (buffer_counter >= image.buffer_size) {
            LOG_INFO("built image");
            output->buffer_size = image.buffer_size;
            output->file_offset = image.file_offset;
            output->height      = image.height;
            output->width       = image.width;
            output->image_type  = image.image_type;
            break;
        }
    }
}

storage_bmp_image_t storage_get_image_from_buffer(uint8_t *buffer) {
    storage_bmp_image_t image;

    uint32_t *offset  = (uint32_t *)&buffer[0x0A];
    uint32_t *width   = (uint32_t *)&buffer[0x12];
    uint32_t *height  = (uint32_t *)&buffer[0x16];
    uint32_t *size    = (uint32_t *)&buffer[0x22];
    uint16_t *bit_num = (uint16_t *)&buffer[0x1C];

    image.width       = (*width);
    image.height      = (*height);
    image.file_offset = (*offset);
    image.buffer_size = (*size);
    image.image_type  = STORAGE_IMAGE_TYPE_MONOCHROME;

    if ((*bit_num) == 4) {
        image.image_type  = STORAGE_IMAGE_TYPE_GRAYSCALE;
        image.buffer_size = image.buffer_size / 4;
    }

    return image;
}

uint8_t storage_get_old_data_byte(uint8_t *buffer) {
    uint8_t byte = 0;

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t low  = buffer[i] & 0x0F;
        uint8_t high = buffer[i] >> 4;

        byte |= (storage_get_old_color(high) << (7 - (2 * i)));
        byte |= (storage_get_old_color(low) << (7 - (2 * i) - 1));
    }

    return byte;
}

uint8_t storage_get_new_data_byte(uint8_t *buffer) {
    uint8_t byte = 0;

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t low  = buffer[i] & 0x0F;
        uint8_t high = buffer[i] >> 4;

        byte |= (storage_get_new_color(high) << (7 - (2 * i)));
        byte |= (storage_get_new_color(low) << (7 - (2 * i) - 1));
    }

    return byte;
}

uint8_t storage_get_new_color(uint8_t byte) {
    if (byte < BMP_DARK_INDEX) {
        return 0;
    }
    else if (byte < BMP_LIGHT_INDEX) {
        return 1;
    }
    else if (byte < BMP_WHITE_INDEX) {
        return 0;
    }
    return 1;
}

uint8_t storage_get_old_color(uint8_t byte) {
    return (byte >= BMP_LIGHT_INDEX);
}
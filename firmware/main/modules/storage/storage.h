#ifndef STORAGE_H
#define STORAGE_H

#include "error_manager.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief This enumeration contains the valid types of images that the module can handle.
 */
typedef enum {
    STORAGE_IMAGE_TYPE_MONOCHROME, /*!< monochrome image */
    STORAGE_IMAGE_TYPE_GRAYSCALE,  /*!< grayscale image */
    STORAGE_IMAGE_TYPE_OLD_DATA,   /*!< old data image */
    STORAGE_IMAGE_TYPE_NEW_DATA,   /*!< new data image */
} storage_image_type_e;

/**
 * @brief This enumeration contains the flags for the file handler function.
 */
typedef enum {
    STORAGE_WRITE_BINARY_MODE_REPLACE, /*!< replace the existing binary data */
    STORAGE_WRITE_BINARY_MODE_APPEND,  /*!< append to the existing binary data */
    STORAGE_WRITE_ASCII_MODE_REPLACE,  /*!< replace the existing ascii data */
    STORAGE_WRITE_ASCII_MODE_APPEND,   /*!< append to the existing ascii data */
    STORAGE_READ_BINARY_MODE,          /*!< read existing binary data */
    STORAGE_READ_ASCII_MODE,           /*!< read existing ascii data */
} storage_mode_e;

/**
 * @brief This structure contains data about the bmp image file.
 */
typedef struct {
    uint16_t             buffer_size; /*!< size of the image buffer */
    uint16_t             width;       /*!< width of the image */
    uint16_t             height;      /*!< height of the image */
    uint8_t              file_offset; /*!< buffer offset */
    storage_image_type_e image_type;  /*!< type of image */
} storage_bmp_image_t;

/**
 * @brief This function is used to initialize the storage module.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_init(void);

/**
 * @brief This function is used to read a .bmp image file.
 *
 * @param[out] output          Properties of the output image.
 * @param[in]  name            Name of the image to be searched for.
 * @param[out] old_data_buffer Buffer containing "old" image data
 * @param[out] new_data_buffer Buffer containing "new" image data
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_read_bmp_file(storage_bmp_image_t *output, uint8_t *name, uint8_t *old_data_buffer,
                               uint8_t *new_data_buffer);

/**
 * @brief This function is used to return file size.
 *
 * @param[in]  name   The name of the .txt file.
 * @param[in]  mode   The type of action to be performed by the file handler.
 * @param[out] size   The size of the file.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_get_file_size(uint8_t *name, storage_mode_e mode, uint32_t *size);

/**
 * @brief This function is used to read a .txt file. If size equals 0, the function will read and return the
 * computed size of the file. If the size is specified, the file handler reads the specified number of bytes from
 * the file.
 *
 * @param[in]  name   The name of the .txt file.
 * @param[in]  mode   The type of action to be performed by the file handler.
 * @param[out] buffer The buffer retrieved from the SD card.
 * @param[out] size   The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_read_txt_file(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint32_t *size);

/**
 * @brief This function is used to read a sequence from a file.
 *
 * @param[in]  name        The name of the file.
 * @param[in]  mode        The type of action to be performed by the file handler.
 * @param[in]  offset      The posision from you want to read.
 * @param[in]  size        The size of the buffer what you want to read.
 * @param[out] buffer      The buffer retrieved from the SD card.
 * @param[out] read_size The size of read buffer

 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_read_file_sequences(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint32_t size,
                                     uint32_t offset, uint32_t *read_size);
/**
 * @brief This function is used to write a given buffer to a .bmp file.
 *
 * @param[in] name   The name of the .bmp file.
 * @param[in] buffer The buffer to be saved in the specified file.
 * @param[in] size   The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_write_bmp_file(uint8_t *name, uint8_t *buffer, uint16_t size);

/**
 * @brief This function is used to write a given buffer to a .txt file.
 *
 * @param[in] name   The name of the .txt file.
 * @param[in] mode   The type of action to be performed by the file handler.
 * @param[in] buffer The buffer to be saved in the specified file.
 * @param[in] size   The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_write_txt_file(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint16_t size);

/**
 * @brief This function is used to write a given buffer to a binary file.
 *
 * @param[in] name   The name of the binary file.
 * @param[in] mode   The type of action to be performed by the file handler.
 * @param[in] buffer The buffer to be saved in the specified file.
 * @param[in] size   The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_write_binary_file(uint8_t *name, storage_mode_e mode, uint8_t *buffer, uint16_t size);

/**
 * @brief This function is used to clear all files from the storage.
 *
 * @param[in] path_to_skip   The path that should be skipped. If empty or nonexistent, everything will be deleted.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t storage_clear(char *path_to_skip);

/**
 * @brief This function is used to clear all files from the storage.
 *
 * @param[in] from   The name of the file to copy from.
 * @param[in] to     The name of the file to copy to.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 * @note There are some limitations regarding the number of files and the filename lengths.
 *       If files are not getting copied, consult and change those values.
 */
status_t storage_copy_file(char *from, char *to);

/**
 * @brief This function is used to clear all files from the storage.
 *
 * @param[in] from   The name of the directory to copy all the files from.
 * @param[in] to     The name of the directory to copy the files to.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 * @note There are some limitations regarding the number of files and the filename lengths.
 *       If files are not getting copied, consult and change those values.
 */
status_t storage_copy_directory_content(char *from, char *to);

#endif
/*! @file display_manager.h
 *
 * @brief This module handles the retrieving and displaying of bitmap and text files stored as FAT32 files.
 */

/*!
 * @defgroup DISPLAY
 * @{*/

#ifndef DISPLAY_MANAGER_H
    #define DISPLAY_MANAGER_H

    #include "GDEW0371W7_driver.h"
    #include "error_manager.h"
    #include "storage.h"
    #include <stdbool.h>

    #define PROFILE_BUFFER_OLD_ADDRESS ((uint32_t)0x000F8C00 + 1) /*!< address of the profile screen buffer */
    #define WEATHER_BUFFER_OLD_ADDRESS ((uint32_t)0x0108600 + 1)  /*!< address of the weather screen buffer */
    #define NOTIFICATIONS_BUFFER_OLD_ADDRESS \
        ((uint32_t)0x0118000 + 1) /*!< address of the notifications screen buffer */

    #define SCREEN_BUFFERS_OFFSET 17504 /*!< offset between old and new data addresses */

/** @brief This enumeration contains flags that decide where to read from or write on the SD card. */
typedef enum {
    BADGE_SCREEN_PROFILE,       /*!< flag indicating the profile screen */
    BADGE_SCREEN_NOTIFICATIONS, /*!< flag indicating the notifications screen */
    BADGE_SCREEN_WEATHER,       /*!< flag indicating the weather screen */
} badge_screen_e;

/**
 * @brief This function is used to initialize the DWM manager.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_init(void);

/**
 * @brief This function is used to write an image buffer on the display buffer at the specified coordinates. The image
 *        is being searched for on the SD card. The name of the image must be passed as a parameter.
 *
 * @param[in]   image           Name of the image.
 * @param[in]   x               The x-coordinate of the top-left corner of the image.
 * @param[in]   y               The y-coordinate of the top-left corner of the image.
 * @param[in]   is_transparent  Controls if the white is being displayed or is overlayed with the other iamges.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_write_image_to_buffer(uint8_t *name, uint16_t x, uint16_t y, bool is_transparent);

/**
 * @brief This function is used to write a given text on the display buffer at the specified coordinates.
 *
 * @param[in]   text            The text to be displayed.
 * @param[in]   length          The length of the buffer.
 * @param[in]   x               The x-coordinate of the top-left corner of the image.
 * @param[in]   y               The y-coordinate of the top-left corner of the image.
 * @param[in]   font_size       The font size multiplier.
 * @param[in]   is_transparent  Controls if the white is being displayed or is overlayed with the other iamges.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_write_text_to_buffer(uint8_t *text, uint8_t length, uint16_t x, uint16_t y, uint8_t font_size,
                                              bool is_transparent);

/**
 * @brief This function is used to enable rendering directly on the screen using the display manager.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_start_text_to_screen(void);

/**
 * @brief This function is used to write a given text directly on the display at the specified coordinates.
 *
 * @param[in]   text            The text to be displayed.
 * @param[in]   length          The length of the buffer.
 * @param[in]   x               The x-coordinate of the top-left corner of the image.
 * @param[in]   y               The y-coordinate of the top-left corner of the image.
 * @param[in]   font_size       The font size multiplier.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_write_text_to_screen(uint8_t *text, uint8_t length, uint16_t x, uint16_t y, uint8_t font_size);

/**
 * @brief This function is used to update the screen using the data that was sent using the
 *        `display_manager_write_text_to_screen` function.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_update_text_to_screen(void);

/**
 * @brief This function draws a horizontal separator.
 *
 * @param[in]   x       The x-coordinate of the separator.
 * @param[in]   y       The y-coordinate of the separator.
 * @param[in]   width   The width of the separator.
 * @param[in]   color   The color of the separator.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_write_horizontal_separator(uint16_t x, uint16_t y, uint16_t width,
                                                    gdew0371w7_driver_color_e color);

/**
 * @brief This function draws a filled rectangle.
 *
 * @param[in]   x       The x-coordinate of the rectangle.
 * @param[in]   y       The y-coordinate of the rectangle.
 * @param[in]   width   The width of the rectangle.
 * @param[in]   height  The height of the rectangle.
 * @param[in]   color   The color of the rectangle.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_write_rectangle_filled(uint16_t x, uint16_t y, uint16_t width, uint8_t height,
                                                gdew0371w7_driver_color_e color);

/**
 * @brief This function is used to display a critical error message, that requires a badge reset.
 *
 * @param[in] buffer   The message of the error
 * @param[in] size     The size of the message
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_render_error_message(uint8_t *buffer, uint8_t size);

/**
 * @brief This function is used to render a full display image. This function makes use of the two global
 *        display data arrays `display_data_old[]` and `display_data_new[]`.
 *
 * @param[in]   type    Type of image to be rendered.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_render_full_display(storage_image_type_e type);

/**
 * @brief This function is used to clear the buffers containing the information about the display.
 * @note be careful when using this function, as the buffers it clears keep track on what is already ready to be
 *       rendered on the display, which enables overlaying
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_clear_display_buffers(void);

/**
 * @brief This function is used to display a centered message on the screen
 *
 * @param[in] buffer - the text to be displayed
 * @param[in] size - the length of the text
 * @param[in] font_size - the font size to be used
 * @param[in] is_transparent - the transparency of the text
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t display_manager_write_centered_message(uint8_t *buffer, uint8_t size, uint16_t y, uint8_t font_size,
                                                bool is_transparent);

#endif
/** @}*/
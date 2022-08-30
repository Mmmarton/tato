/*! @file GDEW0371W7_core.h
 *
 * @brief This module handles the low-level process of sending data to the display.
 *
 * The GDEW0371W7 EPD renders images starting at the top-left corner. Top left is considered to be the left corner
 * opposing the part where the display is connected to the PCB.
 *
 * The partial update renders in the exact same manner as the display, but makes use of a different set of
 * look-up tables and commands. This library makes heavy use of the partial area update as it allows the user
 * to draw shapes, text and other images anywhere on the display.
 *
 * @note Another thing to keep in mind is that each pixel is NOT represented by a hex value inside the array that
 *       will be displayed. Instead, each byte will contain a horizontal array of 8 pixels, inside which each 0
 *       will be rendered black on the display while each 1 will be rendered as white. To obtain grayscale levels,
 *       2 randitions are necessary. The process will be explained below.
 */

/*!
 * @defgroup GDEW0371W7
 * @{*/

#ifndef GDEW0371W7_CORE_H
    #define GDEW0371W7_CORE_H

    #include <stdint.h>

    #include "GDEW0371W7_registers.h"
    #include "spi_driver.h"

    #define DISPLAY_WIDTH  240   /*!< width of the display module */
    #define DISPLAY_HEIGHT 416   /*!< height of the display module */
    #define ARRAY_SIZE     12480 /*!< size = ((width * height) / 8) */

/**
 * @brief This enumeration contains the possible pixel colors that can be displayed using the GDEW0371W7 display.
 *        Based on this enumeration, the operation mode is set when deciding the operation mode when writing something.
 *
 * @note: This display can only display a number of 4 grayscale levels (white, light gray, dark gray and black).
 */
typedef enum {
    GDEW0371W7_COLOR_BW_WHITE,      /*!< monochrome white */
    GDEW0371W7_COLOR_BW_BLACK,      /*!< monochrome black */
    GDEW0371W7_COLOR_GS_WHITE,      /*!< grayscale white (0xFF - 0xFF) */
    GDEW0371W7_COLOR_GS_GRAY_LIGHT, /*!< grayscale gray light (0x00 - 0xFF) */
    GDEW0371W7_COLOR_GS_GRAY_DARK,  /*!< grayscale gray dark (0xFF - 0x00) */
    GDEW0371W7_COLOR_GS_BLACK,      /*!< grayscale black (0x00 - 0x00) */
} gdew0371w7_driver_color_e;

/**
 * @brief This function is used to send a reset command alongside the required data.
 *
 * @return      (void)
 */
void display_reset(void);

/**
 * @brief This function is used to clear the screen.
 *
 * @return      (void)
 */
void display_clear(void);

/**
 * @brief This function is used to set a font for rendering text on the display. 
 *
 * @param[in]   index   Index of the font that the user wishes to use.
 * 
 * @return      (void)
 */
void display_set_font(uint8_t index);

/**
 * @brief This function is used to clear a section of the screen.
 *
 * @param[in]   x       The x-coordinate of the area.
 * @param[in]   y       The y-coordinate of the area.
 * @param[in]   width   The width of the area to be cleared.
 * @param[in]   height  The height of the area to be cleared.
 *
 * @return      (void)
 */
void display_partial_clear(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief This function is used to send a powerup command alongside the required data.
 *
 * @return      (void)
 */
void display_powerup(void);

/**
 * @brief This function is used to send a powerup command alongside the required data.
 *
 * @return      (void)
 */
void display_refresh(void);

/**
 * @brief This function is used to send a sleep command  alongside the required data.
 *
 * @return      (void)
 */
void display_sleep(void);

/**
 * @brief This function is used to send a flush command (also known as 'transfer old data' command). This basically
 *        removes all the old data that should be used at the next refresh.
 *
 * @return      (void)
 */
void display_flush(void);

/**
 * @brief This function is used to send a flush command (also known as 'transfer old data' command). This basically
 *        removes all the old data that should be used at the next refresh.
 *
 * @param[in]   x       The x-coordinate of the area.
 * @param[in]   y       The y-coordinate of the area.
 * @param[in]   width   The width of the area to be flushed.
 * @param[in]   height  The height of the area to be flushed.
 *
 * @return      (void)
 */
void display_partial_flush(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief This function is used to send a partial display update command alongisde the lookup tables (lut) required
 *        for the partial display update.
 *
 * @return      (void)
 */
void display_init_partial_update(void);

/**
 * @brief This function is used to send a partial grayscale display update command alongisde the lookup tables (lut)
 *        required for the partial display update.
 *
 * @return (void)
 */
void display_init_partial_grayscale_update(void);

/**
 * @brief This function is used to draw a straight vertical line.
 * @note: Due to the weirdity of image rendering, it is HIGHLY RECOMMENDED to render the vertical lines first when
 *        overlaping with horizontal lines. To render a horizontal line, the display just lights up a given portion
 *        of the array without doing mathematical calculations. This way, when overlaping an already existing vertical
 *        line will not cause any issues.
 *
 * @param[in]   x   The x-coordinate of the start of the line.
 * @param[in]   y1  The y-coordinate of the start of the line.
 * @param[in]   y2  The y-coordinate at the end of the line.
 *
 * @return      (void)
 */
void display_draw_vline(uint16_t x, uint16_t y1, uint16_t y2);

/**
 * @brief This function is used to draw a straight horizontal line.
 *
 * @param[in]   y   The y-coordinate of the start of the line.
 * @param[in]   x1  The x-coordinate of the start of the line.
 * @param[in]   x2  The x-coordinate at the end of the line.
 *
 * @return      (void)
 */
void display_draw_hline(uint16_t y, uint16_t x1, uint16_t x2);

/**
 * @brief This function is used to draw a single pixel of a given color. It is important to note that while
 *        `GDEW0371W7_COLOR_BW_BLACK` AND `GDEW0371W7_COLOR_GS_BLACK` may be rendered as the same color, the
 *        way they are rendered is different. This must be taken into consideration when drawing a single pixel.
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   color   The color of the pixel after rendering.
 *
 * @return      (void)
 */
void display_draw_pixel(uint16_t x, uint16_t y, gdew0371w7_driver_color_e color);

/**
 * @brief This function is used to fill a partial area with a given color.
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   width   The width of the area to be updated.
 * @param[in]   height  he height of the area to be updated.
 * @param[in]   color   The color of the area.
 *
 */
void display_draw_partial_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                 gdew0371w7_driver_color_e color);

/**
 * @brief This function is used to draw a filled defined area on the display.
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   width   The width of the area to be updated.
 * @param[in]   height  The height of the area to be updated.
 * @param[in]   value   The color of the area (this function only supports black and white).
 *
 * @return      (void)
 */
void display_draw_partial_bw_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value);

/**
 * @brief This function is used to transmit "old" data to the display. This specific data in combination with the "new"
 *        one results in the gray scale levels.
 *
 * COLOR      |    (white)   |   (dark_gray)   |   (light_gray)   |   (black)
 * "OLD" DATA |     0xFF     |       0xFF      |        0x00      |     0x00
 * "NEW" DATA |     0xFF     |       0x00      |        0xFF      |     0x00
 *
 * @note: the `value` parameter might be replace by a color parameter in future implementations, right now, this one
 *        provides the most configurability for the tasks at hand. In order to get the desired level of grayscale,
 *        this function must be called before `display_draw_partial_grayscale_filled`.
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   width   The width of the area to be updated.
 * @param[in]   height  The height of the area to be updated.
 * @param[in]   value   The color of the area (in grayscale).
 *
 * @return      (void)
 */
void display_flush_partial_grayscale_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value);

/**
 * @brief This function is used to transmit "new" data to the display. This specific data in combination with the "old"
 *        one results in the gray scale levels. If `display_flush_partial_grayscale_filled` is not called on the same
 *        area before this one, the result will be a simple black and white block on the display.
 *
 * COLOR      |    (white)   |   (dark_gray)   |   (light_gray)   |   (black)
 * "OLD" DATA |     0xFF     |       0xFF      |        0x00      |     0x00
 * "NEW" DATA |     0xFF     |       0x00      |        0xFF      |     0x00
 *
 * @note: the `value` parameter might be replace by a color parameter in future implementations, right now, this one
 *        provides the most configurability for the tasks at hand.
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   width   The width of the area to be updated.
 * @param[in]   height  The height of the area to be updated.
 * @param[in]   value   The color of the area (in grayscale).
 *
 * @return      (void)
 */
void display_draw_partial_grayscale_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value);

/**
 * @brief This function is used to draw a buffer on a given area defined by the paramteres..
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   width   The width of the area to be updated.
 * @param[in]   height  The height of the area to be updated.
 * @param[in]   buffer  The buffer containing the bitmap that will be printed.
 * @param[in]   size    The size of the buffer.
 *
 * @return      (void)
 */
void display_draw_partial_buffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *buffer,
                                 uint16_t size);

/**
 * @brief This function is used to print an image on the display. When using this function, the buffer MUST have the
 *        size of 12480 which is the maximum size supported by the display. This function only prints black and white
 *        images. A function that also prints grayscale images will be added in future implementations.
 *
 * @param[in]   buffer  The buffer to be printed on the screen.
 *
 * @return      (void)
 */
void display_draw_full_buffer(const uint8_t *buffer);

/**
 * @brief This function returns the width (in pixels) of a single character rendered at the current font size.
 *
 * @param[in]   character   Targeted character.
 *
 * @return (uint8_t)
 * @retval width of a character
 */
uint8_t display_get_character_width(uint8_t character);

/**
 * @brief This function returns the height (in pixels) of a single character rendered at the current font size.
 *
 * @param[in]   character   Targeted character.
 *
 * @return (uint8_t)
 * @retval height of a character
 */
uint8_t display_get_character_height(uint8_t character);

/**
 * @brief This function returns the y-axis offset of the targeted character.
 *
 * @param[in]   character   Targeted character.
 *
 * @return (uint8_t)
 * @retval height of a character
 */
int8_t display_get_character_yoffset(uint8_t character);

/**
 * @brief This function is used to build an array that can be used to render the desired character on the display.
 *
 * @param[in]   character   The character to be displayed on the screen.
 * @param[in]   flip_array  Boolean that decides if the array will be mirrored.
 *
 * @return      (void)
 */
void display_build_character_array(uint8_t *array, uint8_t character, bool flip_array);

/**
 * @brief This function is used to set a partial area that needs to be updated.
 *
 * @param[in]   x       The x-coordinate of the start of the line.
 * @param[in]   y       The y-coordinate of the start of the line.
 * @param[in]   width   The width of the area to be updated.
 * @param[in]   height  The height of the area to be updated.
 *
 * @return      (void)
 */
void display_set_partial_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief This function is used to send a command to the display.
 *
 * @param[in]   command Command to be sent to the display.
 *
 * @return      (void)
 */
void write_command(uint8_t command);

/**
 * @brief This function is used to send a byte of data to the display.
 *
 * @param[in]   data    Data to be sent to the display.
 *
 * @return (void)
 */
void write_data(uint8_t data);

#endif
/** @}*/
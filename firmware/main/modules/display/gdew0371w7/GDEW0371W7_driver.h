/*! @file GDEW0371W7_driver.h
 *
 * @brief This module provides the user with a set of functions that can be used to render data on the display.
 */

/*!
 * @defgroup GDEW0371W7
 * @{*/

#ifndef GDEW0371W7_DRIVER_H
    #define GDEW0371W7_DRIVER_H

    #include "GDEW0371W7_core.h"

/**
 * @brief This function is used to initialize the GDEW0371W7 module driver. IN order for the initialization to be
 *        succesfull, first the `enable_spi_communication` function MUST be called before enabling the SPI
 *        driver. Failing to do so might result in undefined behavior.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_init(void);

/**
 * @brief This function is used to clear the entire display.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_clear(void);

/**
 * @brief This function is used to draw a point at the given coordinates.
 *
 * @param[in]   x   The x-coordinate of the point.
 * @param[in]   y   The y-coordinate of the point.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_draw_point(uint16_t x, uint16_t y, gdew0371w7_driver_color_e color);

/**
 * @brief This function is used to draw a line between two given points. At the moment, it only works for straight
 *        vertical/horizontal lines. Diagonal lines support will be added in future implementations.
 *
 * @param[in]   x1  The x-coordinate of the starting point of the line.
 * @param[in]   x2  The x-coordinate of the ending points of the line.
 * @param[in]   y1  The y-coordinate of the starting point of the line.
 * @param[in]   y2  The y-coordinate of the ending point of the line.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_draw_line(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);

/**
 * @brief This function is used to draw a rectangle at the given coordinates of a defined size. If `filled` is
 *        true, the rectangle will be rendered as a single black full block, otherwise, the rectangle will be
 *        rendered using lines.
 *
 * @param[in]   x       The x-coordinate of the top-left corner of the rectangle.
 * @param[in]   y       The y-coordinate of the top-left corner of the rectangle.
 * @param[in]   width   The width of the rectangle.
 * @param[in]   height  The height of the rectangle.
 * @param[in]   filled  The way the rectangle will be rendered.
 * @param[in]   color   The color of the rectangle.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool filled,
                                          gdew0371w7_driver_color_e color);

/**
 * @brief This function is used to draw an image on a defined area of the display that does NOT cover the entirety
 *        of it. The width and the height of the image must be provided as parameters as well as the buffer itself
 *        alongside its size.
 *
 * @param[in]   x       The x-coordinate of the top-left corner of the image.
 * @param[in]   y       The y-coordinate of the top-left corner of the image.
 * @param[in]   width   The width of the iamge.
 * @param[in]   height  The height of the image.
 * @param[in]   buffer  The bitmap image itself as a buffer.
 * @param[in]   size    The size of the buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_draw_image_partial(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *buffer,
                                              uint16_t size);

/**
 * @brief This function is used to render text on the display.
 *
 * @param[in]   x       The x-coordinate of the starting point of the string.
 * @param[in]   y       The y-coordinate of the ending points of the string.
 * @param[in]   buffer  The string itself as a buffer.
 * @param[in]   size    The sizer of the buffer.
 * @param[in]   width   The width of the text buffer.
 * @param[in]   height  The height of the text buffer.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_draw_text(uint8_t *buffer, uint16_t size, uint16_t x, uint16_t y, uint16_t width,
                                     uint16_t height);

/**
 * @brief This function is used to enable the driver and start the data sending. Every data that the user wishes to
 *        display must be preceeded by this function.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_start(void);

/**
 * @brief This function is used to disable the driver and end the data sending. This function will also force an update
 *        which consists of a rendering and sending the display to sleep in order to prevent artifacts appearing on the
 *        display due to excess electricity. This function must be called after a `gdew0371w7_driver_start` call
 *        followed by the user modifications.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t gdew0371w7_driver_update(void);

#endif
/** @}*/
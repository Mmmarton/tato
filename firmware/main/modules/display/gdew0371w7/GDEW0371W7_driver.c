#include <string.h>

#include "GDEW0371W7_driver.h"
#include "log.h"
#include "utilities.h"

#define MODULE_TAG "GDEW0371W7"

static bool is_init = false;

status_t gdew0371w7_driver_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    status_t driver_status = spi_driver_init();
    if (driver_status != STATUS_OK && driver_status != STATUS_ALREADY_INIT) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    SET_OUTPUT_PIN(GDEW_DRIVER_DC);
    SET_OUTPUT_PIN(GDEW_DRIVER_RST);
    SET_INPUT_PIN(GDEW_DRIVER_BUSY);

    is_init = true;
    SET_PIN_HIGH(GDEW_DRIVER_DC);
    return STATUS_OK;
}

status_t gdew0371w7_driver_clear(void) {
    CHECK_INIT(is_init);
    display_clear();

    return STATUS_OK;
}

status_t gdew0371w7_driver_draw_point(uint16_t x, uint16_t y, gdew0371w7_driver_color_e color) {
    CHECK_INIT(is_init);

    if (((x > DISPLAY_WIDTH) || (y > DISPLAY_HEIGHT))) {
        return STATUS_GDEW_DRIVER_OUT_OF_BOUNDS_ERROR;
    }

    display_draw_pixel(x, y, color);
    return STATUS_OK;
}

status_t gdew0371w7_driver_draw_line(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) {
    CHECK_INIT(is_init);

    if (((x1 > DISPLAY_WIDTH) || (y1 > DISPLAY_HEIGHT)) || ((x2 > DISPLAY_WIDTH) || (y2 > DISPLAY_HEIGHT))) {
        return STATUS_GDEW_DRIVER_OUT_OF_BOUNDS_ERROR;
    }

    if (x1 == x2) {
        if (y1 > y2) {
            display_draw_vline(x1, y2, y1);
        }
        else {
            display_draw_vline(x1, y1, y2);
        }
    }
    else if (y1 == y2) {
        if (x1 > x2) {
            display_draw_hline(y1, x2, x1);
        }
        else {
            display_draw_hline(y1, x1, x2);
        }
    }
    else {
        return STATUS_OK;
    }

    return STATUS_OK;
}

status_t gdew0371w7_driver_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool filled,
                                          gdew0371w7_driver_color_e color) {
    CHECK_INIT(is_init);

    if (((x > DISPLAY_WIDTH) || (y > DISPLAY_HEIGHT)) || (((x + width) > DISPLAY_WIDTH))
        || ((y + height) > DISPLAY_HEIGHT)) {
        return STATUS_GDEW_DRIVER_OUT_OF_BOUNDS_ERROR;
    }

    if (filled == true && color == GDEW0371W7_COLOR_BW_BLACK) {
        display_draw_partial_bw_filled(x, y, width, height, NULL_BYTE);
    }
    else if (filled == true && color == GDEW0371W7_COLOR_BW_WHITE) {
        display_draw_partial_bw_filled(x, y, width, height, ENABLED_BYTE);
    }
    else {
        display_draw_vline(x + width, y, y + height);
        display_draw_vline(x, y, y + height);
        display_draw_hline(y, x, x + width);
        display_draw_hline(y + height, x, x + width);
    }

    return STATUS_OK;
}

status_t gdew0371w7_driver_draw_image_partial(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                            uint8_t *buffer, uint16_t size) {
    CHECK_INIT(is_init);

    if (((x > DISPLAY_WIDTH) || (y > DISPLAY_HEIGHT)) || (((x + width) > DISPLAY_WIDTH))
        || ((y + height) > DISPLAY_HEIGHT)) {
        return STATUS_GDEW_DRIVER_OUT_OF_BOUNDS_ERROR;
    }

    display_draw_partial_buffer(x, y, width, height, buffer, size);

    return STATUS_OK;
}

status_t gdew0371w7_driver_draw_text(uint8_t *buffer, uint16_t size, uint16_t x, uint16_t y, uint16_t width,
                                     uint16_t height) {
    CHECK_INIT(is_init);

    if ((x > DISPLAY_WIDTH) || (y > DISPLAY_HEIGHT)) {
        return STATUS_GDEW_DRIVER_OUT_OF_BOUNDS_ERROR;
    }

    display_draw_partial_buffer(x, y, width, height, buffer, size);
    return STATUS_OK;
}

status_t gdew0371w7_driver_start(void) {
    CHECK_INIT(is_init);

    display_powerup();

    return STATUS_OK;
}

status_t gdew0371w7_driver_update(void) {
    CHECK_INIT(is_init);

    display_refresh();
    display_sleep();

    return STATUS_OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GDEW0371W7_core.h"
#include "fonts.h"
#include "utilities.h"

#define BOOST_SOFT_START    0x06
#define POWER_ON            0x04
#define POWER_OFF           0x02
#define PANEL_SETTINGS      0x00
#define RESOLUTION_SETTINGS 0x61
#define REFRESH_DISPLAY     0x12
#define INTERVAL_SETTINGS   0x50

#define BUSY_LINE_CHECK 0x71
#define DISPLAY_MODE    0xF7

#define SET_PARTIAL_AREA    0x90
#define INIT_PARTIAL_UPDATE 0x91
#define STOP_PARTIAL_UPDATE 0x92

#define SOFT_START_DATA_A 0x17
#define SOFT_START_DATA_B 0x17
#define SOFT_START_DATA_C 0x1D

#define POWER_SETTINGS     0x01
#define POWER_SETTINGS_VGH 0x07
#define POWER_SETTINGS_VGL 0x07
#define POWER_SETTINGS_VDH 0x3f
#define POWER_SETTINGS_VDL 0x3f

#define LUT_FROM_OTP 0x2F
#define PLL_COMAND   0x30
#define PLL_DATA     0x15

#define INTERVAL_DATA_A 0x39
#define INTERVAL_DATA_B 0x07

#define TRANSFER_OLD_DATA 0x10
#define TRANSFER_NEW_DATA 0x13

#define PREPHASE           35
#define OPTIONAL_EXTENSION 0
#define CHANGE_PHASE       35
#define COLOR_EXTENSION    0

static const uint8_t lut_20_LUTC_partial[] = {
    0x00, PREPHASE, OPTIONAL_EXTENSION, CHANGE_PHASE, COLOR_EXTENSION, 1,
};

static const uint8_t lut_21_LUTWW_partial[] = {
    0x00, PREPHASE, OPTIONAL_EXTENSION, CHANGE_PHASE, COLOR_EXTENSION, 1,
};

static const uint8_t lut_22_LUTKW_partial[] = {
    0x48, PREPHASE, OPTIONAL_EXTENSION, CHANGE_PHASE, COLOR_EXTENSION, 1,
};

static const uint8_t lut_23_LUTWK_partial[] = {
    0x84, PREPHASE, OPTIONAL_EXTENSION, CHANGE_PHASE, COLOR_EXTENSION, 1,
};

static const uint8_t lut_24_LUTKK_partial[] = {
    0x00, PREPHASE, OPTIONAL_EXTENSION, CHANGE_PHASE, COLOR_EXTENSION, 1,
};

static const uint8_t lut_25_LUTBD_partial[] = {
    0x00, PREPHASE, OPTIONAL_EXTENSION, CHANGE_PHASE, COLOR_EXTENSION, 1,
};

const unsigned char lut_vcom[]
    = {0x00, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x60, 0x14, 0x14, 0x00, 0x00, 0x01, 0x00, 0x14, 0x00,
       0x00, 0x00, 0x01, 0x00, 0x13, 0x0A, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

const unsigned char lut_ww[] = {
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x10, 0x14,
    0x0A, 0x00, 0x00, 0x01, 0xA0, 0x13, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bw[] = {
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x00, 0x14,
    0x0A, 0x00, 0x00, 0x01, 0x99, 0x0C, 0x01, 0x03, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_wb[] = {
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x00, 0x14,
    0x0A, 0x00, 0x00, 0x01, 0x99, 0x0B, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bb[] = {
    0x80, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x20, 0x14,
    0x0A, 0x00, 0x00, 0x01, 0x50, 0x13, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static gfx_font *font_in_use;

void write_lut_data(const uint8_t *array, size_t size, uint16_t zeros);

void display_draw_bw_pixel(uint16_t x, uint16_t y);
void display_draw_gs_pixel(uint16_t x, uint16_t y, gdew0371w7_driver_color_e color);
void display_transfer_data(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value, uint8_t command);

void     flip_byte_array(uint8_t *array, uint8_t size, uint8_t width);
void     wait_while_busy(void);
uint8_t  get_bit_position(uint8_t exp);
uint16_t set_cursor(uint16_t x, uint16_t y);

void build_bit_array(uint8_t character, uint8_t *array, font_glyph_t properties);
void print_buffer(uint8_t *buffer, uint16_t size);

void display_reset() {
    SET_PIN_LOW(GDEW_DRIVER_RST);
    _delay_ms(10);
    SET_PIN_HIGH(GDEW_DRIVER_RST);
    _delay_ms(10);
}

void display_powerup(void) {
    display_reset();

    write_command(BOOST_SOFT_START);
    write_data(SOFT_START_DATA_A);
    write_data(SOFT_START_DATA_B);
    write_data(SOFT_START_DATA_C);

    write_command(POWER_SETTINGS);
    write_data(POWER_SETTINGS_VGH);
    write_data(POWER_SETTINGS_VGL);
    write_data(POWER_SETTINGS_VDH);
    write_data(POWER_SETTINGS_VDL);

    write_command(POWER_ON);
    wait_while_busy();
    write_command(PANEL_SETTINGS);
    write_data(LUT_FROM_OTP);
    write_command(PLL_COMAND);
    write_data(PLL_DATA);

    write_command(RESOLUTION_SETTINGS);
    write_data(DISPLAY_WIDTH);
    write_data((uint8_t)(DISPLAY_HEIGHT >> 8));
    write_data((uint8_t)(DISPLAY_HEIGHT));

    write_command(INTERVAL_SETTINGS);
    write_data(INTERVAL_DATA_A);
    write_data(INTERVAL_DATA_B);
}

void display_init_partial_update(void) {
    display_powerup();

    write_command(0x20);
    write_lut_data(lut_20_LUTC_partial, sizeof(lut_20_LUTC_partial), 42 - sizeof(lut_20_LUTC_partial));
    write_command(0x21);
    write_lut_data(lut_21_LUTWW_partial, sizeof(lut_21_LUTWW_partial), 42 - sizeof(lut_21_LUTWW_partial));
    write_command(0x22);
    write_lut_data(lut_22_LUTKW_partial, sizeof(lut_22_LUTKW_partial), 42 - sizeof(lut_22_LUTKW_partial));
    write_command(0x23);
    write_lut_data(lut_23_LUTWK_partial, sizeof(lut_23_LUTWK_partial), 42 - sizeof(lut_23_LUTWK_partial));
    write_command(0x24);
    write_lut_data(lut_24_LUTKK_partial, sizeof(lut_24_LUTKK_partial), 42 - sizeof(lut_24_LUTKK_partial));
    write_command(0x25);
    write_lut_data(lut_25_LUTBD_partial, sizeof(lut_25_LUTBD_partial), 42 - sizeof(lut_25_LUTBD_partial));
}

void display_init_partial_grayscale_update(void) {
    display_powerup();

    write_command(PANEL_SETTINGS);
    write_data(0x3f);
    write_command(INTERVAL_SETTINGS);
    write_data(INTERVAL_DATA_A);
    write_data(INTERVAL_DATA_B);

    write_command(0x20);
    write_lut_data(lut_vcom, sizeof(lut_vcom), 0);
    write_command(0x21);
    write_lut_data(lut_ww, sizeof(lut_ww), 0);
    write_command(0x22);
    write_lut_data(lut_bw, sizeof(lut_bw), 0);
    write_command(0x23);
    write_lut_data(lut_wb, sizeof(lut_wb), 0);
    write_command(0x24);
    write_lut_data(lut_bb, sizeof(lut_bb), 0);
    write_command(0x25);
    write_lut_data(lut_25_LUTBD_partial, sizeof(lut_25_LUTBD_partial), 42 - sizeof(lut_25_LUTBD_partial));
}

void display_draw_partial_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                 gdew0371w7_driver_color_e color) {
    switch (color) {
        case GDEW0371W7_COLOR_BW_BLACK:
            display_draw_partial_bw_filled(x, y, width, height, NULL_BYTE);
            break;

        case GDEW0371W7_COLOR_BW_WHITE:
            display_draw_partial_bw_filled(x, y, width, height, ENABLED_BYTE);
            break;

        case GDEW0371W7_COLOR_GS_BLACK:
            display_flush_partial_grayscale_filled(x, y, width, height, NULL_BYTE);
            display_draw_partial_grayscale_filled(x, y, width, height, NULL_BYTE);
            break;

        case GDEW0371W7_COLOR_GS_WHITE:
            display_flush_partial_grayscale_filled(x, y, width, height, ENABLED_BYTE);
            display_draw_partial_grayscale_filled(x, y, width, height, ENABLED_BYTE);
            break;

        case GDEW0371W7_COLOR_GS_GRAY_LIGHT:
            display_flush_partial_grayscale_filled(x, y, width, height, ENABLED_BYTE);
            display_draw_partial_grayscale_filled(x, y, width, height, NULL_BYTE);
            break;

        case GDEW0371W7_COLOR_GS_GRAY_DARK:
            display_flush_partial_grayscale_filled(x, y, width, height, NULL_BYTE);
            display_draw_partial_grayscale_filled(x, y, width, height, ENABLED_BYTE);
            break;

        default:
            return;
    }
}

void display_draw_partial_bw_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value) {
    display_init_partial_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, width, height);

    display_transfer_data(x, y, width, height, value, TRANSFER_NEW_DATA);
    write_command(STOP_PARTIAL_UPDATE);
}

void display_flush_partial_grayscale_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value) {
    display_init_partial_grayscale_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, width, height);

    display_transfer_data(x, y, width, height, value, TRANSFER_OLD_DATA);
    write_command(STOP_PARTIAL_UPDATE);
}

void display_draw_partial_grayscale_filled(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value) {
    display_init_partial_grayscale_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, width, height);

    display_transfer_data(x, y, width, height, value, TRANSFER_NEW_DATA);
    write_command(STOP_PARTIAL_UPDATE);
}

void display_draw_partial_buffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *buffer,
                                 uint16_t size) {
    display_init_partial_update();
    write_command(INIT_PARTIAL_UPDATE);
    if (x % 8 == 0) {
        x++;
    }

    display_set_partial_area(x, y, width, height);
    write_command(TRANSFER_NEW_DATA);
    for (uint16_t i = 0; i < size; i++) {
        write_data(buffer[i]);
    }
    write_command(STOP_PARTIAL_UPDATE);
}

void display_refresh(void) {
    write_command(REFRESH_DISPLAY);
    _delay_ms(10);
    wait_while_busy();
}

void display_sleep(void) {
    write_command(INTERVAL_SETTINGS);
    write_data(DISPLAY_MODE);

    write_command(POWER_OFF);
    write_command(0x07);
    write_data(0xA5);
}

void wait_while_busy(void) {
    uint8_t busy = 0;
    do {
        write_command(BUSY_LINE_CHECK);
        busy = gpio_get_level(GDEW_DRIVER_BUSY);
    } while (busy);
    _delay_ms(1);
}

void display_flush(void) {
    write_command(TRANSFER_OLD_DATA);
    for (uint16_t i = 0; i < ARRAY_SIZE; i++) {
        write_data(ENABLED_BYTE);
    }
}

void display_partial_flush(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    display_init_partial_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, width, height);

    display_transfer_data(x, y, width, height, ENABLED_BYTE, TRANSFER_OLD_DATA);
}

void display_draw_full_buffer(const uint8_t *buffer) {
    display_flush();

    write_command(TRANSFER_NEW_DATA);
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        write_data(buffer[i]);
    }
}

void display_draw_pixel(uint16_t x, uint16_t y, gdew0371w7_driver_color_e color) {
    if ((color == GDEW0371W7_COLOR_BW_BLACK) || (color == GDEW0371W7_COLOR_BW_WHITE)) {
        display_draw_bw_pixel(x, y);
        return;
    }

    display_draw_gs_pixel(x, y, color);
}

void display_draw_vline(uint16_t x, uint16_t y1, uint16_t y2) {
    display_draw_partial_bw_filled(x, y1, 1, y2 - y1, NULL_BYTE);
}

void display_draw_hline(uint16_t y, uint16_t x1, uint16_t x2) {
    display_draw_partial_bw_filled(x1, y, x2 - x1, 1, NULL_BYTE);
}

void display_clear(void) {
    display_flush();

    write_command(TRANSFER_NEW_DATA);
    for (uint16_t i = 0; i < ARRAY_SIZE; i++) {
        write_data(ENABLED_BYTE);
    }
}

void display_partial_clear(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    display_init_partial_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, width, height);

    write_command(TRANSFER_NEW_DATA);
    for (size_t i = 0; i < width * height; i++) {
        write_data(ENABLED_BYTE);
    }

    write_command(STOP_PARTIAL_UPDATE);
}

void display_set_font(uint8_t index) {
    switch (index) {
        case 0:
            font_in_use = &dejavu_book;
            break;
        case 1:
            font_in_use = &dejavu_bold;
            break;
        case 2:
            font_in_use = &dejavu_cond;
            break;
    }
}

void display_build_character_array(uint8_t *array, uint8_t character, bool flip_array) {
    uint8_t height = display_get_character_height(character);
    uint8_t width  = display_get_character_width(character);

    memset(array, 0, height * width);
    build_bit_array(character, array, font_in_use->glyph[character - font_in_use->first]);

    if (flip_array == true) {
        flip_byte_array(array, height * width, width);
    }
}

uint8_t display_get_character_width(uint8_t character) {
    return font_in_use->glyph[character - font_in_use->first].advance;
}

uint8_t display_get_character_height(uint8_t character) {
    return font_in_use->glyph[character - font_in_use->first].height;
}

int8_t display_get_character_yoffset(uint8_t character) {
    return font_in_use->glyph[character - font_in_use->first].yoffset;
}

void build_bit_array(uint8_t character, uint8_t *array, font_glyph_t properties) {
    uint8_t  width   = properties.width;
    uint8_t  height  = properties.height;
    uint8_t  advance = properties.advance;
    uint16_t offset  = properties.offset;

    uint8_t line        = 0;
    uint8_t bit_counter = 7;

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < advance; j++) {
            if (j < width) {
                bit_counter++;
                if (bit_counter == 8) {
                    line        = font_in_use->bitmap[offset++];
                    bit_counter = 0;
                }
                array[i * advance + j] = (line & 128) / 128;
                line <<= 1;
            }
        }
    }
}

void display_set_partial_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint16_t xe = (x + width - 1) | 0x0007;
    uint16_t ye = y + height - 1;
    x &= 0xFFF8;
    write_command(SET_PARTIAL_AREA);
    write_data(x % 256);
    write_data(xe % 256);
    write_data(y / 256);
    write_data(y % 256);
    write_data(ye / 256);
    write_data(ye % 256);
    write_data(0x01);
}

void write_data(uint8_t data) {
    SET_PIN_LOW(GDEW_DRIVER_CSN);

    spi_driver_transact(data);

    SET_PIN_HIGH(GDEW_DRIVER_CSN);
}

void write_lut_data(const uint8_t *array, size_t size, uint16_t zeros) {
    for (size_t i = 0; i < size; i++) {
        write_data(array[i]);
    }
    while (zeros--) {
        write_data(NULL_BYTE);
    }
}

void display_transfer_data(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t value, uint8_t command) {
    uint8_t border_left  = NULL_BYTE;
    uint8_t border_right = NULL_BYTE;

    if ((x % 8) != 0) {
        border_left = ~(ENABLED_BYTE >> (x % 8));
    }
    if (((x + width) % 8) != 0) {
        border_right = ~(ENABLED_BYTE << (8 - ((x + width) % 8)));
    }

    if (width < 8) {
        value = border_left | border_right;
    }

    write_command(command);

    uint16_t relative_position = (x % 8) + width;
    uint16_t limit             = (((relative_position / 8) + 1) * height);

    for (size_t i = 0; i < limit; i++) {
        if (relative_position >= 8) {
            if ((i % ((relative_position / 8) + 1)) == 0) {
                write_data(border_left);
            }
            else if ((i % ((relative_position / 8) + 1)) == (relative_position / 8)) {
                write_data(border_right);
            }
            else {
                write_data(value);
            }
        }
        else {
            write_data(value);
        }
    }
}

void display_draw_bw_pixel(uint16_t x, uint16_t y) {
    display_init_partial_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, 1, 1);

    write_command(TRANSFER_NEW_DATA);
    uint8_t data = (get_bit_position(x % 8));
    write_data(~(data));
    write_command(STOP_PARTIAL_UPDATE);
}

void display_draw_gs_pixel(uint16_t x, uint16_t y, gdew0371w7_driver_color_e color) {
    display_init_partial_grayscale_update();
    write_command(INIT_PARTIAL_UPDATE);
    display_set_partial_area(x, y, 1, 1);

    uint8_t data = (get_bit_position(x % 8));
    switch (color) {
        case GDEW0371W7_COLOR_GS_BLACK:
            write_command(TRANSFER_OLD_DATA);
            write_data(~(data));
            write_command(TRANSFER_NEW_DATA);
            write_data(~(data));
            break;

        case GDEW0371W7_COLOR_GS_WHITE:
            write_command(TRANSFER_OLD_DATA);
            write_data(data);
            write_command(TRANSFER_NEW_DATA);
            write_data(data);
            break;

        case GDEW0371W7_COLOR_GS_GRAY_LIGHT:
            write_command(TRANSFER_OLD_DATA);
            write_data(~(data));
            write_command(TRANSFER_NEW_DATA);
            write_data(data);
            break;

        case GDEW0371W7_COLOR_GS_GRAY_DARK:
            write_command(TRANSFER_OLD_DATA);
            write_data(data);
            write_command(TRANSFER_NEW_DATA);
            write_data(~(data));
            break;

        case GDEW0371W7_COLOR_BW_BLACK:
        case GDEW0371W7_COLOR_BW_WHITE:
        default:
            return;
    }

    write_command(STOP_PARTIAL_UPDATE);
}

void write_command(uint8_t command) {
    SET_PIN_LOW(GDEW_DRIVER_DC);
    SET_PIN_LOW(GDEW_DRIVER_CSN);

    spi_driver_transact(command);

    SET_PIN_HIGH(GDEW_DRIVER_CSN);
    SET_PIN_HIGH(GDEW_DRIVER_DC);
}

void flip_byte_array(uint8_t *array, uint8_t size, uint8_t width) {
    uint8_t low  = 0;
    uint8_t high = size - width;

    while (low < high) {
        for (size_t k = 0; k < width; k++) {
            uint8_t aux     = array[low + k];
            array[low + k]  = array[high + k];
            array[high + k] = aux;
        }

        low += width;
        high -= width;
    }
}

uint8_t get_bit_position(uint8_t exp) {
    return (1 << (7 - exp));
}

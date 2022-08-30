#include <stdlib.h>

#include "GDEW0371W7_driver.h"
#include "display_manager.h"
#include "log.h"
#include "storage.h"
#include "utilities.h"

#include <string.h>

#define MODULE_TAG "DISPLAY_MANAGER"

#define NEW_LINE_START_CHARACTER 0x0D
#define END_LINE_CHARACTER       0x0A

#define MAXIMUM_ROW_SIZE (uint8_t)(DISPLAY_WIDTH / 8)

static uint8_t new_data_buffer[ARRAY_SIZE];
static uint8_t image_buffer[ARRAY_SIZE];
static uint8_t display_data_old[ARRAY_SIZE];
static uint8_t display_data_new[ARRAY_SIZE];

static bool is_init = false;

void get_image_properties(uint8_t *buffer, uint8_t length, uint16_t *size, uint8_t *width, uint8_t *height,
                          uint8_t *text_offset);
void build_text_display_buffer(uint8_t *text, uint8_t length, uint8_t *buffer, uint8_t width, uint8_t height,
                               uint16_t *size, uint8_t text_offset, bool is_flipped);
void write_buffer(uint16_t size, uint16_t width, uint16_t height, uint8_t command, uint16_t x, uint16_t y);

void    write_text_to_image(storage_bmp_image_t image, uint8_t *text_buffer);
void    write_to_display_image(storage_bmp_image_t image, uint16_t x, uint16_t y, storage_image_type_e type,
                               bool is_transparent);
void    write_transparent_component(storage_image_type_e type, uint32_t cursor, uint16_t position);
void    write_opaque_component(storage_image_type_e type, uint32_t cursor, uint16_t position);
uint8_t get_display_overlayed_byte(uint8_t display_byte, uint8_t image_byte);

status_t display_manager_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    gdew0371w7_driver_init();

    is_init = true;
    return STATUS_OK;
}

status_t display_manager_write_image_to_buffer(uint8_t *name, uint16_t x, uint16_t y, bool is_transparent) {
    CHECK_INIT(is_init);
    set_use_sd_spi();
    storage_bmp_image_t image;

    CHECK_FUNCTION(storage_read_bmp_file(&image, (uint8_t *)name, image_buffer, new_data_buffer));

    set_use_display_spi();
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_OLD_DATA, is_transparent);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_NEW_DATA, is_transparent);

    LOG_INFO("image ok");
    return STATUS_OK;
}

status_t display_manager_write_text_to_buffer(uint8_t *text, uint8_t length, uint16_t x, uint16_t y, uint8_t font_size,
                                              bool is_transparent) {
    CHECK_INIT(is_init);

    uint16_t size;
    uint8_t  width;
    uint8_t  height;
    uint8_t  text_offset;

    display_set_font(font_size);
    uint16_t check = 0;
    for (size_t i = 0; i < length; i++) {
        if (check + display_get_character_width(text[i]) > 240) {
            length = i - 1;
            break;
        }

        check += display_get_character_width(text[i]);
    }

    get_image_properties(text, length, &size, &width, &height, &text_offset);
    storage_bmp_image_t image;
    set_use_display_spi();

    uint8_t image_buffer[1024];
    build_text_display_buffer(text, length, image_buffer, width, height, &size, text_offset, true);

    image.width       = width + (8 - (width % 8));
    image.height      = height;
    image.buffer_size = size;
    image.image_type  = STORAGE_IMAGE_TYPE_MONOCHROME;

    write_text_to_image(image, image_buffer);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_OLD_DATA, is_transparent);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_NEW_DATA, is_transparent);

    return STATUS_OK;
}

status_t display_manager_start_text_to_screen(void) {
    CHECK_INIT(is_init);

    set_use_display_spi();
    gdew0371w7_driver_start();

    return STATUS_OK;
}

status_t display_manager_write_text_to_screen(uint8_t *text, uint8_t length, uint16_t x, uint16_t y,
                                              uint8_t font_size) {
    CHECK_INIT(is_init);

    uint16_t size;
    uint8_t  width;
    uint8_t  height;
    uint8_t  text_offset;

    display_set_font(font_size);
    get_image_properties(text, length, &size, &width, &height, &text_offset);

    uint8_t buffer[1024];
    build_text_display_buffer(text, length, buffer, width, height, &size, text_offset, false);

    gdew0371w7_driver_draw_text(buffer, size, x, y, width, height);

    return STATUS_OK;
}

status_t display_manager_update_text_to_screen(void) {
    CHECK_INIT(is_init);

    gdew0371w7_driver_update();

    return STATUS_OK;
}

status_t display_manager_write_horizontal_separator(uint16_t x, uint16_t y, uint16_t width,
                                                    gdew0371w7_driver_color_e color) {
    CHECK_INIT(is_init);

    storage_bmp_image_t image;

    image.height      = 1;
    image.width       = width;
    image.buffer_size = image.width / 8;
    image.image_type  = STORAGE_IMAGE_TYPE_MONOCHROME;

    uint8_t image_buffer[image.buffer_size];
    for (size_t i = 0; i < image.buffer_size; i++) {
        image_buffer[i] = NULL_BYTE;
    }

    write_text_to_image(image, image_buffer);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_OLD_DATA, true);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_NEW_DATA, true);

    return STATUS_OK;
}

status_t display_manager_write_rectangle_filled(uint16_t x, uint16_t y, uint16_t width, uint8_t height,
                                                gdew0371w7_driver_color_e color) {
    CHECK_INIT(is_init);

    storage_bmp_image_t image;

    image.height      = height;
    image.width       = width;
    image.buffer_size = image.width / 8 * height;
    image.image_type  = STORAGE_IMAGE_TYPE_MONOCHROME;

    uint8_t image_buffer[image.buffer_size];
    for (size_t i = 0; i < image.buffer_size; i++) {
        image_buffer[i] = NULL_BYTE;
    }

    write_text_to_image(image, image_buffer);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_OLD_DATA, true);
    write_to_display_image(image, x, y, STORAGE_IMAGE_TYPE_NEW_DATA, true);

    return STATUS_OK;
}

status_t display_manager_render_full_display(storage_image_type_e type) {
    CHECK_INIT(is_init);

    set_use_display_spi();

    gdew0371w7_driver_start();
    if (type == STORAGE_IMAGE_TYPE_GRAYSCALE) {
        display_init_partial_grayscale_update();
        write_buffer(ARRAY_SIZE, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0x10, 0, 0);
        write_buffer(ARRAY_SIZE, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0x13, 0, 0);
    }
    else {
        display_init_partial_update();
        write_buffer(ARRAY_SIZE, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0x13, 0, 0);
    }

    write_command(0x92);
    gdew0371w7_driver_update();

    LOG_INFO("done");
    return STATUS_OK;
}

void write_text_to_image(storage_bmp_image_t image, uint8_t *text_buffer) {
    uint16_t size = image.buffer_size;

    for (size_t i = 0; i < size; i++) {
        image_buffer[i] = text_buffer[i];
    }
    for (size_t i = 0; i < size; i++) {
        new_data_buffer[i] = text_buffer[i];
    }
}

void write_to_display_image(storage_bmp_image_t image, uint16_t x, uint16_t y, storage_image_type_e type,
                            bool is_transparent) {
    uint16_t size   = image.buffer_size;
    uint16_t width  = image.width;
    uint16_t height = image.height;

    uint32_t cursor       = ((((uint32_t)DISPLAY_HEIGHT - y - height) * DISPLAY_WIDTH + x) / 8);
    uint8_t  row_size     = width / 8;
    uint8_t  row_iterator = 0;

    for (size_t i = 0; i < size; i++) {
        if (is_transparent == true) {
            write_transparent_component(type, cursor + row_iterator, (uint16_t)i);
        }
        else {
            write_opaque_component(type, cursor + row_iterator, (uint16_t)i);
        }

        row_iterator++;
        if (row_iterator == row_size) {
            cursor += MAXIMUM_ROW_SIZE;
            row_iterator = 0;
        }
    }
}

void write_transparent_component(storage_image_type_e type, uint32_t cursor, uint16_t position) {
    if (type == STORAGE_IMAGE_TYPE_NEW_DATA) {
        display_data_old[cursor] = get_display_overlayed_byte(display_data_old[cursor], image_buffer[position]);
    }
    else {
        display_data_new[cursor] = get_display_overlayed_byte(display_data_new[cursor], new_data_buffer[position]);
    }
}

void write_opaque_component(storage_image_type_e type, uint32_t cursor, uint16_t position) {
    if (type == STORAGE_IMAGE_TYPE_OLD_DATA) {
        display_data_old[cursor] = image_buffer[position];
    }
    else {
        display_data_new[cursor] = new_data_buffer[position];
    }
}

void write_buffer(uint16_t size, uint16_t width, uint16_t height, uint8_t command, uint16_t x, uint16_t y) {
    uint16_t row_iterator = 0;
    uint16_t row_size     = width / 8;

    for (size_t i = 0; i <= height; i++) {
        for (size_t j = 0; j < size; j++) {
            if (row_iterator == 0) {
                write_command(0x92);
                write_command(0x91);
                display_set_partial_area(x, y + height - 1, width, 1);
                height--;
                write_command(command);
            }

            if (width % 8 == 0) {
                if (row_iterator < row_size) {
                    if (command == 0x10) {
                        write_data(display_data_old[j]);
                    }
                    else {
                        write_data(display_data_new[j]);
                    }
                    row_iterator++;
                }
                if (row_iterator == row_size) {
                    row_iterator = 0;
                }
            }
            else {
                if (row_iterator < row_size) {
                    if (command == 0x10) {
                        write_data(display_data_old[j]);
                    }
                    else {
                        write_data(display_data_new[j]);
                    }
                    row_iterator++;
                }
                else {
                    row_iterator = 0;
                }
            }
        }
    }
}

status_t display_manager_clear_display_buffers(void) {
    CHECK_INIT(is_init);

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        display_data_old[i] = ENABLED_BYTE;
    }

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        display_data_new[i] = ENABLED_BYTE;
    }

    return STATUS_OK;
}

status_t display_manager_write_centered_message(uint8_t *buffer, uint8_t size, uint16_t y, uint8_t font_size,
                                                bool is_transparent) {
    uint8_t width = 0;
    display_set_font(font_size);

    for (size_t i = 0; i < size; i++) {
        width += display_get_character_width(buffer[i]);
    }

    display_manager_write_text_to_buffer(buffer, size, (DISPLAY_WIDTH - width) / 2, y, font_size, is_transparent);
    return STATUS_OK;
}

status_t display_manager_render_error_message(uint8_t *buffer, uint8_t size) {
    display_manager_clear_display_buffers();

    display_manager_write_centered_message(buffer, size, 180, 1, false);
    display_manager_write_centered_message((uint8_t *)"Please reset the device", 23, 215, 1, true);

    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    while (1) {
        _delay_ms(10);
    }

    return STATUS_OK;
}

void get_image_properties(uint8_t *buffer, uint8_t length, uint16_t *size, uint8_t *width, uint8_t *height,
                          uint8_t *text_offset) {
    uint16_t byte_count = 0;
    uint8_t  max_height = 0;
    uint8_t  max_offset = 0;

    for (size_t i = 0; i < length; i++) {
        byte_count += display_get_character_width(buffer[i]);
        if (display_get_character_height(buffer[i]) > max_height) {
            max_height = display_get_character_height(buffer[i]);
        }

        if ((display_get_character_height(buffer[i]) + display_get_character_yoffset(buffer[i])) > max_offset) {
            max_offset = (display_get_character_height(buffer[i]) + display_get_character_yoffset(buffer[i]));
        }
    }

    while ((byte_count % 8) != 0) {
        byte_count++;
    }

    (*width)       = byte_count;
    (*height)      = max_height + max_offset;
    (*text_offset) = max_offset;
    (*size)        = (byte_count / 8) * (*height + 1);
}

void build_text_display_buffer(uint8_t *text, uint8_t length, uint8_t *buffer, uint8_t width, uint8_t height,
                               uint16_t *size, uint8_t text_offset, bool is_flipped) {

    uint8_t  bit_counter  = 7;
    uint16_t byte_counter = 0;
    uint16_t offset       = 0;
    uint8_t  matrix[height][width];

    memset(buffer, 0, 1024);
    memset(matrix, 0, width * height);

    for (size_t k = 0; k < length; k++) {
        uint8_t character[display_get_character_height(text[k]) * display_get_character_width(text[k])];
        uint8_t char_height = display_get_character_height(text[k]);
        uint8_t advance     = display_get_character_width(text[k]);
        int8_t  yoffset     = display_get_character_yoffset(text[k]);

        display_build_character_array(character, text[k], is_flipped);

        uint16_t byte_counter = 0;
        if (is_flipped == true) {
            for (int16_t i = text_offset - (char_height + yoffset);
                 i < text_offset + char_height - (char_height + yoffset); i++) {
                for (int16_t j = offset; j < offset + advance; j++) {
                    matrix[i][j] = character[byte_counter];
                    byte_counter++;
                }
            }
            offset += advance;
        }
        else {
            for (int16_t i = text_offset + char_height - (char_height + yoffset) - 1;
                 i >= text_offset - (char_height + yoffset); i--) {
                for (int16_t j = offset; j < offset + advance; j++) {
                    matrix[i][j] = character[byte_counter];
                    byte_counter++;
                }
            }
            offset += advance;
        }
    }

    if (is_flipped == false) {
        for (size_t i = 0; i < height / 2; i++) {
            for (size_t j = 0; j < width; j++) {
                uint8_t aux               = matrix[i][j];
                matrix[i][j]              = matrix[height - i - 1][j];
                matrix[height - i - 1][j] = aux;
            }
        }
    }

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            buffer[byte_counter] |= (matrix[i][j] << bit_counter);
            bit_counter--;
            if (bit_counter == 0) {
                buffer[byte_counter++] |= (matrix[i][++j] << bit_counter);
                bit_counter = 7;
            }
        }
        byte_counter++;
        bit_counter = 7;
    }

    for (size_t i = 0; i < byte_counter; i++) {
        buffer[i] = (~buffer[i]);
    }

    (*size) = byte_counter;
}

uint8_t get_display_overlayed_byte(uint8_t display_byte, uint8_t image_byte) {
    return (display_byte & image_byte);
}

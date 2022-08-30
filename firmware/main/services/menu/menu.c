#include "menu.h"
#include "GDEW0371W7_driver.h"
#include "controls.h"
#include "display_manager.h"
#include "log.h"
#include "spi_driver.h"
#include "utilities.h"

#include <string.h>

#define MODULE_TAG "MENU"

#define BUTTON_HIGHLIGHT_WIDTH 16 /** size of the selected button highlight */

void entry_render(menu_entry_t button);
void entry_render_selected(menu_entry_t button);
void entry_reset(menu_entry_t button);
void entry_set_coordinates(menu_entry_t *button, uint16_t x, uint16_t y);

void menu_render(menu_t *menu);
void menu_redraw(menu_t *menu);
void menu_up(menu_t *menu);
void menu_down(menu_t *menu);
void menu_select(menu_entry_t button);
void menu_update_page_down(menu_t *menu);
void menu_update_page_up(menu_t *menu);
void menu_update_cursor(menu_t *menu, uint8_t position);

void render_down_arrow(bool is_visible);
void render_up_arrow(bool is_visible);

uint16_t get_button_text_offset(uint8_t *buffer, uint8_t font_size);

static bool    should_redraw_menu       = false;
static bool    should_render_down_arrow = false;
static bool    should_render_up_arrow   = false;
static uint8_t menu_entries_count       = 0;
static uint8_t menu_size                = 0;

void menu_init(menu_t *menu) {
    menu_entries_count = menu->entry_count;
    menu_size          = menu->size;
    menu->cursor       = 0;

    gdew0371w7_driver_start();
    gdew0371w7_driver_clear();
    gdew0371w7_driver_update();

    gdew0371w7_driver_start();

    gdew0371w7_driver_draw_line(10, 10, 0, 415);
    gdew0371w7_driver_draw_line(0, 50, 405, 405);
    gdew0371w7_driver_draw_line(0, 50, 10, 10);

    menu_render(menu);
    gdew0371w7_driver_update();
}

void menu_loop(menu_t *menu) {
    controls_button_e action = 0;

    while (1) {
        if (should_redraw_menu == true) {
            gdew0371w7_driver_start();
            menu_redraw(menu);
            gdew0371w7_driver_update();
            should_redraw_menu = false;
        }

        check_for_new_action(&action, true);
        switch (action) {
            case CONTROLS_BUTTON_1:
                menu_up(menu);
                break;
            case CONTROLS_BUTTON_3:
                menu_down(menu);
                break;
            case CONTROLS_BUTTON_2:
                menu_select(menu->entries[menu->cursor]);
                break;
            default:
                break;
        }
        _delay_ms(10);
    }
}

void entry_render(menu_entry_t button) {
    uint16_t text_offset = get_button_text_offset(button.text, 2);

    gdew0371w7_driver_draw_rectangle(button.x, button.y, BUTTON_WIDTH, BUTTON_HEIGHT, false, GDEW0371W7_COLOR_BW_BLACK);
    display_manager_write_text_to_screen(button.text, strlen((char *)button.text), button.x + text_offset,
                                         button.y + 10, 2);
}

void entry_render_selected(menu_entry_t button) {
    gdew0371w7_driver_draw_rectangle(button.x, button.y, BUTTON_HIGHLIGHT_WIDTH, BUTTON_HEIGHT, true,
                                     GDEW0371W7_COLOR_BW_BLACK);
}

void entry_reset(menu_entry_t button) {
    uint16_t text_offset = get_button_text_offset(button.text, 2);

    display_partial_clear(button.x, button.y, BUTTON_WIDTH, BUTTON_HEIGHT);
    gdew0371w7_driver_draw_rectangle(button.x, button.y, BUTTON_WIDTH, BUTTON_HEIGHT, false, GDEW0371W7_COLOR_BW_BLACK);
    // gdew0371w7_driver_draw_text(button.x + text_offset, button.y + 12, button.text, strlen((char *)button.text) + 1,
    //                             GDEW0371W7_COLOR_BW_BLACK);
    display_manager_write_text_to_screen(button.text, strlen((char *)button.text), button.x + text_offset,
                                         button.y + 10, 2);
}

void entry_set_coordinates(menu_entry_t *button, uint16_t x, uint16_t y) {
    button->x = x;
    button->y = y;
}

void menu_render(menu_t *menu) {
    menu->lower_menu_bound = menu->cursor - 1;
    menu->upper_menu_bound = menu->cursor + menu_entries_count;

    for (size_t i = 0; i < menu_entries_count; i++) {
        entry_set_coordinates(&menu->entries[menu->cursor + i], menu->menu_x, menu->menu_y + (MENU_BUTTON_OFFSET * i));
        entry_render(menu->entries[menu->cursor + i]);
    }

    if (menu->size > menu_entries_count) {
        render_down_arrow(true);
    }

    entry_render_selected(menu->entries[0]);
}

void menu_redraw(menu_t *menu) {
    gdew0371w7_driver_clear();
    gdew0371w7_driver_update();

    gdew0371w7_driver_start();
    gdew0371w7_driver_draw_line(10, 10, 0, 415);
    gdew0371w7_driver_draw_line(0, 50, 405, 405);
    gdew0371w7_driver_draw_line(0, 50, 10, 10);

    for (size_t i = menu->lower_menu_bound + 1; i < menu->upper_menu_bound; i++) {
        entry_render(menu->entries[i]);
    }

    entry_render_selected(menu->entries[menu->cursor]);
    if (should_render_up_arrow == true) {
        render_up_arrow(true);
    }

    if (should_render_down_arrow == true) {
        render_down_arrow(true);
    }
}

void menu_up(menu_t *menu) {
    if (menu->cursor == 0) {
        return;
    }

    gdew0371w7_driver_start();
    menu_update_cursor(menu, menu->cursor - 1);
    if (menu->cursor == menu->lower_menu_bound) {
        menu_update_page_up(menu);
        entry_render_selected(menu->entries[menu->cursor]);
    }
    else {
        entry_reset(menu->entries[menu->cursor + 1]);
        entry_render_selected(menu->entries[menu->cursor]);
    }

    if (menu->cursor == 0) {
        render_up_arrow(false);
    }

    gdew0371w7_driver_update();
    _delay_ms(750);
}

void menu_down(menu_t *menu) {
    if (menu->cursor == menu->size - 1) {
        return;
    }

    gdew0371w7_driver_start();
    menu_update_cursor(menu, menu->cursor + 1);
    if (menu->cursor == menu->upper_menu_bound) {
        menu_update_page_down(menu);
        entry_render_selected(menu->entries[menu->cursor]);
    }
    else {
        entry_reset(menu->entries[menu->cursor - 1]);
        entry_render_selected(menu->entries[menu->cursor]);
    }

    if (menu->cursor == menu->size - 1) {
        render_down_arrow(false);
    }

    gdew0371w7_driver_update();
    _delay_ms(750);
}

void menu_select(menu_entry_t button) {
    if (button.action == NULL) {
        LOG_WARNING("no action bound to this entry");
        return;
    }
    else {
        button.action();
    }
}

void menu_update_page_down(menu_t *menu) {
    menu->lower_menu_bound++;
    menu->upper_menu_bound++;

    for (size_t i = 0; i < menu_entries_count; i++) {
        entry_set_coordinates(&menu->entries[menu->cursor - i], menu->menu_x,
                              menu->menu_y + (MENU_BUTTON_OFFSET * (menu_entries_count - (i + 1))));
        entry_reset(menu->entries[menu->cursor - i]);
        entry_render(menu->entries[menu->cursor - i]);
    }

    render_up_arrow(true);
}

void menu_update_page_up(menu_t *menu) {
    menu->lower_menu_bound--;
    menu->upper_menu_bound--;

    for (size_t i = 0; i < menu_entries_count; i++) {
        entry_set_coordinates(&menu->entries[menu->cursor + i], menu->menu_x, menu->menu_y + (MENU_BUTTON_OFFSET * i));
        entry_reset(menu->entries[menu->cursor + i]);
        entry_render(menu->entries[menu->cursor + i]);
    }

    render_down_arrow(true);
}

void menu_update_cursor(menu_t *menu, uint8_t position) {
    menu->cursor = position;
}

void render_down_arrow(bool is_visible) {
    if (menu_entries_count == menu_size) {
        return;
    }

    uint8_t  base_width = 10;
    uint16_t x          = 105;
    uint16_t y          = MENU_Y_COORD + ((menu_entries_count - 1) * MENU_BUTTON_OFFSET) + BUTTON_HEIGHT + 10;

    if (is_visible) {
        for (uint8_t i = 0; i < base_width; i++) {
            /** draws lines on the center of the screen */
            display_draw_hline(y + i, ((DISPLAY_WIDTH / 2) - base_width) + i,
                               ((DISPLAY_WIDTH / 2) + base_width + 1) - i);
        }
        should_render_down_arrow = true;
    }
    else {
        display_partial_clear(x, y, 3 * base_width, base_width);
        should_render_down_arrow = false;
    }
}

void render_up_arrow(bool is_visible) {
    if (menu_entries_count == menu_size) {
        return;
    }

    uint8_t  base_width = 10;
    uint16_t x          = 105;
    uint16_t y          = MENU_Y_COORD - 10;

    if (is_visible) {
        for (uint8_t i = base_width; i > 0; i--) {
            /** draws lines on the center of the screen */
            display_draw_hline(y - i, ((DISPLAY_WIDTH / 2) - base_width) + i,
                               ((DISPLAY_WIDTH / 2) + base_width + 1) - i);
        }
        should_render_up_arrow = true;
    }
    else {
        display_partial_clear(x, y - base_width, 3 * base_width, base_width);
        should_render_up_arrow = false;
    }
}

uint16_t get_button_text_offset(uint8_t *buffer, uint8_t font_size) {
    uint8_t width = 0;
    display_set_font(font_size);

    for (size_t i = 0; i < strlen((char *)buffer); i++) {
        width += display_get_character_width(buffer[i]);
    }

    return ((BUTTON_WIDTH - width) / 2) + 8;
}

void menu_set_should_redraw_menu(bool should_redraw) {
    should_redraw_menu = should_redraw;
}
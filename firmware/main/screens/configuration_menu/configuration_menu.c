#include "configuration_menu.h"
#include "badge_defines.h"
#include "configuration_manager.h"
#include "controls.h"
#include "display_manager.h"
#include "log.h"
#include "utilities.h"

#include <string.h>

#define MODULE_TAG "CONFIGURATION_MENU"

#define PROPERTY_NAME             (uint8_t *)"First Name: "
#define PROPERTY_SURNAME          (uint8_t *)"Last Name: "
#define PROPERTY_FUNCTION         (uint8_t *)"Job Title: "
#define PROPERTY_ISSUE_DATE       (uint8_t *)"Issued: "
#define PROPERTY_CITY             (uint8_t *)"City: "
#define PROPERTY_AREA             (uint8_t *)"State: "
#define PROPERTY_DWM_CHANNEL      (uint8_t *)"Channel: "
#define PROPERTY_DWM_PREAMBLE_LEN (uint8_t *)"Preamble: "
#define PROPERTY_DWM_SFD_TYPE     (uint8_t *)"SFD: "
#define PROPERTY_DATA_RATE        (uint8_t *)"Data rate: "
#define PROPERTY_WIFI_SSID        (uint8_t *)"SSID: "
#define PROPERTY_WIFI_PASS        (uint8_t *)"Pass: "
#define PROPERTY_DEVICE_ID        (uint8_t *)"Device ID: "

static uint8_t *property_names[]
    = {PROPERTY_NAME,      PROPERTY_SURNAME,     PROPERTY_FUNCTION,  PROPERTY_ISSUE_DATE,       PROPERTY_CITY,
       PROPERTY_AREA,      PROPERTY_DWM_CHANNEL, PROPERTY_DATA_RATE, PROPERTY_DWM_PREAMBLE_LEN, PROPERTY_DWM_SFD_TYPE,
       PROPERTY_WIFI_SSID, PROPERTY_WIFI_PASS,   PROPERTY_DEVICE_ID};
static configuration_record_t configurations[CONFIGURATION_MENU_PROPERTIES_COUNT];

static configuration_menu_pages_e current_page = CONFIGURATION_MENU_PAGE_MAIN;

void display_nfc_configurations(void);
void update_configurations(void);
void dev_id_string_to_buffer(uint8_t *buffer);

void render_configuration_screen(void);
void render_completion_screen(void);
void render_button(uint16_t x, uint16_t y, uint8_t *text, uint8_t length, uint8_t width);

void display_sd_card_configurations(void);

void configuration_menu_loop(void) {
    render_configuration_screen();

    controls_button_e action = 0;

    while (1) {
        check_for_new_action(&action, true);

        switch (action) {
            case CONTROLS_BUTTON_2:
                if (current_page == CONFIGURATION_MENU_PAGE_NFC) {
                    update_configurations();
                    render_completion_screen();
                }
                else if (current_page == CONFIGURATION_MENU_PAGE_MAIN) {
                    display_sd_card_configurations();
                }
                else if (current_page == CONFIGURATION_MENU_PAGE_SD_CARD) {
                    configuration_manager_load_full_configuration();
                    render_completion_screen();
                }
                break;
            case CONTROLS_BUTTON_3:
                if (current_page == CONFIGURATION_MENU_PAGE_MAIN || current_page == CONFIGURATION_MENU_PAGE_NFC) {
                    display_nfc_configurations();
                }
                else if (current_page == CONFIGURATION_MENU_PAGE_SD_CARD) {
                    render_configuration_screen();
                }
                break;
            case CONTROLS_BUTTON_1:
            default:
                break;
        }
        _delay_ms(10);
    }
}

void display_nfc_configurations(void) {
    display_manager_clear_display_buffers();

    display_manager_write_image_to_buffer((uint8_t *)"/background.bmp", 0, 0, true);

    render_button(200, 200, (uint8_t *)"REDO", 4, 35);
    render_button(200, 260, (uint8_t *)"EXIT", 4, 35);

    if (configuration_manager_read_nfc_memory(configurations) == STATUS_CONFIG_MANAGER_INVALID_ENTRY_COUNT) {
        LOG_WARNING("could not retrieve data");
        display_manager_write_text_to_buffer((uint8_t *)"invalid entry count", 19, 60, 310, 1, true);
    }
    else {
        render_button(200, 130, (uint8_t *)"SAVE", 4, 35);

        for (size_t i = 0; i < CONFIGURATION_MENU_PROPERTIES_COUNT - 1; i++) {
            display_manager_write_text_to_buffer(property_names[i], strlen((char *)property_names[i]), 10,
                                                 80 + (i * 15), 1, true);
            display_manager_write_text_to_buffer(configurations[i].text, configurations[i].length, 110, 80 + (i * 15),
                                                 0, true);
        }

        display_manager_write_text_to_buffer(property_names[SD_PROPERTY_INDEX_DEVICE_ID],
                                             strlen((char *)property_names[SD_PROPERTY_INDEX_DEVICE_ID]), 10, 370, 1,
                                             true);

        display_manager_write_text_to_buffer(configurations[SD_PROPERTY_INDEX_DEVICE_ID].text, 16, 110, 370, 0, 1);
        display_manager_write_text_to_buffer(configurations[SD_PROPERTY_INDEX_DEVICE_ID].text + 16, 16, 110, 385, 0, 1);
    }
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    current_page = CONFIGURATION_MENU_PAGE_NFC;
}

void display_sd_card_configurations(void) {
    display_manager_clear_display_buffers();
    if (configuration_manager_read_user_data(configurations) != 0) {
        LOG_WARNING("could not retrieve data");
        return;
    }

    display_manager_write_image_to_buffer((uint8_t *)"/background.bmp", 0, 0, true);

    render_button(200, 130, (uint8_t *)"SAVE", 4, 35);
    render_button(200, 200, (uint8_t *)"BACK", 4, 35);
    render_button(200, 260, (uint8_t *)"EXIT", 4, 35);

    for (size_t i = 0; i < CONFIGURATION_MENU_PROPERTIES_COUNT - 1; i++) {
        display_manager_write_text_to_buffer(property_names[i], strlen((char *)property_names[i]), 10, 80 + (i * 15), 1,
                                             true);
        display_manager_write_text_to_buffer(configurations[i].text, configurations[i].length, 110, 80 + (i * 15), 0,
                                             true);
    }

    display_manager_write_text_to_buffer(property_names[NFC_PROPERTY_INDEX_DEVICE_ID],
                                         strlen((char *)property_names[NFC_PROPERTY_INDEX_DEVICE_ID]), 10, 370, 1,
                                         true);
    display_manager_write_text_to_buffer(configurations[NFC_PROPERTY_INDEX_DEVICE_ID].text, 16, 110, 370, 0, 1);
    display_manager_write_text_to_buffer(configurations[NFC_PROPERTY_INDEX_DEVICE_ID].text + 16, 16, 110, 385, 0, 1);

    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    current_page = CONFIGURATION_MENU_PAGE_SD_CARD;
}

void update_configurations(void) {
    if (configuration_manager_read_nfc_memory(configurations) != 0) {
        LOG_WARNING("could not save data");
        return;
    }

    configuration_manager_update_profile_data(configurations);
    configuration_manager_update_weather_data_nfc(configurations);
    configuration_manager_update_configuration_data_nfc(configurations);

    uint8_t buffer[16];
    dev_id_string_to_buffer(buffer);
    configuration_manager_update_device_id(buffer);
}

void render_button(uint16_t x, uint16_t y, uint8_t *text, uint8_t length, uint8_t width) {
    display_manager_write_horizontal_separator(x, y - 5, width, GDEW0371W7_COLOR_BW_BLACK);
    display_manager_write_text_to_buffer(text, length, x, y, 1, true);
    display_manager_write_horizontal_separator(x, y + 15, width, GDEW0371W7_COLOR_BW_BLACK);
}

void render_completion_screen(void) {
    display_manager_clear_display_buffers();

    display_manager_write_image_to_buffer((uint8_t *)"/background.bmp", 0, 0, true);
    display_manager_write_text_to_buffer((uint8_t *)"DONE", 4, 100, 130, 2, true);
    render_button(200, 260, (uint8_t *)"EXIT", 4, 35);

    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);
    current_page = CONFIGURATION_MENU_PAGE_FINISH;
}

void render_configuration_screen(void) {
    display_manager_clear_display_buffers();

    display_manager_write_image_to_buffer((uint8_t *)"/background.bmp", 0, 0, true);
    display_manager_write_text_to_buffer((uint8_t *)"CONFIGURATION", 13, 10, 100, 2, true);
    display_manager_write_text_to_buffer((uint8_t *)"MENU", 4, 10, 130, 2, true);

    display_manager_write_text_to_buffer((uint8_t *)"Save data to the NFC memory.", 28, 10, 330, 0, true);
    display_manager_write_text_to_buffer((uint8_t *)"Please upload data below.", 25, 10, 345, 0, true);
    display_manager_write_text_to_buffer((uint8_t *)"After scanning, press 'LOAD FROM NFC'.", 38, 10, 360, 0, true);

    display_manager_write_text_to_buffer((uint8_t *)"<-NFC", 5, 60, 385, 1, true);
    display_manager_write_rectangle_filled(30, 380, 24, 24, GDEW0371W7_COLOR_BW_BLACK);

    render_button(128, 130, (uint8_t *)"LOAD FROM SD", 12, 110);
    render_button(125, 200, (uint8_t *)"LOAD FROM NFC", 13, 120);
    render_button(200, 260, (uint8_t *)"EXIT", 4, 35);

    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    current_page = CONFIGURATION_MENU_PAGE_MAIN;
}

void dev_id_string_to_buffer(uint8_t *buffer) {
    char aux_buffer[2];

    if (configurations[NFC_PROPERTY_INDEX_DEVICE_ID].text[0] == '0'
        && configurations[NFC_PROPERTY_INDEX_DEVICE_ID].length == 1) {
        for (size_t i = 0; i < 16; i++) {
            buffer[i] = strtol("0", NULL, 16);
        }
        return;
    }

    for (size_t i = 0; i < 16; i++) {
        memcpy(aux_buffer, &configurations[NFC_PROPERTY_INDEX_DEVICE_ID].text[i * 2], 2);
        buffer[i] = strtol(aux_buffer, NULL, 16);
    }
}
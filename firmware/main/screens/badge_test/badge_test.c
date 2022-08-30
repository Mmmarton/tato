#include "badge_test.h"
#include "adxl345_driver.h"
#include "bme68x_driver.h"
#include "configuration_manager.h"
#include "controls.h"
#include "display_manager.h"
#include "log.h"
#include "m24lr_driver.h"
#include "max_driver.h"
#include "menu.h"
#include "neopixel_driver.h"
#include "pwm_driver.h"
#include "spi_driver.h"
#include "storage.h"
#include "utilities.h"

#include <stdint.h>
#include <string.h>

#define MODULE_TAG "BADGE_TEST"

#define TEMPERATURE_TAG    ((uint8_t *)"TEMPERATURE")
#define PRESSURE_TAG       ((uint8_t *)"PRESSURE")
#define HUMIDITY_TAG       ((uint8_t *)"HUMIDITY")
#define GAS_RESISTANCE_TAG ((uint8_t *)"GAS RESISTANCE")

#define BATTERY_VOLTAGE_TAG ((uint8_t *)"BATTERY VOLTAGE")
#define STATE_OF_CHARGE_TAG ((uint8_t *)"STATE OF CHARGE")

#define X_AXIS_TAG ((uint8_t *)"X-AXIS DATA")
#define Y_AXIS_TAG ((uint8_t *)"Y-AXIS DATA")
#define Z_AXIS_TAG ((uint8_t *)"Z-AXIS DATA")

#define NFC_DATA_TAG ((uint8_t *)"NFC MEMORY DATA")

#define CLEAR_MENU_AREA (display_partial_clear(0, 0, 240, 416)) /** clears menu area */

void auto_test(void);
void test_buzzer(void);
void test_haptics(void);
void test_neopixels(void);
void test_bme_sensor(void);
void test_fuel_gauge(void);
void test_serial(void);
void test_microsd(void);
void test_nfc(void);
void test_accelerometer(void);
void factory_reset(void);

void display_nfc_data(uint8_t *ndef_string, uint8_t length);
void write_entry(uint8_t *buffer, uint16_t y, uint8_t font_size);

static const rgb_color_t led_red = {
    .blue  = 0,
    .green = 0,
    .red   = 255,
};

static const rgb_color_t led_blue = {
    .blue  = 255,
    .green = 0,
    .red   = 0,
};

static const rgb_color_t led_green = {
    .blue  = 0,
    .green = 255,
    .red   = 0,
};

static const rgb_color_t closed_led = {
    .blue  = 0,
    .green = 0,
    .red   = 0,
};

static rgb_color_t red_array[]    = {led_red, led_red, led_red, led_red};
static rgb_color_t green_array[]  = {led_green, led_green, led_green, led_green};
static rgb_color_t blue_array[]   = {led_blue, led_blue, led_blue, led_blue};
static rgb_color_t closed_array[] = {closed_led, closed_led, closed_led, closed_led};

static controls_button_e action = 0;

status_t badge_test_init(void) {
    /** enable LD0 */
    SET_OUTPUT_PIN(5);
    SET_PIN_HIGH(5);

    CHECK_FUNCTION(gdew0371w7_driver_init());
    CHECK_FUNCTION(neopixel_driver_init());
    CHECK_FUNCTION(max_driver_init());
    CHECK_FUNCTION(m24lr_driver_init());

    set_use_display_spi();
    CHECK_FUNCTION(display_manager_init());
    CHECK_FUNCTION(controls_driver_init());

    bme_configuration_t sensor_configuration = BME_DEFAULT_CONFIGURATION();
    CHECK_FUNCTION(bme_driver_init(&sensor_configuration));

    adxl345_driver_configuration_t adxl_configuration = ADXL345_DEFAULT_CONFIGURATION();
    CHECK_FUNCTION(adxl345_driver_init(&adxl_configuration));

    bme_reading_t reading;
    bme_driver_read_sensor_data(&reading);

    return STATUS_OK;
}

void badge_test_loop(void) {
    menu_entry_t auto_test_btn = {
        .text   = (uint8_t *)"AUTO TEST",
        .action = auto_test,
    };

    menu_entry_t haptics_btn = {
        .text   = (uint8_t *)"HAPTICS",
        .action = test_haptics,
    };

    menu_entry_t buzzer_btn = {
        .text   = (uint8_t *)"BUZZER",
        .action = test_buzzer,
    };

    menu_entry_t neopixels_btn = {
        .text   = (uint8_t *)"NEOPIXEL",
        .action = test_neopixels,
    };

    menu_entry_t sensor_btn = {
        .text   = (uint8_t *)"BME688",
        .action = test_bme_sensor,
    };

    menu_entry_t fuel_gauge_btn = {
        .text   = (uint8_t *)"FUEL GAUGE",
        .action = test_fuel_gauge,
    };

    menu_entry_t serial_btn = {
        .text   = (uint8_t *)"SERIAL",
        .action = test_serial,
    };

    menu_entry_t microsd_btn = {
        .text   = (uint8_t *)"MICRO SD",
        .action = test_microsd,
    };

    menu_entry_t accelerometer_btn = {
        .text   = (uint8_t *)"ADXL345",
        .action = test_accelerometer,
    };

    menu_entry_t nfc_btn = {
        .text   = (uint8_t *)"NFC",
        .action = test_nfc,
    };

    menu_entry_t factory_reset_btn = {
        .text   = (uint8_t *)"FULL RESET",
        .action = factory_reset,
    };

    menu_entry_t entries[]
        = {auto_test_btn, haptics_btn, buzzer_btn,        neopixels_btn, sensor_btn,       fuel_gauge_btn,
           serial_btn,    microsd_btn, accelerometer_btn, nfc_btn,       factory_reset_btn};

    menu_t menu;
    for (size_t i = 0; i < 11; i++) {
        menu.entries[i] = entries[i];
    }

    menu.menu_x      = MENU_X_COORD;
    menu.menu_y      = MENU_Y_COORD;
    menu.size        = 11;
    menu.entry_count = 7;

    menu_init(&menu);
    menu_loop(&menu);
}

void auto_test(void) {
    test_haptics();
    test_buzzer();
    test_neopixels();
    test_bme_sensor();
    test_fuel_gauge();
    test_serial();
    test_accelerometer();
    test_nfc();
}

void test_buzzer(void) {
    _delay_ms(10);

    pwm_channel_e pwm_channel = PWM_DRIVER_INVALID_CHANNEL;
    /** known issue regarding the frequency. going over this value might result in the buzzer not working the first time
     * it's running */
    pwm_driver_init(&pwm_channel, BUZZER_PIN, 2150);
    if (pwm_channel == PWM_DRIVER_INVALID_CHANNEL) {
        return;
    }

    pwm_driver_set_duty_cycle(pwm_channel, 50);

    while (1) {
        check_for_new_action(&action, true);
        if (action == CONTROLS_BUTTON_2) {
            break;
        }
        _delay_ms(10);
    }

    pwm_driver_set_duty_cycle(pwm_channel, 0);
    pwm_driver_stop(pwm_channel);
}

void test_haptics(void) {
    _delay_ms(10);
    pwm_channel_e pwm_channel = PWM_DRIVER_INVALID_CHANNEL;
    pwm_driver_init(&pwm_channel, HAPTICS_PIN, 2000);
    if (pwm_channel == PWM_DRIVER_INVALID_CHANNEL) {
        return;
    }

    pwm_driver_set_duty_cycle(pwm_channel, 30);

    while (1) {
        check_for_new_action(&action, true);
        if (action == CONTROLS_BUTTON_2) {
            break;
        }
        _delay_ms(10);
    }

    pwm_driver_set_duty_cycle(pwm_channel, 0);
    pwm_driver_stop(pwm_channel);
}

void test_neopixels(void) {
    _delay_ms(10);
    neopixel_driver_send_led_array(red_array, sizeof(red_array), 5);

    while (1) {
        check_for_new_action(&action, true);
        switch (action) {
            case CONTROLS_BUTTON_1:
                neopixel_driver_send_led_array(blue_array, sizeof(blue_array), 5);
                break;
            case CONTROLS_BUTTON_3:
                neopixel_driver_send_led_array(green_array, sizeof(blue_array), 5);
                break;
            case CONTROLS_BUTTON_2:
                neopixel_driver_send_led_array(closed_array, sizeof(closed_array), 0);
                return;
            default:
                break;
        }
        _delay_ms(10);
    }
}

void test_bme_sensor(void) {
    _delay_ms(10);
    bme_reading_t reading;
    uint8_t       buffer[30];
    bme_driver_read_sensor_data(&reading);
    display_manager_start_text_to_screen();
    CLEAR_MENU_AREA;

    write_entry(TEMPERATURE_TAG, 130, 1);
    write_entry(HUMIDITY_TAG, 180, 1);
    write_entry(PRESSURE_TAG, 230, 1);
    write_entry(GAS_RESISTANCE_TAG, 280, 1);
    display_manager_update_text_to_screen();

    uint16_t second_counter = BME_READING_DELAY * 100;
    while (1) {
        check_for_new_action(&action, true);
        if (action == CONTROLS_BUTTON_2) {
            menu_set_should_redraw_menu(true);
            break;
        }

        if (second_counter >= BME_READING_DELAY * 100) {
            display_manager_start_text_to_screen();
            sprintf((char *)buffer, "%.2f C", reading.temperature);
            write_entry(buffer, 145, 0);

            sprintf((char *)buffer, "%.2f %%", reading.humidity);
            write_entry(buffer, 195, 0);

            sprintf((char *)buffer, "%.2f HPA", reading.pressure / 100.0f);
            write_entry(buffer, 245, 0);

            sprintf((char *)buffer, "%.2f OHMS", reading.gas_resistance / 1000.0f);
            write_entry(buffer, 295, 0);

            display_manager_update_text_to_screen();
            bme_driver_read_sensor_data(&reading);
            second_counter = 0;
        }
        else {
            second_counter++;
            _delay_ms(10);
        }
    }
}

void test_fuel_gauge(void) {
    _delay_ms(10);

    uint8_t buffer[30];
    display_manager_start_text_to_screen();
    CLEAR_MENU_AREA;

    write_entry(BATTERY_VOLTAGE_TAG, 150, 1);
    write_entry(STATE_OF_CHARGE_TAG, 200, 1);

    uint16_t second_counter = MAX_READING_DELAY * 100;
    while (1) {
        check_for_new_action(&action, true);
        if (action == CONTROLS_BUTTON_2) {
            menu_set_should_redraw_menu(true);
            break;
        }

        if (second_counter >= MAX_READING_DELAY * 100) {
            float vcell = 0.0f;
            float soc   = 0.0f;
            max_driver_read_soc(&soc);
            max_driver_read_voltage(&vcell);

            display_manager_start_text_to_screen();
            sprintf((char *)buffer, "%.2f", vcell);
            write_entry(buffer, 165, 0);

            sprintf((char *)buffer, "%.2f %%", (soc));
            write_entry(buffer, 215, 0);

            display_manager_update_text_to_screen();
            second_counter = 0;
        }
        else {
            second_counter++;
            _delay_ms(10);
        }
    }
}

void test_serial(void) {
    _delay_ms(10);
    printf("starting serial testing...\n");

    while (1) {
        check_for_new_action(&action, true);
        switch (action) {
            case CONTROLS_BUTTON_1:
                printf("new up button press\n");
                break;
            case CONTROLS_BUTTON_3:
                printf("new down button press\n");
                break;
            case CONTROLS_BUTTON_2:
                printf("stopping serial testing...\n");
                return;
            default:
                break;
        }
        _delay_ms(10);
    }
}

void test_microsd(void) {
    _delay_ms(10);

    display_manager_clear_display_buffers();
    display_manager_write_image_to_buffer((uint8_t *)"/background.bmp", 0, 0, true);
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    while (1) {
        check_for_new_action(&action, true);
        switch (action) {
            case CONTROLS_BUTTON_1:
                break;
            case CONTROLS_BUTTON_3:
                break;
            case CONTROLS_BUTTON_2:
                menu_set_should_redraw_menu(true);
                return;
            default:
                break;
        }
        _delay_ms(10);
    }
}
void test_nfc(void) {
    _delay_ms(10);

    display_manager_start_text_to_screen();
    CLEAR_MENU_AREA;

    write_entry(NFC_DATA_TAG, 30, 2);

    uint16_t second_counter = NFC_READING_DELAY * 100;
    while (1) {
        controls_button_e action;
        check_for_new_action(&action, true);

        if (action == CONTROLS_BUTTON_2) {
            menu_set_should_redraw_menu(true);
            return;
        }

        if (second_counter >= NFC_READING_DELAY * 100) {
            uint8_t ndef_string[255];
            uint8_t length;
            m24lr_driver_read_ndef(M24LR_DRIVER_NDEF_TYPE_FORUM_WKT, ndef_string, &length);

            display_nfc_data(ndef_string, length);
            display_manager_write_text_to_screen((uint8_t *)"Save data to the NFC memory.", 28, 10, 330, 0);
            display_manager_write_text_to_screen((uint8_t *)"Please upload data below.", 25, 10, 345, 0);

            display_manager_write_text_to_screen((uint8_t *)"<-NFC", 5, 60, 385, 1);
            display_manager_update_text_to_screen();
            second_counter = 0;
        }
        else {
            second_counter++;
            _delay_ms(10);
        }
    }
}

void factory_reset(void) {
    _delay_ms(10);

    LOG_INFO("Resetting to factory settings.");

    display_manager_init();
    display_manager_clear_display_buffers();
    display_manager_write_centered_message((uint8_t *)"Reverting to factory settings.", 30, 200, 1, false);
    display_manager_write_centered_message((uint8_t *)"Please wait...", 14, 220, 0, false);
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    status_t delete_status = storage_clear("backup");
    status_t copy_status   = storage_copy_directory_content("backup", "");

    LOG_INFO("Finish delete with status: %02x", delete_status);
    LOG_INFO("Finish copy with status: %02x", copy_status);

    display_manager_clear_display_buffers();
    if (delete_status || copy_status) {
        display_manager_write_centered_message((uint8_t *)"Factory reset error.", 20, 200, 1, false);
    }
    else {
        display_manager_write_centered_message((uint8_t *)"Factory reset successful.", 25, 200, 1, false);
    }
    display_manager_write_centered_message((uint8_t *)"Please restart.", 15, 220, 0, false);
    display_manager_render_full_display(STORAGE_IMAGE_TYPE_GRAYSCALE);

    while (1) {
        _delay_ms(100);
    }
}

void test_accelerometer(void) {
    _delay_ms(10);

    uint8_t buffer[30];
    display_manager_start_text_to_screen();
    CLEAR_MENU_AREA;

    write_entry(X_AXIS_TAG, 150, 1);
    write_entry(Y_AXIS_TAG, 200, 1);
    write_entry(Z_AXIS_TAG, 250, 1);

    uint16_t second_counter = ADXL_READING_DELAY * 100;
    while (1) {
        check_for_new_action(&action, true);
        if (action == CONTROLS_BUTTON_2) {
            menu_set_should_redraw_menu(true);
            break;
        }
        if (second_counter >= ADXL_READING_DELAY * 100) {
            adxl345_reading_t reading;
            adxl345_driver_get_axis_data(&reading);

            display_manager_start_text_to_screen();
            sprintf((char *)buffer, "%.2f", reading.x_axis);
            write_entry(buffer, 165, 0);

            sprintf((char *)buffer, "%.2f", reading.y_axis);
            write_entry(buffer, 215, 0);

            sprintf((char *)buffer, "%.2f", reading.z_axis);
            write_entry(buffer, 265, 0);

            display_manager_update_text_to_screen();
            second_counter = 0;
        }
        else {
            second_counter++;
            _delay_ms(10);
        }
    }
}

void display_nfc_data(uint8_t *ndef_string, uint8_t length) {
    uint8_t record[255];
    uint8_t lines[20][30];
    uint8_t record_length = 0;
    uint8_t line_count    = 0;

    for (size_t i = 0; i < length; i++) {
        if (ndef_string[i] == CONFIGURATION_NEW_TEXT_ENTRY) {
            record_length = ndef_string[i - 1] - 3;
            i += 4;

            uint8_t record_iterator = 0;
            for (size_t j = i; j < i + record_length; j++) {
                record[record_iterator] = ndef_string[j];
                record_iterator++;
            }
            i = i + record_length;

            record[record_iterator] = 0;

            strcpy((char *)lines[line_count++], (char *)record);
        }
    }

    for (size_t i = 0; i < line_count - 1; i++) {
        display_partial_clear(20, 80 + (25 * i) - 10, 200, 35);
        display_manager_write_text_to_screen(lines[i], strlen((char *)lines[i]), 10, 80 + 25 * i, 1);
    }
}

void write_entry(uint8_t *buffer, uint16_t y, uint8_t font_size) {
    uint8_t width = 0;
    display_set_font(font_size);

    display_partial_clear(50, y - 5, 140, 35);

    for (size_t i = 0; i < strlen((char *)buffer); i++) {
        width += display_get_character_width(buffer[i]);
    }

    uint16_t offset = ((DISPLAY_WIDTH - width) / 2) + 8;
    display_manager_write_text_to_screen(buffer, strlen((char *)buffer), offset, y, font_size);
}

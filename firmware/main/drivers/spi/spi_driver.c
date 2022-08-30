#include "spi_driver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "log.h"

#include <stdbool.h>

#define MODULE_TAG "SPI_DRIVER"

/** spi frequency = SPI_FREQUENCY_MULTIPLIER * 1 MHz */
#define SPI_FREQUENCY_MULTIPLIER 20 /** defines the multiplier for the SPI frequency (MHz) */
#define SPI_TRANSACTION_SIZE     8  /** size of a transaction (in bits) */

static bool is_init = false;

static uint8_t display_channel = SPI_DRIVER_INVALID_CHANNEL;
static uint8_t sd_channel      = SPI_DRIVER_INVALID_CHANNEL;
static uint8_t dwm_channel     = SPI_DRIVER_INVALID_CHANNEL;

typedef struct {
    bool                is_channel_in_use;
    spi_device_handle_t spi_handle;
} spi_channel_t;

static spi_channel_e channel_in_use = SPI_DRIVER_INVALID_CHANNEL;
static spi_channel_t spi_channels[8];

spi_channel_e add_spi_channel(uint8_t pin_num, uint8_t frquency_multiplier);
void          remove_spi_channel(spi_channel_e channel);

status_t spi_driver_init(void) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    spi_bus_config_t bus_config = {
        .miso_io_num     = SPI_PIN_MISO,
        .mosi_io_num     = SPI_PIN_MOSI,
        .sclk_io_num     = SPI_PIN_CLK,
        .quadhd_io_num   = -1,
        .quadwp_io_num   = -1,
        .max_transfer_sz = SPI_BLOCK_SIZE,
    };

    esp_err_t error_check = spi_bus_initialize(HSPI_HOST, &bus_config, 1);
    if (error_check != ESP_OK) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    display_channel = add_spi_channel(SPI_DISPLAY_CS, SPI_FREQUENCY_MULTIPLIER);
    sd_channel      = add_spi_channel(SPI_SD_CS, SPI_FREQUENCY_MULTIPLIER);
    dwm_channel     = add_spi_channel(SPI_DWM_CS, SPI_FREQUENCY_MULTIPLIER);

    is_init = true;
    LOG_INFO("driver init ok");
    return STATUS_OK;
}

/** @todo: please check a way to fix this function */
uint8_t spi_driver_transact(uint8_t data) {
    uint8_t response = 0;

    spi_transaction_t byte = {
        .flags   = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .tx_data = {data},
        .length  = SPI_TRANSACTION_SIZE,
    };

    esp_err_t check = spi_device_polling_transmit(spi_channels[channel_in_use].spi_handle, &byte);
    assert(check == ESP_OK);
    response = byte.rx_data[0];

    return response;
}

spi_channel_e add_spi_channel(uint8_t pin_num, uint8_t frquency_multiplier) {
    spi_channel_e channel = SPI_DRIVER_INVALID_CHANNEL;

    for (size_t i = 0; i < 8; i++) {
        if (spi_channels[i].is_channel_in_use == false) {
            channel = i;

            spi_device_interface_config_t configuration = {
                .clock_speed_hz = frquency_multiplier * 1000 * 1000,
                .mode           = 0,
                .spics_io_num   = pin_num,
                .queue_size     = 1,
            };

            ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &configuration, &spi_channels[i].spi_handle));

            spi_channels[i].is_channel_in_use = true;
            break;
        }
    }

    return channel;
}

void remove_spi_channel(spi_channel_e channel) {
    ESP_ERROR_CHECK(spi_bus_remove_device(spi_channels[channel].spi_handle));
    spi_channels[channel].is_channel_in_use = false;
}

status_t set_use_display_spi(void) {
    CHECK_INIT(is_init);

    channel_in_use = display_channel;
    return STATUS_OK;
}

status_t set_use_sd_spi(void) {
    CHECK_INIT(is_init);

    channel_in_use = sd_channel;
    return STATUS_OK;
}

status_t set_use_dwm_spi(void) {
    CHECK_INIT(is_init);

    channel_in_use = dwm_channel;
    return STATUS_OK;
}

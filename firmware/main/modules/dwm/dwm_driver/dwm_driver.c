#include "dwm_driver.h"
#include "log.h"
#include "spi_driver.h"
#include "utilities.h"

#include <string.h>

#define MODULE_TAG "DWM_DRIVER"

#define ERROR_CHECK(x, y) \
    if (x == DWT_ERROR) { \
        LOG_WARNING(y);   \
        return;           \
    }

#define TX_ANTENNA_DELAY 16385 /** tx antenna delay */
#define RX_ANTENNA_DELAY 16385 /** rx antenna delay */
#define RX_BUFFER_LENGTH 127   /** maximum size of rx_buffer */

#define MAXIMUM_ATTEMPT_NUMBER 10 /** maximum number of attempts when executing a function that failed */

static uint8_t      sfd_values[] = {8, 8, 16, 8};
static dwt_config_t api_config   = {
    .chan           = 5,
    .txPreambLength = DWT_PLEN_128,
    .rxPAC          = DWT_PAC8,
    .txCode         = 9,
    .rxCode         = 9,
    .sfdType        = 1,
    .dataRate       = DWT_BR_6M8,
    .phrMode        = DWT_PHRMODE_STD,
    .phrRate        = DWT_PHRRATE_STD,
    .sfdTO          = (129 + 8 - 8),
    .stsMode        = DWT_STS_MODE_OFF,
    .stsLength      = DWT_STS_LEN_64,
    .pdoaMode       = DWT_PDOA_M0,
};

static bool is_init = false;

static uint8_t  rx_buffer[127];
static uint16_t rx_buffer_size = 0;

void set_configuration(dwm_configuration_t *config);
void set_configuration_data(dwm_configuration_t *config);

void rx_callback_ok(const dwt_cb_data_t *rxd);
void rx_callback_timeout(const dwt_cb_data_t *rxd);
void rx_callback_error(const dwt_cb_data_t *rxd);

void tx_callback_ok(const dwt_cb_data_t *rxd);
void tx_callback_error(const dwt_cb_data_t *rxd);

void spi_callback_error(const dwt_cb_data_t *rxd);

int8_t dwm_api_init(dwm_configuration_t *config);
void   reset_driver(void);

status_t dwm_driver_init(dwm_configuration_t *config) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
        return STATUS_ALREADY_INIT;
    }

    status_t driver_status = spi_driver_init();
    if (driver_status != STATUS_OK && driver_status != STATUS_ALREADY_INIT) {
        LOG_WARNING("driver init failed");
        return STATUS_INIT_FAILED;
    }

    SET_OUTPUT_PIN(DWM3000_DRIVER_CS_PIN);
    SET_PIN_HIGH(DWM3000_DRIVER_CS_PIN);

    RETRY((dwm_api_init(config) != 0), MAXIMUM_ATTEMPT_NUMBER, "dwm api init failed", STATUS_DWM_TIMEOUT_ERROR);

    is_init = true;
    LOG_INFO("driver init ok");
    return STATUS_OK;
}

status_t dwm_driver_request_data(uint8_t *buffer, uint32_t *size) {
    CHECK_INIT(is_init);

    dwt_forcetrxoff();
    dwt_setrxaftertxdelay(1);

    dwt_writetxdata((*size) + 2, buffer, 0);
    dwt_writetxfctrl((*size) + 2, 0, 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    return STATUS_OK;
}

status_t dwm_driver_transmit_data(uint8_t *buffer, uint32_t size) {
    CHECK_INIT(is_init);
    dwt_forcetrxoff();

    dwt_writetxdata((uint16_t)(size + 2), buffer, 0);
    dwt_writetxfctrl((uint16_t)(size + 2), 0, 0);

    dwt_starttx(DWT_START_TX_IMMEDIATE);
    return STATUS_OK;
}

status_t dwm_driver_retrieve_data(uint8_t *buffer, uint32_t *size) {
    CHECK_INIT(is_init);

    (*size) = (uint32_t)rx_buffer_size;
    memcpy(buffer, rx_buffer, (*size));

    if (rx_buffer_size != 0) {
        rx_buffer_size = 0;
        LOG_INFO("frame retrieved");
    }

    return STATUS_OK;
}

void dwm_driver_poll_irq(void) {
    if (dwt_checkirq()) {
        dwt_isr();
    }
}

status_t dwm_driver_enable_sleep(void) {
    CHECK_INIT(is_init);

    dwt_configuresleep(DWT_CONFIG, DWT_PRES_SLEEP | DWT_WAKE_WUP | DWT_SLP_EN);
    dwt_entersleep(DWT_DW_IDLE);

    is_init = false;
    return STATUS_OK;
}

status_t dwm_driver_wakeup(dwm_configuration_t *config) {
    RETRY((dwm_api_init(config) != 0), MAXIMUM_ATTEMPT_NUMBER, "dwm api init failed", STATUS_DWM_TIMEOUT_ERROR);

    return STATUS_OK;
}
int8_t dwm_api_init(dwm_configuration_t *config) {
    set_configuration(config);
    reset_driver();

    RETRY((!dwt_checkidlerc()), MAXIMUM_ATTEMPT_NUMBER, "idle check fail", STATUS_DWM_IDLERC_ERROR);
    LOG_DEBUG("idle check ok");

    RETRY((dwt_initialise(DWT_DW_INIT) != 0), MAXIMUM_ATTEMPT_NUMBER, "API init fail", STATUS_DWM_API_INIT_ERROR);
    LOG_DEBUG("API init ok");

    RETRY((dwt_configure(&api_config) != 0), MAXIMUM_ATTEMPT_NUMBER, "API config fail", STATUS_DWM_API_CONFIG_ERROR);
    LOG_DEBUG("API config ok");

    dwt_forcetrxoff();

    dwt_txconfig_t tx_conf = {
        .PGdly = 0x34,
        .power = 0xfdfdfdfd,
    };
    dwt_configuretxrf(&tx_conf);

    dwt_setrxantennadelay(RX_ANTENNA_DELAY);
    dwt_settxantennadelay(TX_ANTENNA_DELAY);

    dwt_setcallbacks(&tx_callback_ok, &rx_callback_ok, &rx_callback_timeout, &rx_callback_error, &spi_callback_error,
                     NULL);
    dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RPHE | DWT_INT_RFCG | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_RFTO
                         | DWT_INT_SFDT | DWT_INT_RXPTO | DWT_INT_SCRC,
                     0, DWT_ENABLE_INT);

    return 0;
}

void rx_callback_ok(const dwt_cb_data_t *rxd) {
    LOG_INFO("RX callback ok");

    rx_buffer_size = rxd->datalength;
    dwt_readrxdata(rx_buffer, rx_buffer_size, 0);

    /** @todo: there should be optimisations done here */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

void rx_callback_timeout(const dwt_cb_data_t *rxd) {
    LOG_WARNING("RX callback timeout");
}

void rx_callback_error(const dwt_cb_data_t *rxd) {
    LOG_WARNING("RX callback error");
}

void tx_callback_ok(const dwt_cb_data_t *rxd) {
    LOG_INFO("TX callback ok");
}

void tx_callback_error(const dwt_cb_data_t *rxd) {
    LOG_WARNING("TX callback error");
}

void spi_callback_error(const dwt_cb_data_t *rxd) {
    LOG_WARNING("DWM-SPI callback error");
}

void set_configuration(dwm_configuration_t *config) {
    api_config.chan           = config->channel;
    api_config.dataRate       = config->data_rate;
    api_config.sfdType        = config->sfd_type;
    api_config.txPreambLength = config->preamble_len;

    set_configuration_data(config);
}

void set_configuration_data(dwm_configuration_t *config) {
    uint16_t preamble_len                   = 0;
    uint8_t  preamble_aquisition_chunk_size = 32;

    switch (config->preamble_len) {
        case DWM_PREM_LENGTH_64:
            preamble_len                   = 64;
            preamble_aquisition_chunk_size = 8;
            break;

        case DWM_PREM_LENGTH_128:
            preamble_len                   = 128;
            preamble_aquisition_chunk_size = 8;
            break;

        case DWM_PREM_LENGTH_256:
            preamble_len                   = 256;
            preamble_aquisition_chunk_size = 16;
            break;

        case DWM_PREM_LENGTH_512:
            preamble_len = 512;
            break;

        case DWM_PREM_LENGTH_1024:
            preamble_len = 1024;
            break;

        case DWM_PREM_LENGTH_1536:
            preamble_len = 1536;
            break;

        case DWM_PREM_LENGTH_2048:
            preamble_len = 2048;
            break;

        case DWM_PREM_LENGTH_4096:
            preamble_len = 4096;
            break;

        default:
            preamble_len                   = 128;
            preamble_aquisition_chunk_size = 8;
    }

    api_config.sfdTO = preamble_len + 1 + sfd_values[api_config.sfdType] - preamble_aquisition_chunk_size;
}

void reset_driver(void) {
    SET_OUTPUT_PIN(DWM3000_DRIVER_RST_PIN);
    SET_PIN_LOW(DWM3000_DRIVER_RST_PIN);
    _delay_ms(2);

    SET_INPUT_PIN(DWM3000_DRIVER_RST_PIN);
    _delay_ms(2);
}

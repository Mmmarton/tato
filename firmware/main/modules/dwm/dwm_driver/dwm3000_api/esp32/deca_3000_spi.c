#include "../deca_device_api.h"
#include "spi_driver.h"
#include "utilities.h"

#define DWM3000_DRIVER_CS_PIN 15

#define ENABLE_CS_LINE  (SET_PIN_LOW(DWM3000_DRIVER_CS_PIN))
#define DISABLE_CS_LINE (SET_PIN_HIGH(DWM3000_DRIVER_CS_PIN))

int writetospiwithcrc(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodylength,
                      const uint8_t *bodyBuffer, uint8_t crc8) {
    /** dummy function so the API doesen't fail */
    return DWT_SUCCESS;
}

/** @todo: remove the prints below. will be kept until the library is finished */
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodylength, const uint8_t *bodyBuffer) {
    set_use_dwm_spi();
    ENABLE_CS_LINE;
    for (size_t i = 0; i < headerLength; i++) {
        spi_driver_transact(headerBuffer[i]);
    }
    for (size_t i = 0; i < bodylength; i++) {
        spi_driver_transact(bodyBuffer[i]);
    }
    DISABLE_CS_LINE;

    return DWT_SUCCESS;
}

int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer) {
    set_use_dwm_spi();
    ENABLE_CS_LINE;
    for (size_t i = 0; i < headerLength; i++) {
        spi_driver_transact(headerBuffer[i]);
    }
    for (size_t i = 0; i < readlength; i++) {
        readBuffer[i] = spi_driver_transact(0x00);
    }
    DISABLE_CS_LINE;

    return DWT_SUCCESS;
}
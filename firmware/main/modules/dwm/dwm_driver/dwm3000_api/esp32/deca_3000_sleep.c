#include "../deca_device_api.h"
#include "spi_driver.h"
#include "utilities.h"

#define TICKS_PER_MICROSECONDS 80

void deca_usleep(unsigned long time_us) {
    for(size_t i = 0; i < TICKS_PER_MICROSECONDS * time_us; i++) {
        continue;
    }
}
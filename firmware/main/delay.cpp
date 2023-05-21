#include "delay.h"
#include "rom/ets_sys.h"

void delay(uint16_t value) {
    ets_delay_us(value * 1000);
}

void delayUs(uint16_t value) {
    ets_delay_us(value);
}
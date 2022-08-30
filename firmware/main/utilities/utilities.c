#include "utilities.h"

#include <sys/time.h>

long map(long num, long in_min, long in_max, long out_min, long out_max) {
    if (num < in_min || num > in_max) {
        return out_min;
    }

    return (num - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void swap_int(int *x, int *y) {
    int aux = (*x);
    (*x)    = (*y);
    (*y)    = aux;
}

float celsius_to_fahrenheit(float temperature) {
    return (((temperature * 9.0f) / 5.0f) + 32);
}

int32_t get_time(void) {
    struct timeval new_timeval;

    int err = gettimeofday(&new_timeval, NULL);
    assert(err == 0);
    return new_timeval.tv_sec;
}

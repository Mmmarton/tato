#include "badge.h"
#include "badge_test.h"
#include "log.h"
#include "spi_driver.h"
#include "stress_testing.h"
#include "utilities.h"

#define MODULE_TAG "MAIN"

void app_main(void) {

    if (badge_init() == STATUS_OK) {
        badge_loop();
    }

    badge_hibernate();
}

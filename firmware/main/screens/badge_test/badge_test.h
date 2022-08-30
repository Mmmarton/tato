#ifndef BADGE_TEST_H
#define BADGE_TEST_H

#include "error_manager.h"

#define BME_READING_DELAY  5  /*!< delay between BME sensor readings when testing (in seconds) */
#define MAX_READING_DELAY  60 /*!< delay between MAX sensor readings when testing (in seconds) */
#define ADXL_READING_DELAY 2  /*!< delay between ADXL sensor readings when testing (in seconds) */
#define NFC_READING_DELAY  10 /*!< delay between NFC memory readings when testing (in seconds) */

#define BUZZER_PIN  17 /*!< pin of the buzzer */
#define HAPTICS_PIN 4  /*!< pin of the vibrator motor */

/**
 * @brief This function is used to initialize the dependencies of the test menu.
 *
 * @return (status_t)
 * @retval 0 success, others for errors 
 */
status_t badge_test_init(void);

/**
 * @brief This function is start the test menu loop.
 *
 * @return      (void)
 */
void badge_test_loop(void);

#endif
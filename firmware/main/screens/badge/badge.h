#ifndef BADGE_H
#define BADGE_H

#include "error_manager.h"

#define DEVICE_DEFAULT_SLEEP_TIMEOUT 500 /*!< time until the badge goes to sleep (seconds * 100) */

#define ALERT_ON_MS      20  /*!< on time (ms) */
#define ALERT_OFF_MS     10  /*!< off time (ms) */
#define ALERT_TOGGLE_NUM 3   /*!< number of toggles */
#define ALERT_PERIOD     250 /*!< total duration for a single alert cycle */

/**
 * @brief This function is used to initialize the badge.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t badge_init(void);

/**
 * @brief This function is used to start the badge loop that handles the entire logic.
 *
 * @return  (void)
 */
void badge_loop(void);

/**
 * @brief This function is used to put the badge device to sleep.
 *
 * @return  (void)
 */
void badge_hibernate(void);

#endif
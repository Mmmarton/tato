#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "error_manager.h"

#define ON_TIME      50  /*!< alert mode off time (x 10ms) */
#define OFF_TIME     50  /*!< alert mode on time (x 10ms) */
#define TOGGLE_COUNT 3   /*!< number of toggles during alerts */
#define PERIOD       700 /*!< alert period */

#define NOTIFICATION_MANAGER_NOTIFICATION_ID_BITMASK 0b00000111 /*!< bitmask for the notification id */
#define NOTIFICATION_MANAGER_ALERT_MODE_BITMASK      0b11111000 /*!< bitmask for the alert mode settings */

/** @brief This enumeration contains the notification types that can be retrieved. */
typedef enum {
    NOTIFICATION_TYPE_DANGER = 1,
    NOTIFICATION_TYPE_WARNING,
    NOTIFICATION_TYPE_SUCCESS,
    NOTIFICATION_TYPE_INFO,
} notification_type_e;


/** @brief This enumeration contains possible values of the weather type bits. Based on this value, the display renders
 *         a different image. */
typedef enum {
    NOTIFICATION_WEATHER_TYPE_SUNNY,
    NOTIFICATION_WEATHER_TYPE_PARTLY_CLOUDY,
    NOTIFICATION_WEATHER_TYPE_CLOUDY,
    NOTIFICATION_WEATHER_TYPE_STORMY,
    NOTIFICATION_WEATHER_TYPE_WINDY,
    NOTIFICATION_WEATHER_TYPE_RAINY,
    NOTIFICATION_WEATHER_TYPE_SNOWY,
} notification_weather_type_e;

/**
 * @brief This function is used to initialize the notification manager and its required components.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t notification_manager_init(void);

/**
 * @brief This function is used to check if any new notification has arrived. If there is any, the device will lock
 *        inside a loop that repeats every `period / 100` seconds.
 *
 * @param[in] settings     Buzzer, haptics and color settings.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t notification_manager_enter_alert_mode(uint8_t settings);

/**
 * @brief This function is used as a setter for the `has_new_notification` boolean.
 *
 * @param[in]   state   New value of the `has_new_notification` boolean.
 *
 * @return (void)
 */
void notification_manager_set_has_new_notification(bool state);

/**
 * @brief This function is used as a getter for the `has_new_notification` boolean.
 *
 * @return  (bool)
 * @retval  true when a new notification has arrived
 * @retval  false when no new notification has arrived
 */
bool notification_manager_get_has_new_notification(void);

#endif
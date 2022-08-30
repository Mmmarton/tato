/*! @file controls.h
 *
 * @brief This driver handles the push button inputs.
 *
 * @note This module is subject to change in future implementations.
 */

/*!
 * @defgroup CONTROLS
 * @{*/

#ifndef CONTROLS_H
    #define CONTROLS_H

    #include <stdbool.h>

    #include "error_manager.h"

    #define PRESS_COUNT 30 /*!< defines the number of miliseconds that determines a button press */

/**
 * @brief This enum contains the user-defined buttons and their pins.
 */
typedef enum {
    CONTROLS_BUTTON_1   = 36, /*!< pin for the profile page button */
    CONTROLS_BUTTON_2   = 39, /*!< pin for the weather page button */
    CONTROLS_BUTTON_3   = 35, /*!< pin for the notifications page button */
    CONTROLS_BUTTON_SOS = 10, /*!< pin combination for triggering SOS */
} controls_button_e;

/**
 * @brief This function is used to initialize the controls driver.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t controls_driver_init(void);

/**
 * @brief This function is used to check if any new action took place. An action is considered to be a press and
 *        release set. The function returns the pin on which the action took palce.
 *
 * @param[out]  action      Number of the button that was pressed.
 * @param[in]   debounce_on Flag that specifies if debounce should be user or not.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t check_for_new_action(controls_button_e *action, bool debounce_on);

#endif
/** @}*/
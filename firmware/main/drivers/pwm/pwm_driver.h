/*! @file pwm_driver.h
 *
 * @brief This driver is used to enable PWM functionality.
 */

/*!
 * @defgroup PWM
 * @{*/

#ifndef PWM_DRIVER_H
    #define PWM_DRIVER_H

    #include <stdint.h>

    #include "error_manager.h"

/** @brief This enumeration contains available PWM channels. */
typedef enum {
    PWM_DRIVER_CHANNEL_0,       /*!< PWM channel 0 */
    PWM_DRIVER_CHANNEL_1,       /*!< PWM channel 1 */
    PWM_DRIVER_CHANNEL_2,       /*!< PWM channel 2 */
    PWM_DRIVER_CHANNEL_3,       /*!< PWM channel 3 */
    PWM_DRIVER_CHANNEL_4,       /*!< PWM channel 4 */
    PWM_DRIVER_CHANNEL_5,       /*!< PWM channel 5 */
    PWM_DRIVER_CHANNEL_6,       /*!< PWM channel 6 */
    PWM_DRIVER_CHANNEL_7,       /*!< PWM channel 7 */
    PWM_DRIVER_INVALID_CHANNEL, /*!< invalid PWM channel */
} pwm_channel_e;

/**
 * @brief This function is used to initialize and return a PWM channel on a user-defined pin.
 *
 * @param[out]  channel  Configured target channel.
 * @param[in]   pin      The pin to be used for PWM output.
 * @param[in]   frquency The PWM frequency.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t pwm_driver_init(pwm_channel_e *channel, uint8_t pin, uint32_t frequency);

/**
 * @brief This function is used to stop a specific PWM channel.
 *
 * @param[in]   channel The target PWM channel.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t pwm_driver_stop(pwm_channel_e channel);

/**
 * @brief This function is used to set the duty cycle of a PWM channel to a specific value (must be between 0 and 100).
 *
 * @param[in]   channel     The target PWM channel.
 * @param[in]   duty_cycle  The new duty cycle value.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t pwm_driver_set_duty_cycle(pwm_channel_e channel, uint8_t duty_cycle);

#endif
/** @}*/
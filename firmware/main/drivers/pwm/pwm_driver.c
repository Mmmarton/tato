#include "pwm_driver.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "log.h"

#include <math.h>
#include <stdbool.h>

#define MODULE_TAG      "PWM_DRIVER"
#define CHANNEL_COUNT   8 /** total number of PWM channels */
#define DUTY_RESOLUTION LEDC_TIMER_13_BIT

typedef struct {
    bool    is_channel_in_use;
    uint8_t pin_num;
} pwm_channel_t;

static pwm_channel_t pwm_channels[CHANNEL_COUNT];
static bool          is_init = false;

status_t pwm_driver_init(pwm_channel_e *pwm_channel, uint8_t pin, uint32_t frequency) {
    if (is_init == true) {
        LOG_WARNING("driver already init");
    }

    uint8_t   channel     = PWM_DRIVER_INVALID_CHANNEL;
    uint8_t   timer       = PWM_DRIVER_INVALID_CHANNEL;
    esp_err_t error_check = ESP_OK;

    for (size_t i = 0; i < CHANNEL_COUNT; i++) {
        if (pwm_channels[i].is_channel_in_use == true) {
            if (pwm_channels[i].pin_num == pin) {
                LOG_ERROR("pin already in use");
                (*pwm_channel) = PWM_DRIVER_INVALID_CHANNEL;
                return STATUS_PWM_INVALID_CHANNEL_ERROR;
            }
        }

        if (pwm_channels[i].is_channel_in_use == false) {
            channel = i;
            timer   = i;

            pwm_channels[i].pin_num           = pin;
            pwm_channels[i].is_channel_in_use = true;

            break;
        }
    }

    if (channel == PWM_DRIVER_INVALID_CHANNEL || timer == PWM_DRIVER_INVALID_CHANNEL) {
        LOG_ERROR("all pwm channels are already in use");
        (*pwm_channel) = PWM_DRIVER_INVALID_CHANNEL;
        return STATUS_PWM_INVALID_CHANNEL_ERROR;
    }

    ledc_timer_config_t timer_configuration = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = timer,
        .duty_resolution = DUTY_RESOLUTION,
        .freq_hz         = frequency,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    error_check = ledc_timer_config(&timer_configuration);
    if (error_check != ESP_OK) {
        return STATUS_INIT_FAILED;
    }

    ledc_channel_config_t channel_configuration = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = channel,
        .timer_sel  = timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = pin,
        .duty       = 0,
        .hpoint     = 0,
    };
    error_check = ledc_channel_config(&channel_configuration);
    if (error_check != ESP_OK) {
        return STATUS_INIT_FAILED;
    }

    is_init        = true;
    (*pwm_channel) = channel;

    LOG_INFO("pwm started on channel %d", channel);
    return STATUS_OK;
}

status_t pwm_driver_stop(pwm_channel_e channel) {
    CHECK_INIT(is_init);

    pwm_channels[channel].is_channel_in_use = false;
    gpio_set_direction(pwm_channels[channel].pin_num, GPIO_MODE_INPUT);
    ledc_stop(LEDC_LOW_SPEED_MODE, channel, 0);

    is_init = false;
    return STATUS_OK;
}

status_t pwm_driver_set_duty_cycle(pwm_channel_e channel, uint8_t duty_cycle) {
    CHECK_INIT(is_init);

    if (duty_cycle > 100) {
        LOG_WARNING("duty cycle value out of bounds");
        return STATUS_PWM_OUT_OF_BOUNDS_ERROR;
    }

    uint32_t mapped_duty = ((((uint32_t)pow(2, DUTY_RESOLUTION) - 1) * duty_cycle) / 100);

    ledc_channel_config_t channel_configuration = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = channel,
        .timer_sel  = channel,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = pwm_channels[channel].pin_num,
        .duty       = mapped_duty,
        .hpoint     = 0,
    };
    esp_err_t error_check = ledc_channel_config(&channel_configuration);

    if (error_check != ESP_OK) {
        LOG_WARNING("duty cycle could not be set");
        return STATUS_PWM_SET_DUTY_CYCLE_ERROR;
    }

    return STATUS_OK;
}

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "configuration_manager_defines.h"
#include "error_manager.h"
#include "neopixel_driver.h"

#define CONFIGURATION_MANAGER_WEATHER_SIGN_BITMASK  0b10000000 /*!< bitmask used to retrieve the sign */
#define CONFIGURATION_MANAGER_WEATHER_IMAGE_BITMASK 0b01111111 /*!< bitmask used to retrieve the weather image */

/** @brief This enumeration contains the index for each available user editable configuration. */
typedef enum {
    BADGE_CONFIGURATION_NAME,              /*!< profile screen - name */
    BADGE_CONFIGURATION_SURNAME,           /*!< profile screen - surname */
    BADGE_CONFIGURATION_FUNCTION,          /*!< profile screen - function */
    BADGE_CONFIGURATION_ISSUED_DATE,       /*!< profile screen - issued date */
    BADGE_CONFIGURATION_CITY,              /*!< weather screen - city */
    BADGE_CONFIGURATION_AREA,              /*!< weather screen - area */
    BADGE_CONFIGURATION_REGULAR_LED_COLOR, /*!< general settings - regular notification LED color */
    BADGE_CONFIGURATION_ALERT_LED_COLOR,   /*!< general settings - alert notification LED color */
    BADGE_CONFIGURATION_DANGER_LED_COLOR,  /*!< general settings - danger notification LED color */
} configurations_index_e;

/**
 * @brief This function is used to initialize the configuration manager by checking for the existence of the required
 *        files. If the files are not found, this function generates the required files with default values.
 *
 * @return (void)
 */
status_t configuration_manager_init(void);

/**
 * @brief This function is used to read the NFC module internal memory and update the data contained inside the .txt
 *        files that are stored on the SD card.
 *
 * @param[out] configurations Array of records retrieved from the NFC internal memory.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_read_nfc_memory(configuration_record_t *configurations);

/**
 * @brief This function is used to retrieve the data stored on the SD card in order to render the profile screen.
 *
 * @param[out] profile Structure containing the necessary screen data.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_get_profile_data(profile_configuration_t *profile);

/**
 * @brief This function is used to retrieve the data stored on the SD card in order to render the profile screen.
 *
 * @param[out] notifications Structure containing the necessary screen data.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_get_notifications_data(notifications_configuration_t *notifications);

/**
 * @brief This function is used to retrieve the data stored on the SD card in order to render the profile screen.
 *
 * @param[out] weather Structure containing the necessary screen data.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_get_weather_data(weather_configuration_t *weather);

/**
 * @brief This function is used to retrieve data from the SD card inside the configuration structure.
 *
 * @param[out] device_data Configuration data of the device.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_get_configuration_data(device_configuration_t *device_data);

/**
 * @brief This function is used to update the device configuration stored on the SD card.
 *
 * @param[out] device_data Configuration data of the device.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_store_configuration_data(device_configuration_t *device_data);

/**
 * @brief This function is used to update the device ID stored on the SD card.
 *
 * @param[in] device_id Buffer containing the device id.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_update_device_id(uint8_t *device_id);

/**
 * @brief This function is used to update the profile data .txt file stored on the SD card.
 *
 * @param[in] configurations Array of records to be updated.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_update_profile_data(configuration_record_t *configurations);

/**
 * @brief This function is used to update the notifications data .txt file stored on the SD card.
 *
 * @param[in] buffer      Record containing data about the latest notification.
 * @param[in] buffer_size Flag used to determine what action should be performed.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_update_notifications_data(uint8_t *buffer, uint8_t buffer_size);

/**
 * @brief This function is used to update the weather data .txt file stored on the SD card using the data retrieved
 *        from the NFC chip.
 *
 * @param[in] configurations Array of records to be updated.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_update_weather_data_nfc(configuration_record_t *configurations);

/**
 * @brief This function is used to update the configuration data .txt file stored on the SD card using the data
 *        retrieved from the NFC chip.
 *
 * @param[in] notification Record containing data about the latest notification.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_update_configuration_data_nfc(configuration_record_t *configurations);

/**
 * @brief This function is used to update the configuration data .txt file stored on the SD card using the data
 *        retrieved over UWB.
 *
 * @param[in] device_cfg Structure containing the device configuration.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_update_configuration_data_dwm(device_configuration_t *device_cfg, uint8_t *buffer);

/**
 * @brief This function is used to store in the configurations parameter all the information stored on the SD Card
 *
 * @param[in] configurations Empty structure that will be populated with data from SD Card
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t configuration_manager_read_user_data(configuration_record_t *configurations);

status_t configuration_manager_load_full_configuration();

#endif
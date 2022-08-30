#ifndef CONFIGURATION_MANAGER_DEFINES_H
#define CONFIGURATION_MANAGER_DEFINES_H

#include <stdbool.h>
#include <stdint.h>

#define CONFIGURATION_NEW_TEXT_ENTRY  84 /*!< NDEF code for new TEXT entry 'T' */
#define CONFIGURATION_ENTRIES_NUMBER  13 /*!< number of configurations entries to be read from the NFC */
#define CONFIGURATION_MAX_BUFFER_SIZE 64 /*!< maximum size of NFC */

#define PACKED __attribute__((__packed__)) /*!< no offsets in structure memory blocks */

/** @brief This structure contains each record retrieved from the NFC memory and its specific length. */
typedef struct {
    uint8_t text[64]; /*!< text contained inside the record */
    uint8_t length;   /*!< length of the text */
} configuration_record_t;

/** @brief This structure contains each record used in building the profile screen. */
typedef union {
    struct PACKED profile_data {
        uint8_t name[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t surname[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t function[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t issue_date[20];
    } config;
    uint8_t buffer[sizeof(struct profile_data)];
} profile_configuration_t;

/** @brief This structure contains each record used in building the notifications screen. */
typedef union {
    struct PACKED notification_data {
        uint8_t date[20];
        uint8_t notification[4][CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t timestamp[4][30];
        uint8_t timestamp_date[4][3];
    } config;
    uint8_t buffer[sizeof(struct notification_data)];
} notifications_configuration_t;

/** @brief This structure contains each record used in building the weather screen. */
typedef union {
    struct PACKED weather_data {
        uint8_t location[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t area[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t date[20];
        uint8_t temperature;
        uint8_t weather_type;
    } config;
    uint8_t buffer[sizeof(struct weather_data)];
} weather_configuration_t;

typedef union {
    struct PACKED badge_data {
        uint8_t  dwm_channel;
        uint8_t  dwm_data_rate;
        uint8_t  dwm_preamble_len;
        uint8_t  dwm_sfd_type;
        uint8_t  device_id[16];
        uint8_t  date[20];
        uint16_t position_update_period;
        uint8_t  wakeup_sensitivity;
        uint16_t notification_poll_period;
        uint16_t awake_period;
        uint8_t  alert_led[3];
        uint8_t  danger_led[3];
        uint8_t  notification_led[3];
        uint8_t  badge_state;
        bool     is_profile_screen_built;
        bool     is_weather_screen_built;
        bool     is_notifications_screen_built;
        uint8_t  notification_state;
        uint8_t  file_update_state;
        uint8_t  targeted_notification_state;
        uint8_t  wifi_ssid[40];
        uint8_t  wifi_pass[40];
        uint8_t  name[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t  surname[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t  function[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t  issue_date[20];
        uint8_t  location[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t  area[CONFIGURATION_MAX_BUFFER_SIZE];
        uint8_t  weather_date[20];
        uint8_t  temperature;
        uint8_t  weather_type;
    } config;
    uint8_t buffer[sizeof(struct badge_data)];
} full_configuration_t;

/** @brief This structure contains each record used in configuring the device. */
typedef union {
    struct PACKED device_data {
        uint8_t  dwm_channel;
        uint8_t  dwm_data_rate;
        uint8_t  dwm_preamble_len;
        uint8_t  dwm_sfd_type;
        uint8_t  device_id[16];
        uint8_t  date[20];
        uint16_t position_update_period;
        uint8_t  wakeup_sensitivity;
        uint16_t notification_poll_period;
        uint16_t awake_period;
        uint8_t  alert_led[3];
        uint8_t  danger_led[3];
        uint8_t  notification_led[3];
        uint8_t  badge_state;
        bool     is_profile_screen_built;
        bool     is_weather_screen_built;
        bool     is_notifications_screen_built;
        uint8_t  notification_state;
        uint8_t  file_update_state;
        uint8_t  targeted_notification_state;
        uint8_t  wifi_ssid[40];
        uint8_t  wifi_pass[40];
    } config;
    uint8_t buffer[sizeof(struct device_data)];
} device_configuration_t;

/** @brief This enumeration contains the indexes for the device configuration operations. */
typedef enum {
    CONFIGURATION_MANAGER_INDEX_DWM_CHANNEL,
    CONFIGURATION_MANAGER_INDEX_DWM_DATA_RATE,
    CONFIGURATION_MANAGER_INDEX_DWM_PREAMBLE_LENGTH,
    CONFIGURATION_MANAGER_INDEX_DWM_SFD_TYPE,
    CONFIGURATION_MANAGER_INDEX_DEVICE_ID,
    CONFIGURATION_MANAGER_INDEX_DATE,
    CONFIGURATION_MANAGER_INDEX_MOVEMENT_THRESHOLD,
    CONFIGURATION_MANAGER_INDEX_PING_INTERVAL,
    CONFIGURATION_MANAGER_INDEX_ALERT_MODE,
    CONFIGURATION_MANAGER_INDEX_DANGER_MODE,
    CONFIGURATION_MANAGER_INDEX_NOTIFICATION_MODE,
    CONFIGURATION_MANAGER_INDEX_ALERT_MODE_ACTIVE,
} device_configuration_index_e;

/** @brief This enumeration contains the indexes for the NFC properties list. */
typedef enum {
    NFC_PROPERTY_INDEX_NAME,
    NFC_PROPERTY_INDEX_SURNAME,
    NFC_PROPERTY_INDEX_FUNCTION,
    NFC_PROPERTY_INDEX_ISSUE_DATE,
    NFC_PROPERTY_INDEX_CITY,
    NFC_PROPERTY_INDEX_AREA,
    NFC_PROPERTY_INDEX_DWM_CHANNEL,
    NFC_PROPERTY_INDEX_DWM_DATA_RATE,
    NFC_PROPERTY_INDEX_DWM_PREAMBLE_LENGTH,
    NFC_PROPERTY_INDEX_DWM_SFD_TYPE,
    NFC_PROPERTY_INDEX_WIFI_SSID,
    NFC_PROPERTY_INDEX_WIFI_PASSWORD,
    NFC_PROPERTY_INDEX_DEVICE_ID,
} nfc_property_index_e;

/** @brief This enumeration contains the indexes for the SD CARD properties list. */
typedef enum {
    SD_PROPERTY_INDEX_NAME,
    SD_PROPERTY_INDEX_SURNAME,
    SD_PROPERTY_INDEX_FUNCTION,
    SD_PROPERTY_INDEX_ISSUE_DATE,
    SD_PROPERTY_INDEX_CITY,
    SD_PROPERTY_INDEX_AREA,
    SD_PROPERTY_INDEX_DWM_CHANNEL,
    SD_PROPERTY_INDEX_DWM_DATA_RATE,
    SD_PROPERTY_INDEX_DWM_PREAMBLE_LENGTH,
    SD_PROPERTY_INDEX_DWM_SFD_TYPE,
    SD_PROPERTY_INDEX_WIFI_SSID,
    SD_PROPERTY_INDEX_WIFI_PASSWORD,
    SD_PROPERTY_INDEX_DEVICE_ID,
} sd_card_property_index_e;

/** @brief This enumeration contains possible actions that can be performed when dealing with notification data. */
typedef enum {
    CONFIGURATION_MANAGER_NO_ACTION,       /*!< no action should pe performed */
    CONFIGURATION_MANAGER_UPDATE_TIMELINE, /*!< the timeline should be updated by 1 day */
    CONFIGURATION_MANAGER_FORMAT_ENTRY,    /*!< data should be formatted and retrieved */
} configuration_manager_action_e;

#endif
#ifndef BADGE_DEFINES_H
#define BADGE_DEFINES_H

#include <stdint.h>

#define PROFILE_SCREEN_TXT_FILENAME       (uint8_t *)"/screen1.txt"      /*!< profile screen .txt file name */
#define NOTIFICATIONS_SCREEN_TXT_FILENAME (uint8_t *)"/screen2.txt"      /*!< notifications screen .txt file name */
#define WEATHER_SCREEN_TXT_FILENAME       (uint8_t *)"/screen3.txt"      /*!< weather screen .txt file name */
#define CONFIGURATION_FILE_TXT_FILENAME   (uint8_t *)"/config.txt"       /*!< weather screen .txt file name */
#define PROFILE_SCREEN_BMP_FILENAME       (uint8_t *)"/background.bmp"   /*!< default background file name */
#define NOTIFICATIONS_SCREEN_BMP_FILENAME (uint8_t *)"/notification.bmp" /*!< notification background filename */
#define WEATHER_SCREEN_BMP_FILENAME       (uint8_t *)"/weather.bmp"      /*!< weather background file name */
#define FULL_CONFIGURATION_TXT_FILENAME \
    (uint8_t *)"/full_config.txt" /*!< file containing the configuration, profile and weather data for fallback */

#define PROFILE_SCREEN_PALCEHOLDER_BMP_FILENAME (uint8_t *)"/placeholder.bmp" /*!< placeholder image .bmp file name */
#define PROFILE_SCREEN_USER_BMP_FILENAME        (uint8_t *)"/profile.bmp"     /*!< user image .bmp file name */
#define PROFILE_SCREEN_QR_BMP_FILENAME          (uint8_t *)"/qr.bmp" /*!< profile screen qr code .bmp file name */

#define BATTERY_4_LINES_IMAGE_BMP_FILENAME (uint8_t *)"/bat4.bmp" /*!< 4/4 battery icon .bmp filename */
#define BATTERY_3_LINES_IMAGE_BMP_FILENAME (uint8_t *)"/bat3.bmp" /*!< 3/4 battery icon .bmp filename */
#define BATTERY_2_LINES_IMAGE_BMP_FILENAME (uint8_t *)"/bat2.bmp" /*!< 2/4 battery icon .bmp filename */
#define BATTERY_1_LINES_IMAGE_BMP_FILENAME (uint8_t *)"/bat1.bmp" /*!< 1/4 battery icon .bmp filename */
#define BATTERY_0_LINES_IMAGE_BMP_FILENAME (uint8_t *)"/bat0.bmp" /*!< 0/4 battery icon .bmp filename */

/** weather screen - weather condition messages. these messages are stored inside an array, their index matching the
 * described weather conditions .bmp file name. */
#define WEATHER_SCREEN_MSG_CLEAR            (uint8_t *)"CLEAR"
#define WEATHER_SCREEN_MSG_FEW_CLOUDS       (uint8_t *)"FEW CLOUDS"
#define WEATHER_SCREEN_MSG_SCATTERED_CLOUDS (uint8_t *)"SCATTERED CLOUDS"
#define WEATHER_SCREEN_MSG_BROKEN_CLOUDS    (uint8_t *)"BROKEN CLOUDS"
#define WEATHER_SCREEN_MSG_SHOWER_RAIN      (uint8_t *)"SHOWER RAIN"
#define WEATHER_SCREEN_MSG_RAIN             (uint8_t *)"RAIN"
#define WEATHER_SCREEN_MSG_THUNDERSTORM     (uint8_t *)"THUNDERSTORM"
#define WEATHER_SCREEN_MSG_SNOW             (uint8_t *)"SNOW"
#define WEATHER_SCREEN_MSG_FOG              (uint8_t *)"FOG"

#define BATTERY_4_LINES_THRESHOLD 80 /*!< threshold for 4/4 battery icon */
#define BATTERY_3_LINES_THRESHOLD 60 /*!< threshold for 3/4 battery icon */
#define BATTERY_2_LINES_THRESHOLD 30 /*!< threshold for 2/4 battery icon */
#define BATTERY_1_LINES_THRESHOLD 10 /*!< threshold for 1/4 battery icon, if below, empty battery image*/

#define BATTERY_ICON_X_COORD 220 /*!< x position for battery icon display */
#define BATTERY_ICON_Y_COORD 5   /*!< y position for battery icon display */

#define REPORT_TIME_DIFFERENCE 3600 /*!< time difference between different report tranmissions */

#define BADGE_FILE_SETTINGS_BITMASK 0b11110000 /*!< used for extracting the file command */

typedef enum {
    BADGE_FILE_COMMAND_DOWNLOAD,
    BADGE_FILE_COMMAND_FIRMWARE_UPDATE,
    BADGE_FILE_COMMAND_UPDATE_CONFIGURATION,
    BADGE_FILE_COMMAND_UPLOAD
} badge_file_command_e;

/** @brief This enumeration contains a list of all the available system command. */
typedef enum {
    SYSTEM_CMD_TCP_CONNECT,                     /*!< command - connect device to server */
    SYSTEM_CMD_TCP_STATUS,                      /*!< command - status of the latest sent command(if needed) */
    SYSTEM_CMD_TCP_CONFIGURE_ANCHOR,            /*!< command - configure/re-configure anchor */
    SYSTEM_CMD_TCP_REQUEST_BADGE_CONFIGURATION, /*!< command - request badge configuration */
    SYSTEM_CMD_TCP_CONFIGURE_BADGE,             /*!< command - configure/re-configure badge */
    SYSTEM_CMD_UWB_BLINK,                       /*!< command - blink signal */
    SYSTEM_CMD_UWB_SYNC,                        /*!< command - sync signal */
    SYSTEM_CMD_TCP_BLINK_REPORT,                /*!< command - send blink report */
    SYSTEM_CMD_TCP_SYNC_RX_REPORT,              /*!< command - send sync rx report */
    SYSTEM_CMD_TCP_SYNC_TX_REPORT,              /*!< command - send sync tx report */
    SYSTEM_CMD_UWB_SEND_NOTIFICATION,           /*!< command - send badge notification */
    SYSTEM_CMD_UWB_REQUEST_NOTIFICATION,        /*!< command - request badge notification */
    SYSTEM_CMD_TCP_SEND_NOTIFICATION,           /*!< command - send badge notification from server to anchor */
    SYSTEM_CMD_UWB_REQUEST_BADGE_CONFIGURATION, /*!< command - request badge configuration */
    SYSTEM_CMD_UWB_SEND_BADGE_CONFIGURATION,    /*!< command - send badge configuration from anchor to badge */
    SYSTEM_CMD_UWB_REQUEST_NEW_BADGE_ID,        /*!< command - request new badge id */
    SYSTEM_CMD_UWB_SEND_NEW_BADGE_ID,           /*!< command - send new badge id from anchor to badge*/
    SYSTEM_CMD_TCP_REQUEST_NEW_BADGE_ID,        /*!< command - request new badge id from the server */
    SYSTEM_CMD_TCP_SEND_NEW_BADGE_ID,           /*!< command - send new badge id from server to anchor */
    SYSTEM_CMD_TCP_TOGGLE_PINPOINT_MODE,        /*!< command - toggle pinpoint mode */
    SYSTEM_CMD_UWB_SEND_IMAGE_NOTIFICATION,     /*!< command - send targeted notification from anchor to badge */
    SYSTEM_CMD_TCP_SEND_IMAGE_NOTIFICATION,     /*!< command - send targeted notification from server to anchor */
    SYSTEM_CMD_TCP_STOP_IMAGE_NOTIFICATION,     /*!< command - send stop signal for a specific targeted notification */
    SYSTEM_CMD_TCP_SEND_BATTERY_PERCENTAGE,     /*!< command - send the battery percentage of a badge to the server  */
    SYSTEM_CMD_TCP_SEND_BADGE_FIRMWARE_VERSION, /*!< command - send the firmware version of a badge to the server */
    SYSTEM_CMD_TCP_SEND_NOTIFICATION_TARGET,    /*!< command - send the notification target from server to anchor */
    SYSTEM_CMD_UWB_SEND_REPORT_BME_DATA,        /*!< command - send the bme sensor readings of a badge to the anchor  */
    SYSTEM_CMD_TCP_SEND_REPORT_BME_DATA,        /*!< command - send the bme sensor readings of a badge to the server  */
    SYSTEM_CMD_TCP_SEND_FILE_IMAGE,             /*!< command - send file image adress to anchor */
    SYSTEM_CMD_UWB_STATUS,                      /*!< command - send badge status */
    SYSTEM_CMD_TCP_FILE_REQUEST_HEADERS,        /*!< command - send the access token for transfering files to badge */
    SYSTEM_CMD_TCP_SEND_ANCHOR_DATA             /*!< command - send the BME and battery data from the anchor */
} system_cmd_t;

/**
 * @brief This enumeration contains the possible states that the badge can be found in.
 */
typedef enum {
    BADGE_STATE_PROFILE,
    BADGE_STATE_NOTIFICATIONS,
    BADGE_STATE_WEATHER,
    BADGE_STATE_NONE,
} badge_state_e;

#endif
#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdlib.h>

#define PROJ_LOG_LEVEL_DEBUG   0
#define PROJ_LOG_LEVEL_INFO    1
#define PROJ_LOG_LEVEL_WARNING 2
#define PROJ_LOG_LEVEL_ERROR   3

#define LOG_LEVEL PROJ_LOG_LEVEL_INFO

#define LOG_DEBUG(y, ...)   (LOG_DATA(LEVEL_DEBUG, MODULE_TAG, y, ##__VA_ARGS__))
#define LOG_INFO(y, ...)    (LOG_DATA(LEVEL_INFO, MODULE_TAG, y, ##__VA_ARGS__))
#define LOG_WARNING(y, ...) (LOG_DATA(LEVEL_WARNING, MODULE_TAG, y, ##__VA_ARGS__))
#define LOG_ERROR(y, ...)   (LOG_DATA(LEVEL_ERROR, MODULE_TAG, y, ##__VA_ARGS__))

#define FORMAT_LOG(format, ...)           format_log(format, ##__VA_ARGS__)
#define LOG_DATA(level, tag, format, ...) log_event_handler(level, tag, FORMAT_LOG(format, ##__VA_ARGS__))

/** @brief This enumeration provides the log types. These log types will be compared to the log level that has been set
 * in order to decide if the specified log will be displayed or not. */
typedef enum {
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR,
} log_levels_e;

/**
 * @brief This function formats the log message into a single string including all the arguments that
 *        are passed to it.
 *
 * @param[in]   format  The string that describes how the output should look.
 * @param[in]   ...     Undefined number of arguments. Similar to va_list.
 *
 * @return (char *)
 * @retval formatted string based on the provided parameters
 */
char *format_log(char *format, ...);

/**
 * @brief This function is used to log the message given formatted message. It compares the log level inside the `level`
 *        parameter with the `LOG_LEVEL` macro (if it was previously defined). If the level is higher, the message will
 *        be logged. If the level is lower, then the message will be ignored.
 *
 * @param[in]   level   This parameter describes the level of the current log message.
 * @param[in]   tag     This parameter describes the origin of the log message.
 * @param[in]   message This parameter describes the log message itself.
 *
 * @return (void)
 */
void log_event_handler(log_levels_e level, char *tag, char *message);

#endif
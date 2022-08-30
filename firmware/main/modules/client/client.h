#ifndef CLIENT_H
#define CLIENT_H
#define CLIENT_POST_CMD_SEND 100
#define CLIENT_POST_CMD_HALT 400

#include "error_manager.h"
#include "log.h"
#include "lwip/sockets.h"

typedef struct {
    uint16_t     command;
    char         localFileName[64];
    char         remoteFileName[64];
    TaskHandle_t taskHandle;
} client_request_handle;

typedef struct {
    uint16_t     command;
    char         response[256];
    TaskHandle_t taskHandle;
} client_response_handle;

/**
 * @brief This enumeration contains the valid types of requests that the http client driver can perform.
 */
typedef enum {
    CLIENT_REQUEST_TYPE_GET,
    CLIENT_REQUEST_TYPE_POST,
    CLIENT_REQUEST_TYPE_PUT,
} client_request_type_e;

/**
 * @brief This structure contains data about the http client request.
 */

typedef struct {
    uint8_t               host[40];               /*!< ip/address of the target server */
    uint8_t               path[40];               /*!< URI path to target files */
    uint8_t               token[40];              /*!< server authorization token */
    uint8_t               formatted_filename[64]; /*!< name of the formatted file */
    uint8_t               filename[24];           /*!< name of the file to be retrieved */
    uint16_t              port;                   /*!< port of the target server */
    client_request_type_e type;                   /*!< type of request to be performed */
    uint8_t               file_command;           /*!< type of file command (see `badge_file_command_e`) */
    uint8_t               badge_id[16];           /*!< id of the seeder badge */
} client_request_t;

/**
 * @brief This function is used to perform a POST method to a remote server. Details on this method should be contained
 *        inside the `post` structure.
 *
 * @param[in] post Structure that contains details about how the post is made.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t client_http_post(client_request_t *post);

/**
 * @brief This function is used to perform a GET request to a remote server. Details on the request should be
 * contained inside the `request` structure.
 *
 * @param[in] request Structure that contains details about how the request is made.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t client_http_get(client_request_t *request);

/**
 * @brief This function is used to start a connection stream over HTTP.
 *
 * @param[in] request Structure that contains details about how the request is made.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t client_http_stream(client_request_t *request);

#endif
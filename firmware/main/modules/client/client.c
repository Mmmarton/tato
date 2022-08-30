#include "client.h"
#include "display_manager.h"
#include "esp_event.h"
#include "esp_http_client.h"

#include "badge_defines.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#define MODULE_TAG           "CLIENT"
#define BOUNDARY             "X-ESPIDF_MULTIPART"
#define MAX_HTTP_RECV_BUFFER 4096
#define DEVICE_UUID_LENGTH   16
#define MAX_STREAM_WRITE     1024

static uint8_t          request_types[] = {HTTP_METHOD_GET, HTTP_METHOD_POST, HTTP_METHOD_PUT};
static client_request_t request_details;

void extract_header(char *formatted_header, client_request_t *post, char *formatted_body, char *formatted_end,
                    uint32_t buffer_size);
void extract_body(char *formatted_body, client_request_t *post);
void extract_end(char *formatted_header);

static int request_driver_handle(esp_http_client_event_t *request_event) {
    static char *output_buffer = NULL;
    static int   output_len    = 0;

    switch (request_event->event_id) {
        case HTTP_EVENT_ERROR:
            display_manager_render_error_message((uint8_t *)"Image update failed", 20);
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;

        case HTTP_EVENT_ON_DATA:
            LOG_INFO("HTTP_EVENT_ON_DATA, length = %d", request_event->data_len);
            if (!esp_http_client_is_chunked_response(request_event->client)) {
                if (request_event->user_data) {
                    memcpy(request_event->user_data + output_len, request_event->data, request_event->data_len);
                }
                else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *)malloc(esp_http_client_get_content_length(request_event->client));
                        output_len    = 0;

                        if (output_buffer == NULL) {
                            LOG_ERROR("Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, request_event->data, request_event->data_len);
                }
                output_len += request_event->data_len;
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            if (output_buffer != NULL && strlen((char *)request_details.filename)) {
                if (request_details.file_command == BADGE_FILE_COMMAND_DOWNLOAD) {
                    storage_write_bmp_file(request_details.filename, (uint8_t *)output_buffer, output_len);
                }
                else if (request_details.file_command == BADGE_FILE_COMMAND_UPDATE_CONFIGURATION) {
                    storage_write_txt_file((uint8_t *)request_details.filename, STORAGE_WRITE_BINARY_MODE_REPLACE,
                                           (uint8_t *)output_buffer, output_len);
                }
            }

            free(output_buffer);
            output_len    = 0;
            output_buffer = NULL;

            break;

        case HTTP_EVENT_DISCONNECTED:
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        default:
            break;
    }
    return ESP_OK;
}

status_t client_http_post(client_request_t *post) {
    QueueHandle_t queue_request;
    QueueHandle_t queue_response;
    queue_request  = xQueueCreate(1, sizeof(client_request_handle));
    queue_response = xQueueCreate(1, sizeof(client_response_handle));
    configASSERT(queue_request);
    configASSERT(queue_response);

    const struct addrinfo hints = {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *      response;
    char                   receive_buffer[64];
    client_response_handle response_buffer;

    char port[7];
    memset(port, 0, sizeof(port));
    sprintf(port, "%d", post->port);
    int err = getaddrinfo((char *)post->host, port, &hints, &response);

    if (err != 0 || response == NULL) {
        strcpy(response_buffer.response, "DNS lookup failed");
        xQueueSend(queue_response, &response_buffer, 10);
        LOG_ERROR("DNS lookup failed err=%d res=%p", err, response);
        return STATUS_CLIENT_COULD_NOT_POST;
    }

    uint32_t buffer_size = 0;
    storage_get_file_size(post->formatted_filename, STORAGE_READ_BINARY_MODE, &buffer_size);

    char formatted_body[512];
    extract_body(formatted_body, post);

    char formatted_end[128];
    extract_end(formatted_end);

    char formatted_header[512];
    extract_header(formatted_header, post, formatted_body, formatted_end, buffer_size);

    int32_t socket_connection = socket(response->ai_family, response->ai_socktype, 0);
    if (socket_connection < 0) {
        strcpy(response_buffer.response, "Failed to allocate socket");
        xQueueSend(queue_response, &response_buffer, 10);
        LOG_ERROR("... Failed to allocate socket.");
        freeaddrinfo(response);

        return STATUS_CLIENT_COULD_NOT_POST;
    }

    LOG_INFO("... allocated socket");

    if (connect(socket_connection, response->ai_addr, response->ai_addrlen) != 0) {
        strcpy(response_buffer.response, "socket connect failed");
        xQueueSend(queue_response, &response_buffer, 10);
        LOG_ERROR("... socket connect failed errno=%d", errno);
        close(socket_connection);
        freeaddrinfo(response);

        return STATUS_CLIENT_COULD_NOT_POST;
    }

    LOG_INFO("... connected");
    freeaddrinfo(response);

    if (write(socket_connection, formatted_header, strlen(formatted_header)) < 0) {
        LOG_ERROR("... socket send failed");
        close(socket_connection);

        return STATUS_CLIENT_COULD_NOT_POST;
    }
    LOG_INFO("HEADER socket send success");

    if (write(socket_connection, formatted_body, strlen(formatted_body)) < 0) {
        LOG_ERROR("... socket send failed");
        close(socket_connection);

        return STATUS_CLIENT_COULD_NOT_POST;
    }
    LOG_INFO("BODY socket send success");

    uint8_t  buffer[MAX_STREAM_WRITE] = {};
    uint32_t read_size;
    for (size_t i = 0; i < buffer_size; i = i + MAX_STREAM_WRITE) {
        storage_read_file_sequences(post->formatted_filename, STORAGE_READ_BINARY_MODE, buffer, MAX_STREAM_WRITE, i,
                                    &read_size);

        if (write(socket_connection, buffer, read_size) < 0) {
            LOG_ERROR("... socket send failed");
            close(socket_connection);
            return STATUS_CLIENT_COULD_NOT_POST;
        }
    }
    LOG_INFO("DATA socket send success");

    if (write(socket_connection, formatted_end, strlen(formatted_end)) < 0) {
        LOG_ERROR("... socket send failed");
        close(socket_connection);

        return STATUS_CLIENT_COULD_NOT_POST;
    }
    LOG_INFO("END socket send success");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec  = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(socket_connection, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) {
        LOG_ERROR("... failed to set socket receiving timeout");
        close(socket_connection);

        return STATUS_CLIENT_COULD_NOT_POST;
    }
    LOG_INFO("... set socket receiving timeout success");

    int32_t read_data;
    bzero(response_buffer.response, sizeof(response_buffer.response));
    do {
        bzero(receive_buffer, sizeof(receive_buffer));
        LOG_INFO("Start read now=%d", xTaskGetTickCount());
        read_data = read(socket_connection, receive_buffer, sizeof(receive_buffer) - 1);
        LOG_INFO("End read now=%d read data=%d", xTaskGetTickCount(), read_data);
        strcat(response_buffer.response, receive_buffer);
    } while (read_data > 0);

    LOG_INFO("socket: last read return=%d errno=%d.", read_data, errno);
    close(socket_connection);

    return STATUS_OK;
}

status_t client_http_get(client_request_t *request) {
    memcpy(request_details.host, request->host, sizeof(request->host));
    memcpy(request_details.path, request->path, sizeof(request->path));
    memcpy(request_details.formatted_filename, request->formatted_filename, sizeof(request->formatted_filename));
    memcpy(request_details.filename, request->filename, sizeof(request->filename));
    memcpy(request_details.token, request->token, sizeof(request->token));
    request_details.file_command = request->file_command;
    request_details.type         = request->type;
    request_details.port         = request->port;

    char formatted_path[sizeof(request->path) + sizeof(request->formatted_filename)];
    memset(formatted_path, 0, sizeof(formatted_path));

    printf("%s:%d%s\n", request->host, request->port, request->path);
    sprintf(formatted_path, "%s/%s", request->path, request->formatted_filename);
    esp_http_client_config_t client_configuration = {
        .host          = (char *)request->host,
        .path          = formatted_path,
        .port          = request->port,
        .event_handler = request_driver_handle,
    };

    esp_http_client_handle_t client_handler = esp_http_client_init(&client_configuration);
    esp_http_client_set_method(client_handler, request_types[request->type]);
    esp_http_client_set_header(client_handler, "Content-Type", "*/*");
    esp_http_client_set_header(client_handler, "Authorization", (char *)request->token);

    int error_check = esp_http_client_perform(client_handler);
    if (error_check == ESP_OK) {
        LOG_INFO("HTTP GET Status = %d, content_length = %d", esp_http_client_get_status_code(client_handler),
                 esp_http_client_get_content_length(client_handler));

        esp_http_client_cleanup(client_handler);
        return STATUS_OK;
    }

    esp_http_client_cleanup(client_handler);
    return STATUS_CLIENT_COULD_NOT_GET;
}

status_t client_http_stream(client_request_t *request) {
    memcpy(request_details.host, request->host, sizeof(request->host));
    memcpy(request_details.path, request->path, sizeof(request->path));
    memcpy(request_details.formatted_filename, request->formatted_filename, sizeof(request->formatted_filename));
    memcpy(request_details.filename, request->filename, sizeof(request->filename));
    memcpy(request_details.token, request->token, sizeof(request->token));
    request_details.file_command = request->file_command;
    request_details.type         = request->type;
    request_details.port         = request->port;

    char formatted_path[sizeof(request->host) + sizeof(request->path) + sizeof(request->formatted_filename) + 24];
    memset(formatted_path, 0, sizeof(formatted_path));

    sprintf(formatted_path, "http://%s:%d%s%s", request->host, request->port, request->path, request->filename);
    esp_http_client_config_t client_configuration = {
        .url = formatted_path,
    };

    esp_http_client_handle_t client_handler = esp_http_client_init(&client_configuration);
    esp_http_client_set_header(client_handler, "Authorization", (char *)request->token);

    esp_err_t err;
    if ((err = esp_http_client_open(client_handler, 0)) != ESP_OK) {
        LOG_ERROR("Failed to open HTTP connection: %s", esp_err_to_name(err));
        return STATUS_CLIENT_COULD_NOT_SETUP_STREAM;
    }
    int content_length = esp_http_client_fetch_headers(client_handler);

    int offset = 0;
    while (offset != content_length) {
        uint8_t chunk[MAX_HTTP_RECV_BUFFER];
        int     len = esp_http_client_read(client_handler, (char *)chunk, MAX_HTTP_RECV_BUFFER);

        if (len == -1) {
            display_manager_render_error_message((uint8_t *)"Firmware update failed", 23);
        }

        if (offset == 0) {
            storage_write_binary_file(request_details.filename, STORAGE_WRITE_BINARY_MODE_REPLACE, chunk, len);
        }
        else {
            storage_write_binary_file(request_details.filename, STORAGE_WRITE_BINARY_MODE_APPEND, chunk, len);
        }

        offset += len;
    }

    LOG_INFO("HTTP Stream reader Status = %d, content_length = %lld", esp_http_client_get_status_code(client_handler),
             esp_http_client_get_content_length(client_handler));
    esp_http_client_close(client_handler);
    esp_http_client_cleanup(client_handler);

    return STATUS_OK;
}

void extract_header(char *formatted_header, client_request_t *post, char *formatted_body, char *formatted_end,
                    uint32_t buffer_size) {
    uint8_t uuid[32];
    for (size_t i = 0; i < DEVICE_UUID_LENGTH; i++) {
        sprintf((char *)uuid + (i * 2), "%02x", post->badge_id[i]);
    }

    char badge_id[sizeof(uuid) + 5];
    memset(badge_id, 0, sizeof(badge_id));
    memcpy(badge_id, uuid, 8);
    memcpy(badge_id + 8, "-", 1);
    memcpy(badge_id + 9, uuid + 8, 4);
    memcpy(badge_id + 13, "-", 1);
    memcpy(badge_id + 14, uuid + 12, 4);
    memcpy(badge_id + 18, "-", 1);
    memcpy(badge_id + 19, uuid + 16, 4);
    memcpy(badge_id + 23, "-", 1);
    memcpy(badge_id + 24, uuid + 20, 12);

    char header[128];
    sprintf(header, "POST %s/%s HTTP/1.1\r\n", post->path, badge_id);
    strcpy(formatted_header, header);
    sprintf(header, "Host: %s:%d\r\n", post->host, post->port);
    strcat(formatted_header, header);
    sprintf(header, "User-Agent: esp-idf/%d.%d.%d esp32\r\n", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR,
            ESP_IDF_VERSION_PATCH);
    strcat(formatted_header, header);
    sprintf(header, "Accept: */*\r\n");
    strcat(formatted_header, header);
    sprintf(header, "Content-Type: multipart/form-data; boundary=%s\r\n", BOUNDARY);
    strcat(formatted_header, header);

    uint32_t data_length = strlen(formatted_body) + strlen(formatted_end) + buffer_size;
    sprintf(header, "Content-Length: %d\r\n\r\n", data_length);
    strcat(formatted_header, header);
}

void extract_body(char *formatted_body, client_request_t *post) {
    char header[128];
    sprintf(header, "--%s\r\n", BOUNDARY);
    strcpy(formatted_body, header);
    sprintf(header, "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n", post->formatted_filename);
    strcat(formatted_body, header);
    sprintf(header, "Content-Type:application/octet-stream\r\n\r\n");
    strcat(formatted_body, header);
    strcat(formatted_body, "\0");
}

void extract_end(char *formatted_end) {
    char header[128];
    sprintf(header, "\r\n--%s--\r\n\r\n", BOUNDARY);
    strcpy(formatted_end, header);
    strcat(formatted_end, "\0");
}
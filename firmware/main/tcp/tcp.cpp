#include "tcp.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TAG ("TCP")

static int  socketId;
static bool initialized = false;

int8_t TCP::init(char *address, uint16_t port) {
    int addr_family = 0;
    int ip_protocol = 0;

    ESP_ERROR_CHECK(esp_netif_init());

    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, address, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port   = htons(port);
    addr_family          = AF_INET;
    ip_protocol          = IPPROTO_IP;

    socketId = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (socketId < 0) {
        ESP_LOGE(TAG, "Socket creation failed");
        return TCP_STATUS_SOCKET_CREATION_FAIL;
    }

    int err = connect(socketId, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Connection failed %d", err);
        return TCP_STATUS_CONNECTION_FAIL;
    }

    initialized = true;
    return TCP_STATUS_OK;
}

int8_t TCP::write(char *data) {
    if (!initialized) {
        return TCP_STATUS_OK;
    }

    uint8_t size = send(socketId, data, strlen(data), 0);

    if (size != strlen(data)) {
        ESP_LOGE(TAG, "Write failed");
        return TCP_STATUS_IO_ERROR;
    }

    return TCP_STATUS_OK;
}

int8_t TCP::read(char *buffer, uint8_t &length) {
    if (!initialized) {
        return TCP_STATUS_OK;
    }

    int8_t receivedLength = recv(socketId, buffer, sizeof(buffer) - 1, 0);
    if (receivedLength < 0) {
        ESP_LOGE(TAG, "Read failed");
        return TCP_STATUS_IO_ERROR;
    }

    buffer[receivedLength++] = 0;
    length                   = receivedLength;

    return TCP_STATUS_OK;
}

int8_t TCP::end(void) {
    if (!initialized) {
        return TCP_STATUS_OK;
    }

    if (socketId == -1) {
        ESP_LOGE(TAG, "End failed");
        return TCP_STATUS_INVALID_SOCKET;
    }

    shutdown(socketId, 0);
    close(socketId);

    return TCP_STATUS_OK;
}
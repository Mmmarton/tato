#ifndef _TCP_H
#define _TCP_H

#include <stdint.h>

#define TCP_STATUS_OK                   0
#define TCP_STATUS_SOCKET_CREATION_FAIL 1
#define TCP_STATUS_IO_ERROR             2
#define TCP_STATUS_INVALID_SOCKET       3
#define TCP_STATUS_CONNECTION_FAIL      4

namespace TCP {
int8_t init(char *address, uint16_t port);
int8_t write(char *data);
int8_t read(char *buffer, uint8_t &length);
int8_t end(void);
}; // namespace TCP

#endif
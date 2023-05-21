#ifndef _WIFI_H
#define _WIFI_H

#include <stdint.h>

#define RETRY_COUNT 5

namespace WIFI {
void init(char *ssid, char *pass);
}; // namespace WIFI

#endif
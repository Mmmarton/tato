#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <esp_http_server.h>

void start_websocket();
httpd_ws_frame_t *ws_event_handler(uint8_t *message);

#endif
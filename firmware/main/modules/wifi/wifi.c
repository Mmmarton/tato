#include "wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <string.h>

#define MODULE_TAG "WIFI"

#define WIFI_CONNECTED_BIT BIT0 /*!< bit for successful connection */
#define WIFI_FAIL_BIT      BIT1 /*!< bit for failed connection */

static bool is_wifi_connected = false;
static bool is_init           = false;

static uint8_t            retry_counter = 0;
static EventGroupHandle_t wifi_event_group;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

status_t wifi_init(uint8_t *network_ssid, uint8_t *network_pass) {
    if (is_init == true) {
        LOG_WARNING("module already init");
        return STATUS_ALREADY_INIT;
    }

    esp_err_t flash_init_error = nvs_flash_init();
    if (flash_init_error == ESP_ERR_NVS_NO_FREE_PAGES || flash_init_error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        flash_init_error = nvs_flash_init();
    }

    ESP_ERROR_CHECK(flash_init_error);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t wifi_default_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_default_config));

    wifi_config_t user_config = {.sta = {.threshold.authmode = WIFI_AUTH_WPA2_PSK,
                                         .pmf_cfg            = {
                                             .capable  = true,
                                             .required = false,
                                         }}};

    memcpy(user_config.sta.ssid, network_ssid, strlen((char *)network_ssid));
    memcpy(user_config.sta.password, network_pass, strlen((char *)network_pass));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &user_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_event_group = xEventGroupCreate();

    EventBits_t bits
        = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if ((bits && WIFI_CONNECTED_BIT) && is_wifi_connected) {
        LOG_INFO("connected to SSID: '%s'", (char *)network_ssid);

        is_init = true;
        return STATUS_OK;
    }

    return STATUS_WIFI_COULD_NOT_CONNECT;
}

status_t wifi_stop(void) {
    CHECK_INIT(is_init);

    esp_wifi_stop();
    esp_wifi_deinit();

    is_init = false;
    return STATUS_OK;
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_counter < 10) {
            esp_wifi_connect();
            retry_counter++;
            LOG_WARNING("could not connect the the AP");
        }
        else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            LOG_ERROR("could not establish a connection to the requested AP");
            if (is_wifi_connected) {
                ESP_ERROR_CHECK(esp_wifi_stop());
            }
            else {
                return;
            }
        }
        LOG_INFO("retrying... attempt nr. %d/%d", retry_counter, 10);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        retry_counter = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        is_wifi_connected = true;
    }
}

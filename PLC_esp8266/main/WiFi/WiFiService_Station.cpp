#include "HttpServer/http_server.h"
#include "WiFiService.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "settings.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

#define INFINITY_CONNECT_RETRY -1

static const char *TAG_WiFiService_Station = "WiFiService.Station";
extern device_settings settings;

EventBits_t WiFiService::StationTask() {
    int32_t max_retry_count;
    wifi_config_t wifi_config = {};

    SAFETY_SETTINGS(
        memcpy(wifi_config.sta.ssid, settings.wifi.ssid, sizeof(wifi_config.sta.ssid)); //
        memcpy(wifi_config.sta.password,
               settings.wifi.password,
               sizeof(wifi_config.sta.password));                //
        max_retry_count = settings.wifi.connect_max_retry_count; //
    );

    bool has_wifi_sta_settings = wifi_config.sta.ssid[0] != 0;
    if (!has_wifi_sta_settings) {
        ESP_LOGW(TAG_WiFiService_Station, "no creds saved");
        return 0;
    }

    ESP_LOGI(TAG_WiFiService_Station, "start");

    int connect_retry_num = 0;
    EventBits_t uxBits = 0;
    do {
        TickType_t start_loop_ticks = xTaskGetTickCount();

        if (uxBits == 0 || (uxBits & FAILED_BIT) != 0) {
            Connect(&wifi_config);
        }

        uxBits = xEventGroupWaitBits(event,
                                     CONNECTED_BIT | FAILED_BIT | STOP_BIT | NEW_REQUEST_BIT,
                                     true,
                                     false,
                                     portMAX_DELAY);

        ESP_LOGI(TAG_WiFiService_Station, "process, uxBits:0x%08X", uxBits);

        bool connected = (uxBits & CONNECTED_BIT) != 0;
        if (connected) {
            ESP_LOGI(TAG_WiFiService_Station, "Connected to ap");
            connect_retry_num = 0;
            start_http_server();
        }

        bool any_failure = (uxBits & FAILED_BIT) != 0;
        if (any_failure) {
            bool retry_connect =
                (max_retry_count == INFINITY_CONNECT_RETRY || connect_retry_num < max_retry_count);

            if (retry_connect) {
                connect_retry_num++;
                ESP_LOGI(TAG_WiFiService_Station,
                         "failed. reconnect, num:%d of %d",
                         connect_retry_num,
                         max_retry_count);
                Disconnect();
                vTaskDelayUntil(&start_loop_ticks, 1000 / portTICK_RATE_MS);
            } else {
                ESP_LOGI(TAG_WiFiService_Station, "failed. unable reconnect");
                break;
            }
        }

    } while (uxBits != 0 && (uxBits & (STOP_BIT | NEW_REQUEST_BIT)) == 0);

    stop_http_server();
    Disconnect();

    ESP_LOGW(TAG_WiFiService_Station, "finish");
    return uxBits;
}

void WiFiService::Connect(wifi_config_t *wifi_config) {
    ESP_LOGI(TAG_WiFiService_Station, "Connect");

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
     * However these modes are deprecated and not advisable to be used. Incase your Access point
     * doesn't support WPA2, these mode can be enabled by commenting below line */

    if (wifi_config->sta.password[0] != 0) {
        wifi_config->sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiService::Disconnect() {
    ESP_LOGI(TAG_WiFiService_Station, "Disconnect");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
}

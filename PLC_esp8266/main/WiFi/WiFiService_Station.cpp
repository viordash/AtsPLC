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

void WiFiService::StationTask(RequestItem *request) {
    ESP_LOGW(TAG_WiFiService_Station, "start");

    int connect_retry_num = 0;
    int32_t max_retry_count;
    wifi_config_t wifi_config = {};

    SAFETY_SETTINGS({
        memcpy(wifi_config.sta.ssid, settings.wifi_station.ssid, sizeof(wifi_config.sta.ssid)); //
        memcpy(wifi_config.sta.password,
               settings.wifi_station.password,
               sizeof(wifi_config.sta.password)); //
        max_retry_count = settings.wifi_station.connect_max_retry_count;
    });

    bool has_wifi_sta_settings = wifi_config.sta.ssid[0] != 0;
    if (!has_wifi_sta_settings) {
        ESP_LOGW(TAG_WiFiService_Station, "no creds saved");
        SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_Error);
        requests.RemoveStation();
        return;
    }

    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, this));

    EventBits_t uxBits = 0;
    while (true) {

        if (uxBits == 0 || (uxBits & FAILED_BIT) != 0) {
            Connect(&wifi_config);
        }

        uxBits = xEventGroupWaitBits(event,
                                     CONNECTED_BIT | FAILED_BIT | STOP_BIT | NEW_REQUEST_BIT
                                         | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     portMAX_DELAY);

        ESP_LOGI(TAG_WiFiService_Station, "event uxBits:0x%08X", uxBits);

        bool cancel = (uxBits & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_Station, "Cancel");
            SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_NoStation);
            stop_http_server();
            Disconnect();
            break;
        }

        bool connected = (uxBits & CONNECTED_BIT) != 0;
        if (connected) {
            ESP_LOGI(TAG_WiFiService_Station, "Connected to AP");
            SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_Connected);
            start_http_server();
            continue;
        }

        bool any_failure = (uxBits & FAILED_BIT) != 0;
        if (any_failure) {
            SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_NoStation);
            stop_http_server();
            Disconnect();

            bool retry_connect =
                (max_retry_count == INFINITY_CONNECT_RETRY || connect_retry_num < max_retry_count);

            if (retry_connect) {
                connect_retry_num++;
                ESP_LOGI(TAG_WiFiService_Station,
                         "failed. reconnect, num:%d of %d",
                         connect_retry_num,
                         max_retry_count);

                const TickType_t reconnect_delay = 3000 / portTICK_RATE_MS;
                xEventGroupWaitBits(event,
                                    STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT,
                                    false,
                                    false,
                                    reconnect_delay);

            } else {
                ESP_LOGI(TAG_WiFiService_Station, "failed. unable reconnect");
                break;
            }
        }
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler));
    ESP_ERROR_CHECK(
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));

    requests.RemoveStation();
    ESP_LOGW(TAG_WiFiService_Station, "finish");
}

void WiFiService::wifi_event_handler(void *arg,
                                     esp_event_base_t event_base,
                                     int32_t event_id,
                                     void *event_data) {
    (void)event_data;
    auto wifi_service = static_cast<WiFiService *>(arg);
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG_WiFiService_Station, "start wifi event");
            esp_wifi_connect();
            return;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG_WiFiService_Station, "connect to the AP fail");
            xEventGroupSetBits(wifi_service->event, WiFiService::FAILED_BIT);
            return;

        case WIFI_EVENT_STA_STOP:
            ESP_LOGI(TAG_WiFiService_Station, "stop wifi event");
            return;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG_WiFiService_Station, "wifi connected event");
            return;

        default:
            ESP_LOGW(TAG_WiFiService_Station,
                     "unhandled event, event_base:%s, event_id:%d",
                     event_base,
                     event_id);
            break;
    }
}

void WiFiService::ip_event_handler(void *arg,
                                   esp_event_base_t event_base,
                                   int32_t event_id,
                                   void *event_data) {
    auto wifi_service = static_cast<WiFiService *>(arg);
    auto event = (ip_event_got_ip_t *)event_data;

    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG_WiFiService_Station, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
            xEventGroupSetBits(wifi_service->event, CONNECTED_BIT);
            return;

        default:
            ESP_LOGW(TAG_WiFiService_Station,
                     "unhandled event, event_base:%s, event_id:%d",
                     event_base,
                     event_id);
            break;
    }
}
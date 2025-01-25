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

    int connect_retries_num = 0;
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

    bool has_connect = false;
    bool delay_before_reconnect = false;
    uint32_t ulNotifiedValue = 0;
    Connect(&wifi_config);

    while (true) {
        if (!delay_before_reconnect) {
            xTaskNotifyWait(0,
                            CANCEL_REQUEST_BIT | CONNECTED_BIT | FAILED_BIT,
                            &ulNotifiedValue,
                            portMAX_DELAY);
        } else {
            delay_before_reconnect = false;
        }
        ESP_LOGI(TAG_WiFiService_Station, "event uxBits:0x%08X", ulNotifiedValue);

        bool to_stop = (ulNotifiedValue & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        bool cancel = (ulNotifiedValue & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_Station, "Cancel");
            break;
        }
        bool one_more_request = requests.OneMoreInQueue();
        bool any_failure = (ulNotifiedValue & FAILED_BIT) != 0;
        if (any_failure) {
            bool retry_connect = (max_retry_count == INFINITY_CONNECT_RETRY
                                  || connect_retries_num < max_retry_count);
            if (!retry_connect) {
                ESP_LOGI(TAG_WiFiService_Station, "failed. unable reconnect");
                break;
            }
            has_connect = false;

            const int retries_num_before_no_station = 3;
            if (connect_retries_num >= retries_num_before_no_station) {
                SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_NoStation);
                if (one_more_request) {
                    ESP_LOGI(TAG_WiFiService_Station,
                             "Stop connecting to station due to new request");
                    break;
                }
            }

            connect_retries_num++;
            ESP_LOGI(TAG_WiFiService_Station,
                     "failed. reconnect, num:%d of %d",
                     connect_retries_num,
                     max_retry_count);

            stop_http_server();
            Disconnect();

            const TickType_t reconnect_delay = 3000 / portTICK_RATE_MS;
            delay_before_reconnect =
                xTaskNotifyWait(0,
                                CANCEL_REQUEST_BIT | CONNECTED_BIT | FAILED_BIT,
                                &ulNotifiedValue,
                                reconnect_delay)
                == pdFALSE;
            if (delay_before_reconnect) {
                Connect(&wifi_config);
                ulNotifiedValue = 0;
            }
            continue;
        }

        bool connected = (ulNotifiedValue & CONNECTED_BIT) != 0;
        if (connected) {
            ESP_LOGI(TAG_WiFiService_Station, "Connected to AP");
            SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_Connected);
            start_http_server();
            connect_retries_num = 0;
            has_connect = true;
        }

        if (one_more_request && has_connect) {
            ESP_LOGI(TAG_WiFiService_Station, "Disconnect station due to new request");
            break;
        }
    }
    stop_http_server();
    Disconnect();

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler));
    ESP_ERROR_CHECK(
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));

    const TickType_t wait_disconnection = 500 / portTICK_RATE_MS;
    if (xTaskNotifyWait(0, CONNECTED_BIT | FAILED_BIT, &ulNotifiedValue, wait_disconnection)
        == pdTRUE) {
        ESP_LOGI(TAG_WiFiService_Station, "fully disconnected");
    }

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
            xTaskNotify(wifi_service->task_handle, FAILED_BIT, eNotifyAction::eSetBits);
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
            xTaskNotify(wifi_service->task_handle, CONNECTED_BIT, eNotifyAction::eSetBits);
            return;

        default:
            ESP_LOGW(TAG_WiFiService_Station,
                     "unhandled event, event_base:%s, event_id:%d",
                     event_base,
                     event_id);
            break;
    }
}
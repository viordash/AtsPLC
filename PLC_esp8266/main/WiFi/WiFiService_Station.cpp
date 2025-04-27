#include "HttpServer/http_server.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
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

static const char *TAG_WiFiService_Station = "WiFiService.Station";
extern CurrentSettings::device_settings settings;

void WiFiService::StationTask(RequestItem *request) {
    ESP_LOGD(TAG_WiFiService_Station, "start");

    int connect_retries_num = 0;
    int32_t max_retry_count;
    uint32_t reconnect_delay_ms;
    uint32_t scan_station_rssi_period_ms;
    uint32_t min_worktime_ms;
    wifi_config_t wifi_config = {};

    SAFETY_SETTINGS({
        memcpy(wifi_config.sta.ssid, settings.wifi_station.ssid, sizeof(wifi_config.sta.ssid)); //
        memcpy(wifi_config.sta.password,
               settings.wifi_station.password,
               sizeof(wifi_config.sta.password)); //
        max_retry_count = settings.wifi_station.connect_max_retry_count;
        reconnect_delay_ms = settings.wifi_station.reconnect_delay_ms;
        scan_station_rssi_period_ms = settings.wifi_station.scan_station_rssi_period_ms;
        min_worktime_ms = settings.wifi_station.min_worktime_ms;
    });

    bool has_wifi_sta_settings = wifi_config.sta.ssid[0] != 0;
    if (!has_wifi_sta_settings) {
        ESP_LOGW(TAG_WiFiService_Station, "no creds saved");
        requests.RemoveStation();
        station_rssi = LogicElement::MinValue;
        return;
    }

    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, this));

    bool has_connect = false;
    uint64_t connection_start_time = 0;
    uint32_t ulNotifiedValue = 0;
    Connect(&wifi_config);

    bool cancel = false;
    while (true) {
        if (ulNotifiedValue == 0) {
            if (xTaskNotifyWait(0,
                                CANCEL_REQUEST_BIT | CONNECTED_BIT | FAILED_BIT,
                                &ulNotifiedValue,
                                scan_station_rssi_period_ms / portTICK_PERIOD_MS)
                != pdPASS) {
                if (has_connect && !ObtainStationRssi()) {
                    ulNotifiedValue = FAILED_BIT;
                }
            }
        }
        ESP_LOGD(TAG_WiFiService_Station, "event uxBits:0x%08X", ulNotifiedValue);
        uint32_t notified_event = ulNotifiedValue;
        ulNotifiedValue = 0;

        bool to_stop = (notified_event & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        cancel = (notified_event & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_Station, "Cancel");
            break;
        }

        bool one_more_request = requests.OneMoreInQueue();
        bool any_failure = (notified_event & FAILED_BIT) != 0;
        if (any_failure) {
            has_connect = false;
            uint32_t reconnect_delay;
            bool retry_connect = (max_retry_count == INFINITY_CONNECT_RETRY
                                  || connect_retries_num < max_retry_count);
            if (!retry_connect) {
                ESP_LOGW(TAG_WiFiService_Station, "failed. unable reconnect");
                station_rssi = LogicElement::MinValue;
                Controller::WakeupProcessTask();
                reconnect_delay = portMAX_DELAY;
            } else {
                const int retries_num_before_no_station = 3;
                if (connect_retries_num >= retries_num_before_no_station) {
                    station_rssi = LogicElement::MinValue;
                    Controller::WakeupProcessTask();
                    if (one_more_request) {
                        ESP_LOGI(TAG_WiFiService_Station,
                                 "Stop connecting to station due to new request");
                        break;
                    }
                }

                connect_retries_num++;
                ESP_LOGI(TAG_WiFiService_Station,
                         "'%s' failed. reconnect, num:%d of %d",
                         settings.wifi_station.ssid,
                         connect_retries_num,
                         max_retry_count);

                reconnect_delay = reconnect_delay_ms / portTICK_RATE_MS;
            }

            stop_http_server();
            Disconnect();

            bool delay_before_reconnect =
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

        bool connected = (notified_event & CONNECTED_BIT) != 0;
        if (connected) {
            start_http_server();
            connect_retries_num = 0;
            has_connect = true;
            connection_start_time = (uint64_t)esp_timer_get_time();
            if (ObtainStationRssi()) {
                Controller::WakeupProcessTask();
            }
            ESP_LOGI(TAG_WiFiService_Station, "ConnectToStation, rssi:%u", station_rssi);
        }

        if (one_more_request && has_connect) {
            int64_t timespan =
                (connection_start_time + (min_worktime_ms * 1000)) - (uint64_t)esp_timer_get_time();

            if (timespan > 0) {
                const TickType_t delay_before_disconnect = (timespan / 1000) / portTICK_RATE_MS;
                ESP_LOGI(TAG_WiFiService_Station,
                         "Wait %u ticks before disconnect",
                         delay_before_disconnect);
                xTaskNotifyWait(0,
                                CANCEL_REQUEST_BIT | FAILED_BIT,
                                &ulNotifiedValue,
                                delay_before_disconnect);
            }
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
        != pdTRUE) {
        ESP_LOGI(TAG_WiFiService_Station, "not fully disconnected");
    }

    requests.RemoveStation();
    if (!cancel) {
        requests.Station();
    }
    ESP_LOGD(TAG_WiFiService_Station, "finish");
}

bool WiFiService::ObtainStationRssi() {
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) {
        ESP_LOGI(TAG_WiFiService_Station, "ObtainStationRssi, no connection");
        return false;
    }

    CurrentSettings::wifi_station_settings wifi_station;
    SAFETY_SETTINGS({ wifi_station = settings.wifi_station; });

    station_rssi = ScaleRssiToPercent04(ap.rssi, wifi_station.max_rssi, wifi_station.min_rssi);
    ESP_LOGI(TAG_WiFiService_Station, "rssi:%d[%u]", ap.rssi, station_rssi);
    return true;
}

void WiFiService::wifi_event_handler(void *arg,
                                     esp_event_base_t event_base,
                                     int32_t event_id,
                                     void *event_data) {
    (void)event_data;
    auto wifi_service = static_cast<WiFiService *>(arg);
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGD(TAG_WiFiService_Station, "start wifi event");
            esp_wifi_connect();
            return;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGD(TAG_WiFiService_Station, "connect to the AP fail");
            xTaskNotify(wifi_service->task_handle, FAILED_BIT, eNotifyAction::eSetBits);
            return;

        case WIFI_EVENT_STA_STOP:
            ESP_LOGD(TAG_WiFiService_Station, "stop wifi event");
            return;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGD(TAG_WiFiService_Station, "wifi connected event");
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
    (void)event_base;
    (void)event_id;

    auto wifi_service = static_cast<WiFiService *>(arg);
    auto event = (ip_event_got_ip_t *)event_data;

    ESP_LOGD(TAG_WiFiService_Station, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
    xTaskNotify(wifi_service->task_handle, CONNECTED_BIT, eNotifyAction::eSetBits);
}
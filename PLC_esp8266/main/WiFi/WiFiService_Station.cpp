#include "HttpServer/http_server.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "WiFiService.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "lwip/ip4_addr.h"
#include "settings.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG_WiFiService_Station = "WiFiService.Station";
extern CurrentSettings::device_settings settings;

void WiFiService::StationTask(RequestItem *request) {
    ESP_LOGD(TAG_WiFiService_Station, "start");

    uint32_t reconnect_delay_ms;
    uint32_t scan_station_rssi_period_ms;
    uint32_t min_worktime_ms;
    wifi_config_t wifi_config = {};

    SAFETY_SETTINGS({
        memcpy(wifi_config.sta.ssid, settings.wifi_station.ssid, sizeof(wifi_config.sta.ssid)); //
        memcpy(wifi_config.sta.password,
               settings.wifi_station.password,
               sizeof(wifi_config.sta.password)); //
        reconnect_delay_ms = settings.wifi_station.reconnect_delay_ms;
        scan_station_rssi_period_ms = settings.wifi_station.scan_station_rssi_period_ms;
        min_worktime_ms = settings.wifi_station.min_worktime_ms;
    });

    bool has_wifi_sta_settings = wifi_config.sta.ssid[0] != 0;
    if (!has_wifi_sta_settings) {
        ESP_LOGW(TAG_WiFiService_Station, "no creds saved");
        station_rssi = LogicElement::MinValue;
        uint32_t new_request_flags = 0;
        WaitEvent(&new_request_flags, portMAX_DELAY);
        return;
    }

    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, this));

    bool has_connect = false;
    bool failed = false;
    uint64_t connection_start_time = 0;
    uint32_t ulNotifiedValue = 0;
    Connect(&wifi_config);

    while (true) {
        if (ulNotifiedValue == 0) {
            if (!WaitEvent(&ulNotifiedValue, scan_station_rssi_period_ms / portTICK_PERIOD_MS)) {
                if (has_connect && !ObtainStationRssi()) {
                    ulNotifiedValue = STA_FAILED_BIT;
                }
            }
        }
        ESP_LOGD(TAG_WiFiService_Station, "event uxBits:0x%08X", (unsigned int)ulNotifiedValue);
        uint32_t notified_event = ulNotifiedValue;
        ulNotifiedValue = 0;

        bool cancel = (notified_event & STA_BREAK_BIT) != 0;
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_Station, "Cancel");
            break;
        }

        bool any_failure = (notified_event & STA_FAILED_BIT) != 0;
        if (any_failure) {
            has_connect = false;
            failed = true;

            const uint8_t retries_num_before_no_station = 3;
            if (station_connect_failures < retries_num_before_no_station) {
                station_connect_failures++;
            } else if (station_rssi != LogicElement::MinValue) {
                station_rssi = LogicElement::MinValue;
                Controller::WakeupProcessTask();
            }

            ESP_LOGI(TAG_WiFiService_Station,
                     "'%.32s' failed, num:%u",
                     (const char *)wifi_config.sta.ssid,
                     (unsigned int)station_connect_failures);
            break;
        }

        bool connected = (notified_event & STA_CONNECTED_BIT) != 0;
        if (connected) {
            start_http_server();
            station_connect_failures = 0;
            has_connect = true;
            connection_start_time = (uint64_t)esp_timer_get_time();
            if (ObtainStationRssi()) {
                Controller::WakeupProcessTask();
            }
            ESP_LOGI(TAG_WiFiService_Station, "ConnectToStation, rssi:%u", station_rssi);
        }

        if (has_connect && requests.HasAnother(request)) {
            int64_t timespan;
            while ((timespan = (connection_start_time + (min_worktime_ms * 1000))
                             - (uint64_t)esp_timer_get_time())
                   > 0) {
                const TickType_t delay_before_disconnect = (timespan / 1000) / portTICK_PERIOD_MS;
                ESP_LOGI(TAG_WiFiService_Station,
                         "Wait %u ticks before disconnect",
                         (unsigned int)delay_before_disconnect);
                bool timeout = !WaitEvent(&ulNotifiedValue, delay_before_disconnect);
                if (timeout) {
                    break;
                }
            }
            ESP_LOGI(TAG_WiFiService_Station, "Disconnect station due to new request");
            break;
        }
    }
    stop_http_server();
    Controller::StopSntp();
    Disconnect();

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler));
    ESP_ERROR_CHECK(
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));

    const TickType_t wait_disconnection = 500 / portTICK_PERIOD_MS;
    uint64_t disconnection_start_time = failed ? (uint64_t)esp_timer_get_time() : 0;
    if (!WaitEvent(&ulNotifiedValue, wait_disconnection)) {
        ESP_LOGI(TAG_WiFiService_Station, "not fully disconnected");
    }

    if (failed && !requests.HasAnother(request)) {
        uint32_t disconnection_elapsed_ms =
            ((uint64_t)esp_timer_get_time() - disconnection_start_time) / 1000;
        if (disconnection_elapsed_ms < reconnect_delay_ms) {
            WaitEvent(&ulNotifiedValue,
                      (reconnect_delay_ms - disconnection_elapsed_ms) / portTICK_PERIOD_MS);
        }
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
    ESP_LOGD(TAG_WiFiService_Station, "rssi:%d[%u]", (int)ap.rssi, (unsigned int)station_rssi);
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
            xEventGroupSetBits(wifi_service->events, STA_FAILED_BIT);
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
                     (int)event_id);
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

    ESP_LOGD(TAG_WiFiService_Station,
             "got ip:%s",
             ip4addr_ntoa((const ip4_addr_t *)&event->ip_info.ip));
    xEventGroupSetBits(wifi_service->events, STA_CONNECTED_BIT);
}
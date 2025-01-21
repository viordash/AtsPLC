#include "WiFiService.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "settings.h"
#include "sys_gpio.h"
#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#define INFINITY_CONNECT_RETRY -1

static const char *TAG_WiFiService = "WiFiService";
extern device_settings settings;

WiFiService::WiFiService() {
}

WiFiService::~WiFiService() {
}

void WiFiService::Start() {
    ESP_LOGW(TAG_WiFiService, "Start");
    event = xEventGroupCreate();
    xEventGroupSetBits(event, WiFiService::STARTED_BIT);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_Error);
    ConnectToStation();
    ESP_ERROR_CHECK(xTaskCreate(WiFiService::Task, "wifi_task", 2048, this, tskIDLE_PRIORITY, NULL)
                            != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void WiFiService::Stop() {
    ESP_LOGW(TAG_WiFiService, "Stop");
    if (!Started()) {
        return;
    }

    xEventGroupSetBits(event, STOP_BIT);

    EventBits_t uxBits = xEventGroupWaitBits(event, WiFiService::RUNNED_BIT, false, false, 0);
    bool runned = (uxBits & WiFiService::RUNNED_BIT) != 0;
    if (runned) {
        ESP_LOGI(TAG_WiFiService, "Stop, is_runned, uxBits:0x%08X", uxBits);

        uxBits = xEventGroupWaitBits(event, WiFiService::STOPPED_BIT, false, false, portMAX_DELAY);
        if (uxBits & WiFiService::STOPPED_BIT) {
            ESP_LOGI(TAG_WiFiService, "stopped, uxBits:0x%08X", uxBits);
        }
    }
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(tcpip_adapter_clear_default_wifi_handlers());

    xEventGroupClearBits(event, STARTED_BIT);
    vEventGroupDelete(event);
}

bool WiFiService::Started() {
    EventBits_t uxBits = xEventGroupWaitBits(event, STARTED_BIT, false, false, 0);

    if (uxBits & STARTED_BIT) {
        ESP_LOGI(TAG_WiFiService, "is_runned, uxBits:0x%08X", uxBits);
    }
    return uxBits & STARTED_BIT;
}

WiFiStationConnectStatus WiFiService::ConnectToStation() {
    bool was_added = requests.Station();
    if (was_added) {
        xEventGroupSetBits(event, NEW_REQUEST_BIT);
        ESP_LOGI(TAG_WiFiService, "ConnectToStation, new req");
    }

    ESP_LOGD(TAG_WiFiService, "ConnectToStation, was_added:%u", was_added);
    return GetWiFiStationConnectStatus();
}

void WiFiService::DisconnectFromStation() {
    bool removed = requests.RemoveStation();
    ESP_LOGI(TAG_WiFiService, "DisconnectFromStation, removed:%d", removed);
    if (removed) {
        xEventGroupSetBits(event, CANCEL_REQUEST_BIT);
    }
}

void WiFiService::SetWiFiStationConnectStatus(WiFiStationConnectStatus new_status) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    station_connect_status = new_status;
}

WiFiStationConnectStatus WiFiService::GetWiFiStationConnectStatus() {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    return station_connect_status;
}

uint8_t WiFiService::Scan(const char *ssid) {
    int request_re_add_delay_ms;
    SAFETY_SETTINGS(
        { request_re_add_delay_ms = settings.wifi_scanner.delay_re_adding_request_ms; });

    if (Controller::RequestWakeupMs((void *)ssid, request_re_add_delay_ms)) {
        requests.Scan(ssid);
        xEventGroupSetBits(event, NEW_REQUEST_BIT);
    }

    uint8_t rssi;
    bool found = FindSsidInScannedList(ssid, &rssi);
    ESP_LOGD(TAG_WiFiService, "Scan, ssid:%s, found:%d, rssi:%u", ssid, found, rssi);
    if (!found) {
        rssi = LogicElement::MinValue;
    }
    return rssi;
}

void WiFiService::CancelScan(const char *ssid) {
    RemoveSsidFromScannedList(ssid);
    bool removed = requests.RemoveScanner(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelScan, ssid:%s, removed:%d", ssid, removed);
    if (removed) {
        xEventGroupSetBits(event, CANCEL_REQUEST_BIT);
    }
}

void WiFiService::Generate(const char *ssid) {
    int request_re_add_delay_ms;
    SAFETY_SETTINGS(
        { request_re_add_delay_ms = settings.wifi_access_point.delay_re_adding_request_ms; });

    if (Controller::RequestWakeupMs((void *)ssid, request_re_add_delay_ms)) {
        requests.AccessPoint(ssid);
        xEventGroupSetBits(event, NEW_REQUEST_BIT);
        ESP_LOGI(TAG_WiFiService, "Generate, ssid:%s", ssid);
    }
}

void WiFiService::CancelGenerate(const char *ssid) {
    bool removed = requests.RemoveAccessPoint(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelGenerate, ssid:%s, removed:%d", ssid, removed);
    if (removed) {
        xEventGroupSetBits(event, CANCEL_REQUEST_BIT);
    }
}

void WiFiService::Task(void *parm) {
    ESP_LOGI(TAG_WiFiService, "Start task");
    auto wifi_service = static_cast<WiFiService *>(parm);

    EventBits_t uxBits;
    do {
        uxBits = xEventGroupWaitBits(wifi_service->event,
                                     STOP_BIT | NEW_REQUEST_BIT,
                                     true,
                                     false,
                                     portMAX_DELAY);

        RequestItem new_request;
        while ((uxBits & STOP_BIT) == 0 && wifi_service->requests.Pop(&new_request)) {
            ESP_LOGD(TAG_WiFiService, "New request, type:%u", new_request.Type);

            switch (new_request.Type) {
                case wqi_Station:
                    wifi_service->StationTask(&new_request);
                    break;

                case wqi_Scanner:
                    wifi_service->ScannerTask(&new_request);
                    break;

                case wqi_AccessPoint:
                    wifi_service->AccessPointTask(&new_request);
                    break;

                default:
                    break;
            }
            ESP_LOGD(TAG_WiFiService, "end request, type:%u", new_request.Type);
        }
    } while (uxBits != 0 && (uxBits & STOP_BIT) == 0);

    ESP_LOGW(TAG_WiFiService, "Finish task");
    xEventGroupSetBits(wifi_service->event, WiFiService::STOPPED_BIT);
    vTaskDelete(NULL);
}

void WiFiService::Connect(wifi_config_t *wifi_config) {
    ESP_LOGD(TAG_WiFiService, "Connect");

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
     * However these modes are deprecated and not advisable to be used. Incase your Access point
     * doesn't support WPA2, these mode can be enabled by commenting below line */

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    if (wifi_config != NULL) {
        if (wifi_config->sta.password[0] != 0) {
            wifi_config->sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        }
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
    }

    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiService::Disconnect() {
    ESP_LOGD(TAG_WiFiService, "Disconnect");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
}

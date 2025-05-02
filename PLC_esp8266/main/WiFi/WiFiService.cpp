#include "WiFiService.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "settings.h"
#include "sys_gpio.h"
#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#define INFINITY_CONNECT_RETRY -1

static const char *TAG_WiFiService = "WiFiService";
extern CurrentSettings::device_settings settings;

WiFiService::WiFiService() : task_handle(NULL) {
    station_rssi = LogicElement::MinValue;
}

WiFiService::~WiFiService() {
}

void WiFiService::Start() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        xTaskCreate(WiFiService::Task, "wifi_task", 2048, this, tskIDLE_PRIORITY, &task_handle)
                != pdPASS
            ? ESP_FAIL
            : ESP_OK);

    ESP_LOGW(TAG_WiFiService, "Start, task_handle:%p", task_handle);
}

uint8_t WiFiService::ConnectToStation() {
    if (requests.Station()) {
        xTaskNotify(task_handle, 0, eNotifyAction::eNoAction);
        ESP_LOGD(TAG_WiFiService, "ConnectToStation");
    }
    ESP_LOGD(TAG_WiFiService, "ConnectToStation, rssi:%u", station_rssi);
    return station_rssi;
}

void WiFiService::DisconnectFromStation() {
    bool removed = requests.RemoveStation();
    ESP_LOGI(TAG_WiFiService, "DisconnectFromStation, removed:%d", removed);
    if (removed) {
        xTaskNotify(task_handle, CANCEL_REQUEST_BIT, eNotifyAction::eSetBits);
    }
}

uint8_t WiFiService::Scan(const char *ssid) {
    uint8_t rssi;
    bool found = FindScannedSsid(ssid, &rssi);
    if (!found) {
        rssi = LogicElement::MinValue;
    }

    if (requests.Scan(ssid)) {
        xTaskNotify(task_handle, 0, eNotifyAction::eNoAction);
        ESP_LOGD(TAG_WiFiService, "Scan, ssid:%s, found:%d, rssi:%u", ssid, found, rssi);
    }
    return rssi;
}

void WiFiService::CancelScan(const char *ssid) {
    RemoveScannedSsid(ssid);
    bool removed = requests.RemoveScanner(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelScan, ssid:%s, removed:%d", ssid, removed);
    if (removed) {
        xTaskNotify(task_handle, CANCEL_REQUEST_BIT, eNotifyAction::eSetBits);
    }
}

size_t WiFiService::AccessPoint(const char *ssid, const char *password, const char *mac) {
    if (requests.AccessPoint(ssid, password, mac)) {
        xTaskNotify(task_handle, 0, eNotifyAction::eNoAction);
        ESP_LOGD(TAG_WiFiService, "AccessPoint, ssid:%s", ssid);
    }
    return GetApClientsCount(ssid);
}

void WiFiService::CancelAccessPoint(const char *ssid) {
    RemoveApClients(ssid);
    bool removed = requests.RemoveAccessPoint(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelAccessPoint, ssid:%s, removed:%d", ssid, removed);
    if (removed) {
        xTaskNotify(task_handle, CANCEL_REQUEST_BIT, eNotifyAction::eSetBits);
    }
}

void WiFiService::Task(void *parm) {
    ESP_LOGI(TAG_WiFiService, "Start task");
    auto wifi_service = static_cast<WiFiService *>(parm);

    uint32_t ulNotifiedValue = 0;
    while (true) {
        xTaskNotifyWait(0, 0, &ulNotifiedValue, portMAX_DELAY);
        if ((ulNotifiedValue & STOP_BIT) != 0) {
            break;
        }
        ESP_LOGD(TAG_WiFiService, "new request, uxBits:0x%08X", ulNotifiedValue);
        RequestItem new_request;
        while (wifi_service->requests.Pop(&new_request)) {
            ESP_LOGI(TAG_WiFiService, "exec request, type:%u", new_request.Type);

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
    }

    ESP_LOGW(TAG_WiFiService, "Finish task");
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
    esp_wifi_disconnect();
    esp_wifi_stop();
}

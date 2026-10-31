#include "WiFiService.h"
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

static const char *TAG_WiFiService = "WiFiService";
extern CurrentSettings::device_settings settings;

WiFiService::WiFiService()
    : scanned_ssids{}, scanned_ssids_count{ 0 }, station_rssi{ LogicElement::MinValue },
      station_connect_failures{ 0 }, ap_clients{}, ap_clients_count{ 0 }, events(NULL) {
}

WiFiService::~WiFiService() {
}

void WiFiService::Start() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    events = xEventGroupCreate();

    TaskHandle_t task_handle;
    ESP_ERROR_CHECK(
        xTaskCreate(WiFiService::Task, "wifi_task", 2048, this, tskIDLE_PRIORITY, &task_handle)
                != pdPASS
            ? ESP_FAIL
            : ESP_OK);

    ESP_LOGW(TAG_WiFiService, "Start, task_handle:%p", task_handle);
}

bool WiFiService::WaitEvent(uint32_t *notified_flags, TickType_t ticks_to_wait) {
    *notified_flags = xEventGroupWaitBits(events, ~0xff000000U, true, false, ticks_to_wait);
    return *notified_flags != 0;
}

bool WiFiService::ClearBreakFlags(uint32_t ignore_flags) {
    uint32_t flags = 0;
    WaitEvent(&flags, 0);
    uint32_t break_flags = flags & ignore_flags;
    if (break_flags == 0) {
        return false;
    }
    xEventGroupSetBits(events, break_flags);
    return true;
}

uint8_t WiFiService::ConnectToStation() {
    if (requests.Station()) {
        if (!ClearBreakFlags(SCAN_BREAK_BIT | AP_BREAK_BIT)) {
            xEventGroupSetBits(events, WAKEUP_BIT);
        }
        ESP_LOGD(TAG_WiFiService, "ConnectToStation new req");
    } else {
        ESP_LOGD(TAG_WiFiService, "ConnectToStation req, rssi:%u", station_rssi);
    }
    return station_rssi;
}

void WiFiService::DisconnectFromStation() {
    requests.RemoveStation();
    ESP_LOGI(TAG_WiFiService, "DisconnectFromStation");
    xEventGroupSetBits(events, STA_BREAK_BIT);
}

uint8_t WiFiService::Scan(const char *ssid) {
    uint8_t rssi;
    bool found = FindScannedSsid(ssid, &rssi);
    if (!found) {
        rssi = LogicElement::MinValue;
    }

    if (requests.Scan(ssid)) {
        if (!ClearBreakFlags(STA_BREAK_BIT | AP_BREAK_BIT)) {
            xEventGroupSetBits(events, WAKEUP_BIT);
        }
        ESP_LOGD(TAG_WiFiService,
                 "Scan new req, ssid:%s, found:%u, rssi:%u",
                 ssid,
                 (unsigned int)found,
                 (unsigned int)rssi);
    } else {
        ESP_LOGD(TAG_WiFiService, "Scan req, ssid:%s", ssid);
    }
    return rssi;
}

void WiFiService::CancelScan(const char *ssid) {
    requests.RemoveScan(ssid);
    RemoveScannedSsid(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelScan, ssid:%s", ssid);
    xEventGroupSetBits(events, SCAN_BREAK_BIT);
}

size_t WiFiService::AccessPoint(const char *ssid, const char *password, const char *mac) {
    if (requests.AccessPoint(ssid, password, mac)) {
        if (!ClearBreakFlags(STA_BREAK_BIT | SCAN_BREAK_BIT)) {
            xEventGroupSetBits(events, WAKEUP_BIT);
        }
        ESP_LOGD(TAG_WiFiService, "AccessPoint new req, ssid:%s", ssid);
    } else {
        ESP_LOGD(TAG_WiFiService, "AccessPoint req, ssid:%s", ssid);
    }
    return GetApClientsCount(ssid);
}

void WiFiService::CancelAccessPoint(const char *ssid, const char *password, const char *mac) {
    requests.RemoveAccessPoint(ssid, password, mac);
    RemoveApClients(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelAccessPoint, ssid:%s", ssid);
    xEventGroupSetBits(events, AP_BREAK_BIT);
}

void WiFiService::RaiseSettingsChanged() {
    ESP_LOGI(TAG_WiFiService, "RaiseSettingsChanged");
    xEventGroupSetBits(events, SETTINGS_CHANGED_BIT);
}

void WiFiService::Task(void *parm) {
    ESP_LOGI(TAG_WiFiService, "Start task");
    auto wifi_service = static_cast<WiFiService *>(parm);

    while (true) {
        RequestItem new_request;
        if (!wifi_service->requests.Pop(&new_request)) {
            new_request = { RequestItemType::wqi_Station, {} };
            ESP_LOGD(TAG_WiFiService, "no requests, default to station");
        }
        ESP_LOGI(TAG_WiFiService, "exec request, type:%u", (unsigned int)new_request.Type);

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
        ESP_LOGD(TAG_WiFiService, "end request, type:%u", (unsigned int)new_request.Type);
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
        ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, wifi_config));
    }

    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiService::Disconnect() {
    ESP_LOGD(TAG_WiFiService, "Disconnect");
    esp_wifi_disconnect();
    esp_wifi_stop();
}

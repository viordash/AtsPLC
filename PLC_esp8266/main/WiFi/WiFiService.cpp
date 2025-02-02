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
extern CurrentSettings::device_settings settings;

WiFiService::WiFiService() {
    station_connect_status = WiFiStationConnectStatus ::wscs_Error;
}

WiFiService::~WiFiService() {
}

void WiFiService::Start() {

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    SetWiFiStationConnectStatus(WiFiStationConnectStatus::wscs_Error);
    ESP_ERROR_CHECK(
        xTaskCreate(WiFiService::Task, "wifi_task", 2048, this, tskIDLE_PRIORITY, &task_handle)
                != pdPASS
            ? ESP_FAIL
            : ESP_OK);

    ConnectToStation();
    ESP_LOGW(TAG_WiFiService, "Start, task_handle:%p", task_handle);
}

void WiFiService::Stop() {
    ESP_LOGW(TAG_WiFiService, "Stop");
    if (!Started()) {
        return;
    }

    xTaskNotify(task_handle, STOP_BIT, eNotifyAction::eSetBits);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(tcpip_adapter_clear_default_wifi_handlers());
}

bool WiFiService::Started() {
    TaskStatus_t xTaskDetails;
    vTaskGetInfo(task_handle, &xTaskDetails, pdFALSE, eTaskState::eInvalid);

    if (xTaskDetails.eCurrentState == eTaskState::eRunning) {
        ESP_LOGI(TAG_WiFiService, "is_runned");
    }
    return xTaskDetails.eCurrentState == eTaskState::eRunning;
}

WiFiStationConnectStatus WiFiService::ConnectToStation() {
    auto status = GetWiFiStationConnectStatus();
    if (requests.Station()) {
        xTaskNotify(task_handle, 0, eNotifyAction::eNoAction);
        ESP_LOGD(TAG_WiFiService, "ConnectToStation, status:%u", status);
    }
    return status;
}

void WiFiService::DisconnectFromStation() {
    bool removed = requests.RemoveStation();
    ESP_LOGI(TAG_WiFiService, "DisconnectFromStation, removed:%d", removed);
    if (removed) {
        xTaskNotify(task_handle, CANCEL_REQUEST_BIT, eNotifyAction::eSetBits);
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
    uint8_t rssi;
    bool found = FindSsidInScannedList(ssid, &rssi);
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
    RemoveSsidFromScannedList(ssid);
    bool removed = requests.RemoveScanner(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelScan, ssid:%s, removed:%d", ssid, removed);
    if (removed) {
        xTaskNotify(task_handle, CANCEL_REQUEST_BIT, eNotifyAction::eSetBits);
    }
}

void WiFiService::Generate(const char *ssid) {
    if (requests.AccessPoint(ssid)) {
        xTaskNotify(task_handle, 0, eNotifyAction::eNoAction);
        ESP_LOGD(TAG_WiFiService, "Generate, ssid:%s", ssid);
    }
}

void WiFiService::CancelGenerate(const char *ssid) {
    bool removed = requests.RemoveAccessPoint(ssid);
    ESP_LOGI(TAG_WiFiService, "CancelGenerate, ssid:%s, removed:%d", ssid, removed);
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
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
}

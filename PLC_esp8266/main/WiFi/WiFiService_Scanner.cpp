
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "WiFiService.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG_WiFiService_Scanner = "WiFiService.Scanner";
extern CurrentSettings::device_settings settings;

#define CHANNELS_COUNT 14

bool WiFiService::StartScan(const char *ssid,
                            CurrentSettings::wifi_scanner_settings *scanner_settings) {
    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = true;
    scan_config.ssid = (uint8_t *)ssid;

    scan_config.scan_type = wifi_scan_type_t::WIFI_SCAN_TYPE_PASSIVE;
    scan_config.scan_time.passive = scanner_settings->per_channel_scan_time_ms;

    Connect(NULL);

    esp_err_t err = esp_wifi_scan_start(&scan_config, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WiFiService_Scanner, "esp_wifi_scan_start err 0x%X", (unsigned int)err);
        return false;
    }
    return true;
}

int8_t WiFiService::Scanning(RequestItem *request,
                             CurrentSettings::wifi_scanner_settings *scanner_settings,
                             bool *canceled) {
    esp_err_t err;

    wifi_ap_record_t ap_info[1] = {};

    uint32_t scan_max_time_ms = (scanner_settings->per_channel_scan_time_ms * CHANNELS_COUNT);
    auto current_time = (uint64_t)esp_timer_get_time();
    auto timeout_time = current_time + (scan_max_time_ms * 1000);

    uint32_t ulNotifiedValue = 0;
    while (true) {
        uint16_t number = sizeof(ap_info) / sizeof(ap_info[0]);
        err = esp_wifi_scan_get_ap_records(&number, ap_info);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_WiFiService_Scanner,
                     "esp_wifi_scan_get_ap_records err 0x%X",
                     (unsigned int)err);
            break;
        }

        for (int i = 0; i < number; i++) {
            ESP_LOGI(TAG_WiFiService_Scanner,
                     "SSID:'%s', RSSI:%d, Channel:%d, reqSSID:'%s'",
                     ap_info[i].ssid,
                     ap_info[i].rssi,
                     ap_info[i].primary,
                     request->Payload.Scanner.ssid);
            if (strcmp((const char *)ap_info[i].ssid, (const char *)request->Payload.Scanner.ssid)
                    == 0
                && ap_info[i].rssi > scanner_settings->min_rssi) {
                return ap_info[i].rssi;
            }
        }

        bool notify_wait_timeout =
            xTaskNotifyWait(0,
                            CANCEL_REQUEST_BIT,
                            &ulNotifiedValue,
                            scanner_settings->per_channel_scan_time_ms / portTICK_PERIOD_MS)
            == pdFALSE;

        int64_t timespan = timeout_time - (uint64_t)esp_timer_get_time();
        ESP_LOGD(TAG_WiFiService_Scanner,
                 "process, uxBits:0x%08X, timespan:%lld",
                 (unsigned int)ulNotifiedValue,
                 (long long)timespan);

        if (timespan <= 0) {
            break;
        }
        bool to_stop = !notify_wait_timeout && (ulNotifiedValue & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        *canceled = !notify_wait_timeout && (ulNotifiedValue & CANCEL_REQUEST_BIT) != 0
                 && !requests.Contains(request);
        if (*canceled) {
            ESP_LOGI(TAG_WiFiService_Scanner,
                     "Cancel request, ssid:%s",
                     request->Payload.Scanner.ssid);
            break;
        }
    }
    return scanner_settings->min_rssi;
}

void WiFiService::StopScan() {
    esp_wifi_scan_stop();
    Disconnect();
}

void WiFiService::ScannerTask(RequestItem *request) {
    ESP_LOGI(TAG_WiFiService_Scanner, "start, ssid:%s", request->Payload.Scanner.ssid);

    CurrentSettings::wifi_scanner_settings scanner_settings;
    SAFETY_SETTINGS({ scanner_settings = settings.wifi_scanner; });

    int8_t rssi = scanner_settings.min_rssi;

    bool canceled = false;
    if (StartScan(request->Payload.Scanner.ssid, &scanner_settings)) {
        rssi = Scanning(request, &scanner_settings, &canceled);
    }
    StopScan();

    if (rssi > scanner_settings.min_rssi) {
        AddScannedSsid(
            request->Payload.Scanner.ssid,
            ScaleRssiToPercent04(rssi, scanner_settings.max_rssi, scanner_settings.min_rssi));
    } else {
        RemoveScannedSsid(request->Payload.Scanner.ssid);
    }
    requests.RemoveScanner(request->Payload.Scanner.ssid);
    if (!canceled) {
        Controller::WakeupProcessTask();
    }

    ESP_LOGI(TAG_WiFiService_Scanner,
             "finish, ssid:%s, rssi:%d[%d]",
             request->Payload.Scanner.ssid,
             (int)rssi,
             ScaleRssiToPercent04(rssi, scanner_settings.max_rssi, scanner_settings.min_rssi));
}

uint8_t WiFiService::ScaleRssiToPercent04(int8_t rssi, int8_t max_rssi, int8_t min_rssi) {
    float fl = ((float)rssi - min_rssi) / (max_rssi - min_rssi);
    fl = fl * (LogicElement::MaxValue - LogicElement::MinValue) + LogicElement::MinValue;
    if (fl < (float)LogicElement::MinValue) {
        fl = (float)LogicElement::MinValue;
    } else if (fl > (float)LogicElement::MaxValue) {
        fl = (float)LogicElement::MaxValue;
    }
    return (uint8_t)fl;
}

void WiFiService::AddScannedSsid(const char *ssid, uint8_t rssi) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.insert({ ssid, rssi });
    if (!it.second) {
        it.first->second = rssi;
    }
    ESP_LOGD(TAG_WiFiService_Scanner, "AddScannedSsid, cnt:%u", (unsigned int)scanned_ssid.size());
}

bool WiFiService::FindScannedSsid(const char *ssid, uint8_t *rssi) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.find(ssid);
    bool found = it != scanned_ssid.end();
    ESP_LOGD(TAG_WiFiService_Scanner, "FindScannedSsid, found:%u", (unsigned int)found);
    if (found) {
        *rssi = it->second;
    }
    return found;
}

void WiFiService::RemoveScannedSsid(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.erase(ssid);
    ESP_LOGD(TAG_WiFiService_Scanner,
             "RemoveScannedSsid, cnt:%u",
             (unsigned int)scanned_ssid.size());
}
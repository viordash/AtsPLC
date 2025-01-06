
#include "LogicProgram/Controller.h"
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

static const char *TAG_WiFiService_Scanner = "WiFiService.Scanner";
extern device_settings settings;

#define PASSIVE_PER_CHANNEL_SCAN_TIME_MS 500
#define CHANNELS_COUNT 14
#define MAX_AVAILABLE_RSSI -26
#define MIN_AVAILABLE_RSSI -120

bool WiFiService::StartScan(const char *ssid) {
    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = true;
    scan_config.ssid = (uint8_t *)ssid;

    scan_config.scan_type = wifi_scan_type_t::WIFI_SCAN_TYPE_PASSIVE;
    scan_config.scan_time.passive = PASSIVE_PER_CHANNEL_SCAN_TIME_MS;

    Connect(NULL);

    esp_err_t err = esp_wifi_scan_start(&scan_config, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WiFiService_Scanner, "esp_wifi_scan_start err 0x%X", err);
        return false;
    }
    return true;
}

int8_t WiFiService::Scanning(RequestItem *request) {
    esp_err_t err;

    wifi_ap_record_t ap_info[1] = {};

    uint32_t scan_max_time_ms = (PASSIVE_PER_CHANNEL_SCAN_TIME_MS * CHANNELS_COUNT);
    auto current_time = (uint64_t)esp_timer_get_time();
    auto timeout_time = current_time + (scan_max_time_ms * 1000);
    int64_t timespan;

    EventBits_t uxBits = 0;
    do {
        uint16_t number = sizeof(ap_info) / sizeof(ap_info[0]);
        err = esp_wifi_scan_get_ap_records(&number, ap_info);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_WiFiService_Scanner, "esp_wifi_scan_get_ap_records err 0x%X", err);
            break;
        }

        for (int i = 0; i < number; i++) {
            ESP_LOGI(TAG_WiFiService_Scanner,
                     "SSID:'%s', RSSI:%d, Channel:%d",
                     ap_info[i].ssid,
                     ap_info[i].rssi,
                     ap_info[i].primary);
            if (strcmp((const char *)ap_info[i].ssid, (const char *)request->Payload.Scanner.ssid)
                    == 0
                && ap_info[i].rssi > MIN_AVAILABLE_RSSI) {
                return ap_info[i].rssi;
            }
        }

        uxBits = xEventGroupWaitBits(event,
                                     STOP_BIT | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     PASSIVE_PER_CHANNEL_SCAN_TIME_MS / portTICK_PERIOD_MS);

        timespan = timeout_time - (uint64_t)esp_timer_get_time();
        ESP_LOGD(TAG_WiFiService_Scanner,
                 "process, uxBits:0x%08X, timespan:%lld",
                 uxBits,
                 (long long)timespan);

        bool cancel = (uxBits & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_Scanner,
                     "Cancel request, ssid:%s",
                     request->Payload.Scanner.ssid);
            break;
        }
    } while ((uxBits & STOP_BIT) == 0 && timespan > 0);
    return MIN_AVAILABLE_RSSI;
}

void WiFiService::StopScan() {
    esp_wifi_scan_stop();
    Disconnect();
}

void WiFiService::ScannerTask(RequestItem *request) {
    ESP_LOGW(TAG_WiFiService_Scanner, "start, ssid:%s", request->Payload.Scanner.ssid);

    int8_t rssi = MIN_AVAILABLE_RSSI;

    if (StartScan(request->Payload.Scanner.ssid)) {
        rssi = Scanning(request);
    }
    StopScan();

    if (rssi > MIN_AVAILABLE_RSSI) {
        AddSsidToScannedList(request->Payload.Scanner.ssid, ScaleRssiToPercent04(rssi));
    } else {
        RemoveSsidFromScannedList(request->Payload.Scanner.ssid);
    }
    requests.RemoveScanner(request->Payload.Scanner.ssid);
    Controller::WakeupProcessTask();

    ESP_LOGW(TAG_WiFiService_Scanner,
             "finish, ssid:%s, rssi:%d[%u]",
             request->Payload.Scanner.ssid,
             rssi,
             ScaleRssiToPercent04(rssi));
}

uint8_t WiFiService::ScaleRssiToPercent04(int8_t rssi) {
    float fl = ((float)rssi - MIN_AVAILABLE_RSSI) / (MAX_AVAILABLE_RSSI - MIN_AVAILABLE_RSSI);
    fl = fl * (LogicElement::MaxValue - LogicElement::MinValue) + LogicElement::MinValue;
    if (fl < (float)LogicElement::MinValue) {
        fl = (float)LogicElement::MinValue;
    } else if (fl > (float)LogicElement::MaxValue) {
        fl = (float)LogicElement::MaxValue;
    }
    return (uint8_t)fl;
}

void WiFiService::AddSsidToScannedList(const char *ssid, uint8_t rssi) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.insert({ ssid, rssi });
    if (!it.second) {
        it.first->second = rssi;
    }
    ESP_LOGD(TAG_WiFiService_Scanner,
             "AddSsidToScannedList, cnt:%u",
             (unsigned)scanned_ssid.size());
}

bool WiFiService::FindSsidInScannedList(const char *ssid, uint8_t *rssi) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.find(ssid);
    bool found = it != scanned_ssid.end();
    ESP_LOGD(TAG_WiFiService_Scanner, "FindSsidInScannedList, found:%u", found);
    if (found) {
        *rssi = it->second;
    }
    return found;
}

void WiFiService::RemoveSsidFromScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.erase(ssid);
    ESP_LOGD(TAG_WiFiService_Scanner,
             "RemoveSsidFromScannedList, cnt:%u",
             (unsigned)scanned_ssid.size());
}
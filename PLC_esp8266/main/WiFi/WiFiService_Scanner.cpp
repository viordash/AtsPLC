
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

void WiFiService::ScannerTask(RequestItem *request) {
    ESP_LOGW(TAG_WiFiService_Scanner, "start, ssid:%s", request->Payload.Scanner.ssid);

    EventBits_t uxBits = 0;
    bool found = false;
    do {
        uxBits = xEventGroupWaitBits(event,
                                     STOP_BIT | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     /*portMAX_DELAY*/ 5000 / portTICK_RATE_MS);
        ESP_LOGI(TAG_WiFiService_Scanner, "process, uxBits:0x%08X", uxBits);
        bool timeout = (uxBits & (STOP_BIT | CANCEL_REQUEST_BIT)) == 0;
        if (timeout) {
            ESP_LOGI(TAG_WiFiService_Scanner, "found");
            found = true;
            break;
        }
        bool cancel = (uxBits & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_Scanner, "Cancel request, ssid:%s", request->Payload.Scanner.ssid);
            break;
        }
    } while (uxBits != 0 && (uxBits & STOP_BIT) == 0);

    if (found) {
        AddSsidToScannedList(request->Payload.Scanner.ssid);
    } else {
        RemoveSsidFromScannedList(request->Payload.Scanner.ssid);
    }
    requests.RemoveScanner(request->Payload.Scanner.ssid);
    Controller::WakeupProcessTask();

    ESP_LOGW(TAG_WiFiService_Scanner, "finish");
}

void WiFiService::AddSsidToScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.insert(ssid);
    ESP_LOGD(TAG_WiFiService_Scanner,
             "AddSsidToScannedList, cnt:%u",
             (unsigned)scanned_ssid.size());
}

bool WiFiService::FindSsidInScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.find(ssid);
    ESP_LOGD(TAG_WiFiService_Scanner, "FindSsidInScannedList, found:%u", it != scanned_ssid.end());
    return it != scanned_ssid.end();
}

void WiFiService::RemoveSsidFromScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.erase(ssid);
    ESP_LOGD(TAG_WiFiService_Scanner,
             "RemoveSsidFromScannedList, cnt:%u",
             (unsigned)scanned_ssid.size());
}
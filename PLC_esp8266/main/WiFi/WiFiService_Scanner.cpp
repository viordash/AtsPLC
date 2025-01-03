
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

bool WiFiService::ScannerTask(RequestItem *request) {
    ESP_LOGW(TAG_WiFiService_Scanner, "start, ssid:%s", request->Payload.Scanner.ssid);

    bool has_new_request = false;
    EventBits_t uxBits = 0;
    bool found = false;
    do {
        uxBits = xEventGroupWaitBits(event,
                                     STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     /*portMAX_DELAY*/ 5000 / portTICK_RATE_MS);
        ESP_LOGI(TAG_WiFiService_Scanner, "process, uxBits:0x%08X", uxBits);
        if ((uxBits & NEW_REQUEST_BIT) != 0) {
            has_new_request = true;
        }

        bool timeout = (uxBits & (STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT)) == 0;
        if (timeout) {
            ESP_LOGI(TAG_WiFiService_Scanner, "found");
            found = true;
            break;
        }

    } while (uxBits != 0 && (uxBits & (STOP_BIT | NEW_REQUEST_BIT | CANCEL_REQUEST_BIT)) == 0);

    if (found) {
        requests.ScannerDone(request->Payload.Scanner.ssid);
        AddSsidToScannedList(request->Payload.Scanner.ssid);
    } else {
        requests.RemoveScanner(request->Payload.Scanner.ssid);
        RemoveSsidFromScannedList(request->Payload.Scanner.ssid);
    }
    Controller::WakeupProcessTask();

    ESP_LOGW(TAG_WiFiService_Scanner, "finish, has_new_request:%u", has_new_request);
    return has_new_request;
}

void WiFiService::AddSsidToScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.insert(ssid);
    ESP_LOGI(TAG_WiFiService_Scanner, "AddSsidToScannedList, cnt:%u", scanned_ssid.size());
}

bool WiFiService::FindSsidInScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    auto it = scanned_ssid.find(ssid);
    ESP_LOGI(TAG_WiFiService_Scanner, "FindSsidInScannedList, found:%u", it != scanned_ssid.end());
    return it != scanned_ssid.end();
}

void WiFiService::RemoveSsidFromScannedList(const char *ssid) {
    std::lock_guard<std::mutex> lock(scanned_ssid_lock_mutex);
    scanned_ssid.erase(ssid);
    ESP_LOGI(TAG_WiFiService_Scanner, "RemoveSsidFromScannedList, cnt:%u", scanned_ssid.size());
}
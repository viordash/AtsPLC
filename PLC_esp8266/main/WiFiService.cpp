#include "WiFiService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>


WiFiService::WiFiService() {
}

WiFiService::~WiFiService() {
}

bool WiFiService::Scan(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto res = requests.insert({ WiFiService::RequestItemType::wqi_ScanSsid, ssid, false });
    bool was_inserted = res.second;
    if (was_inserted) {
        return false;
    }
    return res.first->status;
}

void WiFiService::Generate(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    requests.insert({ WiFiService::RequestItemType::wqi_ScanSsid, ssid, false });
}

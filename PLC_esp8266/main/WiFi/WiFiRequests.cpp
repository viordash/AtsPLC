#include "WiFiRequests.h"
#include "esp_log.h"
#include <cassert>

static const char *TAG_WiFiRequests = "WiFiRequests";

bool WiFiRequests::Equals(const RequestItem *a, const RequestItem *b) const {
    if (a->type != b->type) {
        return false;
    }
    switch (a->type) {
        case wqi_Station:
            return true;

        case wqi_Scanner:
            return a->Payload.Scanner.ssid == b->Payload.Scanner.ssid;

        case wqi_AccessPoint:
            return a->Payload.AccessPoint.ssid == b->Payload.AccessPoint.ssid;
    }
    return true;
}

bool WiFiRequests::Add(RequestItem *new_request) {
    ESP_LOGI(TAG_WiFiRequests, "Add, type:%u", new_request->type);

    std::lock_guard<std::mutex> lock(lock_mutex);
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (Equals(&request, new_request)) {
            return false;
        }
    }
    push_front(*new_request);
    return true;
}

bool WiFiRequests::RemoveScanner(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_Scanner && it->Payload.Scanner.ssid == ssid) {
            ESP_LOGI(TAG_WiFiRequests, "RemoveScanner, removed:'%s'", ssid);
            erase(it);
            return true;
        }
    }
    ESP_LOGI(TAG_WiFiRequests, "RemoveScanner, not found:'%s'", ssid);
    return false;
}

bool WiFiRequests::RemoveAccessPoint(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_AccessPoint
            && it->Payload.AccessPoint.ssid == ssid) {
            ESP_LOGI(TAG_WiFiRequests, "RemoveAccessPoint, removed:'%s'", ssid);
            erase(it);
            return true;
        }
    }
    ESP_LOGI(TAG_WiFiRequests, "RemoveAccessPoint, not found:'%s'", ssid);
    return false;
}

RequestItem WiFiRequests::Pop() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    assert(!empty());
    auto request = back();
    return request;
}

void WiFiRequests::StationDone() {
    ESP_LOGI(TAG_WiFiRequests, "StationDone");
    std::lock_guard<std::mutex> lock(lock_mutex);
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_Station) {
            erase(it);
            ESP_LOGI(TAG_WiFiRequests, "StationDone, found");
            return;
        }
    }
    ESP_LOGI(TAG_WiFiRequests, "StationDone, not found");
}

void WiFiRequests::ScannerDone(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_Scanner && it->Payload.Scanner.ssid == ssid) {
            erase(it);
            ESP_LOGI(TAG_WiFiRequests, "ScannerDone, found:%s", ssid);
            return;
        }
    }
    ESP_LOGI(TAG_WiFiRequests, "ScannerDone, not found:'%s'", ssid);
}

void WiFiRequests::AccessPointDone(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_AccessPoint
            && it->Payload.AccessPoint.ssid == ssid) {
            erase(it);
            ESP_LOGI(TAG_WiFiRequests, "AccessPointDone, ssid:%s", ssid);
            return;
        }
    }
    ESP_LOGI(TAG_WiFiRequests, "AccessPointDone, not found:'%s'", ssid);
}

#include "WiFiRequests.h"
#include "esp_log.h"
#include <cassert>

static const char *TAG_WiFiRequests = "WiFiRequests";

bool WiFiRequests::Equals(const RequestItem *a, const RequestItem *b) const {
    if (a->Type != b->Type) {
        return false;
    }
    switch (a->Type) {
        case wqi_Station:
            return true;

        case wqi_Scanner:
            return a->Payload.Scanner.ssid == b->Payload.Scanner.ssid;

        case wqi_AccessPoint:
            return a->Payload.AccessPoint.ssid == b->Payload.AccessPoint.ssid;
    }
    return true;
}

std::list<RequestItem>::iterator WiFiRequests::Find(RequestItem *new_request) {
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (Equals(&request, new_request)) {
            return it;
        }
    }
    return end();
}

bool WiFiRequests::Scan(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (!exists) {
        push_front(request);
    }
    ESP_LOGD(TAG_WiFiRequests, "Scan, ssid:%s, exists:%u", ssid, exists);
    return !exists;
}

bool WiFiRequests::RemoveScanner(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (exists) {
        erase(item);
    }
    ESP_LOGI(TAG_WiFiRequests, "RemoveScanner, ssid:%s, exists:%u", ssid, exists);
    return exists;
}

bool WiFiRequests::AccessPoint(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (!exists) {
        push_front(request);
    }
    ESP_LOGD(TAG_WiFiRequests, "AccessPoint, ssid:%s, exists:%u", ssid, exists);
    return !exists;
}

bool WiFiRequests::RemoveAccessPoint(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (exists) {
        erase(item);
    }
    ESP_LOGI(TAG_WiFiRequests, "RemoveAccessPoint, ssid:%s, exists:%u", ssid, exists);
    return exists;
}

bool WiFiRequests::Station() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (!exists) {
        push_front(request);
    }
    ESP_LOGI(TAG_WiFiRequests, "Station, exists:%u", exists);
    return !exists;
}

bool WiFiRequests::RemoveStation() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (exists) {
        erase(item);
    }
    ESP_LOGI(TAG_WiFiRequests, "RemoveStation, exists:%u", exists);
    return exists;
}

bool WiFiRequests::Pop(RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    if (empty()) {
        return false;
    }
    *request = back();
    return true;
}

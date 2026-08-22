#include "WiFiRequests.h"
#include "esp_log.h"
#include <cassert>

static const char *TAG_WiFiRequests = "WiFiRequests";

WiFiRequests::WiFiRequests() : items{}, count{ 0 } {
}

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
            return a->Payload.AccessPoint.ssid == b->Payload.AccessPoint.ssid
                && a->Payload.AccessPoint.password == b->Payload.AccessPoint.password
                && a->Payload.AccessPoint.mac == b->Payload.AccessPoint.mac;
    }
    return true;
}

bool WiFiRequests::Find(const RequestItem *request) const {
    for (size_t i = 0; i < count; i++) {
        if (Equals(&items[i], request)) {
            return true;
        }
    }
    return false;
}

bool WiFiRequests::Add(RequestItem *request) {
    if (Find(request)) {
        return false;
    }
    if (count >= WiFi_RequestsLimit) {
        ESP_LOGE(TAG_WiFiRequests,
                 "Add, items count exceeded limit to %u",
                 (unsigned int)WiFi_RequestsLimit);
        return false;
    }
    items[count] = std::move(*request);
    count++;
    return true;
}

bool WiFiRequests::Contains(const RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return Find(request);
}

bool WiFiRequests::HasAnother(const RequestItem *current) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    for (size_t i = 0; i < count; i++) {
        if (!Equals(&items[i], current)) {
            return true;
        }
    }
    return false;
}

bool WiFiRequests::Scan(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    bool new_req = Add(&request);
    ESP_LOGD(TAG_WiFiRequests, "Scan, ssid:%s, new_req:%u", ssid, new_req);
    return new_req;
}

bool WiFiRequests::AccessPoint(const char *ssid, const char *password, const char *mac) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    request.Payload.AccessPoint.password = password;
    request.Payload.AccessPoint.mac = mac;
    std::lock_guard<std::mutex> lock(lock_mutex);
    bool new_req = Add(&request);
    ESP_LOGD(TAG_WiFiRequests, "AccessPoint, ssid:%s, new_req:%u", ssid, new_req);
    return new_req;
}

bool WiFiRequests::Station() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    bool new_req = Add(&request);
    ESP_LOGD(TAG_WiFiRequests, "Station, is new req:%u", new_req);
    return new_req;
}

bool WiFiRequests::Pop(RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    if (count == 0) {
        return false;
    }
    *request = std::move(items[0]);
    count--;
    for (size_t i = 0; i < count; i++) {
        items[i] = std::move(items[i + 1]);
    }
    return true;
}

size_t WiFiRequests::Size() const {
    return count;
}

const RequestItem *WiFiRequests::First() const {
    return &items[0];
}
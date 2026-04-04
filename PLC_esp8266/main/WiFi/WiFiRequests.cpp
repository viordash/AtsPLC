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

std::list<RequestItem>::iterator WiFiRequests::Find(RequestItem *request) {
    for (auto it = items.begin(); it != items.end(); it++) {
        const auto &req = *it;
        if (Equals(&req, request)) {
            return it;
        }
    }
    return items.end();
}

bool WiFiRequests::Contains(RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(request);
    return item != items.end();
}

bool WiFiRequests::OneMoreInQueue() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return items.size() > 0;
}

bool WiFiRequests::Scan(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool new_req = item == items.end();
    if (new_req) {
        items.push_front(std::move(request));
    }
    ESP_LOGD(TAG_WiFiRequests, "Scan, ssid:%s, new_req:%u", ssid, new_req);
    return new_req;
}

bool WiFiRequests::RemoveScanner(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != items.end();
    if (exists) {
        items.erase(item);
    }
    ESP_LOGD(TAG_WiFiRequests, "RemoveScanner, ssid:%s, exists:%u", ssid, exists);
    return exists;
}

bool WiFiRequests::AccessPoint(const char *ssid, const char *password, const char *mac) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    request.Payload.AccessPoint.password = password;
    request.Payload.AccessPoint.mac = mac;
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool new_req = item == items.end();
    if (new_req) {
        items.push_front(std::move(request));
    }
    ESP_LOGD(TAG_WiFiRequests, "AccessPoint, ssid:%s, new_req:%u", ssid, new_req);
    return new_req;
}

bool WiFiRequests::RemoveAccessPoint(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != items.end();
    if (exists) {
        items.erase(item);
    }
    ESP_LOGD(TAG_WiFiRequests, "RemoveAccessPoint, ssid:%s, exists:%u", ssid, exists);
    return exists;
}

bool WiFiRequests::Station() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool new_req = item == items.end();
    if (new_req) {
        items.push_front(std::move(request));
    }
    ESP_LOGD(TAG_WiFiRequests, "Station, is new req:%u", new_req);
    return new_req;
}

bool WiFiRequests::RemoveStation() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != items.end();
    if (exists) {
        items.erase(item);
    }
    ESP_LOGD(TAG_WiFiRequests, "RemoveStation, exists:%u", exists);
    return exists;
}

bool WiFiRequests::Pop(RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    if (items.empty()) {
        return false;
    }
    *request = std::move(items.back());
    items.pop_back();
    return true;
}

size_t WiFiRequests::GetSize() const {
    return items.size();
}

const RequestItem &WiFiRequests::GetBack() const {
    return items.back();
}

void WiFiRequests::PopBack() {
    items.pop_back();
}

std::list<RequestItem>::iterator WiFiRequests::GetBegin() {
    return items.begin();
}

std::list<RequestItem>::iterator WiFiRequests::GetEnd() {
    return items.end();
}

std::list<RequestItem>::const_iterator WiFiRequests::GetBegin() const {
    return items.begin();
}

std::list<RequestItem>::const_iterator WiFiRequests::GetEnd() const {
    return items.end();
}

size_t WiFiRequests::Size() const {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return items.size();
}

std::list<RequestItem>::const_iterator WiFiRequests::Begin() const {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return items.begin();
}

std::list<RequestItem>::const_iterator WiFiRequests::End() const {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return items.end();
}

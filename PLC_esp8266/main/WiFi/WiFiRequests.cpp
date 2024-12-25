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

std::list<RequestItem>::iterator WiFiRequests::Add(RequestItem *new_request) {
    ESP_LOGI(TAG_WiFiRequests, "Add, type:%u", new_request->type);

    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (Equals(&request, new_request)) {
            return it;
        }
    }
    push_front(*new_request);
    return end();
}

void WiFiRequests::Remove(std::list<RequestItem>::const_iterator it) {
    erase(it);
}

RequestItem WiFiRequests::Pop() {
    assert(!empty());
    auto request = back();
    return request;
}

void WiFiRequests::StationDone() {
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_Station) {
            erase(it);
            break;
        }
    }
}

void WiFiRequests::ScannerDone(const char *ssid) {
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_Scanner && it->Payload.Scanner.ssid == ssid) {
            it->Payload.Scanner.status = true;
            break;
        }
    }
}

void WiFiRequests::AccessPointDone(const char *ssid) {
    for (auto it = begin(); it != end(); it++) {
        auto request = *it;
        if (request.type == RequestItemType::wqi_AccessPoint
            && it->Payload.AccessPoint.ssid == ssid) {
            erase(it);
            break;
        }
    }
}

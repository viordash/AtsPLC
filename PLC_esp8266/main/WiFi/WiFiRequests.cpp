#include "WiFiRequests.h"
#include "esp_log.h"
#include <cassert>

static const char *TAG_WiFiRequests = "WiFiRequests";

bool WiFiRequests::Equals(const RequestItem *a, const RequestItem *b) const {
    if (a->type != b->type) {
        return false;
    }
    switch (a->type) {
        case wqi_Scanner:
            return a->Payload.Scanner.ssid == b->Payload.Scanner.ssid;

        case wqi_AccessPoint:
            return a->Payload.AccessPoint.ssid == b->Payload.AccessPoint.ssid;

        default:
            return true;
    }
}

std::list<RequestItem>::iterator WiFiRequests::AddRequest(RequestItem *new_request) {
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

RequestItem WiFiRequests::PopRequest() {
    assert(!empty());
    auto request = back();
    pop_back();
    return request;
}

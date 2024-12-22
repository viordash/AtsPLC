#include "WiFiRequests.h"

bool WiFiRequests::Equals(const RequestItem *a, const RequestItem *b) const {
    if (a->type != b->type) {
        return false;
    }
    switch (a->type) {
        case wqi_Scaner:
            return a->Payload.Scaner.ssid == b->Payload.Scaner.ssid;

        case wqi_AccessPoint:
            return a->Payload.AccessPoint.ssid == b->Payload.AccessPoint.ssid;

        default:
            return true;
    }
}

std::list<RequestItem>::iterator WiFiRequests::AddRequest(RequestItem *new_request) {
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
    if (empty()) {
        RequestItem request = {};
        request.type = RequestItemType::wqi_Station;
        return request;
    } else {
        auto request = back();
        pop_back();
        return request;
    }
}

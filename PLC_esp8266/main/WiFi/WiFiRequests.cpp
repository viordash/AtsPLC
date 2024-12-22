#include "WiFiRequests.h"

bool WiFiRequests::Equals(const RequestItem *a, const RequestItem *b) const {
    if (a->type != b->type) {
        return false;
    }
    switch (a->type) {
        case wqi_ScanSsid:
            return a->Payload.ScanSsid.ssid == b->Payload.ScanSsid.ssid;

        case wqi_GenerateSsid:
            return a->Payload.GenerateSsid.ssid == b->Payload.GenerateSsid.ssid;

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
        request.type = RequestItemType::wqi_Connect;
        return request;
    } else {
        auto request = back();
        pop_back();
        return request;
    }
}

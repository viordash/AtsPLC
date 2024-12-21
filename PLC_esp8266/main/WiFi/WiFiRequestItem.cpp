#include "WiFiRequestItem.h"
#include <unordered_set>

bool RequestItemComparator::operator()(const RequestItem &a, const RequestItem &b) const {
    if (a.type != b.type) {
        return false;
    }
    switch (a.type) {
        case wqi_ScanSsid:
            return a.Payload.ScanSsid.ssid == b.Payload.ScanSsid.ssid;

        case wqi_GenerateSsid:
            return a.Payload.GenerateSsid.ssid == b.Payload.GenerateSsid.ssid;

        default:
            return true;
    }
}

size_t RequestItemHash::operator()(const RequestItem &e1) const {
    switch (e1.type) {
        case wqi_ScanSsid:
            return std::hash<size_t>()(((size_t)e1.Payload.ScanSsid.ssid * 10) + (size_t)e1.type);

        case wqi_GenerateSsid:
            return std::hash<size_t>()(((size_t)e1.Payload.GenerateSsid.ssid * 10)
                                       + (size_t)e1.type);

        default:
            return std::hash<size_t>()(e1.type);
    };
};
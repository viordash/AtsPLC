#pragma once

#include <stdlib.h>

enum RequestItemType { //
    wqi_Connect = 0,
    wqi_ScanSsid,
    wqi_GenerateSsid
};

typedef struct {
    RequestItemType type;
    union {
        struct {
            const char *ssid;
            bool status;
        } ScanSsid;
        struct {
            const char *ssid;
        } GenerateSsid;
    } Payload;
} RequestItem;

struct RequestItemComparator {
    bool operator()(const RequestItem &a, const RequestItem &b) const;
};

struct RequestItemHash {
    size_t operator()(const RequestItem &e1) const;
};
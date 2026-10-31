#pragma once

#include <stdint.h>
#include <unistd.h>

struct ScannedSsid {
    const char *ssid;
    uint8_t rssi;
};

#pragma once

#include "config.h"
#include <stdint.h>
#include <unistd.h>

using t_mac = uint64_t;

struct ApClients {
    const char *ssid;
    t_mac clients[WiFi_Hotspot_Max_Clients];
    size_t count;
};

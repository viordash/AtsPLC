#include "LogicProgram/NetworksList.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_NetworksList = "NetworksList";

NetworksList::NetworksList() {
}

NetworksList::~NetworksList() {
    while (!empty()) {
        auto it = begin();
        auto network = *it;
        erase(it);
        ESP_LOGD(TAG_NetworksList, "delete network: %p", network);
        delete network;
    }
}

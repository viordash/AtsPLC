#include "LogicProgram/Ladder.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Ladder = "Ladder";

Ladder::Ladder() {
}

Ladder::~Ladder() {
    while (!empty()) {
        auto it = begin();
        auto network = *it;
        erase(it);
        ESP_LOGD(TAG_Ladder, "delete network: %p", network);
        delete network;
    }
}

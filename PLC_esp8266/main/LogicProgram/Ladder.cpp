#include "LogicProgram/Ladder.h"
#include "Display/ScrollBar.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
extern "C" {
#include "hotreload_service.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Ladder::Ladder() {
}

Ladder::~Ladder() {
    RemoveAll();
}

void Ladder::RemoveAll() {
    while (!empty()) {
        auto it = begin();
        auto network = *it;
        erase(it);
        ESP_LOGD(TAG_Ladder, "delete network: %p", network);
        delete network;
    }
}

bool Ladder::DoAction() {
    bool any_changes = false;
    for (auto it = begin(); it != end(); ++it) {
        any_changes |= (*it)->DoAction();
    }
    return any_changes;
}

IRAM_ATTR bool Ladder::Render(uint8_t *fb) {
    bool res = true;

    for (size_t i = hotreload->view_top_index; i < size(); i++) {
        uint8_t network_number = i - hotreload->view_top_index;
        if (network_number >= Ladder::MaxViewPortCount) {
            break;
        }
        res &= at(i)->Render(fb, i - hotreload->view_top_index);
    }

    ScrollBar::Render(fb, size(), Ladder::MaxViewPortCount, hotreload->view_top_index);
    return res;
}

void Ladder::Append(Network *network) {
    ESP_LOGI(TAG_Ladder, "append network: %p", network);
    push_back(network);
}

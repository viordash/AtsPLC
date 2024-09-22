#include "LogicProgram/Ladder.h"
#include "Display/ScrollBar.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Ladder::Ladder() {
    view_top_index = 0;
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
    view_top_index = 0;
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

    for (size_t i = 0; i < std::min(Ladder::MaxViewPortCount, size()); i++) {
        res &= at(i + view_top_index)->Render(fb, i);
    }

    ScrollBar::Render(fb, size(), Ladder::MaxViewPortCount, view_top_index);
    return res;
}

void Ladder::Append(Network *network) {
    ESP_LOGI(TAG_Ladder, "append network: %p", network);
    push_back(network);
}

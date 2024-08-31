#include "LogicProgram/Network.h"
#include "Display/display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Network = "Network";

Network::Network(uint8_t network_number, LogicItemState state) : LogicElement() {
    this->network_number = network_number;
    this->state = state;
}

Network::~Network() {
    while (!empty()) {
        auto it = begin();
        auto element = *it;
        erase(it);
        ESP_LOGD(TAG_Network, "delete elem: %p", element);
        delete element;
    }
}

bool Network::DoAction() {
    return DoAction(false, state);
}

bool Network::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    bool any_changes = false;

    for (auto it = begin(); it != end(); ++it) {
        auto element = *it;
        prev_elem_changed = element->DoAction(prev_elem_changed, prev_elem_state);
        prev_elem_state = element->state;
        any_changes |= prev_elem_changed;
    }
    return any_changes;
}

bool Network::Render(uint8_t *fb) {
    Point start_point = { 0,
                          (uint8_t)(INCOME_RAIL_TOP + INCOME_RAIL_HEIGHT * network_number
                                    + INCOME_RAIL_OUTCOME_TOP) };
    return Render(fb, state, &start_point);
}

bool Network::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    switch (prev_elem_state) {
        case LogicItemState::lisActive:
            res = draw_active_income_rail(fb, network_number);
            break;

        default:
            res = draw_passive_income_rail(fb, network_number);
            break;
    }
    start_point->x += INCOME_RAIL_WIDTH;
    for (auto it = begin(); res && it != end(); ++it) {
        auto element = *it;
        res = element->Render(fb, prev_elem_state, start_point);
        prev_elem_state = element->state;
    }

    if (!res) {
        return res;
    }
    res = draw_outcome_rail(fb, network_number);

    return res;
}

void Network::Append(LogicElement *element) {
    ESP_LOGI(TAG_Network, "append elem: %p", element);
    push_back(element);
}
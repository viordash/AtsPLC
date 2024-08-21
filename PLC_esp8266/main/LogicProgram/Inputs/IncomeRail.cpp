#include "LogicProgram/Inputs/IncomeRail.h"
#include "Display/display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IncomeRail::IncomeRail(const Controller *controller, uint8_t network_number, LogicItemState state)
    : InputBase(controller,
                { 0, (uint8_t)(INCOME_RAIL_TOP + INCOME_RAIL_HEIGHT * network_number) }) {
    this->network_number = network_number;
    this->state = state;
}

IncomeRail::~IncomeRail() {
    while (!empty()) {
        auto it = begin();
        auto first = *it;
        delete first;
    }
}

Point IncomeRail::OutcomingPoint() {
    uint8_t x_pos = INCOME_RAIL_WIDTH;
    uint8_t y_pos = incoming_point.y + INCOME_RAIL_OUTCOME_TOP;
    return { x_pos, y_pos };
}

bool IncomeRail::DoAction() {
    return DoAction(false);
}

bool IncomeRail::DoAction(bool prev_changed) {
    bool any_changes = false;

    for (auto it = begin(); it != end(); ++it) {
        auto element = *it;
        prev_changed = element->DoAction(prev_changed);
        any_changes |= prev_changed;
    }
    return any_changes;
}

bool IncomeRail::Render(uint8_t *fb) {
    return Render(fb, state);
}

bool IncomeRail::Render(uint8_t *fb, LogicItemState state) {
    bool res = true;
    switch (state) {
        case LogicItemState::lisActive:
            res = draw_active_income_rail(fb, network_number);
            break;

        default:
            res = draw_passive_income_rail(fb, network_number);
            break;
    }

    for (auto it = begin(); res && it != end(); ++it) {
        auto element = *it;
        res = element->Render(fb, state);
        state = element->GetState();
    }

    return res;
}

void IncomeRail::Append(LogicElement *element) {
    push_back(element);
}
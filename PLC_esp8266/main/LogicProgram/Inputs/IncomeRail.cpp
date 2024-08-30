#include "LogicProgram/Inputs/IncomeRail.h"
#include "Display/display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IncomeRail::IncomeRail(uint8_t network_number, LogicItemState state) : LogicElement() {
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
    // uint8_t x_pos = INCOME_RAIL_WIDTH;
    // uint8_t y_pos = incoming_point.y + INCOME_RAIL_OUTCOME_TOP;
    // return { x_pos, y_pos };
    return { 0, 0 };
}

bool IncomeRail::DoAction() {
    return DoAction(false, state);
}

bool IncomeRail::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    bool any_changes = false;

    for (auto it = begin(); it != end(); ++it) {
        auto element = *it;
        prev_elem_changed = element->DoAction(prev_elem_changed, prev_elem_state);
        prev_elem_state = element->state;
        any_changes |= prev_elem_changed;
    }
    return any_changes;
}

bool IncomeRail::Render(uint8_t *fb) {
    return Render(fb, state, { 0, 0 });
}

bool IncomeRail::Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) {
    bool res = true;
    switch (prev_elem_state) {
        case LogicItemState::lisActive:
            res = draw_active_income_rail(fb, network_number);
            break;

        default:
            res = draw_passive_income_rail(fb, network_number);
            break;
    }

    for (auto it = begin(); res && it != end(); ++it) {
        auto element = *it;
        res = element->Render(fb, prev_elem_state, start_point);
        prev_elem_state = element->state;
    }

    return res;
}

void IncomeRail::Append(LogicElement *element) {
    push_back(element);
}
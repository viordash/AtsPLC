#include "LogicProgram/Inputs/IncomeRail.h"
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
    StatefulElement *next = nextElement;

    while (next != NULL) {
        prev_changed = next->DoAction(prev_changed);
        any_changes |= prev_changed;
        next = next->nextElement;
    }
    return any_changes;
}

bool IncomeRail::Render(uint8_t *fb) {
    switch (state) {
        case LogicItemState::lisActive:
            return draw_active_income_rail(fb, network_number);

        default:
            return draw_passive_income_rail(fb, network_number);
    }
}
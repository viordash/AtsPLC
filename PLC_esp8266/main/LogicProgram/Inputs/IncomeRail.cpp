#include "LogicProgram/Inputs/IncomeRail.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IncomeRail::IncomeRail(const Controller &controller, uint8_t network_number)
    : InputBase(controller,
                { 0, (uint8_t)(INCOME_RAIL_TOP + INCOME_RAIL_HEIGHT * network_number) },
                LogicItemState::lisActive) {
    this->network_number = network_number;
}

IncomeRail::~IncomeRail() {
}

bool IncomeRail::Render(uint8_t *fb) {
    return draw_income_rail(fb, network_number);
}

Point IncomeRail::OutcomingPoint() {
    uint8_t x_pos = 2;
    uint8_t y_pos = incoming_point.y + (INCOME_RAIL_HEIGHT / 2);
    return { x_pos, y_pos };
}

bool IncomeRail::DoAction() {
    return true;
}
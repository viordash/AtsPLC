#include "LogicProgram/Outputs/OutcomeRail.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutcomeRail::OutcomeRail(const Controller &controller, uint8_t network_number)
    : OutputBase(controller,
                 MapIO::Outcome,
                 { 0, (uint8_t)(OUTCOME_RAIL_TOP + OUTCOME_RAIL_HEIGHT * network_number) }) {
    this->network_number = network_number;
    state = LogicItemState::lisActive;
}

OutcomeRail::~OutcomeRail() {
}

bool OutcomeRail::DoAction() {
    return true;
}

const Bitmap *OutcomeRail::GetCurrentBitmap() {
    return NULL;
}

void OutcomeRail::Render(uint8_t *fb) {
    draw_outcome_rail(fb, network_number);
}

Point OutcomeRail::OutcomingPoint() {
    return incoming_point;
}
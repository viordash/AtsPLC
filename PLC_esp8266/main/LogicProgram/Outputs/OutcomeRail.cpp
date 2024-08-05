#include "LogicProgram/Outputs/OutcomeRail.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutcomeRail::OutcomeRail(uint8_t network_number)
    : OutputBase(MapIO::Outcome,
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
    (void)fb;
    draw_outcome_rail(network_number);
}

Point OutcomeRail::OutcomingPoint() {
    return incoming_point;
}
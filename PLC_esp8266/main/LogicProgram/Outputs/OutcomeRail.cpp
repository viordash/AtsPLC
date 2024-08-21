#include "LogicProgram/Outputs/OutcomeRail.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutcomeRail::OutcomeRail(const CommonOutput *incoming_item, uint8_t network_number)
    : LogicElement(incoming_item->controller) {
    this->network_number = network_number;
}

OutcomeRail::~OutcomeRail() {
}

bool OutcomeRail::DoAction(bool prev_changed) {
    return prev_changed;
}

bool OutcomeRail::Render(uint8_t *fb, LogicItemState prev_state) {
    (void)state;
    return draw_outcome_rail(fb, network_number);
}

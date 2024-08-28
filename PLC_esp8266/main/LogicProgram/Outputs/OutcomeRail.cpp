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

bool OutcomeRail::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_state;
    return prev_elem_changed;
}

bool OutcomeRail::Render(uint8_t *fb, LogicItemState prev_elem_state) {
    (void)state;
    (void)prev_elem_state;
    return draw_outcome_rail(fb, network_number);
}

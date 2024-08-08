#include "LogicProgram/Outputs/OutcomeRail.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutcomeRail::OutcomeRail(uint8_t network_number) : DisplayItemBase() {
    this->network_number = network_number;
}

OutcomeRail::~OutcomeRail() {
}

void OutcomeRail::Render(uint8_t *fb) {
    draw_outcome_rail(fb, network_number);
}

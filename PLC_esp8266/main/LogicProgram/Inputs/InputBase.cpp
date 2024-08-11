#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller &controller,
                     const Point &incoming_point,
                     LogicItemState init_state)
    : LogicElement(controller, init_state), DisplayChainItem(incoming_point) {
}

InputBase::InputBase(InputBase *incoming_item)
    : LogicElement(incoming_item->controller, LogicItemState::lisPassive),
      DisplayChainItem(incoming_item) {
}

InputBase::~InputBase() {
}

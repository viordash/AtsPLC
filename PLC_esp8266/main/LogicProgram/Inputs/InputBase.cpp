#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller &controller,
                     const Point &incoming_point,
                     LogicItemState incoming_item_state)
    : LogicElement(controller), DisplayChainItem(incoming_point, incoming_item_state) {
}

InputBase::InputBase(const InputBase &prev_item)
    : LogicElement(prev_item.controller), DisplayChainItem(prev_item) {
}

InputBase::~InputBase() {
}

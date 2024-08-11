#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller &controller,
                     const MapIO io_adr,
                     const Point &incoming_point,
                     LogicItemState incoming_item_state)
    : LogicInputElement(controller, io_adr), DisplayChainItem(incoming_point, incoming_item_state),
      LabeledLogicItem(MapIONames[io_adr]) {
}

InputBase::InputBase(const MapIO io_adr, const InputBase &prev_item)
    : LogicInputElement(prev_item.controller, io_adr), DisplayChainItem(prev_item),
      LabeledLogicItem(MapIONames[io_adr]) {
}

InputBase::~InputBase() {
}

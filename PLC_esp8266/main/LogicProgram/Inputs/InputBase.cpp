#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller &controller, const MapIO io_adr, const Point &incoming_point)
    : LogicInputElement(controller, io_adr), ChainItem(incoming_point),
      LabeledLogicItem(MapIONames[io_adr]) {
    this->prev_item = NULL;
}

InputBase::InputBase(const MapIO io_adr, InputBase &prev_item)
    : LogicInputElement(prev_item.controller, io_adr), ChainItem(prev_item.OutcomingPoint()),
      LabeledLogicItem(MapIONames[io_adr]) {
    this->prev_item = &prev_item;
}

InputBase::~InputBase() {
}

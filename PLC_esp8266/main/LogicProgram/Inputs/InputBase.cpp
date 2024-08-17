#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller *controller, const Point &incoming_point)
    : StatefulElement(controller), DisplayChainItem(incoming_point) {
}

InputBase::~InputBase() {
}

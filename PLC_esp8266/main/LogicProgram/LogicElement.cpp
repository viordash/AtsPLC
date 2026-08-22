#include "LogicProgram/LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicElement::LogicElement() : EditableElement() {
    this->state = LogicItemState::lisPassive;
}

LogicElement::~LogicElement() {
}

LogicItemState LogicElement::GetState() {
    return state;
}

bool LogicElement::DoActionGuard(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (prev_elem_changed) {
        return true;
    }

    if (prev_elem_state == LogicItemState::lisActive) {
        return true;
    }

    if (prev_elem_state == LogicItemState::lisStop) {
        return true;
    }

    return false;
}

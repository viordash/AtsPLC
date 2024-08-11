#include "LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicElement::LogicElement(const Controller &controller, LogicItemState init_state) {
    this->controller = controller;
    this->state = init_state;
}

LogicElement::~LogicElement() {
}

LogicItemState LogicElement::GetState() {
    return state;
}

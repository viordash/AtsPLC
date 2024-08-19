#include "StatefulElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatefulElement::StatefulElement(const Controller *controller) : LogicElement(controller) {
    this->state = LogicItemState::lisPassive;
}

StatefulElement::~StatefulElement() {
}

LogicItemState StatefulElement::GetState() {
    return state;
}
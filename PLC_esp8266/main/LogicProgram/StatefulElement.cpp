#include "StatefulElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatefulElement::StatefulElement(const Controller *controller) {
    this->controller = controller;
    this->state = LogicItemState::lisPassive;
    this->nextElement = NULL;
}

StatefulElement::~StatefulElement() {
}

LogicItemState StatefulElement::GetState() {
    return state;
}

void StatefulElement::Bind(StatefulElement *element) {
    this->nextElement = element;
}
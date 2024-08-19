#include "LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicElement::LogicElement(const Controller *controller) : DisplayItemBase() {
    this->controller = controller;
    this->nextElement = NULL;
}

LogicElement::~LogicElement() {
}

void LogicElement::Bind(LogicElement *element) {
    this->nextElement = element;
}
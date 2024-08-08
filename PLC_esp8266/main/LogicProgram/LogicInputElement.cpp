#include "LogicInputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicInputElement::LogicInputElement(const Controller &controller, const MapIO io_adr)
    : AddressableLogicElement(controller, io_adr) {
}

LogicInputElement::~LogicInputElement() {
}

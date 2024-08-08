#include "LogicOutputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicOutputElement::LogicOutputElement(const Controller &controller, const MapIO io_adr)
    : AddressableLogicElement(controller, io_adr) {
}

LogicOutputElement::~LogicOutputElement() {
}

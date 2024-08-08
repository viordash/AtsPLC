#include "AddressableLogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AddressableLogicElement::AddressableLogicElement(const Controller &controller, const MapIO addr)
    : LogicElement(controller) {
    this->name = MapIONames[addr];
}

AddressableLogicElement::~AddressableLogicElement() {
}

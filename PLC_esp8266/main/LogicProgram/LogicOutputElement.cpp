#include "LogicOutputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicOutputElement::LogicOutputElement(const MapIO io_adr) {
    (void)io_adr;
}

LogicOutputElement::~LogicOutputElement() {
}

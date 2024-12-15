#include "InputOutputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputOutputElement::InputOutputElement() : InputElement() {
    Output = NULL;
}

InputOutputElement::~InputOutputElement() {
}

void InputOutputElement::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    switch (io_adr) {
        case MapIO::O1:
            Output = &Controller::O1;
            break;
        case MapIO::O2:
            Output = &Controller::O2;
            break;
        case MapIO::V1:
            Output = &Controller::V1;
            break;
        case MapIO::V2:
            Output = &Controller::V2;
            break;
        case MapIO::V3:
            Output = &Controller::V3;
            break;
        case MapIO::V4:
            Output = &Controller::V4;
            break;

        default:
            Output = NULL;
            break;
    }
}

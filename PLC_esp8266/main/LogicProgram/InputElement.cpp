#include "InputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputElement::InputElement() {
    GetValue = NULL;
}

void InputElement::SetIoAdr(const MapIO io_adr) {
    this->io_adr = io_adr;
    switch (io_adr) {
        case MapIO::DI:
            GetValue = Controller::GetDIRelativeValue;
            break;
        case MapIO::AI:
            GetValue = Controller::GetAIRelativeValue;
            break;
        case MapIO::V1:
            GetValue = Controller::GetV1RelativeValue;
            break;
        case MapIO::V2:
            GetValue = Controller::GetV2RelativeValue;
            break;
        case MapIO::V3:
            GetValue = Controller::GetV3RelativeValue;
            break;
        case MapIO::V4:
            GetValue = Controller::GetV4RelativeValue;
            break;

        case MapIO::O1:
            GetValue = Controller::GetO1RelativeValue;
            break;
        case MapIO::O2:
            GetValue = Controller::GetO1RelativeValue;
            break;

        default:
            GetValue = NULL;
            break;
    }
}

MapIO InputElement::GetIoAdr() {
    return this->io_adr;
}
#include "LogicOutputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicOutputElement::LogicOutputElement(const MapIO io_adr) {
    switch (io_adr) {
        case MapIO::O1:
            SetValue = Controller::SetO1RelativeValue;
            break;
        case MapIO::O2:
            SetValue = Controller::SetO2RelativeValue;
            break;
        case MapIO::V1:
            SetValue = Controller::SetV1RelativeValue;
            break;
        case MapIO::V2:
            SetValue = Controller::SetV2RelativeValue;
            break;
        case MapIO::V3:
            SetValue = Controller::SetV3RelativeValue;
            break;
        case MapIO::V4:
            SetValue = Controller::SetV4RelativeValue;
            break;

        default:
            SetValue = NULL;
            break;
    }
}

LogicOutputElement::~LogicOutputElement() {
}

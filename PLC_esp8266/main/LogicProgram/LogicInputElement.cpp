#include "LogicInputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicInputElement::LogicInputElement(const Controller &controller, const MapIO io_adr)
    : LogicElement(controller) {
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

        default:
            GetValue = NULL;
            break;
    }
}

LogicInputElement::~LogicInputElement() {
}

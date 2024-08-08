#include "LogicInputElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogicInputElement::LogicInputElement(const Controller &controller, const MapIO io_adr)
    : AddressableLogicElement(controller, io_adr) {
    switch (io_adr) {
        case MapIO::DI:
            GetValue = std::bind(&Controller::GetDIRelativeValue, controller);
            break;
        case MapIO::AI:
            GetValue = std::bind(&Controller::GetAIRelativeValue, controller);
            break;
        case MapIO::V1:
            GetValue = std::bind(&Controller::GetV1RelativeValue, controller);
            break;
        case MapIO::V2:
            GetValue = std::bind(&Controller::GetV2RelativeValue, controller);
            break;
        case MapIO::V3:
            GetValue = std::bind(&Controller::GetV3RelativeValue, controller);
            break;
        case MapIO::V4:
            GetValue = std::bind(&Controller::GetV4RelativeValue, controller);
            break;

        default:
            GetValue = NULL;
            break;
    }
}

LogicInputElement::~LogicInputElement() {
}

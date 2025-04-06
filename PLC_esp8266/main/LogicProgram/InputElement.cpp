#include "LogicProgram/InputElement.h"
#include "LogicProgram/Bindings/DateTimeBinding.h"
#include "LogicProgram/Bindings/WiFiApBinding.h"
#include "LogicProgram/Bindings/WiFiBinding.h"
#include "LogicProgram/Bindings/WiFiStaBinding.h"
#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/Inputs/CommonComparator.h"
#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/Inputs/Indicator.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputElement::InputElement() {
    Input = NULL;
}

InputElement::~InputElement() {
}

void InputElement::SetIoAdr(const MapIO io_adr) {
    switch (io_adr) {
        case MapIO::DI:
            Input = &Controller::DI;
            break;
        case MapIO::AI:
            Input = &Controller::AI;
            break;
        case MapIO::V1:
            Input = &Controller::V1;
            break;
        case MapIO::V2:
            Input = &Controller::V2;
            break;
        case MapIO::V3:
            Input = &Controller::V3;
            break;
        case MapIO::V4:
            Input = &Controller::V4;
            break;

        case MapIO::O1:
            Input = &Controller::O1;
            break;
        case MapIO::O2:
            Input = &Controller::O2;
            break;

        default:
            Input = NULL;
            break;
    }
}

MapIO InputElement::GetIoAdr() {
    if (Input == &Controller::DI) {
        return MapIO::DI;
    }
    if (Input == &Controller::AI) {
        return MapIO::AI;
    }
    if (Input == &Controller::V1) {
        return MapIO::V1;
    }
    if (Input == &Controller::V2) {
        return MapIO::V2;
    }
    if (Input == &Controller::V3) {
        return MapIO::V3;
    }
    if (Input == &Controller::V4) {
        return MapIO::V4;
    }
    if (Input == &Controller::O1) {
        return MapIO::O1;
    }
    if (Input == &Controller::O2) {
        return MapIO::O2;
    }
    return MapIO::DI;
}

InputElement *InputElement::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_InputNC:
        case TvElementType::et_InputNO:
            return static_cast<CommonInput *>(logic_element);

        case TvElementType::et_ComparatorEq:
        case TvElementType::et_ComparatorGE:
        case TvElementType::et_ComparatorGr:
        case TvElementType::et_ComparatorLE:
        case TvElementType::et_ComparatorLs:
            return static_cast<CommonComparator *>(logic_element);

        case TvElementType::et_Indicator:
            return static_cast<Indicator *>(logic_element);

        case TvElementType::et_WiFiBinding:
            return static_cast<WiFiBinding *>(logic_element);

        case TvElementType::et_WiFiStaBinding:
            return static_cast<WiFiStaBinding *>(logic_element);

        case TvElementType::et_WiFiApBinding:
            return static_cast<WiFiApBinding *>(logic_element);

        case TvElementType::et_DateTimeBinding:
            return static_cast<DateTimeBinding *>(logic_element);

        default:
            return NULL;
    }
}
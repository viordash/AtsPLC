#include "LogicProgram/InputElement.h"
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
    GetValue = NULL;
    Input = NULL;
}

void InputElement::SetIoAdr(const MapIO io_adr) {
    this->io_adr = io_adr;
    delete Input;
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
            GetValue = Controller::GetO1RelativeValue;
            break;
        case MapIO::O2:
            GetValue = Controller::GetO2RelativeValue;
            break;

        default:
            GetValue = NULL;
            break;
    }
}

MapIO InputElement::GetIoAdr() {
    return this->io_adr;
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

        case TvElementType::et_DirectOutput:
        case TvElementType::et_SetOutput:
        case TvElementType::et_ResetOutput:
        case TvElementType::et_IncOutput:
        case TvElementType::et_DecOutput:
            return static_cast<CommonOutput *>(logic_element);

        default:
            return NULL;
    }
}
#include "LogicProgram/InputElement.h"
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
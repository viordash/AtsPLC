#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "LogicProgram/Bindings/DateTimeBinding.h"
#include "LogicProgram/Bindings/WiFiApBinding.h"
#include "LogicProgram/Bindings/WiFiBinding.h"
#include "LogicProgram/Bindings/WiFiStaBinding.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/Indicator.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Inputs/TimerMSecs.h"
#include "LogicProgram/Inputs/TimerSecs.h"
#include "LogicProgram/Outputs/DecOutput.h"
#include "LogicProgram/Outputs/DirectOutput.h"
#include "LogicProgram/Outputs/IncOutput.h"
#include "LogicProgram/Outputs/ResetOutput.h"
#include "LogicProgram/Outputs/SetOutput.h"
#include "LogicProgram/Settings/SettingsElement.h"
#include "LogicProgram/Wire.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_LogicElementFactory = "LogicElementFactory";

LogicElement *LogicElementFactory::Create(TvElementType element_type) {
    ESP_LOGD(TAG_LogicElementFactory, "Create: element type:%u", element_type);

    LogicElement *element = NULL;

    switch (element_type) {
        case et_InputNC:
            element = new InputNC();
            break;
        case et_InputNO:
            element = new InputNO();
            break;
        case et_TimerSecs:
            element = new TimerSecs();
            break;
        case et_TimerMSecs:
            element = new TimerMSecs();
            break;
        case et_ComparatorEq:
            element = new ComparatorEq();
            break;
        case et_ComparatorGE:
            element = new ComparatorGE();
            break;
        case et_ComparatorGr:
            element = new ComparatorGr();
            break;
        case et_ComparatorLE:
            element = new ComparatorLE();
            break;
        case et_ComparatorLs:
            element = new ComparatorLs();
            break;
        case et_DirectOutput:
            element = new DirectOutput();
            break;
        case et_SetOutput:
            element = new SetOutput();
            break;
        case et_ResetOutput:
            element = new ResetOutput();
            break;
        case et_IncOutput:
            element = new IncOutput();
            break;
        case et_DecOutput:
            element = new DecOutput();
            break;
        case et_Wire:
            element = new Wire();
            break;
        case et_Indicator:
            element = new Indicator();
            break;
        case et_WiFiBinding:
            element = new WiFiBinding();
            break;
        case et_WiFiStaBinding:
            element = new WiFiStaBinding();
            break;
        case et_WiFiApBinding:
            element = new WiFiApBinding();
            break;
        case et_Settings:
            element = new SettingsElement();
            break;
        case et_DateTimeBinding:
            element = new DateTimeBinding();
            break;
        default:
            break;
    }
    return element;
}

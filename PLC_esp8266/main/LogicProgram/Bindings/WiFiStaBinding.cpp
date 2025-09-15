#include "LogicProgram/Bindings/WiFiStaBinding.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiStaBinding = "WiFiStaBinding";

WiFiStaBinding::WiFiStaBinding() : CommonWiFiBinding() {
}

WiFiStaBinding::WiFiStaBinding(const MapIO io_adr) : CommonWiFiBinding(io_adr) {
}

WiFiStaBinding::~WiFiStaBinding() {
}

ActionStatus WiFiStaBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return { false, state };
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        Controller::BindVariableToStaWiFi(GetIoAdr());
    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        Controller::UnbindVariable(GetIoAdr());
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_WiFiStaBinding, ".");
    }
    return { any_changes, state };
}

IRAM_ATTR void
WiFiStaBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    Point top_left = { start_point->x, (uint8_t)(start_point->y + Top) };

    CommonWiFiBinding::Render(fb, prev_elem_state, start_point);

    top_left.x += LeftPadding + 22;
    top_left.x += bitmap.size.width + 1;

    ASSERT(draw_text_f6X12(fb, top_left.x, top_left.y + 6, "STA CLNT") > 0);
}

TvElementType WiFiStaBinding::GetElementType() {
    return TvElementType::et_WiFiStaBinding;
}

WiFiStaBinding *WiFiStaBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiStaBinding:
            return static_cast<WiFiStaBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiStaBinding::Change() {
    ESP_LOGI(TAG_WiFiStaBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiStaBinding::EditingPropertyId::wsbepi_None:
            CommonWiFiBinding::Change();
            break;

        default:
            EndEditing();
            break;
    }
}

const AllowedIO WiFiStaBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

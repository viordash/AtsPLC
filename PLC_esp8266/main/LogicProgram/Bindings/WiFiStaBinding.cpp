#include "LogicProgram/Bindings/WiFiStaBinding.h"
#include "Display/bitmaps/wifi_sta_binding.h"
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
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiBinding = "WiFiStaBinding";

WiFiStaBinding::WiFiStaBinding() : LogicElement(), InputElement() {
}

WiFiStaBinding::WiFiStaBinding(const MapIO io_adr, const char *ssid) : WiFiStaBinding() {
    SetIoAdr(io_adr);
}

WiFiStaBinding::~WiFiStaBinding() {
}

void WiFiStaBinding::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
}

bool WiFiStaBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        Controller::BindVariableToWiFi(GetIoAdr(), NULL);
    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        Controller::UnbindVariable(GetIoAdr());
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_WiFiBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR bool
WiFiStaBinding::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, LeftPadding);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, LeftPadding, false);
    }
    if (!res) {
        return res;
    }

    start_point->x += LeftPadding;

    Point top_left = { start_point->x, (uint8_t)(start_point->y + Top) };
    Point bottom_left = { start_point->x, (uint8_t)(top_left.y + Height) };
    Point top_right = { (uint8_t)(start_point->x + Width), top_left.y };

    bool blink_body_on_editing = editable_state == EditableElement::ElementState::des_Editing
                              && (WiFiStaBinding::EditingPropertyId)editing_property_id
                                     == WiFiStaBinding::EditingPropertyId::wsbepi_None
                              && Blinking_50();
    if (!blink_body_on_editing) {
        res = draw_horz_line(fb, top_left.x, top_left.y, Width);
        if (!res) {
            return res;
        }
        res = draw_horz_line(fb, bottom_left.x, bottom_left.y, Width);
        if (!res) {
            return res;
        }
        res = draw_vert_line(fb, top_left.x, top_left.y, Height);
        if (!res) {
            return res;
        }
        res = draw_vert_line(fb, top_right.x, top_right.y, Height);
        if (!res) {
            return res;
        }
    }

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (WiFiStaBinding::EditingPropertyId)editing_property_id
                                      == WiFiStaBinding::EditingPropertyId::wsbepi_ConfigureIOAdr
                               && Blinking_50();
    res =
        blink_label_on_editing || (draw_text_f8X14(fb, top_left.x + 4, top_left.y + 4, label) > 0);
    if (!res) {
        return res;
    }
    top_left.x += 22;
    if (!blink_body_on_editing) {
        draw_bitmap(fb, top_left.x, top_left.y + 5, &bitmap);
    }
    top_left.x += bitmap.size.width + 1;

    res = draw_text_f6X12(fb, top_left.x, top_left.y + 6, "STA CLNT") > 0;

    if (!res) {
        return res;
    }

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    return res;
}

size_t WiFiStaBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    auto io_adr = GetIoAdr();
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiStaBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;

    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    SetIoAdr(_io_adr);
    return readed;
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

void WiFiStaBinding::SelectPrior() {
    ESP_LOGI(TAG_WiFiBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiStaBinding::EditingPropertyId::wsbepi_None:
            break;
        case WiFiStaBinding::EditingPropertyId::wsbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr--;
            if (io_adr < 0) {
                io_adr = allowed_inputs.count - 1;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        default:
            break;
    }
}

void WiFiStaBinding::SelectNext() {
    ESP_LOGI(TAG_WiFiBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiStaBinding::EditingPropertyId::wsbepi_None:
            break;
        case WiFiStaBinding::EditingPropertyId::wsbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr++;
            if (io_adr >= (int)allowed_inputs.count) {
                io_adr = 0;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        default:
            break;
    }
}

void WiFiStaBinding::PageUp() {
}

void WiFiStaBinding::PageDown() {
}

void WiFiStaBinding::Change() {
    ESP_LOGI(TAG_WiFiBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiStaBinding::EditingPropertyId::wsbepi_None:
            editing_property_id = WiFiStaBinding::EditingPropertyId::wsbepi_ConfigureIOAdr;
            break;

        default:
            EndEditing();
            break;
    }
}
void WiFiStaBinding::Option() {
}

const AllowedIO WiFiStaBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

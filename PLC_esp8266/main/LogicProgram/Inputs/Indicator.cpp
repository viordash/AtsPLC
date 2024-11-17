#include "LogicProgram/Inputs/Indicator.h"
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

static const char *TAG_Indicator = "Indicator";

Indicator::Indicator() : LogicElement(), InputElement() {
}

Indicator::Indicator(const MapIO io_adr) : Indicator() {
    SetIoAdr(io_adr);
}

Indicator::~Indicator() {
}

void Indicator::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
}

bool Indicator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    state = prev_elem_state;
    return prev_elem_changed;
}

IRAM_ATTR bool Indicator::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
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
                              && (Indicator::EditingPropertyId)editing_property_id
                                     == Indicator::EditingPropertyId::ciepi_None
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
                               && (Indicator::EditingPropertyId)editing_property_id
                                      == Indicator::EditingPropertyId::ciepi_ConfigureInputAdr
                               && Blinking_50();
    res = blink_label_on_editing || draw_text_f8X14(fb, start_point->x + 4, top_left.y + 4, label);
    if (!res) {
        return res;
    }
    top_left.x += 23;
    res = draw_vert_line(fb, top_left.x, top_left.y, Height);
    if (!res) {
        return res;
    }

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    return res;
}

size_t Indicator::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t Indicator::Deserialize(uint8_t *buffer, size_t buffer_size) {
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

TvElementType Indicator::GetElementType() {
    return TvElementType::et_Indicator;
}

Indicator *Indicator::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_Indicator:
            return static_cast<Indicator *>(logic_element);

        default:
            return NULL;
    }
}

void Indicator::SelectPrior() {
    ESP_LOGI(TAG_Indicator, "SelectPrior");

    auto allowed_inputs = GetAllowedInputs();
    auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
    io_adr--;
    if (io_adr < 0) {
        io_adr = allowed_inputs.count - 1;
    }
    SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
}

void Indicator::SelectNext() {
    ESP_LOGI(TAG_Indicator, "SelectNext");

    auto allowed_inputs = GetAllowedInputs();
    auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
    io_adr++;
    if (io_adr >= (int)allowed_inputs.count) {
        io_adr = 0;
    }
    SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
}

void Indicator::PageUp() {
}

void Indicator::PageDown() {
}

void Indicator::Change() {
    ESP_LOGI(TAG_Indicator, "Change");
    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_None:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureInputAdr;
            break;

        default:
            EndEditing();
            break;
    }
}

const AllowedIO Indicator::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2,
                                 MapIO::V3, MapIO::V4, MapIO::O1, MapIO::O2 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}
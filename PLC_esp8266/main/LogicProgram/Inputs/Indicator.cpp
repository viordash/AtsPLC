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
    high_scale = 100.0f;
    low_scale = 0.0f;
    decimal_point = 2;
    UpdateScale();
    PrintOutValue(0);
}

Indicator::Indicator(const MapIO io_adr) : Indicator() {
    SetIoAdr(io_adr);
}

Indicator::~Indicator() {
}

void Indicator::UpdateScale() {
    sprintf(str_format, "%%%u.%uf", max_symbols_count, decimal_point);
}

void Indicator::PrintOutValue(uint8_t eng_value) {
    int32_t eng_range = LogicElement::MaxValue - LogicElement::MinValue;
    float real_range = high_scale - low_scale;
    float ratio = real_range / eng_range;
    float scaled_val = low_scale + (eng_value * ratio);
    snprintf(str_value, sizeof(str_value), str_format, scaled_val);
}

void Indicator::PrintLowScale() {
    sprintf(str_format, "%%0%u.%uf", max_symbols_count, decimal_point);
    snprintf(str_value, sizeof(str_value), str_format, low_scale);
}

void Indicator::AcceptLowScale() {
}

void Indicator::PrintHighScale() {
    sprintf(str_format, "%%0%u.%uf", max_symbols_count, decimal_point);
    snprintf(str_value, sizeof(str_value), str_format, high_scale);
}

void Indicator::AcceptHighScale() {
}

void Indicator::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
}

bool Indicator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        if (Controller::RequestWakeupMs(this, update_period_ms)) {
            any_changes = true;

            switch (editing_property_id) {
                case Indicator::EditingPropertyId::ciepi_None:
                    PrintOutValue(GetValue());
                    break;
                default:
                    break;
            }
        }
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_Indicator, ".");
    }
    return any_changes;
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
                                      == Indicator::EditingPropertyId::ciepi_ConfigureIOAdr
                               && Blinking_50();
    res = blink_label_on_editing || draw_text_f8X14(fb, top_left.x + 4, top_left.y + 4, label);
    if (!res) {
        return res;
    }
    top_left.x += 23;
    res = draw_vert_line(fb, top_left.x, top_left.y, Height);
    if (!res) {
        return res;
    }

    bool show_scales = editable_state == EditableElement::ElementState::des_Editing
                    && (Indicator::EditingPropertyId)editing_property_id
                           != Indicator::EditingPropertyId::ciepi_None
                    && (Indicator::EditingPropertyId)editing_property_id
                           != Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;

    if (show_scales) {
        top_left.x += 4;
        res = RenderScales(fb, &top_left);
    } else {
        top_left.x += 4;
        res = draw_text_f8X14(fb, top_left.x + 4, top_left.y + 4, str_value);
    }
    if (!res) {
        return res;
    }

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    return res;
}

bool Indicator::RenderScales(uint8_t *fb, Point *start_point) {
    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
            return draw_text_f6X12(fb, start_point->x, start_point->y + 8, str_value);

        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
            break;

        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            break;
    }
    return false;
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

void Indicator::SelectPriorSymbol(char *symbol, char extra) {
    switch (*symbol) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *symbol = *symbol - 1;
            break;
        case '0':
            *symbol = extra;
            break;
        default:
            *symbol = '9';
            break;
    }
}

void Indicator::SelectNextSymbol(char *symbol, char extra) {
    switch (*symbol) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            *symbol = *symbol + 1;
            break;
        case '9':
            *symbol = extra;
            break;
        default:
            *symbol = '0';
            break;
    }
}

void Indicator::SelectPrior() {
    ESP_LOGI(TAG_Indicator, "SelectPrior");

    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr--;
            if (io_adr < 0) {
                io_adr = allowed_inputs.count - 1;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            SelectPriorSymbol(&str_value[0], '9');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            SelectPriorSymbol(&str_value[1], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            SelectPriorSymbol(&str_value[2], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            SelectPriorSymbol(&str_value[3], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            SelectPriorSymbol(&str_value[4], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            SelectPriorSymbol(&str_value[5], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            SelectPriorSymbol(&str_value[6], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            SelectPriorSymbol(&str_value[7], '9');
            break;
    }
}

void Indicator::SelectNext() {
    ESP_LOGI(TAG_Indicator, "SelectNext");

    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr++;
            if (io_adr >= (int)allowed_inputs.count) {
                io_adr = 0;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            SelectNextSymbol(&str_value[0], '0');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            SelectNextSymbol(&str_value[1], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            SelectNextSymbol(&str_value[2], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            SelectNextSymbol(&str_value[3], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            SelectNextSymbol(&str_value[4], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            SelectNextSymbol(&str_value[5], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            SelectNextSymbol(&str_value[6], '.');
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            SelectNextSymbol(&str_value[7], '0');
            break;
    }
}

void Indicator::PageUp() {
    this->SelectPrior();
}

void Indicator::PageDown() {
    this->SelectNext();
}

void Indicator::Change() {
    ESP_LOGI(TAG_Indicator, "Change editing_property_id:%d", editing_property_id);
    switch (editing_property_id) {
        case Indicator::EditingPropertyId::ciepi_None:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureIOAdr:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0;
            PrintLowScale();
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0;
            AcceptLowScale();
            PrintHighScale();
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6:
            editing_property_id = Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7;
            break;
        case Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7:
            editing_property_id = Indicator::EditingPropertyId::ciepi_None;
            AcceptHighScale();
            UpdateScale();
            EndEditing();
            break;
    }
}

const AllowedIO Indicator::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2,
                                 MapIO::V3, MapIO::V4, MapIO::O1, MapIO::O2 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}
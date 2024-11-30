#include "LogicProgram/Bindings/WiFiBinding.h"
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

static const char *TAG_WiFiBinding = "WiFiBinding";

WiFiBinding::WiFiBinding() : LogicElement(), InputElement() {
}

WiFiBinding::WiFiBinding(const MapIO io_adr) : WiFiBinding() {
    SetIoAdr(io_adr);
}

WiFiBinding::~WiFiBinding() {
}

void WiFiBinding::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
}

bool WiFiBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;

    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_WiFiBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR bool
WiFiBinding::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
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
                              && (WiFiBinding::EditingPropertyId)editing_property_id
                                     == WiFiBinding::EditingPropertyId::wbepi_None
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
                               && (WiFiBinding::EditingPropertyId)editing_property_id
                                      == WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr
                               && Blinking_50();
    res = blink_label_on_editing || draw_text_f8X14(fb, top_left.x + 4, top_left.y + 4, label);
    if (!res) {
        return res;
    }
    top_left.x += 23;
    if (!blink_body_on_editing) {
        res = draw_vert_line(fb, top_left.x, top_left.y, Height);
        if (!res) {
            return res;
        }
    }

    bool show_scales = editable_state == EditableElement::ElementState::des_Editing
                    && (WiFiBinding::EditingPropertyId)editing_property_id
                           != WiFiBinding::EditingPropertyId::wbepi_None
                    && (WiFiBinding::EditingPropertyId)editing_property_id
                           != WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr;

    if (show_scales) {
        top_left.x += 4;
        // res = RenderScales(fb, top_left.x + 4, top_left.y);
    } else {
        top_left.x += 4;
        res = draw_text_f8X14(fb, top_left.x + 4, top_left.y + 4, ssid);
    }
    if (!res) {
        return res;
    }

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    return res;
}

bool WiFiBinding::RenderSsid(uint8_t *fb, uint8_t x, uint8_t y) {
    char blink_ssid[sizeof(ssid) + 6];

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_0:
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_1:
            snprintf(blink_ssid, sizeof(blink_ssid), "%s lo", ssid);
            break;
    }

    if (Blinking_50()) {
        switch (editing_property_id) {
            case WiFiBinding::EditingPropertyId::wbepi_Ssid_0:
                blink_ssid[0] = ' ';
                break;
            case WiFiBinding::EditingPropertyId::wbepi_Ssid_1:
                blink_ssid[1] = ' ';
                break;
        }
    }

    return draw_text_f6X12(fb, x, y, blink_ssid) > 0;
}

size_t WiFiBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&ssid, sizeof(ssid), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;
    char _ssid[sizeof(ssid)];

    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    if (!Record::Read(&_ssid, sizeof(_ssid), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_ssid) == 0 || strlen(_ssid) >= sizeof(_ssid)) {
        return 0;
    }

    SetIoAdr(_io_adr);
    strcpy(ssid, _ssid);
    return readed;
}

TvElementType WiFiBinding::GetElementType() {
    return TvElementType::et_WiFiBinding;
}

WiFiBinding *WiFiBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiBinding:
            return static_cast<WiFiBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiBinding::SelectPriorSymbol(char *symbol, char extra) {
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

void WiFiBinding::SelectNextSymbol(char *symbol, char extra) {
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

void WiFiBinding::SelectPrior() {
    ESP_LOGI(TAG_WiFiBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr--;
            if (io_adr < 0) {
                io_adr = allowed_inputs.count - 1;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        case WiFiBinding::EditingPropertyId::wbepi_Ssid_0:
            SelectPriorSymbol(&ssid[0], '-');
            break;
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_1:
            SelectPriorSymbol(&ssid[1], '.');
            break;
    }
}

void WiFiBinding::SelectNext() {
    ESP_LOGI(TAG_WiFiBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr: {
            auto allowed_inputs = GetAllowedInputs();
            auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
            io_adr++;
            if (io_adr >= (int)allowed_inputs.count) {
                io_adr = 0;
            }
            SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
            break;
        }

        case WiFiBinding::EditingPropertyId::wbepi_Ssid_0:
            SelectNextSymbol(&ssid[0], '-');
            break;
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_1:
            SelectNextSymbol(&ssid[1], '.');
            break;
    }
}

void WiFiBinding::PageUp() {
    this->SelectPrior();
}

void WiFiBinding::PageDown() {
    this->SelectNext();
}

void WiFiBinding::Change() {
    ESP_LOGI(TAG_WiFiBinding, "Change editing_property_id:%d", editing_property_id);
    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr;
            break;
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_Ssid_0;
            break;
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_0:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_Ssid_1;
            break;
    }
}

const AllowedIO WiFiBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2,
                                 MapIO::V3, MapIO::V4, MapIO::O1, MapIO::O2 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

#include "LogicProgram/Bindings/WiFiBinding.h"
#include "Display/bitmaps/wifi_binding.h"
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

WiFiBinding::WiFiBinding(const MapIO io_adr, const char *ssid) : WiFiBinding() {
    SetIoAdr(io_adr);
    SetSsid(ssid);
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

    bool show_scales = editable_state == EditableElement::ElementState::des_Editing
                    && (WiFiBinding::EditingPropertyId)editing_property_id
                           != WiFiBinding::EditingPropertyId::wbepi_None
                    && (WiFiBinding::EditingPropertyId)editing_property_id
                           != WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr;

    if (show_scales) {
        res = RenderEditedSsid(fb, top_left.x, top_left.y + 4);
    } else {
        if (ssid_size <= 8) {
            res = draw_text_f6X12(fb, top_left.x, top_left.y + 6, ssid) > 0;
        } else {
            res = RenderSsidWithElipsis(fb, top_left.x, top_left.y + 6, 3);
        }
    }

    if (!res) {
        return res;
    }

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    return res;
}
bool WiFiBinding::RenderSsidWithElipsis(uint8_t *fb, uint8_t x, uint8_t y, int leverage) {
    char elipsis = ssid[leverage];
    ssid[leverage] = 0;
    int width = draw_text_f6X12(fb, x, y, ssid);
    ssid[leverage] = elipsis;
    if (width <= 0) {
        return false;
    }
    x += width;
    width = draw_text_f4X7(fb, x, y + 4, "...");
    x += width;
    return draw_text_f6X12(fb, x, y, &ssid[ssid_size - leverage]) > 0;
}

bool WiFiBinding::RenderEditedSsid(uint8_t *fb, uint8_t x, uint8_t y) {
    char blink_ssid[displayed_ssid_max_size + 1];
    int char_pos = editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char;

    if (char_pos < displayed_ssid_max_size) {
        strncpy(blink_ssid, ssid, sizeof(blink_ssid));
    } else {
        strncpy(blink_ssid, &ssid[char_pos - (displayed_ssid_max_size - 1)], sizeof(blink_ssid));
        char_pos = displayed_ssid_max_size - 1;
    }
    blink_ssid[sizeof(blink_ssid) - 1] = 0;

    if (Blinking_50()) {
        blink_ssid[char_pos] = ' ';
    }

    return draw_text_f6X12(fb, x, y + 2, blink_ssid) > 0;
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
    SetSsid(_ssid);
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

void WiFiBinding::SelectPriorSymbol(char *symbol) {
    if (*symbol > '!' && *symbol <= '~') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '~';
    }
}

void WiFiBinding::SelectNextSymbol(char *symbol) {
    if (*symbol >= '!' && *symbol < '~') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '!';
    }
}

void WiFiBinding::SelectPrior() {
    ESP_LOGI(TAG_WiFiBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
            break;
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

        default:
            SelectPriorSymbol(
                &ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char]);
            break;
    }
}

void WiFiBinding::SelectNext() {
    ESP_LOGI(TAG_WiFiBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
            break;
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

        default:
            SelectNextSymbol(
                &ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char]);
            break;
    }
}

void WiFiBinding::PageUp() {
    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            this->SelectPrior();
            break;

        default:
            ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                place_new_char;
            break;
    }
}

void WiFiBinding::PageDown() {
    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            this->SelectNext();
            break;

        default:
            ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                place_new_char;
            break;
    }
}

bool WiFiBinding::IsLastSsidChar() {
    char ch = ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char];
    return ch == 0 || ch == place_new_char;
}

void WiFiBinding::Change() {
    ESP_LOGI(TAG_WiFiBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr;
            break;
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char;
            break;

        default:
            if (editing_property_id == WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char
                || IsLastSsidChar()) {
                editing_property_id = WiFiBinding::EditingPropertyId::wbepi_None;
                EndEditing();
            } else {
                editing_property_id++;
                if (IsLastSsidChar()) {
                    ssid[editing_property_id
                         - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] = place_new_char;
                }
            }
            break;
    }
}
void WiFiBinding::EndEditing() {
    ssid_size = 0;
    while (ssid_size < sizeof(ssid) && ssid[ssid_size] != 0 && ssid[ssid_size] != place_new_char) {
        ssid_size++;
    }
    ssid[ssid_size] = 0;
    EditableElement::EndEditing();
}

const AllowedIO WiFiBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const char *WiFiBinding::GetSsid() {
    return ssid;
}

void WiFiBinding::SetSsid(const char *ssid) {
    strncpy(this->ssid, ssid, sizeof(this->ssid) - 1);
    this->ssid[sizeof(this->ssid) - 1] = 0;
    ssid_size = strlen(this->ssid);
}
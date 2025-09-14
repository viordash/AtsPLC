#include "LogicProgram/Bindings/WiFiBinding.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiBinding = "WiFiBinding";

WiFiBinding::WiFiBinding() : CommonWiFiBinding() {
    ssid[0] = 0;
    ssid_size = 0;
}

WiFiBinding::WiFiBinding(const MapIO io_adr, const char *ssid) : CommonWiFiBinding(io_adr) {
    SetSsid(ssid);
}

WiFiBinding::~WiFiBinding() {
}

bool WiFiBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        Controller::BindVariableToInsecureWiFi(GetIoAdr(), GetSsid());
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

IRAM_ATTR void
WiFiBinding::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    Point top_left = { start_point->x, (uint8_t)(start_point->y + Top) };

    CommonWiFiBinding::Render(fb, prev_elem_state, start_point);

    top_left.x += LeftPadding + 22;
    top_left.x += bitmap.size.width + 1;

    bool show_edit_ssid = editable_state == EditableElement::ElementState::des_Editing
                       && (WiFiBinding::EditingPropertyId)editing_property_id
                              != WiFiBinding::EditingPropertyId::wbepi_None
                       && (WiFiBinding::EditingPropertyId)editing_property_id
                              != WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr;

    if (show_edit_ssid) {
        RenderEditedSsid(fb, top_left.x, top_left.y + 4);
    } else {
        if (ssid_size <= 8) {
            ASSERT(draw_text_f6X12(fb, top_left.x, top_left.y + 6, ssid) >= 0);
        } else {
            RenderSsidWithElipsis(fb, top_left.x, top_left.y + 6, 3);
        }
    }
}

void WiFiBinding::RenderSsidWithElipsis(FrameBuffer *fb, uint8_t x, uint8_t y, int leverage) {
    char elipsis = ssid[leverage];
    ssid[leverage] = 0;
    int width = draw_text_f6X12(fb, x, y, ssid);
    ssid[leverage] = elipsis;
    if (width <= 0) {
        return;
    }
    x += width;
    width = draw_text_f4X7(fb, x, y + 4, "...");
    ASSERT(width > 0);
    x += width;
    ASSERT(draw_text_f6X12(fb, x, y, &ssid[ssid_size - leverage]) > 0);
}

void WiFiBinding::RenderEditedSsid(FrameBuffer *fb, uint8_t x, uint8_t y) {
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

    ASSERT(draw_text_f4X7(fb, x + 3, y - 2, "SSID:") > 0);
    ASSERT(draw_text_f6X12(fb, x, y + 5, blink_ssid) > 0);
}

size_t WiFiBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = CommonWiFiBinding::Serialize(buffer, buffer_size);
    if (writed == 0) {
        return 0;
    }
    if (!Record::Write(&ssid, sizeof(ssid), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = CommonWiFiBinding::Deserialize(buffer, buffer_size);
    char _ssid[sizeof(ssid)];

    if (readed == 0) {
        return 0;
    }

    if (!Record::Read(&_ssid, sizeof(_ssid), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_ssid) == 0 || strlen(_ssid) >= sizeof(_ssid)) {
        return 0;
    }
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
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            CommonWiFiBinding::SelectPrior();
            break;

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
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            CommonWiFiBinding::SelectNext();
            break;

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

bool WiFiBinding::ChangeSsid() {
    if (editing_property_id == WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char
        || IsLastSsidChar()) {
        return false;
    }

    editing_property_id++;
    if (IsLastSsidChar()) {
        ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
            place_new_char;
    }
    return true;
}

void WiFiBinding::Change() {
    ESP_LOGI(TAG_WiFiBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
            CommonWiFiBinding::Change();
            break;
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char;
            if (IsLastSsidChar()) {
                ssid[editing_property_id - WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                    place_new_char;
            }
            break;

        default:
            if (editing_property_id <= WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                if (!ChangeSsid()) {
                    editing_property_id = WiFiBinding::EditingPropertyId::wbepi_None;
                    EndEditing();
                }
            }
            break;
    }
}

void WiFiBinding::Option() {
    ESP_LOGI(TAG_WiFiBinding, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            break;

        default:
            editing_property_id = WiFiBinding::EditingPropertyId::wbepi_None;
            EndEditing();
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
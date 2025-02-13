#include "LogicProgram/Bindings/WiFiApBinding.h"
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

static const char *TAG_WiFiApBinding = "WiFiApBinding";

WiFiApBinding::WiFiApBinding() : CommonWiFiBinding() {
}

WiFiApBinding::WiFiApBinding(const MapIO io_adr, const char *client_mac)
    : CommonWiFiBinding(io_adr) {
    SetClientMac(client_mac);
}

WiFiApBinding::~WiFiApBinding() {
}

bool WiFiApBinding::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive && state != LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
        // Controller::BindVariableToWiFi(GetIoAdr(), client_mac);
    } else if (prev_elem_state != LogicItemState::lisActive
               && state != LogicItemState::lisPassive) {
        state = LogicItemState::lisPassive;
        // Controller::UnbindVariable(GetIoAdr());
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_WiFiApBinding, ".");
    }
    return any_changes;
}

IRAM_ATTR bool
WiFiApBinding::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    Point top_left = { start_point->x, (uint8_t)(start_point->y + Top) };

    bool res = CommonWiFiBinding::Render(fb, prev_elem_state, start_point);
    if (!res) {
        return res;
    }

    top_left.x += LeftPadding + 22;
    top_left.x += bitmap.size.width + 1;

    bool show_edit_client_mac = editable_state == EditableElement::ElementState::des_Editing
                             && (WiFiApBinding::EditingPropertyId)editing_property_id
                                    != WiFiApBinding::EditingPropertyId::wbepi_None
                             && (WiFiApBinding::EditingPropertyId)editing_property_id
                                    != WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr;

    if (show_edit_client_mac) {
        res = RenderEditedClientMac(fb, top_left.x, top_left.y + 4);
    } else {
        res = draw_text_f6X12(fb, top_left.x, top_left.y + 6, "AP CLNT") > 0;
    }
    return res;
}

bool WiFiApBinding::RenderEditedClientMac(uint8_t *fb, uint8_t x, uint8_t y) {
    char blink_client_mac[client_mac_size + 1];
    int char_pos =
        editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char;

    if (char_pos < client_mac_size) {
        strncpy(blink_client_mac, client_mac, sizeof(blink_client_mac));
    } else {
        strncpy(blink_client_mac,
                &client_mac[char_pos - (client_mac_size - 1)],
                sizeof(blink_client_mac));
        char_pos = client_mac_size - 1;
    }
    blink_client_mac[sizeof(blink_client_mac) - 1] = 0;

    if (Blinking_50()) {
        blink_client_mac[char_pos] = ' ';
    }

    return draw_text_f6X12(fb, x, y + 2, blink_client_mac) > 0;
}

size_t WiFiApBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = CommonWiFiBinding::Serialize(buffer, buffer_size);
    if (writed == 0) {
        return 0;
    }
    if (!Record::Write(&client_mac, sizeof(client_mac), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiApBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = CommonWiFiBinding::Deserialize(buffer, buffer_size);
    char _client_mac[sizeof(client_mac)];

    if (readed == 0) {
        return 0;
    }

    if (!Record::Read(&_client_mac, sizeof(_client_mac), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_client_mac) == 0 || strlen(_client_mac) >= sizeof(_client_mac)) {
        return 0;
    }
    SetClientMac(_client_mac);
    return readed;
}

TvElementType WiFiApBinding::GetElementType() {
    return TvElementType::et_WiFiBinding;
}

WiFiApBinding *WiFiApBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiBinding:
            return static_cast<WiFiApBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiApBinding::SelectPriorSymbol(char *symbol) {
    if (*symbol > '!' && *symbol <= '~') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '~';
    }
}

void WiFiApBinding::SelectNextSymbol(char *symbol) {
    if (*symbol >= '!' && *symbol < '~') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '!';
    }
}

void WiFiApBinding::SelectPrior() {
    ESP_LOGI(TAG_WiFiApBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr: {
            CommonWiFiBinding::SelectPrior();
            break;
        }

        default:
            SelectPriorSymbol(
                &client_mac[editing_property_id
                            - WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char]);
            break;
    }
}

void WiFiApBinding::SelectNext() {
    ESP_LOGI(TAG_WiFiApBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            CommonWiFiBinding::SelectNext();
            break;

        default:
            SelectNextSymbol(
                &client_mac[editing_property_id
                            - WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char]);
            break;
    }
}

void WiFiApBinding::PageUp() {
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char:
            this->SelectPrior();
            break;

        default:
            client_mac[editing_property_id
                       - WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char] =
                place_new_char;
            break;
    }
}

void WiFiApBinding::PageDown() {
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char:
            this->SelectNext();
            break;

        default:
            client_mac[editing_property_id
                       - WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char] =
                place_new_char;
            break;
    }
}

void WiFiApBinding::Change() {
    ESP_LOGI(TAG_WiFiApBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
            CommonWiFiBinding::Change();
            break;
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char;
            break;

        default:
            if (editing_property_id
                == WiFiApBinding::EditingPropertyId::wbepi_ClientMac_Last_Char) {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_None;
                EndEditing();
            } else {
                editing_property_id++;
                char *ch =
                    &client_mac[editing_property_id
                                - WiFiApBinding::EditingPropertyId::wbepi_ClientMac_First_Char];
                if (*ch == 0) {
                    *ch = place_new_char;
                }
            }
            break;
    }
}

void WiFiApBinding::EndEditing() {
    // client_mac_size = 0;
    // while (client_mac_size < sizeof(client_mac) && client_mac[client_mac_size] != 0
    //        && client_mac[client_mac_size] != place_new_char) {
    //     client_mac_size++;
    // }
    // client_mac[client_mac_size] = 0;
    EditableElement::EndEditing();
}

const AllowedIO WiFiApBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const char *WiFiApBinding::GetClientMac() {
    return client_mac;
}

void WiFiApBinding::SetClientMac(const char *client_mac) {
    strncpy(this->client_mac, client_mac, sizeof(this->client_mac) - 1);
    this->client_mac[sizeof(this->client_mac) - 1] = 0;
}
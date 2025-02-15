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

WiFiApBinding::WiFiApBinding() : WiFiBinding() {
}

WiFiApBinding::WiFiApBinding(const MapIO io_adr, const char *ssid, const char *password)
    : WiFiBinding(io_adr, ssid) {
    SetPassword(password);
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
        // Controller::BindVariableToWiFi(GetIoAdr(), ssid);
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

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            res = draw_text_f6X12(fb, top_left.x, top_left.y + 6, "AP CLNT") > 0;
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                res = RenderEditedSsid(fb, top_left.x, top_left.y + 4);
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                res = RenderEditedPassword(fb, top_left.x, top_left.y + 4);
            }

            break;
    }
    return res;
}

bool WiFiApBinding::RenderEditedPassword(uint8_t *fb, uint8_t x, uint8_t y) {
    char blink_password[displayed_password_max_size + 1];
    int char_pos =
        editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char;

    if (char_pos < displayed_password_max_size) {
        strncpy(blink_password, password, sizeof(blink_password));
    } else {
        strncpy(blink_password,
                &password[char_pos - (displayed_password_max_size - 1)],
                sizeof(blink_password));
        char_pos = displayed_password_max_size - 1;
    }
    blink_password[sizeof(blink_password) - 1] = 0;

    if (Blinking_50()) {
        blink_password[char_pos] = ' ';
    }

    if (draw_text_f4X7(fb, x + 3, y - 2, "PASSWORD:") <= 0) {
        return false;
    }
    return draw_text_f6X12(fb, x, y + 5, blink_password) > 0;
}

size_t WiFiApBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = WiFiBinding::Serialize(buffer, buffer_size);
    if (writed == 0) {
        return 0;
    }
    if (!Record::Write(&password, sizeof(password), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiApBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = WiFiBinding::Deserialize(buffer, buffer_size);
    if (readed == 0) {
        return 0;
    }
    char _password[sizeof(password)];
    if (!Record::Read(&_password, sizeof(_password), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (strlen(_password) == 0 || strlen(_password) >= sizeof(_password)) {
        return 0;
    }
    SetPassword(_password);
    return readed;
}

TvElementType WiFiApBinding::GetElementType() {
    return TvElementType::et_WiFiApBinding;
}

WiFiApBinding *WiFiApBinding::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiApBinding:
            return static_cast<WiFiApBinding *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiApBinding::SelectPrior() {
    ESP_LOGI(TAG_WiFiApBinding, "SelectPrior");

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::SelectPrior();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::SelectPrior();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                SelectPriorSymbol(
                    &password[editing_property_id
                              - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char]);
            }

            break;
    }
}

void WiFiApBinding::SelectNext() {
    ESP_LOGI(TAG_WiFiApBinding, "SelectNext");

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::SelectNext();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::SelectNext();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                SelectNextSymbol(
                    &password[editing_property_id
                              - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char]);
            }
            break;
    }
}

void WiFiApBinding::PageUp() {
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::PageUp();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::PageUp();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                password[editing_property_id
                         - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                    place_new_char;
            }
            break;
    }
}

void WiFiApBinding::PageDown() {
    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::PageDown();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::PageDown();
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                password[editing_property_id
                         - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                    place_new_char;
            }
            break;
    }
}

bool WiFiApBinding::IsLastPasswordChar() {
    char ch =
        password[editing_property_id - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char];
    return ch == 0 || ch == place_new_char;
}

void WiFiApBinding::Change() {
    ESP_LOGI(TAG_WiFiApBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            WiFiBinding::Change();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                if (IsLastSsidChar()) {
                    editing_property_id =
                        WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char;
                } else {
                    editing_property_id++;
                    if (IsLastSsidChar()) {
                        ssid[editing_property_id
                             - WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char] =
                            place_new_char;
                    }
                }
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                if (IsLastPasswordChar()) {
                    editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_None;
                    EndEditing();
                } else {
                    editing_property_id++;
                    if (IsLastPasswordChar()) {
                        password[editing_property_id
                                 - WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char] =
                            place_new_char;
                    }
                }
            }
            break;
    }
}

void WiFiApBinding::Option() {
    ESP_LOGI(TAG_WiFiApBinding, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
            WiFiBinding::Option();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char;
            } else if (editing_property_id
                       <= WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char) {
                editing_property_id = WiFiApBinding::EditingPropertyId::wbepi_None;
                EndEditing();
            }
            break;
    }
}

void WiFiApBinding::EndEditing() {
    password_size = 0;
    while (password_size < sizeof(password) && password[password_size] != 0
           && password[password_size] != place_new_char) {
        password_size++;
    }
    password[password_size] = 0;
    WiFiBinding::EndEditing();
}

const AllowedIO WiFiApBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const char *WiFiApBinding::GetPassword() {
    return password;
}

void WiFiApBinding::SetPassword(const char *password) {
    ESP_LOGI(TAG_WiFiApBinding, "SetPassword:%s", password);
    strncpy(this->password, password, sizeof(this->password) - 1);
    this->password[sizeof(this->password) - 1] = 0;
    password_size = strlen(this->password);
}
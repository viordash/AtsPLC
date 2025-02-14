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

WiFiApBinding::WiFiApBinding(const MapIO io_adr, const char *ssid) : WiFiBinding(io_adr, ssid) {
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

    bool show_edit_ssid = editable_state == EditableElement::ElementState::des_Editing
                       && (WiFiApBinding::EditingPropertyId)editing_property_id
                              != WiFiApBinding::EditingPropertyId::wbepi_None
                       && (WiFiApBinding::EditingPropertyId)editing_property_id
                              != WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr;

    if (show_edit_ssid) {
        res = RenderEditedSsid(fb, top_left.x, top_left.y + 4);
    } else {
        res = draw_text_f6X12(fb, top_left.x, top_left.y + 6, "AP CLNT") > 0;
    }
    return res;
}

size_t WiFiApBinding::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = WiFiBinding::Serialize(buffer, buffer_size);
    if (writed == 0) {
        return 0;
    }
    return writed;
}

size_t WiFiApBinding::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = WiFiBinding::Deserialize(buffer, buffer_size);

    if (readed == 0) {
        return 0;
    }

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
            }
            break;
    }
}

void WiFiApBinding::Change() {
    ESP_LOGI(TAG_WiFiApBinding, "Change editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiApBinding::EditingPropertyId::wbepi_None:
        case WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::Change();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::Change();
            }
            break;
    }
}

void WiFiApBinding::Option() {
    ESP_LOGI(TAG_WiFiApBinding, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case WiFiBinding::EditingPropertyId::wbepi_None:
        case WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr:
        case WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char:
            WiFiBinding::Option();
            break;

        default:
            if (editing_property_id <= WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char) {
                WiFiBinding::Option();
            }
            break;
    }
}

void WiFiApBinding::EndEditing() {
    WiFiBinding::EndEditing();
}

const AllowedIO WiFiApBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}


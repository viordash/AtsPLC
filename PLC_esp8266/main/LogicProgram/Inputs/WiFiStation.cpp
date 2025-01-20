#include "LogicProgram/Inputs/WiFiStation.h"
#include "Display/bitmaps/wif_sta_active.h"
#include "Display/bitmaps/wif_sta_error.h"
#include "Display/bitmaps/wif_sta_inactive.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiStation = "WiFiStation";

WiFiStation::WiFiStation() : LogicElement() {
    station_connect_status = WiFiStationConnectStatus::wscs_Error;
}

WiFiStation::~WiFiStation() {
}

IRAM_ATTR bool
WiFiStation::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    const Bitmap *bitmap = &bitmap_passive;

    switch (station_connect_status) {
        case WiFiStationConnectStatus::wscs_Connected:
            if (prev_elem_state == LogicItemState::lisActive) {
                bitmap = &bitmap_active;
            } else {
                bitmap = &bitmap_passive;
            }
            break;

        case WiFiStationConnectStatus::wscs_Error:
            bitmap = &bitmap_error;
            break;

        default:
            bitmap = &bitmap_passive;
            break;
    }

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, LeftPadding);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, LeftPadding, false);
    }
    if (!res) {
        return res;
    }
    start_point->x += LeftPadding;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (WiFiStation::EditingPropertyId)editing_property_id
                                       == WiFiStation::EditingPropertyId::ciepi_None
                                && Blinking_50();
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }

    start_point->x += bitmap->size.width;

    res = EditableElement::Render(fb, start_point);
    return res;
}

bool WiFiStation::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    state = LogicItemState::lisPassive;
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisPassive) {
        Controller::DisconnectFromWiFiStation();
    } else if (prev_elem_state == LogicItemState::lisActive) {
        station_connect_status = Controller::ConnectToWiFiStation();
        if (station_connect_status == WiFiStationConnectStatus::wscs_Connected) {
            state = LogicItemState::lisActive;
        }
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_WiFiStation, ".");
    }

    return any_changes;
}

size_t WiFiStation::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    uint8_t dummy = 0;
    if (!Record::Write(&dummy, sizeof(dummy), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t WiFiStation::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t dummy;
    if (!Record::Read(&dummy, sizeof(dummy), buffer, buffer_size, &readed)) {
        return 0;
    }
    return readed;
}

TvElementType WiFiStation::GetElementType() {
    return TvElementType::et_WiFiStation;
}

WiFiStation *WiFiStation::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_WiFiStation:
            return static_cast<WiFiStation *>(logic_element);

        default:
            return NULL;
    }
}

void WiFiStation::SelectPrior() {
    ESP_LOGI(TAG_WiFiStation, "SelectPrior");
}

void WiFiStation::SelectNext() {
    ESP_LOGI(TAG_WiFiStation, "SelectNext");
}

void WiFiStation::PageUp() {
}

void WiFiStation::PageDown() {
}

void WiFiStation::Change() {
    ESP_LOGI(TAG_WiFiStation, "Change");
    switch (editing_property_id) {

        default:
            EndEditing();
            break;
    }
}

void WiFiStation::Option() {
}
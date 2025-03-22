#include "LogicProgram/Settings/SettingsElement.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_SettingsElement = "SettingsElement";

static std::map<SettingsElement::Discriminator, const char *> DiscriminatorNames = {
    { SettingsElement::Discriminator::t_wifi_station_settings_ssid, "w_sta_ssid" },
    { SettingsElement::Discriminator::t_wifi_station_settings_password, "w_sta_passw" },
    { SettingsElement::Discriminator::t_wifi_station_settings_connect_max_retry_count,
      "w_sta_retry_cnt" },
    { SettingsElement::Discriminator::t_wifi_station_settings_reconnect_delay_ms,
      "w_sta_recon_delay" },
    { SettingsElement::Discriminator::t_wifi_station_settings_scan_station_rssi_period_ms,
      "w_sta_scan_rssi_period" },
    { SettingsElement::Discriminator::t_wifi_station_settings_max_rssi, "w_sta_max_rssi" },
    { SettingsElement::Discriminator::t_wifi_station_settings_min_rssi, "w_sta_min_rssi" },
    { SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms,
      "w_scan_chan_time" },
    { SettingsElement::Discriminator::t_wifi_scanner_settings_max_rssi, "w_scan_max_rssi" },
    { SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi, "w_scan_min_rssi" },
    { SettingsElement::Discriminator::t_wifi_access_point_settings_generation_time_ms,
      "w_ap_gener_time" },
    { SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden,
      "w_ap_ssid_hidden" },
};

SettingsElement::SettingsElement() : LogicElement() {
    memset(&value, 0, sizeof(value));
    discriminator = Discriminator::t_wifi_station_settings_ssid;
}

SettingsElement::~SettingsElement() {
}

bool SettingsElement::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    state = prev_elem_state;
    return prev_elem_changed;
}

IRAM_ATTR bool
SettingsElement::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;

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
                              && (SettingsElement::EditingPropertyId)editing_property_id
                                     == SettingsElement::EditingPropertyId::cwbepi_None
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

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    if (!res) {
        return res;
    }

    top_left.x += LeftPadding;

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator: {
            const char *name = DiscriminatorNames[discriminator];
            res = draw_text_f4X7(fb, top_left.x + 3, top_left.y - 2, name) > 0;
            if (!res) {
                return false;
            }
            res = RenderValue(fb, top_left.x, top_left.y);
            if (!res) {
                return res;
            }
        }

        default:
            break;
    }
    return res;
}

bool SettingsElement::RenderValue(uint8_t *fb, uint8_t x, uint8_t y) {
    switch (discriminator) {
        case Discriminator::t_wifi_station_settings_ssid:
            return draw_text_f6X12(fb, x, y + 5, "value.string") > 0;
        default:
            break;
    }
    return false;
}

size_t SettingsElement::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();

    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&discriminator, sizeof(discriminator), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&value, sizeof(value), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t SettingsElement::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    Discriminator _discriminator;
    if (!Record::Read(&_discriminator, sizeof(_discriminator), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateDiscriminator(&_discriminator)) {
        return 0;
    }

    Value _value;
    if (!Record::Read(&_value, sizeof(_value), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateValue(&_discriminator, &_value)) {
        return 0;
    }

    this->discriminator = _discriminator;
    this->value = _value;
    return readed;
}

bool SettingsElement::ValidateDiscriminator(Discriminator *discriminator) {
    switch (*discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password:
        case t_wifi_station_settings_connect_max_retry_count:
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
        case t_wifi_access_point_settings_generation_time_ms:
        case t_wifi_access_point_settings_ssid_hidden:
            return true;

        default:
            return false;
    }
}

bool SettingsElement::ValidateValue(Discriminator *discriminator, Value *value) {
    switch (*discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password: {
            size_t len = strnlen(value->string_value, sizeof(value->string_value));
            return len > 0 && len < sizeof(value->string_value);
        }

        case t_wifi_station_settings_connect_max_retry_count:
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
        case t_wifi_access_point_settings_generation_time_ms:
        return true;

        case t_wifi_access_point_settings_ssid_hidden:
            return (int)value->bool_value == 0 or (int)value->bool_value == 1;

        default:
            return false;
    }
    return false;
}

void SettingsElement::SelectPrior() {
    ESP_LOGI(TAG_SettingsElement, "SelectPrior");

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            break;
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:

            break;

        default:
            break;
    }
}

void SettingsElement::SelectNext() {
    ESP_LOGI(TAG_SettingsElement, "SelectNext");

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            break;
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:

            break;

        default:
            break;
    }
}

void SettingsElement::PageUp() {
}

void SettingsElement::PageDown() {
}

void SettingsElement::Change() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator;
            break;

        default:
            EndEditing();
            break;
    }
}
void SettingsElement::Option() {
}

TvElementType SettingsElement::GetElementType() {
    return TvElementType::et_Settings;
}

SettingsElement *SettingsElement::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_Settings:
            return static_cast<SettingsElement *>(logic_element);

        default:
            return NULL;
    }
}
#include "LogicProgram/Settings/SettingsElement.h"
#include "Datetime/Datetime.h"
#include "Display/bitmaps/settings.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern CurrentSettings::device_settings settings;

static const char *TAG_SettingsElement = "SettingsElement";

SettingsElement::SettingsElement() : LogicElement() {
    value[0] = 0;
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
        draw_bitmap(fb, top_left.x + 1, top_left.y + 6, &bitmap);
    }

    start_point->x += Width;

    res = EditableElement::Render(fb, start_point);
    if (!res) {
        return res;
    }

    top_left.x += bitmap.size.width;
    top_left.y += 3;

    bool blink_name = editable_state == EditableElement::ElementState::des_Editing
                   && (SettingsElement::EditingPropertyId)editing_property_id
                          == SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator
                   && Blinking_50();

    if (!blink_name) {
        res = RenderName(fb, top_left.x, top_left.y);
        if (!res) {
            return res;
        }
    }

    top_left.x += 4;
    bool show_edit_value = editable_state == EditableElement::ElementState::des_Editing
                        && (SettingsElement::EditingPropertyId)editing_property_id
                               != SettingsElement::EditingPropertyId::cwbepi_None
                        && (SettingsElement::EditingPropertyId)editing_property_id
                               != SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator;

    if (show_edit_value) {
        res = RenderEditedValue(fb, top_left.x, top_left.y);
    } else {
        res = RenderValue(fb, top_left.x, top_left.y);
    }

    return res;
}

bool SettingsElement::RenderName(uint8_t *fb, uint8_t x, uint8_t y) {
    const char *name;

    switch (discriminator) {
        case t_wifi_station_settings_ssid: {
            name = "STA: ssid";
            break;
        }
        case t_wifi_station_settings_password: {
            name = "STA: password";
            break;
        }
        case t_wifi_station_settings_connect_max_retry_count:
            name = "STA: retry count max";
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
            name = "STA: reconn pause,mS";
            break;
        case t_wifi_station_settings_scan_station_rssi_period_ms:
            name = "STA: rssi period, mS";
            break;
        case t_wifi_station_settings_max_rssi:
            name = "STA: max rssi, dBm";
            break;
        case t_wifi_station_settings_min_rssi:
            name = "STA: min rssi, dBm";
            break;
        case t_wifi_station_settings_min_worktime_ms:
            name = "STA: worktime, ms";
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            name = "SCAN: scan time, mS";
            break;
        case t_wifi_scanner_settings_max_rssi:
            name = "SCAN: max rssi, dBm";
            break;
        case t_wifi_scanner_settings_min_rssi:
            name = "SCAN: min rssi, dBm";
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            name = "AP: br-cast time, mS";
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            name = "AP: hidden ssid";
            break;
        case t_current_date:
            name = "Date YYYY-MM-DD";
            break;
        case t_current_time:
            name = "Time hh:mm:ss";
            break;
        case t_datetime_sntp_server_primary:
            name = "Primary SNTP server";
            break;
        case t_datetime_sntp_server_secondary:
            name = "Second. SNTP server";
            break;
        case t_datetime_timezone:
            name = "Timezone";
            break;
        case t_adc_scan_period_ms:
            name = "ADC: scan period, mS";
            break;
        default:
            return false;
    }

    bool res = draw_text_f4X7(fb, x, y - 2, name) > 0;
    return res;
}

bool SettingsElement::RenderValue(uint8_t *fb, uint8_t x, uint8_t y) {
    char display_value[value_size + 1];

    ReadValue(display_value, true);

    size_t len = strlen(display_value);
    if (len == 0) {
        return true;
    }
    bool res = true;
    if (len <= displayed_value_max_size) {
        x += (displayed_value_max_size - len) * get_text_f6X12_width() - 2;
        res = draw_text_f6X12(fb, x, y + 6, display_value) > 0;
    } else {
        res = RenderValueWithElipsis(fb,
                                     x,
                                     y + 6,
                                     displayed_value_max_size / 2 - 1,
                                     display_value,
                                     len);
    }
    return res;
}

bool SettingsElement::RenderValueWithElipsis(uint8_t *fb,
                                             uint8_t x,
                                             uint8_t y,
                                             int leverage,
                                             char *display_value,
                                             size_t len) {
    char elipsis = display_value[leverage];
    display_value[leverage] = 0;
    int width = draw_text_f6X12(fb, x, y, display_value);
    display_value[leverage] = elipsis;
    if (width <= 0) {
        return false;
    }
    x += width;
    width = draw_text_f4X7(fb, x, y + 4, "...");
    x += width;
    return draw_text_f6X12(fb, x, y, &display_value[len - leverage]) > 0;
}

bool SettingsElement::RenderEditedValue(uint8_t *fb, uint8_t x, uint8_t y) {
    char blink_value[displayed_value_max_size + 1];
    int char_pos =
        editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;

    if (char_pos < displayed_value_max_size) {
        strncpy(blink_value, value, sizeof(blink_value));
    } else {
        strncpy(blink_value,
                &value[char_pos - (displayed_value_max_size - 1)],
                sizeof(blink_value));
        char_pos = displayed_value_max_size - 1;
    }
    blink_value[sizeof(blink_value) - 1] = 0;

    if (Blinking_50()) {
        blink_value[char_pos] = ' ';
    }

    return draw_text_f6X12(fb, x, y + 5, blink_value) > 0;
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

    this->discriminator = _discriminator;
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
        case t_wifi_station_settings_min_worktime_ms:
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
        case t_wifi_access_point_settings_generation_time_ms:
        case t_wifi_access_point_settings_ssid_hidden:
        case t_current_date:
        case t_current_time:
        case t_datetime_sntp_server_primary:
        case t_datetime_sntp_server_secondary:
        case t_datetime_timezone:
        case t_adc_scan_period_ms:
            return true;

        default:
            return false;
    }
}

bool SettingsElement::IsLastValueChar() {
    char ch =
        value[editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char];
    return ch == 0 || ch == place_new_char;
}

bool SettingsElement::ChangeValue() {
    if (editing_property_id == SettingsElement::EditingPropertyId::cwbepi_Value_Last_Char
        || IsLastValueChar()) {
        return false;
    }

    editing_property_id++;
    if (IsLastValueChar()) {
        value[editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] =
            place_new_char;
    }
    return true;
}

void SettingsElement::ReadValue(char *string_buffer, bool friendly_format) {
    CurrentSettings::device_settings curr_settings;
    SAFETY_SETTINGS({ curr_settings = settings; });

    switch (discriminator) {
        case t_wifi_station_settings_ssid: {
            const size_t max_len = sizeof(curr_settings.wifi_station.ssid);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.wifi_station.ssid, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_wifi_station_settings_password: {
            const size_t max_len = sizeof(curr_settings.wifi_station.password);
            static_assert(value_size + 1 > max_len);
            if (friendly_format) {
                strcpy(string_buffer, "********");
            } else {
                strncpy(string_buffer, curr_settings.wifi_station.password, max_len);
                string_buffer[max_len] = 0;
            }
            break;
        }
        case t_wifi_station_settings_connect_max_retry_count:
            if (friendly_format
                && curr_settings.wifi_station.connect_max_retry_count == INFINITY_CONNECT_RETRY) {
                strcpy(string_buffer, "infinity");
            } else {
                sprintf(string_buffer,
                        "%d",
                        (int)curr_settings.wifi_station.connect_max_retry_count);
            }
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_station.reconnect_delay_ms);
            break;
        case t_wifi_station_settings_scan_station_rssi_period_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_station.scan_station_rssi_period_ms);
            break;
        case t_wifi_station_settings_max_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_station.max_rssi);
            break;
        case t_wifi_station_settings_min_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_station.min_rssi);
            break;
        case t_wifi_station_settings_min_worktime_ms:
            sprintf(string_buffer, "%u", (unsigned int)curr_settings.wifi_station.min_worktime_ms);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_scanner.per_channel_scan_time_ms);
            break;
        case t_wifi_scanner_settings_max_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_scanner.max_rssi);
            break;
        case t_wifi_scanner_settings_min_rssi:
            sprintf(string_buffer, "%d", (int)curr_settings.wifi_scanner.min_rssi);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            sprintf(string_buffer,
                    "%u",
                    (unsigned int)curr_settings.wifi_access_point.generation_time_ms);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            if (friendly_format) {
                strcpy(string_buffer,
                       curr_settings.wifi_access_point.ssid_hidden ? "true" : "false");
            } else {
                sprintf(string_buffer,
                        "%u",
                        (unsigned int)curr_settings.wifi_access_point.ssid_hidden);
            }
            break;
        case t_current_date: {
            Datetime dt;
            Controller::GetSystemDatetime(&dt);
            sprintf(string_buffer, "%04d-%02d-%02d", (int)dt.year, (int)dt.month, (int)dt.day);
            break;
        }
        case t_current_time: {
            Datetime dt;
            Controller::GetSystemDatetime(&dt);
            sprintf(string_buffer, "%02d:%02d:%02d", (int)dt.hour, (int)dt.minute, (int)dt.second);
            break;
        }
        case t_datetime_sntp_server_primary: {
            const size_t max_len = sizeof(curr_settings.datetime.sntp_server_primary);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.datetime.sntp_server_primary, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_datetime_sntp_server_secondary: {
            const size_t max_len = sizeof(curr_settings.datetime.sntp_server_secondary);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.datetime.sntp_server_secondary, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_datetime_timezone: {
            const size_t max_len = sizeof(curr_settings.datetime.timezone);
            static_assert(value_size + 1 > max_len);
            strncpy(string_buffer, curr_settings.datetime.timezone, max_len);
            string_buffer[max_len] = 0;
            break;
        }
        case t_adc_scan_period_ms:
            sprintf(string_buffer, "%u", (unsigned int)curr_settings.adc.scan_period_ms);
            break;

        default:
            break;
    }
}

void SettingsElement::SelectPriorStringSymbol(char *symbol) {
    if (*symbol > '!' && *symbol <= '~') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '~';
    }
}

void SettingsElement::SelectNextStringSymbol(char *symbol) {
    if (*symbol >= '!' && *symbol < '~') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '!';
    }
}

void SettingsElement::SelectPriorNumberSymbol(char *symbol, char extra) {
    if (extra != 0 && *symbol == '0') {
        *symbol = extra;
    } else if (*symbol > '0' && *symbol <= '9') {
        *symbol = *symbol - 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '9';
    }
}
void SettingsElement::SelectNextNumberSymbol(char *symbol, char extra) {
    if (extra != 0 && *symbol == '9') {
        *symbol = extra;
    } else if (*symbol >= '0' && *symbol < '9') {
        *symbol = *symbol + 1;
    } else if (*symbol != place_new_char) {
        *symbol = place_new_char;
    } else {
        *symbol = '0';
    }
}

void SettingsElement::SelectBoolSymbol(char *symbol) {
    if (*symbol != '0') {
        *symbol = '0';
    } else {
        *symbol = '1';
    }
}

void SettingsElement::SelectPriorSymbol(char *symbol, bool first) {
    switch (discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password:
            SelectPriorStringSymbol(symbol);
            break;
        case t_wifi_station_settings_connect_max_retry_count:
            SelectPriorNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_min_worktime_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
            SelectPriorNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
            SelectPriorNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            SelectBoolSymbol(symbol);
            break;
        case t_current_date:
            SelectPriorNumberSymbol(symbol, '-');
            break;
        case t_current_time:
            SelectPriorNumberSymbol(symbol, ':');
            break;
        case t_datetime_sntp_server_primary:
        case t_datetime_sntp_server_secondary:
        case t_datetime_timezone:
            SelectPriorStringSymbol(symbol);
            break;
        case t_adc_scan_period_ms:
            SelectPriorNumberSymbol(symbol, 0);
            break;

        default:
            break;
    }
}

void SettingsElement::SelectNextSymbol(char *symbol, bool first) {
    switch (discriminator) {
        case t_wifi_station_settings_ssid:
        case t_wifi_station_settings_password:
            SelectNextStringSymbol(symbol);
            break;
        case t_wifi_station_settings_connect_max_retry_count:
            SelectNextNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
        case t_wifi_station_settings_scan_station_rssi_period_ms:
        case t_wifi_station_settings_min_worktime_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        case t_wifi_station_settings_max_rssi:
        case t_wifi_station_settings_min_rssi:
            SelectNextNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        case t_wifi_scanner_settings_max_rssi:
        case t_wifi_scanner_settings_min_rssi:
            SelectNextNumberSymbol(symbol, first ? '-' : 0);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            SelectBoolSymbol(symbol);
            break;
        case t_current_date:
            SelectNextNumberSymbol(symbol, '-');
            break;
        case t_current_time:
            SelectNextNumberSymbol(symbol, ':');
            break;
        case t_datetime_sntp_server_primary:
        case t_datetime_sntp_server_secondary:
        case t_datetime_timezone:
            SelectNextStringSymbol(symbol);
            break;
        case t_adc_scan_period_ms:
            SelectNextNumberSymbol(symbol, 0);
            break;
        default:
            break;
    }
}

void SettingsElement::SelectPrior() {
    ESP_LOGI(TAG_SettingsElement, "SelectPrior");

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            break;
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator: {
            auto _discriminator = (Discriminator)(discriminator - 1);
            if (!ValidateDiscriminator(&_discriminator)) {
                _discriminator = Discriminator::t_adc_scan_period_ms;
            }
            discriminator = _discriminator;
            break;
        }

        default: {
            int pos =
                editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;
            SelectPriorSymbol(&value[pos], pos == 0);
            break;
        }
    }
}

void SettingsElement::SelectNext() {
    ESP_LOGI(TAG_SettingsElement, "SelectNext");

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            break;
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator: {
            auto _discriminator = (Discriminator)(discriminator + 1);
            if (!ValidateDiscriminator(&_discriminator)) {
                _discriminator = Discriminator::t_wifi_station_settings_ssid;
            }
            discriminator = _discriminator;
            break;
        }

        default: {
            int pos =
                editing_property_id - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;
            SelectNextSymbol(&value[pos], pos == 0);
            break;
        }
    }
}

void SettingsElement::PageUp() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
        case SettingsElement::EditingPropertyId::cwbepi_Value_First_Char:
            this->SelectPrior();
            break;

        default:
            value[editing_property_id
                  - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] = place_new_char;
            break;
    }
}

void SettingsElement::PageDown() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
        case SettingsElement::EditingPropertyId::cwbepi_Value_First_Char:
            this->SelectNext();
            break;

        default:
            value[editing_property_id
                  - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] = place_new_char;
            break;
    }
}

void SettingsElement::Change() {
    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator;
            break;

        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_Value_First_Char;
            memset(value, 0, sizeof(value));
            ReadValue(value, false);
            if (IsLastValueChar()) {
                value[editing_property_id
                      - SettingsElement::EditingPropertyId::cwbepi_Value_First_Char] =
                    place_new_char;
            }
            break;

        default:
            if (editing_property_id <= SettingsElement::EditingPropertyId::cwbepi_Value_Last_Char) {
                if (!ChangeValue()) {
                    editing_property_id = SettingsElement::EditingPropertyId::cwbepi_None;
                    EndEditing();
                }
            }
            break;
    }
}
void SettingsElement::Option() {
    ESP_LOGI(TAG_SettingsElement, "Option editing_property_id:%d", editing_property_id);

    switch (editing_property_id) {
        case SettingsElement::EditingPropertyId::cwbepi_None:
        case SettingsElement::EditingPropertyId::cwbepi_SelectDiscriminator:
            break;

        default:
            editing_property_id = SettingsElement::EditingPropertyId::cwbepi_None;
            EndEditing();
            break;
    }
}

void SettingsElement::WriteString(char *dest, size_t dest_size) {
    size_t size = 0;
    while (size < dest_size && value[size] != 0 && value[size] != place_new_char) {
        *dest++ = value[size];
        size++;
    }
    if (size < dest_size) {
        *dest = 0;
    }
}

void SettingsElement::EndEditing() {
    CurrentSettings::device_settings curr_settings;
    SAFETY_SETTINGS({ curr_settings = settings; });

    switch (discriminator) {
        case t_wifi_station_settings_ssid:
            WriteString(curr_settings.wifi_station.ssid, sizeof(curr_settings.wifi_station.ssid));
            break;
        case t_wifi_station_settings_password:
            WriteString(curr_settings.wifi_station.password,
                        sizeof(curr_settings.wifi_station.password));
            break;
        case t_wifi_station_settings_connect_max_retry_count:
            curr_settings.wifi_station.connect_max_retry_count = atoi(value);
            break;
        case t_wifi_station_settings_reconnect_delay_ms:
            curr_settings.wifi_station.reconnect_delay_ms = atol(value);
            break;
        case t_wifi_station_settings_scan_station_rssi_period_ms:
            curr_settings.wifi_station.scan_station_rssi_period_ms = atol(value);
            break;
        case t_wifi_station_settings_max_rssi:
            curr_settings.wifi_station.max_rssi = atoi(value);
            break;
        case t_wifi_station_settings_min_rssi:
            curr_settings.wifi_station.min_rssi = atoi(value);
            break;
        case t_wifi_station_settings_min_worktime_ms:
            curr_settings.wifi_station.min_worktime_ms = atol(value);
            break;
        case t_wifi_scanner_settings_per_channel_scan_time_ms:
            curr_settings.wifi_scanner.per_channel_scan_time_ms = atol(value);
            break;
        case t_wifi_scanner_settings_max_rssi:
            curr_settings.wifi_scanner.max_rssi = atoi(value);
            break;
        case t_wifi_scanner_settings_min_rssi:
            curr_settings.wifi_scanner.min_rssi = atoi(value);
            break;
        case t_wifi_access_point_settings_generation_time_ms:
            curr_settings.wifi_access_point.generation_time_ms = atol(value);
            break;
        case t_wifi_access_point_settings_ssid_hidden:
            curr_settings.wifi_access_point.ssid_hidden = atol(value) != 0;
            break;
        case t_current_date:
            if (!ParseDateValue()) {
                return;
            }
            break;
        case t_current_time:
            if (!ParseTimeValue()) {
                return;
            }
            break;
        case t_datetime_sntp_server_primary:
            WriteString(curr_settings.datetime.sntp_server_primary,
                        sizeof(curr_settings.datetime.sntp_server_primary));
            break;
        case t_datetime_sntp_server_secondary:
            WriteString(curr_settings.datetime.sntp_server_secondary,
                        sizeof(curr_settings.datetime.sntp_server_secondary));
            break;
        case t_datetime_timezone:
            WriteString(curr_settings.datetime.timezone, sizeof(curr_settings.datetime.timezone));
            break;
        case t_adc_scan_period_ms:
            curr_settings.adc.scan_period_ms = atol(value);
            break;
        default:
            break;
    }

    if (discriminator != t_current_date && discriminator != t_current_time) {
        if (validate_settings(&curr_settings)) {
            SAFETY_SETTINGS(              //
                settings = curr_settings; //
                store_settings();         //
            );

            switch (discriminator) {
                case t_datetime_sntp_server_primary:
                case t_datetime_sntp_server_secondary:
                case t_datetime_timezone:
                    Controller::RestartSntp();
                    break;
                default:
                    break;
            }

        } else {
            ESP_LOGE(TAG_SettingsElement, "Settings changing has some error");
        }
    }

    ESP_LOGI(TAG_SettingsElement, "Settings changed successfully");
    EditableElement::EndEditing();
}

bool SettingsElement::ParseDateValue() {
    Datetime dt;
    Controller::GetSystemDatetime(&dt);
    const int date_elements_count = 3;
    int count = sscanf(value, "%4d-%2d-%2d", (int *)&dt.year, (int *)&dt.month, (int *)&dt.day);
    if (count != date_elements_count) {
        ESP_LOGW(TAG_SettingsElement, "ReadDateValue, invalid count:%d", count);
        return false;
    }
    return Controller::ManualSetSystemDatetime(&dt);
}

bool SettingsElement::ParseTimeValue() {
    Datetime dt;
    Controller::GetSystemDatetime(&dt);
    const int date_elements_count = 3;
    int count = sscanf(value, "%2d:%2d:%2d", (int *)&dt.hour, (int *)&dt.minute, (int *)&dt.second);
    if (count != date_elements_count) {
        ESP_LOGW(TAG_SettingsElement, "ReadTimeValue, invalid count:%d", count);
        return false;
    }
    return Controller::ManualSetSystemDatetime(&dt);
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
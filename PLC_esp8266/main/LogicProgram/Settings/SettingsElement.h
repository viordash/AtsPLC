#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class SettingsElement : public LogicElement {
  public:
    static const uint8_t str_value_size = 64;
    typedef union {
        char string_value[str_value_size + 1];
        int32_t int_value;
        uint32_t uint_value;
        bool bool_value;
    } Value;

    typedef enum { //
        t_wifi_station_settings_ssid = 0,
        t_wifi_station_settings_password,
        t_wifi_station_settings_connect_max_retry_count,
        t_wifi_station_settings_reconnect_delay_ms,
        t_wifi_station_settings_scan_station_rssi_period_ms,
        t_wifi_station_settings_max_rssi,
        t_wifi_station_settings_min_rssi,

        t_wifi_scanner_settings_per_channel_scan_time_ms,
        t_wifi_scanner_settings_max_rssi,
        t_wifi_scanner_settings_min_rssi,

        t_wifi_access_point_settings_generation_time_ms,
        t_wifi_access_point_settings_ssid_hidden
    } Discriminator;

  protected:
    Value value;
    Discriminator discriminator;
    bool RenderValue(uint8_t *fb, uint8_t x, uint8_t y);

  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_SelectDiscriminator,
        cwbepi_SetValue
    } EditingPropertyId;

    static const uint8_t LeftPadding = 12;
    static const uint8_t RightPadding = 12;
    static const uint8_t Width =
        OUTCOME_RAIL_RIGHT - INCOME_RAIL_WIDTH - LeftPadding - RightPadding;
    static const uint8_t Top = -13;
    static const uint8_t Height = 22;

    explicit SettingsElement();
    virtual ~SettingsElement();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;

    TvElementType GetElementType() override final;
    static SettingsElement *TryToCast(LogicElement *logic_element);
};

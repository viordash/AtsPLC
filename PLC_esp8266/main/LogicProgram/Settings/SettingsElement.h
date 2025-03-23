#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class SettingsElement : public LogicElement {
  public:
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
    static const uint8_t max_value_size = 64;
    static const uint8_t displayed_value_max_size = 12;
    char value[max_value_size + 1];
    uint8_t value_size;

    Discriminator discriminator;
    bool ValidateDiscriminator(Discriminator *discriminator);

    bool RenderValue(uint8_t *fb, uint8_t x, uint8_t y);
    bool RenderValueWithElipsis(uint8_t *fb,
                                uint8_t x,
                                uint8_t y,
                                int leverage,
                                char *display_value,
                                size_t len);
    bool RenderEditedValue(uint8_t *fb, uint8_t x, uint8_t y);

  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_SelectDiscriminator,
        cwbepi_Ssid_First_Char,
        cwbepi_Ssid_Last_Char = cwbepi_Ssid_First_Char + max_value_size - 1
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

    const char *GetValue();
    void SetValue(const char *value);
};

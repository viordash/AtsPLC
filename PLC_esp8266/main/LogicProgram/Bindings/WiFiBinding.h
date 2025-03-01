#pragma once

#include "LogicProgram/Bindings/CommonWiFiBinding.h"
#include <stdint.h>
#include <unistd.h>

class WiFiBinding : public CommonWiFiBinding {
  protected:
    static const char place_new_char = 0x02;
    static const uint8_t max_ssid_size = 24;
    static const uint8_t displayed_ssid_max_size = 8;
    char ssid[max_ssid_size + 1];
    uint8_t ssid_size;

    const AllowedIO GetAllowedInputs() override;
    void SelectPriorSymbol(char *symbol);
    void SelectNextSymbol(char *symbol);

    bool RenderSsidWithElipsis(uint8_t *fb, uint8_t x, uint8_t y, int leverage);
    bool RenderEditedSsid(uint8_t *fb, uint8_t x, uint8_t y);
    bool IsLastSsidChar();
    bool ChangeSsid();

  public:
    typedef enum { //
        wbepi_None = CommonWiFiBinding::EditingPropertyId::cwbepi_None,
        wbepi_ConfigureIOAdr = CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr,
        wbepi_Ssid_First_Char,
        wbepi_Ssid_Last_Char = wbepi_Ssid_First_Char + max_ssid_size - 1
    } EditingPropertyId;

    explicit WiFiBinding();
    explicit WiFiBinding(const MapIO io_adr, const char *ssid);
    virtual ~WiFiBinding();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;
    TvElementType GetElementType() override;

    static WiFiBinding *TryToCast(LogicElement *logic_element);

    void EndEditing() override;
    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;

    const char *GetSsid();
    void SetSsid(const char *ssid);
};

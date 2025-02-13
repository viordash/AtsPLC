#pragma once

#include "LogicProgram/Bindings/CommonWiFiBinding.h"
#include <stdint.h>
#include <unistd.h>

class WiFiApBinding : public CommonWiFiBinding {
  protected:
    static const char place_new_char = 'X';
    static const uint8_t client_mac_size = 17;
    char client_mac[client_mac_size + 1];

    const AllowedIO GetAllowedInputs() override final;
    void SelectPriorSymbol(char *symbol);
    void SelectNextSymbol(char *symbol);

    bool RenderEditedClientMac(uint8_t *fb, uint8_t x, uint8_t y);

  public:
    typedef enum { //
        wbepi_None = CommonWiFiBinding::EditingPropertyId::cwbepi_None,
        wbepi_ConfigureIOAdr = CommonWiFiBinding::EditingPropertyId::cwbepi_ConfigureIOAdr,
        wbepi_ClientMac_First_Char,
        wbepi_ClientMac_Last_Char = wbepi_ClientMac_First_Char + client_mac_size
    } EditingPropertyId;

    explicit WiFiApBinding();
    explicit WiFiApBinding(const MapIO io_adr, const char *client_mac);
    virtual ~WiFiApBinding();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static WiFiApBinding *TryToCast(LogicElement *logic_element);

    void EndEditing() override;
    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;

    const char *GetClientMac();
    void SetClientMac(const char *client_mac);
};

#pragma once

#include "LogicProgram/Bindings/WiFiBinding.h"
#include <stdint.h>
#include <unistd.h>

class WiFiApBinding : public WiFiBinding {
  protected:
    const AllowedIO GetAllowedInputs() override final;

  public:
    typedef enum { //
        wbepi_None = WiFiBinding::EditingPropertyId::wbepi_None,
        wbepi_ConfigureIOAdr = WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr,
        wbepi_Ssid_First_Char,
        wbepi_Ssid_Last_Char = wbepi_Ssid_First_Char + max_ssid_size
    } EditingPropertyId;

    explicit WiFiApBinding();
    explicit WiFiApBinding(const MapIO io_adr, const char *ssid);
    virtual ~WiFiApBinding();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;
    TvElementType GetElementType() override;

    static WiFiApBinding *TryToCast(LogicElement *logic_element);

    void EndEditing() override;
    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};

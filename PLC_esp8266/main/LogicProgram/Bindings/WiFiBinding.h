#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class WiFiBinding : public LogicElement, public InputElement, public LabeledLogicItem {
  public:
    typedef enum { //
        wbepi_None = EditableElement::EditingPropertyId::cepi_None,
        wbepi_ConfigureIOAdr,
        wbepi_Ssid_0,
        wbepi_Ssid_1
    } EditingPropertyId;

  private:
    const static Bitmap bitmap;

  protected:
    static const uint8_t max_ssid_size = 16;
    char ssid[max_ssid_size + 1];
    const AllowedIO GetAllowedInputs();
    void SelectPriorSymbol(char *symbol, char extra);
    void SelectNextSymbol(char *symbol, char extra);

    bool RenderSsid(uint8_t *fb, uint8_t x, uint8_t y);

  public:
    static const uint8_t LeftPadding = 12;
    static const uint8_t RightPadding = 12;
    static const uint8_t Width =
        OUTCOME_RAIL_RIGHT - INCOME_RAIL_WIDTH - LeftPadding - RightPadding;
    static const uint8_t Top = -13;
    static const uint8_t Height = 22;

    explicit WiFiBinding();
    explicit WiFiBinding(const MapIO io_adr, const char *ssid);
    virtual ~WiFiBinding();

    void SetIoAdr(const MapIO io_adr) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static WiFiBinding *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;

    const char *GetSsid();
    void SetSsid(const char *ssid);
};
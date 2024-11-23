#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class Indicator : public LogicElement, public InputElement, public LabeledLogicItem {
  public:
    typedef enum { //
        ciepi_None = EditableElement::EditingPropertyId::cepi_None,
        ciepi_ConfigureIOAdr,
        ciepi_ConfigureMultiplier_0,
        ciepi_ConfigureMultiplier_1,
        ciepi_ConfigureMultiplier_2,
        ciepi_ConfigureMultiplier_3,
        ciepi_ConfigureMultiplier_4,
        ciepi_ConfigureMultiplier_5,
        ciepi_ConfigureMultiplier_6,
        ciepi_ConfigureMultiplier_7
    } EditingPropertyId;

  protected:
    static const uint8_t max_symbols_count = 8;
    float low_scale;
    float high_scale;
    uint8_t decimal_point;
    char str_value[max_symbols_count + 1];
    char str_format[max_symbols_count];
    const AllowedIO GetAllowedInputs();
    void UpdateScale();
    void PrintOutValue(uint8_t eng_value);

  public:
    const uint8_t LeftPadding = 12;
    const uint8_t RightPadding = 12;
    const uint8_t Width = OUTCOME_RAIL_RIGHT - INCOME_RAIL_WIDTH - LeftPadding - RightPadding;
    const uint8_t Top = -13;
    const uint8_t Height = 22;
    explicit Indicator();
    explicit Indicator(const MapIO io_adr);
    virtual ~Indicator();

    void SetIoAdr(const MapIO io_adr) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static Indicator *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
};

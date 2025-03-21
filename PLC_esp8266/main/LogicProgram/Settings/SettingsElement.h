#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class SettingsElement : public LogicElement {
  protected:
  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_SelectParameter
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

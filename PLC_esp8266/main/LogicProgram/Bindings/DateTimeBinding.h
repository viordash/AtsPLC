#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class DateTimeBinding : public LogicElement, public InputElement, public LabeledLogicItem {
  public:
    typedef enum { //
        t_second = 0,
        t_minute,
        t_hour,
        t_day,
        t_weekday,
        t_month,
        t_year
    } DatetimePart;

  protected:
    const static Bitmap bitmap;
    const AllowedIO GetAllowedInputs();

  public:
    typedef enum { //
        cwbepi_None = EditableElement::EditingPropertyId::cepi_None,
        cwbepi_ConfigureIOAdr,
        cwbepi_SelectDatetimePart
    } EditingPropertyId;

    DatetimePart datetime_part;

    static const uint8_t LeftPadding = 12;
    static const uint8_t RightPadding = 12;
    static const uint8_t Width =
        OUTCOME_RAIL_RIGHT - INCOME_RAIL_WIDTH - LeftPadding - RightPadding;
    static const uint8_t Top = -13;
    static const uint8_t Height = 22;

    explicit DateTimeBinding();
    explicit DateTimeBinding(const MapIO io_adr);
    virtual ~DateTimeBinding();

    void SetIoAdr(const MapIO io_adr) override final;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override;
    bool ValidateDatetimePart(DatetimePart datetime_part);
    TvElementType GetElementType() override;

    static DateTimeBinding *TryToCast(LogicElement *logic_element);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
    void Option() override;
};

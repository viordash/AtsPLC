#pragma once

#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonTimer : public LogicElement {
  public:
    typedef enum { //
        ctepi_None = EditableElement::EditingPropertyId::cepi_None,
        ctepi_ConfigureDelayTime
    } EditingPropertyId;

  protected:
    uint64_t delay_time_us;
    char str_time[16];
    int str_size;

    virtual const Bitmap *GetCurrentBitmap(LogicItemState state) = 0;

    const uint8_t LeftPadding = 8;

  public:
    explicit CommonTimer();
    ~CommonTimer();

    ActionStatus DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    static CommonTimer *TryToCast(LogicElement *logic_element);
};

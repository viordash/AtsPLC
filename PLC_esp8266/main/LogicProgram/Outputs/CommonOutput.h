#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputOutputElement.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class CommonOutput : public LogicElement, public InputOutputElement, public LabeledLogicItem {
  protected:
    virtual const Bitmap *GetCurrentBitmap(LogicItemState state) = 0;

  public:
    CommonOutput();
    ~CommonOutput();
    void SetIoAdr(const MapIO io_adr) override final;

    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;

    static CommonOutput *TryToCast(LogicElement *logic_element);

    void SelectNext() override;
    void SelectPrior() override;
};

#pragma once

#include "Display/display.h"
#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class CommonComparator : public CommonInput {
  protected:
    int str_size;
    char str_reference[5];
    uint8_t ref_percent04;

    virtual bool CompareFunction() = 0;

  public:
    CommonComparator();
    CommonComparator(uint8_t ref_percent04, const MapIO io_adr);
    virtual ~CommonComparator();

    void SetReference(uint8_t ref_percent04);
    uint8_t GetReference();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;

    static CommonComparator *TryToCast(LogicElement *logic_element);
};

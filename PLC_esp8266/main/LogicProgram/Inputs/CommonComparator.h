#pragma once

#include "Display/display.h"
#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class CommonComparator : public CommonInput {
  private:
    char str_reference[5];
    int str_size;

  protected:
    uint8_t ref_percent04;

    virtual bool CompareFunction() = 0;

  public:
    CommonComparator();
    ~CommonComparator();

    void SetReference(uint8_t ref_percent04);

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
};

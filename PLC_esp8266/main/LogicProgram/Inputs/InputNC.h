#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class InputNC : public CommonInput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;

  public:
    explicit InputNC();
    explicit InputNC(const MapIO io_adr);
    ~InputNC();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static InputNC *TryToCast(CommonInput *common_input);

    void SelectNext() override;
    void SelectPrior() override;
    void Change() override;
    bool EditingCompleted() override;
};

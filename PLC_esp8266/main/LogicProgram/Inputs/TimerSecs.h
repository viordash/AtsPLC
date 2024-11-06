#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerSecs : public CommonTimer {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;
    const static uint64_t force_render_period_us = 1000000;

    static const uint32_t step_s = 1;
    static const uint32_t faststep_s = 10;
    static const uint32_t min_delay_time_s = 1;
    static const uint32_t max_delay_time_s = 99999;

    uint64_t force_render_time_us;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;

  protected:
  public:
    explicit TimerSecs();
    TimerSecs(uint32_t delay_time_s);
    ~TimerSecs();

    void SetTime(uint32_t delay_time_s);
    uint64_t GetTimeUs();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static TimerSecs *TryToCast(CommonTimer *common_timer);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
};

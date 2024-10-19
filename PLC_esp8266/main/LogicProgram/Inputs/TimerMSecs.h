#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerMSecs : public CommonTimer {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    static const uint32_t step_ms = 50;
    static const uint32_t faststep_ms = 250;
    static const uint32_t min_delay_time_ms = 1;
    static const uint32_t max_delay_time_ms = 99999;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;

  public:
    explicit TimerMSecs();
    TimerMSecs(uint32_t delay_time_ms);
    ~TimerMSecs();

    void SetTime(uint32_t delay_time_ms);
    uint64_t GetTimeUs();

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    static TimerMSecs *TryToCast(CommonTimer *common_timer);

    void SelectPrior() override;
    void SelectNext() override;
    void PageUp() override;
    void PageDown() override;
    void Change() override;
};

#pragma once

#include <atomic>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseInput {
  protected:
    std::atomic<bool> required_reading;
    std::atomic<uint8_t> value;

  public:
    ControllerBaseInput();
    ControllerBaseInput(const ControllerBaseInput &) = delete;
    ControllerBaseInput &operator=(const ControllerBaseInput &) = delete;
    virtual ~ControllerBaseInput();

    virtual void Init();
    virtual void FetchValue() = 0;
    uint8_t ReadValue();
    uint8_t PeekValue();
    void UpdateValue(uint8_t new_value);
    virtual void CancelReadingProcess();
};

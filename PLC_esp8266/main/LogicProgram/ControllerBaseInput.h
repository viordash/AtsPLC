#pragma once

#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseInput {
  protected:
    bool required_reading;
    uint8_t value;
    std::mutex lock_value;

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
    virtual void CancelReadingProcess() {};
};

#pragma once

#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseInput {
  private:
    uint8_t value;
    std::mutex lock_value;

  protected:
    bool required;

  public:
    virtual ~ControllerBaseInput();

    void Init();
    virtual bool SampleValue() = 0;
    uint8_t ReadValue();
    uint8_t PeekValue();
    bool UpdateValue(uint8_t new_value);
    virtual void CancelReadingValue(){};
};

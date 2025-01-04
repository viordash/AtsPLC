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
    virtual ~ControllerBaseInput();

    virtual void Init();
    virtual bool FetchValue() = 0;
    uint8_t ReadValue();
    uint8_t PeekValue();
    bool UpdateValue(uint8_t new_value);
    virtual void CancelReadingProcess(){};
};

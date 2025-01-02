#pragma once

#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseInput {
  private:
    uint8_t in_value;
    std::mutex lock_value;

  protected:
    bool required_reading;

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

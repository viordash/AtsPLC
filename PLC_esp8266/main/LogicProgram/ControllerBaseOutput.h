#pragma once

#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseOutput {
  protected:

  public:
    virtual ~ControllerBaseOutput();
    virtual void SetValue(uint8_t new_value) = 0;
};

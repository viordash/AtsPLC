#pragma once

#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseOutput {
  protected:

  public:
    virtual ~ControllerBaseOutput();
    virtual void WriteValue(uint8_t new_value) = 0;
};

#pragma once

#include <mutex>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseOutput {
  protected:
    uint8_t out_value;
    bool required_writing;

  public:
    ControllerBaseOutput();
    virtual ~ControllerBaseOutput();

    virtual void Init();
    virtual void CommitChanges() = 0;
    void WriteValue(uint8_t new_value);
};

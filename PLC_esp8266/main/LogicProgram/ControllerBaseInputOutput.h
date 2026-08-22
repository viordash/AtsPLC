#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include <atomic>
#include <stdint.h>
#include <unistd.h>

class ControllerBaseInputOutput : public ControllerBaseInput {
  protected:
    uint8_t out_value;
    std::atomic<bool> required_writing;

  public:
    ControllerBaseInputOutput();
    virtual ~ControllerBaseInputOutput();

    virtual void Init() override;
    virtual void CommitChanges() = 0;
    void WriteValue(uint8_t new_value);
};

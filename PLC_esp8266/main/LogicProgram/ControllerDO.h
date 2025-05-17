#pragma once

#include "LogicProgram/ControllerBaseInputOutput.h"
#include "sys_gpio.h"
#include <stdint.h>
#include <unistd.h>

class ControllerDO : public ControllerBaseInputOutput {
  protected:
    gpio_output gpio;

  public:
    explicit ControllerDO(gpio_output gpio);

    void Init() override;
    void FetchValue() override;
    void CommitChanges() override;
};

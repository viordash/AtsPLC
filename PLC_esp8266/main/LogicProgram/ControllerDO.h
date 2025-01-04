#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "sys_gpio.h"
#ifdef __cplusplus
}
#endif

#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/ControllerBaseOutput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerDO : public ControllerBaseInput, public ControllerBaseOutput {
  protected:
    bool value_changed;
    gpio_output gpio;

  public:
    explicit ControllerDO(gpio_output gpio);

    void Init() override;
    bool FetchValue() override;
    void CommitChanges() override;
};

#pragma once
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Controller {
  private:
    /* data */
  public:
    Controller(/* args */);
    ~Controller();

    uint8_t GetAIRelativeValue();
    uint8_t GetDIRelativeValue();
    uint8_t GetO1RelativeValue();
    uint8_t GetO2RelativeValue();
    uint8_t GetV1RelativeValue();
    uint8_t GetV2RelativeValue();
    uint8_t GetV3RelativeValue();
    uint8_t GetV4RelativeValue();
};

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

    static uint8_t GetAIRelativeValue();
    static uint8_t GetDIRelativeValue();
    static uint8_t GetO1RelativeValue();
    static uint8_t GetO2RelativeValue();
    static uint8_t GetV1RelativeValue();
    static uint8_t GetV2RelativeValue();
    static uint8_t GetV3RelativeValue();
    static uint8_t GetV4RelativeValue();
};

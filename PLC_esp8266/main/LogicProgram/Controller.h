#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Controller {
  private:
    bool runned;
    EventGroupHandle_t gpio_events;

  public:
    explicit Controller(EventGroupHandle_t gpio_events);
    ~Controller();

    void Start();
    void Stop();

    static void ProcessTask(void *parm);

    static uint8_t GetAIRelativeValue();
    static uint8_t GetDIRelativeValue();
    static uint8_t GetO1RelativeValue();
    static uint8_t GetO2RelativeValue();
    static uint8_t GetV1RelativeValue();
    static uint8_t GetV2RelativeValue();
    static uint8_t GetV3RelativeValue();
    static uint8_t GetV4RelativeValue();

    static void SetO1RelativeValue(uint8_t value);
    static void SetO2RelativeValue(uint8_t value);
    static void SetV1RelativeValue(uint8_t value);
    static void SetV2RelativeValue(uint8_t value);
    static void SetV3RelativeValue(uint8_t value);
    static void SetV4RelativeValue(uint8_t value);
};

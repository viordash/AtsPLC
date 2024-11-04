#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <forward_list>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class ProcessTicksService {
  protected:
    static const uint32_t default_delay = 100 / portTICK_PERIOD_MS;
    std::forward_list<TickType_t> ticks;
    std::mutex lock_mutex;

    TickType_t last_tick;

  public:
    ProcessTicksService(/* args */);
    ~ProcessTicksService();

    void Request(uint32_t delay_ms);
    TickType_t Get();
};

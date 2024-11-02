#pragma once

#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class ProcessTicksService {
  protected:
    static const uint32_t default_delay_ms = 100;
    static const int min_time_step_ms = 10;
    std::list<uint32_t> delays;

  public:
    ProcessTicksService(/* args */);
    ~ProcessTicksService();

    void Request(uint32_t delay_ms);
    uint32_t PopTicksToWait();
};

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <set>

class ProcessTicksService {
  protected:
    static const uint8_t default_delay_ms = 100;
    std::set<uint32_t> delays;

  public:
    ProcessTicksService(/* args */);
    ~ProcessTicksService();

    void Request(uint32_t delay_ms);
    uint32_t PopTicksToWait();
};

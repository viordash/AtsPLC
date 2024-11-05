#pragma once

#include <forward_list>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class ProcessTicksService {
  protected:
    static const uint32_t default_delay = -1;
    std::forward_list<uint32_t> ticks;
    std::mutex lock_mutex;

    int32_t GetTimespan(uint32_t from, uint32_t to);

  public:
    void Request(uint32_t delay_ms);
    uint32_t Get();
};

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
    std::forward_list<uint64_t> moments;

    int64_t GetTimespan(uint64_t from, uint64_t to);
    int64_t ConvertToSysTickCount(int64_t timespan);

  public:
    void Request(uint32_t delay_ms);
    uint32_t Get();
};

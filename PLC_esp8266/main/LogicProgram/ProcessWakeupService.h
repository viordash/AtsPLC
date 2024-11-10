#pragma once

#include <forward_list>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_set>

struct ProcessWakeupRequestData {
    void *id;
    uint32_t next_tick;
};

class ProcessWakeupService {
  protected:
    static const uint32_t default_delay = -1;
    std::forward_list<ProcessWakeupRequestData> requests;
    std::unordered_set<void *> ids;

    int32_t GetTimespan(uint32_t from, uint32_t to);

  public:
    void Request(void *id, uint32_t delay_ms);
    void RemoveRequest(void *id);
    uint32_t Get();
    int RemoveExpired();
};

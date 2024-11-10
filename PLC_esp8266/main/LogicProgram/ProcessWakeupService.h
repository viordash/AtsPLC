#pragma once

#include <mutex>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_set>

struct ProcessWakeupRequestData {
    void *id;
    uint32_t next_tick;
};

struct ProcessWakeupRequestDataCmp {
    static int32_t GetTimespan(uint32_t from, uint32_t to) {
        uint32_t timespan = to - from;
        return (int32_t)timespan;
    }

    bool operator()(const ProcessWakeupRequestData &fk1,
                    const ProcessWakeupRequestData &fk2) const {

        int timespan = GetTimespan(fk1.next_tick, fk2.next_tick);
        bool further_large_values = timespan > 0;
        if (further_large_values) {
            return true;
        }
        if (fk1.next_tick > fk2.next_tick) {
            return false;
        }
        return fk1.id < fk2.id;
    }
};

class ProcessWakeupService {
  protected:
    static const uint32_t default_delay = -1;
    std::set<ProcessWakeupRequestData, ProcessWakeupRequestDataCmp> requests;
    std::unordered_set<void *> ids;

  public:
    void Request(void *id, uint32_t delay_ms);
    void RemoveRequest(void *id);
    uint32_t Get();
    int RemoveExpired();
};

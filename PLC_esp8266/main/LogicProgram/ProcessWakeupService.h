#pragma once

#include <mutex>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_set>

enum ProcessWakeupRequestPriority {
    pwrp_Idle,
    pwrp_Critical,
};

struct ProcessWakeupRequestData {
    void *id;
    uint64_t next_time;
    ProcessWakeupRequestPriority priority;
};

struct ProcessWakeupRequestDataCmp {
    static int64_t GetTimespan(uint64_t from, uint64_t to) {
        uint64_t timespan = to - from;
        return (int64_t)timespan;
    }

    bool operator()(const ProcessWakeupRequestData &fk1,
                    const ProcessWakeupRequestData &fk2) const {

        int64_t timespan = GetTimespan(fk1.next_time, fk2.next_time);
        bool further_large_values = timespan > 0;
        if (further_large_values) {
            return true;
        }
        if (fk1.next_time > fk2.next_time) {
            return false;
        }
        return fk1.id < fk2.id;
    }
};

class ProcessWakeupService {
  protected:
    static const uint64_t idle_dead_band_us = 100000;
    static const uint32_t default_delay = -1;
    std::set<ProcessWakeupRequestData, ProcessWakeupRequestDataCmp> requests;
    std::unordered_set<void *> ids;

  public:
    bool Request(void *id, uint32_t delay_ms, ProcessWakeupRequestPriority priority);
    void RemoveRequest(void *id);
    uint32_t Get();
    int RemoveExpired();
};

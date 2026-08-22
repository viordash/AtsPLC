#pragma once

#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

enum ProcessWakeupRequestPriority {
    pwrp_Idle,
    pwrp_Critical,
};

struct ProcessWakeupRequestData {
    const void *id;
    uint64_t next_time;
    ProcessWakeupRequestPriority priority;

    ProcessWakeupRequestData(const void *id,
                             uint64_t next_time,
                             ProcessWakeupRequestPriority priority)
        : id(id), next_time(next_time), priority(priority) {
    }

    ProcessWakeupRequestData(ProcessWakeupRequestData &&other) noexcept
        : ProcessWakeupRequestData(other.id, other.next_time, other.priority) {
    }

    ProcessWakeupRequestData &operator=(ProcessWakeupRequestData &&other) noexcept {
        if (this != &other) {
            id = other.id;
            next_time = other.next_time;
            priority = other.priority;
        }
        return *this;
    }

    ProcessWakeupRequestData(ProcessWakeupRequestData &) = delete;
    ProcessWakeupRequestData &operator=(ProcessWakeupRequestData &) = delete;
};

struct ProcessWakeupRequestDataCmp {
    static int64_t GetTimespan(uint64_t from, uint64_t to) {
        uint64_t timespan = to - from;
        return (int64_t)timespan;
    }

    bool operator()(const ProcessWakeupRequestData &a, const ProcessWakeupRequestData &b) const {
        int64_t timespan = GetTimespan(a.next_time, b.next_time);
        bool a_earlier_than_b = timespan > 0;

        if (a_earlier_than_b) {
            return true;
        }
        if (a.next_time != b.next_time) {
            return false;
        }
        return a.id < b.id;
    }
};

class ProcessWakeupService {
  protected:
    static const uint64_t idle_dead_band_us = 100000;
    static const uint32_t default_delay = -1;
    static const size_t reserved_requests_count = 16;
    std::vector<ProcessWakeupRequestData> requests;
    std::mutex lock_mutex;

    std::vector<ProcessWakeupRequestData>::iterator Find(const void *id);
    std::vector<ProcessWakeupRequestData>::iterator
    UpperBound(const ProcessWakeupRequestData &request);

  public:
    ProcessWakeupService();

    bool Request(const void *id, uint32_t delay_ms, ProcessWakeupRequestPriority priority);
    void RemoveRequest(const void *id);
    uint32_t Get();
    void RemoveExpired();
};

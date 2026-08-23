#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ProcessWakeupService = "ProcessWakeupService";

ProcessWakeupService::ProcessWakeupService() {
    requests.reserve(reserved_requests_count);
}

std::vector<ProcessWakeupRequestData>::iterator ProcessWakeupService::Find(const void *id) {
    for (auto it = requests.begin(); it != requests.end(); it++) {
        if (it->id == id) {
            return it;
        }
    }
    return requests.end();
}

std::vector<ProcessWakeupRequestData>::iterator
ProcessWakeupService::UpperBound(const ProcessWakeupRequestData &request) {
    return std::upper_bound(requests.begin(),
                            requests.end(),
                            request,
                            ProcessWakeupRequestDataCmp());
}

bool ProcessWakeupService::Contains(const void *id) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    bool request_already_in = Find(id) != requests.end();
    return request_already_in;
}

bool ProcessWakeupService::Request(const void *id,
                                   uint32_t delay_ms,
                                   ProcessWakeupRequestPriority priority) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    bool request_already_in = Find(id) != requests.end();
    if (request_already_in) {
        ESP_LOGD(TAG_ProcessWakeupService,
                 "Request already in:%u, %p, size:%u",
                 (unsigned int)delay_ms,
                 id,
                 (unsigned int)requests.size());
        return false;
    }

    auto current_time = (uint64_t)esp_timer_get_time();
    auto next_time = current_time + ((uint64_t)delay_ms * 1000);

    ProcessWakeupRequestData request = { id, next_time, priority };
    auto upper = UpperBound(request);
    bool request_joined = false;
    if (upper != requests.end()) {
        auto &upper_req = *upper;
        bool request_can_be_joined = request.priority == ProcessWakeupRequestPriority::pwrp_Idle
                                  && (upper_req.next_time - next_time) < idle_dead_band_us;
        if (request_can_be_joined) {
            ESP_LOGD(TAG_ProcessWakeupService,
                     "Request is joined in:%u, %p, diff:%d, next:%u",
                     (unsigned int)delay_ms,
                     id,
                     (int)(upper_req.next_time - next_time),
                     (unsigned int)upper_req.next_time);
            request.next_time = upper_req.next_time;
            request_joined = true;
        }
    }

    if (request_joined) {
        upper = UpperBound(request);
    }
    requests.insert(upper, std::move(request));

    ESP_LOGD(TAG_ProcessWakeupService,
             "Request:%u, %p, size:%u, time:%u",
             (unsigned int)delay_ms,
             id,
             (unsigned int)requests.size(),
             (unsigned int)(current_time / 1000));
    return true;
}

void ProcessWakeupService::RemoveRequest(const void *id) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto it = Find(id);
    if (it == requests.end()) {
        return;
    }
    requests.erase(it);

    ESP_LOGD(TAG_ProcessWakeupService,
             "RemoveRequest: %p, size:%u, systick:%u",
             id,
             (unsigned int)requests.size(),
             (unsigned int)esp_timer_get_time());
}

uint32_t ProcessWakeupService::Get() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    if (requests.empty()) {
        ESP_LOGD(TAG_ProcessWakeupService, "Get def:%u", (unsigned int)default_delay);
        return default_delay;
    }

    auto current_time = (uint64_t)esp_timer_get_time();
    auto &req = requests.front();
    int64_t timespan = req.next_time - current_time;

    uint32_t wait_ticks =
        ((timespan / 1000) + (portTICK_PERIOD_MS - portTICK_PERIOD_MS / 2)) / portTICK_PERIOD_MS;

    ESP_LOGD(TAG_ProcessWakeupService,
             "Get:%u, %p, size:%u, time:%u",
             (unsigned int)wait_ticks,
             req.id,
             (unsigned int)requests.size(),
             (unsigned int)(current_time / 1000));
    if (timespan < 0) {
        return 0;
    }
    return wait_ticks;
}

void ProcessWakeupService::RemoveExpired() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto current_time = (uint64_t)esp_timer_get_time();
    auto expired_end = requests.begin();
    while (expired_end != requests.end()) {
        int64_t timespan = expired_end->next_time - current_time;
        bool expired = timespan <= (portTICK_PERIOD_MS / 2) * 1000;
        if (!expired) {
            break;
        }
        ESP_LOGD(TAG_ProcessWakeupService,
                 "RemoveExpired: %p, systick:%u",
                 expired_end->id,
                 (unsigned int)(current_time / 1000));
        expired_end++;
    }
    requests.erase(requests.begin(), expired_end);
}
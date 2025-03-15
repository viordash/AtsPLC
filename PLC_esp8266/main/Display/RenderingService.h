#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#ifdef __cplusplus
}
#endif

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class Ladder;
class RenderingService {
  public:
    struct TaskArg {
        RenderingService *service;
        Ladder *ladder;
    };

  protected:
    const uint32_t min_period_ms = 150;
    uint64_t loop_time_us;

    TaskArg task_arg;

    TaskHandle_t task_handle;
    static void Task(void *param);
    void UpdateLoopTime();
    bool Skip(uint32_t next_awake_time_interval);

  public:
    RenderingService();
    ~RenderingService();
    void Start(Ladder *ladder);
    void Stop();
    void Do(uint32_t next_awake_time_interval);
};

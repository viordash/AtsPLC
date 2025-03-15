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
#include <atomic>
#include <stdint.h>
#include <unistd.h>

class Ladder;
class RenderingService {
  public:
    struct TaskArg {
        RenderingService *service;
        Ladder *ladder;
    };
    static const int STOP_RENDER_TASK = BIT0;
    static const int DO_RENDERING = BIT1;

  protected:
    std::atomic<bool> on_rendering;
    TaskArg task_arg;

    TaskHandle_t task_handle;
    static void Task(void *param);

  public:
    RenderingService();
    ~RenderingService();
    void Start(Ladder *ladder);
    void Stop();
    void Do();
};

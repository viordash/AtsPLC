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
#include "Display/display.h"
#include <mutex>
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

    struct CachedBitmap {
        uint8_t *bitmap;
        uint32_t last_change_time_ms;
        int32_t view_offset;
        uint32_t view_count;
    };

  protected:
    TaskArg task_arg;

    TaskHandle_t task_handle;
    std::mutex render_mutex;
    uint32_t last_change_time_ms;
    static void Task(void *param);

  public:
    RenderingService();
    RenderingService(RenderingService &) = delete;
    ~RenderingService();
    void Start(Ladder *ladder);
    void Stop();
    void Do();

    CachedBitmap BeginRenderOnExternal();
    void EndRenderOnExternal();
};

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
  protected:
    TaskHandle_t task_handle;
    static void Task(void *param);

  public:
    RenderingService();
    ~RenderingService();
    void Start(Ladder *ladder);
    void Stop();
    void Do();
};

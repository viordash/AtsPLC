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

class ServiceModeHandler {
  public:
    enum Mode { sm_SmartConfig, sm_BackupLogic, sm_RestoreLogic, sm_ResetToDefault };

  protected:
    static void RenderMainMenu(Mode mode);
    static Mode ChangeModeToPrev(Mode mode);
    static Mode ChangeModeToNext(Mode mode);
    static void Execute(Mode mode);

  public:
    static void Start(EventGroupHandle_t gpio_events);
};

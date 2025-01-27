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
  protected:
    enum ServiceMode { sm_SmartConfig, sm_BackupLogic, sm_RestoreLogic, sm_ResetToDefault };

    static void RenderMainMenu(ServiceMode mode);
    static ServiceMode ChangeModeToPrev(ServiceMode mode);
    static ServiceMode ChangeModeToNext(ServiceMode mode);
    static void Execute(ServiceMode mode);

  public:
    static void Start(EventGroupHandle_t gpio_events);
};

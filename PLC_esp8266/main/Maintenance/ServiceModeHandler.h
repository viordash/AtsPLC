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

#define EXPECTED_BUTTONS                                                                           \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN)

class ServiceModeHandler {
  public:
    enum Mode { sm_SmartConfig, sm_BackupLogic, sm_RestoreLogic, sm_ResetToDefault };

  protected:
    static void RenderMainMenu(Mode mode);
    static Mode ChangeModeToPrev(Mode mode);
    static Mode ChangeModeToNext(Mode mode);
    static void Execute(EventGroupHandle_t gpio_events, Mode mode);
    static void SmartConfig(EventGroupHandle_t gpio_events);

  public:
    static void Start(EventGroupHandle_t gpio_events);
};

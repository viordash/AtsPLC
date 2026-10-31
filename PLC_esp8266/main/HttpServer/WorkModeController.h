#pragma once

#include "BaseController.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/WorkMode.h"

class WorkModeController : public BaseController {
  protected:
    static EffectiveWorkMode GetCurrentMode();
    static EventBits_t Convert2Events(EffectiveWorkMode mode);

  public:
    WorkModeController();
    ~WorkModeController();

    size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) override;

    static esp_err_t GetWorkMode(httpd_req_t *req);
    static esp_err_t SetWorkMode(httpd_req_t *req);
};

#pragma once

#include "BaseController.h"

class InputController : public BaseController {
  public:
    InputController();
    ~InputController();

    size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) override;

    static esp_err_t KeyPress(httpd_req_t *req);
};

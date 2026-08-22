#pragma once

#include "BaseController.h"

class MainController : public BaseController {
  public:
    MainController();
    ~MainController();

    size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) override;

    static esp_err_t DefaultHandlerGet(httpd_req_t *req);
};
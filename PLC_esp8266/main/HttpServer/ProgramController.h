#pragma once

#include "BaseController.h"

class ProgramController : public BaseController {
  public:
    ProgramController();
    ~ProgramController();

    size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) override;

    static esp_err_t Upload(httpd_req_t *req);
    static esp_err_t Download(httpd_req_t *req);
};

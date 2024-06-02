#pragma once

#include "BaseController.h"

class MainController : public BaseController {
  public:
    MainController();
    ~MainController();

    std::vector<httpd_uri_t *> GetUriHandlers() override;

    static esp_err_t DefaultHandlerGet(httpd_req_t *req);

  private:
    httpd_uri_t uriDefaultGet;
};
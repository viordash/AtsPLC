#pragma once

#include "BaseController.h"

class UpdateController : public BaseController {
  public:
    UpdateController();
    ~UpdateController();

    std::vector<httpd_uri_t *> GetUriHandlers() override;

    static esp_err_t Handler(httpd_req_t *req);

  private:
    httpd_uri_t uriHandler;

    bool ReceiveFile(httpd_req_t *req, char *buffer);
};
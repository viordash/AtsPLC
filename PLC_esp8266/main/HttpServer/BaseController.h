#pragma once

#include "lassert.h"
#include <esp_http_server.h>

#define HTTPD_RESP_USE_STRLEN -1
class BaseController {
  public:
    BaseController();
    virtual ~BaseController();

    virtual size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) = 0;

  protected:
    static esp_err_t SendError(httpd_req_t *req, const char *status, const char *message);

    esp_err_t SendHttpError_400(httpd_req_t *req);
    esp_err_t SendHttpError_408(httpd_req_t *req);
    esp_err_t SendHttpError_500(httpd_req_t *req);
};
#pragma once

#include "lassert.h"
#include <esp_http_server.h>

#define HTTPD_RESP_USE_STRLEN -1
class BaseController {
  public:
    static constexpr size_t max_uri_handlers = 8;

    BaseController();
    virtual ~BaseController();

    virtual size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) = 0;

  protected:
    esp_err_t
    GetUrlQueryParamValue(httpd_req_t *req, const char *key, char *value, size_t valueSize);

    esp_err_t SendHttpError_400(httpd_req_t *req);
    esp_err_t SendHttpError_408(httpd_req_t *req);
    esp_err_t SendHttpError_500(httpd_req_t *req);

  private:
    esp_err_t SendHttpError(httpd_req_t *req, const char *status, const char *msg);
};
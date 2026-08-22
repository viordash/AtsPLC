#pragma once

#include "BaseController.h"
#include "Display/RenderingService.h"

class DisplayController : public BaseController {
  public:
    explicit DisplayController(RenderingService &rendering_service);
    ~DisplayController();

    size_t GetUriHandlers(httpd_uri_t *handlers, size_t capacity) override;

    static esp_err_t GetDeviceConfig(httpd_req_t *req);
    static esp_err_t GetBitmap(httpd_req_t *req);
    esp_err_t CreateBitmapDataResponse(httpd_req_t *req,
                                       uint8_t *bitmap,
                                       const char *etag,
                                       int32_t view_offset,
                                       uint32_t view_count);
    esp_err_t CreateNotModifiedResponse(httpd_req_t *req, const char *etag);
    esp_err_t SetForceRefreshHeader(httpd_req_t *req, char *dst, size_t size);

  private:
    RenderingService &rendering_service;
};

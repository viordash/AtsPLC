#pragma once

#include "BaseController.h"
#include "esp_ota_ops.h"

class UpdateController : public BaseController {
  public:
    UpdateController();
    ~UpdateController();

    std::vector<httpd_uri_t *> GetUriHandlers() override;

    static esp_err_t Handler(httpd_req_t *req);

  private:
    httpd_uri_t uriHandler;

    bool ReceiveFile(httpd_req_t *req, char *buffer);
    bool BeginOta(const esp_partition_t **update_partition,
                  esp_ota_handle_t *update_handle,
                  size_t image_size);
    bool FinishOta(const esp_partition_t *update_partition, esp_ota_handle_t *update_handle);
};
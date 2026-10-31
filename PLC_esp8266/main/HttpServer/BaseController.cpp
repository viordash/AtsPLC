#include "BaseController.h"
#include "esp_log.h"

static const char *TAG_BaseController = "base_controller";

BaseController::BaseController() {
}

BaseController::~BaseController() {
}

esp_err_t BaseController::SendHttpError_400(httpd_req_t *req) {
    return SendError(req,
                     "400 Bad Request",
                     "Server unable to understand request due to invalid syntax");
}

esp_err_t BaseController::SendHttpError_408(httpd_req_t *req) {
    return SendError(req, "408 Request Timeout", "Server closed this connection");
}

esp_err_t BaseController::SendHttpError_500(httpd_req_t *req) {
    return SendError(req, "500 Server Error", "Server has encountered an unexpected error");
}

esp_err_t BaseController::SendError(httpd_req_t *req, const char *status, const char *message) {
    char response[128];
    int size = snprintf(response, sizeof(response), "{\"error\":\"%s\"}", message);

    ESP_LOGE(TAG_BaseController, "SendError %s : %s", status, message);

    esp_err_t res = httpd_resp_set_status(req, status);
    if (res != ESP_OK) {
        return res;
    }
    res = httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    if (res != ESP_OK) {
        return res;
    }
    return httpd_resp_send(req, response, size);
}

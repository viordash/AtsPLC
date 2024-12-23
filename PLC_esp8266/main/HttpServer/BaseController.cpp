#include "BaseController.h"
#include "esp_log.h"

static const char *TAG_BaseController = "base_controller";

BaseController::BaseController() {
}

BaseController::~BaseController() {
}

esp_err_t BaseController::GetUrlQueryParamValue(httpd_req_t *req,
                                                const char *key,
                                                char *value,
                                                size_t valueSize) {
    char *buf;
    esp_err_t res;
    size_t buf_len;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len <= 1) {
        return ESP_FAIL;
    }
    buf = new char[buf_len];
    if ((res = httpd_req_get_url_query_str(req, buf, buf_len)) == ESP_OK) {
        if ((res = httpd_query_key_value(buf, key, value, valueSize)) == ESP_OK) {
            ESP_LOGD(TAG_BaseController, "Found URL query parameter => %s=%s", key, value);
        }
    }
    delete[] buf;

    return res;
}

esp_err_t BaseController::SendHttpError_400(httpd_req_t *req) {
    return SendHttpError(req,
                         "400 Bad Request",
                         "Server unable to understand request due to invalid syntax");
}

esp_err_t BaseController::SendHttpError_408(httpd_req_t *req) {
    return SendHttpError(req, "408 Request Timeout", "Server closed this connection");
}

esp_err_t BaseController::SendHttpError_500(httpd_req_t *req) {
    return SendHttpError(req, "500 Server Error", "Server has encountered an unexpected error");
}

esp_err_t BaseController::SendHttpError(httpd_req_t *req, const char *status, const char *msg) {
    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    return httpd_resp_send(req, msg, strlen(msg));
}
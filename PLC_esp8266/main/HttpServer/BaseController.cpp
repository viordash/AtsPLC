#include "BaseController.h"
#include "esp_log.h"

static const char *TAG = "base_controller";

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
            ESP_LOGD(TAG, "Found URL query parameter => %s=%s", key, value);
        }
    }
    delete[] buf;

    return res;
}

#include "UpdateController.h"
#include <esp_log.h>
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG = "update_controller";

#define SCRATCH_BUFSIZE 4096

UpdateController::UpdateController() : BaseController() {

    uriHandler = {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = UpdateController::Handler,
        .user_ctx = this //
    };
}

UpdateController::~UpdateController() {
}

std::vector<httpd_uri_t *> UpdateController::GetUriHandlers() {
    std::vector<httpd_uri_t *> handlers = { &uriHandler };
    return handlers;
}

esp_err_t UpdateController::Handler(httpd_req_t *req) {
    char resp[HTTPD_MAX_URI_LEN + 64];
    sprintf(resp, "%s%s", "Update POST Response", req->uri);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Handler resp: '%s'", resp);
    return ESP_OK;
}
#include "MainController.h"
#include <esp_log.h>
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG = "main_controller";

MainController::MainController() : BaseController() {
    uriDefaultGet = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = MainController::DefaultHandlerGet,
        .user_ctx = this //
    };
}

MainController::~MainController() {
}

std::vector<httpd_uri_t *> MainController::GetUriHandlers() {
    std::vector<httpd_uri_t *> handlers = { &uriDefaultGet };
    return handlers;
}

esp_err_t MainController::DefaultHandlerGet(httpd_req_t *req) {
    char resp[HTTPD_MAX_URI_LEN + 64];
    sprintf(resp, "%s%s", "URI GET Response", req->uri);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "DefaultHandlerGet resp: '%s'", resp);
    return ESP_OK;
}

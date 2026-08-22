#include "MainController.h"
#include <esp_log.h>
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG_MainController = "main_controller";

MainController::MainController() : BaseController() {
}

MainController::~MainController() {
}

size_t MainController::GetUriHandlers(httpd_uri_t *handlers, size_t capacity) {
    const size_t count = 1;
    ASSERT(capacity >= count);
    handlers[0] = { .uri = "/",
                    .method = HTTP_GET,
                    .handler = MainController::DefaultHandlerGet,
                    .user_ctx = this };
    return count;
}

esp_err_t MainController::DefaultHandlerGet(httpd_req_t *req) {
    char resp[HTTPD_MAX_URI_LEN + 64];
    sprintf(resp, "%s%s", "URI GET Response", req->uri);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG_MainController, "DefaultHandlerGet resp: '%s'", resp);
    return ESP_OK;
}

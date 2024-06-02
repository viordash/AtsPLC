#include "UpdateController.h"
#include <esp_log.h>
#include <sys/param.h>
#include <sys/stat.h>

static const char *TAG = "update_controller";

#define SCRATCH_BUFSIZE 1024

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
    char *buffer = new char[SCRATCH_BUFSIZE];

    auto controller = static_cast<UpdateController *>(req->user_ctx);

    auto res = controller->ReceiveFile(req, buffer);

    delete[] buffer;

    if (!res) {
        return ESP_FAIL;
    }

    char resp[HTTPD_MAX_URI_LEN + 64];
    sprintf(resp, "%s%s, res:%d", "Update POST Response", req->uri, res);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Handler resp: '%s'", resp);
    return ESP_OK;
}

bool UpdateController::ReceiveFile(httpd_req_t *req, char *buffer) {
    ESP_LOGI(TAG, "ReceiveFile, size: '%u'", req->content_len);

    int file_size = (int)req->content_len;
    while (file_size > 0) {
        int received = httpd_req_recv(req, buffer, MIN(SCRATCH_BUFSIZE, file_size));
        if (received < 0) {
            ESP_LOGE(TAG, "ReceiveFile, error: '%d'", received);
            switch (received) {
                case HTTPD_SOCK_ERR_TIMEOUT:
                    SendHttpError_408(req);
                    break;

                case HTTPD_SOCK_ERR_INVALID:
                    SendHttpError_400(req);
                    break;

                default:
                    SendHttpError_500(req);
                    break;
            }
            return false;
        }
        file_size -= received;
        ESP_LOGI(TAG, "ReceiveFile, received: %u, remained: %u", received, file_size);
    }
    return true;
}
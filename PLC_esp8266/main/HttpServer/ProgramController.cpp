#include "ProgramController.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Ladder.h"
#include "esp_log.h"
#include "lassert.h"
#include <sys/param.h>

static const char *TAG_ProgramController = "ProgramController";

ProgramController::ProgramController() : BaseController() {
}

ProgramController::~ProgramController() {
}

size_t ProgramController::GetUriHandlers(httpd_uri_t *handlers, size_t capacity) {
    const size_t count = 2;
    ASSERT(capacity >= count);
    handlers[0] = { .uri = "/program/upload",
                    .method = HTTP_POST,
                    .handler = ProgramController::Upload,
                    .user_ctx = this };
    handlers[1] = { .uri = "/program/download",
                    .method = HTTP_GET,
                    .handler = ProgramController::Download,
                    .user_ctx = this };
    return count;
}

esp_err_t ProgramController::Upload(httpd_req_t *req) {
    if (Controller::GetLadder().GetWorkMode() != WorkMode::Stop) {
        ESP_LOGE(TAG_ProgramController, "Upload is not allowed, device is running");
        SendError(req, "400 Bad Request", "device is running, stop it before uploading");
        return ESP_FAIL;
    }

    if (req->content_len > PROGRAM_MAXSIZE) {
        ESP_LOGE(TAG_ProgramController,
                 "Uploaded program size exceeds the limit, '%u' > '%u'",
                 (unsigned int)req->content_len,
                 (unsigned int)PROGRAM_MAXSIZE);
        SendError(req, "400 Bad Request", "uploaded program size exceeds the limit");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG_ProgramController, "Upload, content_len: %u", (unsigned int)req->content_len);

    char *buffer = new char[req->content_len];

    int received = httpd_req_recv(req, buffer, req->content_len);
    if (received != (int)req->content_len) {
        ESP_LOGE(TAG_ProgramController, "Upload, error: '%d'", received);
        switch (received) {
            case HTTPD_SOCK_ERR_TIMEOUT:
                SendError(req, "408 Request Timeout", "Server closed this connection");
                break;

            case HTTPD_SOCK_ERR_INVALID:
                SendError(req, "400 Bad Request", "invalid request");
                break;

            default:
                SendError(req, "500 Server Error", "receiving error");
                break;
        }
        delete[] buffer;
        return ESP_FAIL;
    }
    ESP_LOGI(TAG_ProgramController, "Upload completed, total: %d bytes", received);

    Controller::GetLadder().RemoveAll();
    if (Controller::GetLadder().Deserialize((uint8_t *)buffer, received) == 0) {
        ESP_LOGE(TAG_ProgramController, "Uploaded program has errors");
        SendError(req, "400 Bad Request", "uploaded program has errors");
        Controller::GetLadder().Load();
        delete[] buffer;
        return ESP_FAIL;
    }

    delete[] buffer;

    esp_err_t res = httpd_resp_send(req, NULL, 0);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_ProgramController, "Upload httpd_resp_send error");
        return res;
    }

    return ESP_OK;
}

esp_err_t ProgramController::Download(httpd_req_t *req) {
    ESP_LOGI(TAG_ProgramController, "Download");

    size_t program_size = Controller::GetLadder().Serialize(NULL, 0);
    if (program_size == 0) {
        ESP_LOGE(TAG_ProgramController, "Download, get ladder data error");
        SendError(req, "500 Server Error", "get ladder data error");
        return ESP_FAIL;
    }

    uint8_t *program_data = new uint8_t[program_size];

    if (Controller::GetLadder().Serialize(program_data, program_size) != program_size) {
        ESP_LOGE(TAG_ProgramController, "Download, serialize ladder error");
        SendError(req, "500 Server Error", "serialize ladder error");
        delete[] program_data;
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=\"program.dat\"");

    esp_err_t res = httpd_resp_send(req, (const char *)program_data, program_size);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_ProgramController, "Download httpd_resp_send error");
    }

    delete[] program_data;
    return res;
}

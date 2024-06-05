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
    char *buffer = new char[SCRATCH_BUFSIZE];

    auto controller = static_cast<UpdateController *>(req->user_ctx);

    auto res = controller->ReceiveFile(req, buffer);

    delete[] buffer;

    if (!res) {
        return ESP_FAIL;
    }

    httpd_resp_send(req, "Upload new firmware successfull", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

bool UpdateController::ReceiveFile(httpd_req_t *req, char *buffer) {
    ESP_LOGI(TAG, "ReceiveFile, size: '%u'", req->content_len);

    esp_err_t err;
    const esp_partition_t *update_partition;
    esp_ota_handle_t update_handle;
    if (!BeginOta(&update_partition, &update_handle, req->content_len)) {
        return false;
    }

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
        ESP_LOGD(TAG, "ReceiveFile, received: %u, remained: %u", received, file_size);

        err = esp_ota_write(update_handle, (const void *)buffer, received);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
            SendHttpError_500(req);
            return false;
        }
    }

    if (!FinishOta(update_partition, &update_handle)) {
        SendHttpError_500(req);
        return false;
    }

    return true;
}

bool UpdateController::BeginOta(const esp_partition_t **update_partition,
                                esp_ota_handle_t *update_handle,
                                size_t image_size) {
    esp_err_t err;
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG,
                 "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address,
                 running->address);
        ESP_LOGW(TAG,
                 "(This can happen if either the OTA boot data or preferred boot image become "
                 "corrupted somehow.)");
    }

    ESP_LOGI(TAG,
             "Running partition type %d subtype %d (offset 0x%08x)",
             running->type,
             running->subtype,
             running->address);

    *update_partition = esp_ota_get_next_update_partition(NULL);

    if (*update_partition == NULL) {
        ESP_LOGE(TAG, "update_partition == NULL");
        return false;
    }
    ESP_LOGI(TAG,
             "Writing to partition subtype %d at offset 0x%x",
             (*update_partition)->subtype,
             (*update_partition)->address);

    err = esp_ota_begin(*update_partition, image_size, update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
        return false;
    }

    ESP_LOGI(TAG, "BeginOta succeeded");

    return true;
}

bool UpdateController::FinishOta(const esp_partition_t *update_partition,
                                 esp_ota_handle_t *update_handle) {
    esp_err_t err;
    if (esp_ota_end(*update_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        return false;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
        return false;
    }
    ESP_LOGI(TAG, "FinishOta succeeded");
    return true;
}